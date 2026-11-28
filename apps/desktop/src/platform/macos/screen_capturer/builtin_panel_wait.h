#ifndef _BUILTIN_PANEL_WAIT_H_
#define _BUILTIN_PANEL_WAIT_H_

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace crossdesk {

// The built-in panel can briefly disappear from ScreenCaptureKit on wake while
// CoreGraphics still reports it online. A stream bound to that panel waits a
// short grace period for it to return, then falls back to a listed display so
// viewers are not left without a picture if the panel never comes back.
//
// The wait is checked only when the capturer enumerates displays, which a
// failed capture does on its retry backoff. The fallback therefore happens on
// the first retry after the grace ends, so it takes at least |kGrace|.
class BuiltinPanelWait {
 public:
  using Clock = std::chrono::steady_clock;
  static constexpr Clock::duration kGrace = std::chrono::seconds(10);

  // |missing| is true when the stream's built-in panel is online but absent
  // from ScreenCaptureKit's display list. Returns whether to keep waiting.
  bool ShouldWait(bool missing, Clock::time_point now) {
    if (!missing) {
      missing_since_.reset();
      return false;
    }
    if (!missing_since_) missing_since_ = now;
    return now - *missing_since_ < kGrace;
  }

  void Reset() { missing_since_.reset(); }

 private:
  std::optional<Clock::time_point> missing_since_;
};

// What CoreGraphics reports about the main display when the capturer opens
// its stream slots.
struct MainDisplayState {
  uint32_t id = 0;
  bool builtin = false;
  bool online = false;
  // True when it mirrors another display and so has no picture of its own.
  bool mirrors_another = false;
};

// Lists the displays the capturer opens stream slots for, in slot order; a
// viewer's first stream captures the first slot. |listed| is
// ScreenCaptureKit's display list and |online| the displays CoreGraphics
// reports online, without mirrors.
//
// An empty list means every display is asleep or the enumeration failed, so
// the online displays are used instead and none of them loses its slot. On
// wake, ScreenCaptureKit may list a secondary display before the built-in
// panel, so an online built-in main panel missing from the list gets the
// first slot. That stream then waits for the panel instead of opening on the
// secondary display. A panel that mirrors another display gets no slot.
inline std::vector<uint32_t> PlanCaptureDisplays(
    std::vector<uint32_t> listed, std::vector<uint32_t> online,
    const MainDisplayState& main_display) {
  if (listed.empty()) return online;
  if (main_display.builtin && main_display.online &&
      !main_display.mirrors_another &&
      std::find(listed.begin(), listed.end(), main_display.id) ==
          listed.end()) {
    listed.insert(listed.begin(), main_display.id);
  }
  return listed;
}

struct CaptureDisplayChoice {
  // Index into the listed displays, or -1 to capture nothing for now.
  int index = -1;
  // True when this choice leaves a requested built-in panel that stayed
  // missing for the whole grace period.
  bool gave_up_on_builtin = false;
};

// Picks the display a stream slot captures from ScreenCaptureKit's list.
// |requested| is the display the slot was opened for (0 if none was known)
// and |active| the one it captures now, which differs after a fallback.
// |requested_builtin_online| is true when |requested| is the built-in panel
// and CoreGraphics still reports it online. |requested_mirror_source| is the
// display |requested| mirrors (0 if none); ScreenCaptureKit lists only the
// source, which shows the same picture.
//
// The requested display wins whenever it is listed, so a slot that fell back
// returns to it once it is back. A mirrored display is captured through its
// listed source at once, without the built-in grace period. While the
// requested display stays absent, a fallback that is still listed is kept
// instead of being picked again.
inline CaptureDisplayChoice ChooseCaptureDisplay(
    const std::vector<uint32_t>& listed, uint32_t requested, uint32_t active,
    bool requested_builtin_online, int logical_index, BuiltinPanelWait& wait,
    BuiltinPanelWait::Clock::time_point now,
    uint32_t requested_mirror_source = 0) {
  const auto find = [&listed](uint32_t display) {
    if (display == 0) return -1;
    const auto it = std::find(listed.begin(), listed.end(), display);
    return it == listed.end() ? -1 : static_cast<int>(it - listed.begin());
  };

  CaptureDisplayChoice choice;
  if (listed.empty()) return choice;

  if (const int requested_index = find(requested); requested_index >= 0) {
    wait.Reset();
    choice.index = requested_index;
    return choice;
  }
  if (const int mirror_index = find(requested_mirror_source);
      mirror_index >= 0) {
    wait.Reset();
    choice.index = mirror_index;
    return choice;
  }
  if (active != requested) {
    if (const int active_index = find(active); active_index >= 0) {
      choice.index = active_index;
      return choice;
    }
  }

  const bool builtin_missing = requested != 0 && requested_builtin_online;
  if (wait.ShouldWait(builtin_missing, now)) return choice;
  choice.gave_up_on_builtin = builtin_missing;
  choice.index =
      logical_index >= 0 && logical_index < static_cast<int>(listed.size())
          ? logical_index
          : 0;
  return choice;
}

// Whether a return probe whose display list is |listed| ends the fallback of a
// slot that asks for |requested| and captures |active| now.
// ChooseCaptureDisplay prefers |requested| and then the display it mirrors,
// |requested_mirror_source| (0 if none), which ScreenCaptureKit lists in its
// place, so either one being listed ends the fallback. A display the stream
// already captures is left out: listing it changes nothing, and a return for
// it would only restart the same capture on every probe.
inline bool FallbackReturnListed(const std::vector<uint32_t>& listed,
                                 uint32_t requested, uint32_t active,
                                 uint32_t requested_mirror_source = 0) {
  const auto ends_fallback = [&listed, active](uint32_t display) {
    return display != 0 && display != active &&
           std::find(listed.begin(), listed.end(), display) != listed.end();
  };
  return ends_fallback(requested) || ends_fallback(requested_mirror_source);
}

// Paces the checks that end a fallback. A display coming back into
// ScreenCaptureKit's list raises no event, and screen-parameter changes cover
// only layout changes, so a healthy stream that captures a fallback asks for
// the list on this interval to learn when its requested display is back.
class FallbackReturnProbe {
 public:
  using Clock = std::chrono::steady_clock;
  static constexpr Clock::duration kInterval = std::chrono::seconds(5);

  // |fallback_active| is true while a healthy stream captures a display other
  // than the one its slot asks for and that display is still online. Returns
  // whether to check the display list now.
  bool Due(bool fallback_active, Clock::time_point now) {
    if (!fallback_active) {
      next_check_.reset();
      return false;
    }
    if (!next_check_) {
      next_check_ = now + kInterval;
      return false;
    }
    if (now < *next_check_) return false;
    next_check_ = now + kInterval;
    return true;
  }

  // Marks |generation| as the display enumeration started because a probe
  // found the requested display listed again. That enumeration only looks for
  // a better display, so when it fails, lists nothing or times out, the
  // capturer keeps a fallback that still delivers frames instead of stopping
  // it for recovery.
  void ReturnStarted(uint64_t generation) { return_generation_ = generation; }

  // Called once enumeration |generation| has answered or been abandoned.
  // Returns whether it was the return enumeration, and ends that mark.
  bool ReturnAnswered(uint64_t generation) {
    if (return_generation_ != generation) return false;
    return_generation_.reset();
    return true;
  }

  void Reset() {
    next_check_.reset();
    return_generation_.reset();
  }

 private:
  std::optional<Clock::time_point> next_check_;
  std::optional<uint64_t> return_generation_;
};

// Starts the display enumeration of a new configuration. The capturer passes
// its own |generation|, |in_flight| and |reconfigure_pending|. While another
// configuration is in flight nothing starts: an ordinary request is queued for
// that one to run once it ends, and a return check is dropped, since the next
// probe asks again. Returns the new generation, or nothing when none started.
inline std::optional<uint64_t> BeginEnumeration(FallbackReturnProbe& probe,
                                                uint64_t& generation,
                                                bool& in_flight,
                                                bool& reconfigure_pending,
                                                bool return_check) {
  if (in_flight) {
    if (!return_check) reconfigure_pending = true;
    return std::nullopt;
  }
  in_flight = true;
  reconfigure_pending = false;
  ++generation;
  if (return_check) probe.ReturnStarted(generation);
  return generation;
}

struct EnumerationAnswer {
  // The answer belongs to the configuration in flight and is processed.
  bool current = false;
  // It answers a return check, whose failure keeps a working fallback.
  bool return_enumeration = false;
};

// Takes the answer to enumeration |answered| while the capturer is at
// |generation|. Only the current configuration's answer ends the flight. A
// late answer to an abandoned or superseded enumeration changes nothing, so it
// cannot end the flight of the enumeration that replaced it.
inline EnumerationAnswer AcceptEnumerationAnswer(FallbackReturnProbe& probe,
                                                 uint64_t answered,
                                                 uint64_t generation,
                                                 bool& in_flight) {
  EnumerationAnswer answer;
  if (answered != generation) return answer;
  answer.current = true;
  in_flight = false;
  answer.return_enumeration = probe.ReturnAnswered(answered);
  return answer;
}

// What the capturer does with a display enumeration that failed, listed no
// displays or ran past its timeout.
enum class EnumerationFailure {
  // Stop the stream and retry on the recovery backoff.
  recover,
  // Keep the fallback stream, which still delivers frames.
  keep_stream,
  // Keep it, and start the reconfiguration another change queued meanwhile.
  keep_stream_and_reconfigure,
};

// Decides for a failed enumeration. Only a return enumeration whose fallback
// still delivers frames keeps the stream, and it then takes over the queued
// reconfiguration, which the capturer must start. Recovery leaves
// |reconfigure_pending| to RecoverCapture.
inline EnumerationFailure ResolveEnumerationFailure(bool return_enumeration,
                                                    bool delivering_frames,
                                                    bool& reconfigure_pending) {
  if (!return_enumeration || !delivering_frames) {
    return EnumerationFailure::recover;
  }
  const bool queued = reconfigure_pending;
  reconfigure_pending = false;
  return queued ? EnumerationFailure::keep_stream_and_reconfigure
                : EnumerationFailure::keep_stream;
}

// Decides for the configuration in flight once it ran past its timeout. The
// capturer passes its own |generation|, |in_flight| and |reconfigure_pending|.
// A return enumeration that has not answered while its fallback still
// delivers frames is abandoned: the flight ends and moves to a new generation,
// so its late answer is ignored as stale.
inline EnumerationFailure ResolveConfigurationTimeout(
    FallbackReturnProbe& probe, uint64_t& generation, bool& in_flight,
    bool& reconfigure_pending, bool delivering_frames) {
  const EnumerationFailure action = ResolveEnumerationFailure(
      probe.ReturnAnswered(generation), delivering_frames, reconfigure_pending);
  if (action != EnumerationFailure::recover) {
    in_flight = false;
    ++generation;
  }
  return action;
}

// Holds the geometry a stream slot moves to until ScreenCaptureKit confirms
// that the stream captures the new display. Remote input maps through the
// slot's published geometry, so publishing it when the display is chosen
// would aim clicks at a display the viewer does not see yet, for example
// while a fallback returns to its requested display.
template <typename Info>
class PendingSlotGeometry {
 public:
  // Holds |info| for |slot| until configuration |generation| is confirmed.
  // A later hold replaces this one.
  void Hold(uint64_t generation, size_t slot, Info info) {
    pending_ = Entry{generation, slot, std::move(info)};
  }

  // Called once configuration |generation| has switched the stream. Publishes
  // the hold into |published| when that configuration made it. Generations
  // only grow, so an older hold can never be confirmed and is dropped.
  // Returns whether the hold was published.
  bool Confirm(uint64_t generation, std::vector<Info>& published) {
    if (!pending_ || pending_->generation > generation) return false;
    Entry entry = std::move(*pending_);
    pending_.reset();
    if (entry.generation != generation || entry.slot >= published.size()) {
      return false;
    }
    published[entry.slot] = std::move(entry.info);
    return true;
  }

  void Reset() { pending_.reset(); }

 private:
  struct Entry {
    uint64_t generation;
    size_t slot;
    Info info;
  };
  std::optional<Entry> pending_;
};

}  // namespace crossdesk

#endif
