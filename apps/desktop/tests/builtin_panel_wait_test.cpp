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

}  // namespace

int main() {
  bool ok = TestGracePeriod();
  ok &= TestDisplayChoice();
  return ok ? 0 : 1;
}
