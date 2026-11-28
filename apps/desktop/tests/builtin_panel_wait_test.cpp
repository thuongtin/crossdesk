#include <chrono>
#include <iostream>

#include "builtin_panel_wait.h"

namespace {

bool Expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << '\n';
  return false;
}

}  // namespace

int main() {
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
  ok &= Expect(wait.ShouldWait(true, start + grace - milliseconds(1)),
               "the panel must be waited for until the grace period ends");
  ok &= Expect(!wait.ShouldWait(true, start + grace),
               "a panel that stays missing must let the stream fall back");
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

  return ok ? 0 : 1;
}
