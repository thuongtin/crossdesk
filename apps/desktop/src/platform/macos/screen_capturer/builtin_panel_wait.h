#ifndef _BUILTIN_PANEL_WAIT_H_
#define _BUILTIN_PANEL_WAIT_H_

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <optional>
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
// and CoreGraphics still reports it online.
//
// The requested display wins whenever it is listed, so a slot that fell back
// returns to it once it is back. While it stays absent, a fallback that is
// still listed is kept instead of being picked again.
inline CaptureDisplayChoice ChooseCaptureDisplay(
    const std::vector<uint32_t>& listed, uint32_t requested, uint32_t active,
    bool requested_builtin_online, int logical_index, BuiltinPanelWait& wait,
    BuiltinPanelWait::Clock::time_point now) {
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

  void Reset() { next_check_.reset(); }

 private:
  std::optional<Clock::time_point> next_check_;
};

}  // namespace crossdesk

#endif
