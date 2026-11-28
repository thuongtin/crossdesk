#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include "builtin_panel_wait.h"

namespace {

bool Expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << '\n';
  return false;
}

bool TestGracePeriod() {
  using crossdesk::BuiltinPanelWait;
  using std::chrono::milliseconds;

  const auto start = BuiltinPanelWait::Clock::now();
  const auto grace = BuiltinPanelWait::kGrace;
  bool ok = true;

  BuiltinPanelWait wait;
  ok &= Expect(!wait.ShouldWait(false, start),
               "a listed panel must never hold the stream back");
  ok &= Expect(wait.ShouldWait(true, start),
               "a panel that just vanished on wake must be waited for");
  // ShouldWait() only answers at the moments the capturer asks. The capturer
  // asks on its retry backoff, so the real fallback lands on the first retry
  // at or after the grace, never before it.
  ok &= Expect(wait.ShouldWait(true, start + grace - milliseconds(1)),
               "a check inside the grace period must keep waiting");
  ok &= Expect(!wait.ShouldWait(true, start + grace),
               "a check once the grace period is over must allow fallback");
  ok &= Expect(!wait.ShouldWait(true, start + grace * 3),
               "the fallback must not wait again while the panel stays missing");

  ok &= Expect(!wait.ShouldWait(false, start + grace * 4),
               "a panel that returns must end the wait");
  const auto later = start + grace * 5;
  ok &= Expect(wait.ShouldWait(true, later),
               "a later disappearance must start a new grace period");
  ok &= Expect(wait.ShouldWait(true, later + grace - milliseconds(1)),
               "the new grace period must count from the later disappearance");

  wait.Reset();
  const auto restarted = later + grace * 2;
  ok &= Expect(wait.ShouldWait(true, restarted),
               "a restarted capture must wait for the panel again");
  return ok;
}

bool TestDisplayChoice() {
  using crossdesk::BuiltinPanelWait;
  using crossdesk::ChooseCaptureDisplay;

  constexpr uint32_t builtin = 1;
  constexpr uint32_t external = 2;
  const std::vector<uint32_t> both = {builtin, external};
  const std::vector<uint32_t> external_only = {external};
  const auto start = BuiltinPanelWait::Clock::now();
  const auto grace = BuiltinPanelWait::kGrace;
  bool ok = true;

  BuiltinPanelWait wait;
  auto choice =
      ChooseCaptureDisplay(both, builtin, builtin, true, 0, wait, start);
  ok &= Expect(choice.index == 0 && !choice.gave_up_on_builtin,
               "a listed requested display must be captured");

  // Display 1 asks for the built-in panel, which vanishes on wake while an
  // external monitor stays listed.
  choice = ChooseCaptureDisplay(external_only, builtin, builtin, true, 0, wait,
                                start);
  ok &= Expect(choice.index == -1,
               "a missing built-in panel must be waited for first");
  choice = ChooseCaptureDisplay(external_only, builtin, builtin, true, 0, wait,
                                start + grace + std::chrono::seconds(2));
  ok &= Expect(choice.index == 0 && choice.gave_up_on_builtin,
               "the first retry after the grace must fall back and say why");

  // The slot now captures the external monitor but still asks for the panel.
  choice = ChooseCaptureDisplay(external_only, builtin, external, true, 0, wait,
                                start + grace * 2);
  ok &= Expect(choice.index == 0 && !choice.gave_up_on_builtin,
               "a fallback that is still listed must be kept quietly");
  choice = ChooseCaptureDisplay(both, builtin, external, true, 0, wait,
                                start + grace * 3);
  ok &= Expect(choice.index == 0,
               "the slot must return to the built-in panel once it is listed");

  // Returning ended the old wait, so a new disappearance waits again.
  choice = ChooseCaptureDisplay(external_only, builtin, builtin, true, 0, wait,
                                start + grace * 4);
  ok &= Expect(choice.index == -1,
               "a later disappearance must start a new grace period");

  // The requested panel and the fallback are both gone: the grace already
  // ran out, so the slot must not go black for another one.
  BuiltinPanelWait expired;
  ChooseCaptureDisplay(external_only, builtin, builtin, true, 0, expired, start);
  const std::vector<uint32_t> third_only = {3};
  choice = ChooseCaptureDisplay(third_only, builtin, external, true, 0, expired,
                                start + grace);
  ok &= Expect(choice.index == 0,
               "losing the fallback after the grace must fall back at once");

  BuiltinPanelWait unused;
  choice = ChooseCaptureDisplay(both, external, external, false, 1, unused,
                                start);
  ok &= Expect(choice.index == 1,
               "the requested display must be found wherever it is listed");
  choice = ChooseCaptureDisplay(external_only, 7, 7, false, 0, unused, start);
  ok &= Expect(choice.index == 0 && !choice.gave_up_on_builtin,
               "a missing external display must fall back without waiting");
  choice = ChooseCaptureDisplay(both, 7, 7, false, 1, unused, start);
  ok &= Expect(choice.index == 1,
               "a fallback must prefer the display at the slot's position");
  choice = ChooseCaptureDisplay(external_only, 7, 7, false, 4, unused, start);
  ok &= Expect(choice.index == 0,
               "a slot past the end of the list must use the first display");
  choice = ChooseCaptureDisplay(both, 0, external, false, 0, unused, start);
  ok &= Expect(choice.index == 1,
               "a slot without a requested display must keep its capture");
  choice = ChooseCaptureDisplay(both, 0, 0, false, 0, unused, start);
  ok &= Expect(choice.index == 0,
               "a slot without any display must take its position");
  choice = ChooseCaptureDisplay({}, builtin, builtin, true, 0, unused, start);
  ok &= Expect(choice.index == -1 && !choice.gave_up_on_builtin,
               "an empty list must not pick a display");
  return ok;
}

bool TestFallbackReturnProbe() {
  using crossdesk::FallbackReturnProbe;
  using std::chrono::milliseconds;

  const auto start = FallbackReturnProbe::Clock::now();
  const auto interval = FallbackReturnProbe::kInterval;
  bool ok = true;

  FallbackReturnProbe probe;
  ok &= Expect(!probe.Due(false, start),
               "a slot on its requested display must never probe");
  ok &= Expect(!probe.Due(true, start),
               "a fresh fallback must not probe at once");
  ok &= Expect(!probe.Due(true, start + interval - milliseconds(1)),
               "a fallback must not probe before the interval");
  ok &= Expect(probe.Due(true, start + interval),
               "a healthy fallback must probe once the interval is over");
  ok &= Expect(!probe.Due(true, start + interval + milliseconds(1)),
               "a probe must not repeat right after it ran");
  ok &= Expect(probe.Due(true, start + interval * 2),
               "a fallback that stays must keep probing on the interval");

  ok &= Expect(!probe.Due(false, start + interval * 3),
               "returning to the requested display must stop probing");
  const auto later = start + interval * 4;
  ok &= Expect(!probe.Due(true, later),
               "a new fallback must wait a full interval again");
  ok &= Expect(probe.Due(true, later + interval),
               "the new interval must count from the new fallback");

  probe.Reset();
  ok &= Expect(!probe.Due(true, later + interval * 3),
               "a restarted capture must wait a full interval again");
  return ok;
}

bool TestReturnEnumeration() {
  using crossdesk::FallbackReturnProbe;

  const auto start = FallbackReturnProbe::Clock::now();
  bool ok = true;

  FallbackReturnProbe probe;
  ok &= Expect(!probe.ReturnAnswered(1),
               "an enumeration no probe started must not count as a return");

  probe.ReturnStarted(7);
  // The enumeration is in flight, so the health check sees no fallback to
  // probe. That must not forget which enumeration the probe started.
  probe.Due(false, start);
  ok &= Expect(!probe.ReturnAnswered(6),
               "an older enumeration must not count as the return");
  ok &= Expect(probe.ReturnAnswered(7),
               "the enumeration a probe started must count as the return");
  ok &= Expect(!probe.ReturnAnswered(7),
               "a return must be answered only once");

  probe.ReturnStarted(8);
  ok &= Expect(!probe.ReturnAnswered(9),
               "a newer enumeration must not count as the return");
  ok &= Expect(probe.ReturnAnswered(8),
               "a newer answer must not end the return it did not start");

  probe.ReturnStarted(10);
  probe.Reset();
  ok &= Expect(!probe.ReturnAnswered(10),
               "a restarted capture must forget the return in flight");
  return ok;
}

bool TestEnumerationFailure() {
  using crossdesk::EnumerationFailure;
  using crossdesk::ResolveEnumerationFailure;

  bool ok = true;
  bool pending = true;
  ok &= Expect(ResolveEnumerationFailure(false, true, pending) ==
                   EnumerationFailure::recover,
               "a failed normal enumeration must recover");
  ok &= Expect(pending,
               "recovery must leave the queued reconfiguration to RecoverCapture");
  ok &= Expect(ResolveEnumerationFailure(true, false, pending) ==
                   EnumerationFailure::recover,
               "a failed return without frames must recover");

  pending = false;
  ok &= Expect(ResolveEnumerationFailure(true, true, pending) ==
                   EnumerationFailure::keep_stream,
               "a failed return must keep a fallback that delivers frames");
  pending = true;
  ok &= Expect(ResolveEnumerationFailure(true, true, pending) ==
                   EnumerationFailure::keep_stream_and_reconfigure,
               "a kept fallback must start the reconfiguration queued meanwhile");
  ok &= Expect(!pending, "the queued reconfiguration must be taken once");
  return ok;
}

bool TestConfigurationTimeout() {
  using crossdesk::EnumerationFailure;
  using crossdesk::FallbackReturnProbe;
  using crossdesk::ResolveConfigurationTimeout;

  bool ok = true;

  // A normal configuration that ran past its timeout recovers. RecoverCapture
  // revokes its generation, so the state is left for it.
  FallbackReturnProbe probe;
  uint64_t generation = 5;
  bool in_flight = true;
  bool pending = true;
  ok &= Expect(ResolveConfigurationTimeout(probe, generation, in_flight, pending,
                                           true) == EnumerationFailure::recover,
               "a normal configuration that timed out must recover");
  ok &= Expect(generation == 5 && in_flight && pending,
               "recovery must leave the flight to RecoverCapture");

  // The return enumeration hangs while the fallback delivers frames and a
  // screen change queued a reconfiguration behind it.
  generation = 6;
  probe.ReturnStarted(generation);
  ok &= Expect(ResolveConfigurationTimeout(probe, generation, in_flight, pending,
                                           true) ==
                   EnumerationFailure::keep_stream_and_reconfigure,
               "a return that timed out must keep the fallback and reconfigure");
  ok &= Expect(!in_flight, "an abandoned return must end the flight");
  ok &= Expect(!pending, "the queued reconfiguration must be taken once");
  ok &= Expect(generation == 7,
               "an abandoned return must move to a new generation");
  // OnShareableContentCreated ignores an answer whose generation is not the
  // current one, and the mark is gone, so a late answer changes nothing.
  ok &= Expect(generation != 6 && !probe.ReturnAnswered(6),
               "a late answer to the abandoned return must be stale");

  generation = 8;
  in_flight = true;
  pending = false;
  probe.ReturnStarted(generation);
  ok &= Expect(ResolveConfigurationTimeout(probe, generation, in_flight, pending,
                                           true) == EnumerationFailure::keep_stream,
               "a return that timed out alone must only keep the fallback");
  ok &= Expect(generation == 9 && !in_flight,
               "a kept fallback must still revoke the abandoned return");

  generation = 10;
  in_flight = true;
  probe.ReturnStarted(generation);
  ok &= Expect(ResolveConfigurationTimeout(probe, generation, in_flight, pending,
                                           false) == EnumerationFailure::recover,
               "a return that timed out without frames must recover");
  ok &= Expect(generation == 10 && in_flight,
               "recovery after a return must leave the flight to RecoverCapture");

  // Once the return enumeration has answered, the same generation goes on to
  // reconfigure the stream. A timeout there is a real failure.
  generation = 11;
  probe.ReturnStarted(generation);
  probe.ReturnAnswered(generation);
  ok &= Expect(ResolveConfigurationTimeout(probe, generation, in_flight, pending,
                                           true) == EnumerationFailure::recover,
               "a stream reconfiguration that timed out must recover");
  return ok;
}

}  // namespace

int main() {
  bool ok = TestGracePeriod();
  ok &= TestDisplayChoice();
  ok &= TestFallbackReturnProbe();
  ok &= TestReturnEnumeration();
  ok &= TestEnumerationFailure();
  ok &= TestConfigurationTimeout();
  return ok ? 0 : 1;
}
