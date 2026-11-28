#include <iostream>

#include "application/frame_wait_hint.h"

namespace {

bool Expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << '\n';
  return false;
}

}  // namespace

int main() {
  using crossdesk::CaptureState;
  using crossdesk::frame_wait::Hint;
  using crossdesk::frame_wait::Select;

  bool ok = true;
  ok &= Expect(Select(0, CaptureState::permission_denied) == Hint::receiving,
               "a fresh connection must keep the receiving hint");
  ok &= Expect(Select(2999, CaptureState::display_unavailable) == Hint::receiving,
               "a reported reason must wait for the reason delay");
  ok &= Expect(Select(3000, CaptureState::permission_denied) == Hint::permission_denied,
               "a missing permission must be explained after the reason delay");
  ok &= Expect(Select(3000, CaptureState::display_unavailable) == Hint::display_unavailable,
               "a sleeping display must be explained after the reason delay");
  ok &= Expect(Select(9999, CaptureState::failed) == Hint::receiving,
               "a capture failure may still recover before the generic delay");
  ok &= Expect(Select(10000, CaptureState::failed) == Hint::capture_failed,
               "a lasting capture failure must be explained");
  ok &= Expect(Select(10000, CaptureState::capturing) == Hint::no_video,
               "a capturing host without video must suggest reconnecting");
  ok &= Expect(Select(9999, CaptureState::unknown) == Hint::receiving,
               "an unknown state must wait for the generic delay");
  ok &= Expect(Select(10000, CaptureState::unknown) == Hint::no_picture,
               "a legacy host must get the generic hint, not an endless wait");
  ok &= Expect(Select(10000, CaptureState::starting) == Hint::no_picture,
               "a host stuck starting must get the generic hint");
  return ok ? 0 : 1;
}
