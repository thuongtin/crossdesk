#ifndef _FRAME_WAIT_HINT_H_
#define _FRAME_WAIT_HINT_H_

#include <cstdint>

#include <remote_action.h>

namespace crossdesk::frame_wait {

// What a connected stream window says while it has no picture yet.
enum class Hint {
  receiving,
  // No reason is known; the host cannot report one or did not.
  no_picture,
  // The host captures its screen, so the video is lost on the way.
  no_video,
  permission_denied,
  display_unavailable,
  capture_failed,
};

// A first frame normally arrives within a second or two. Give the host a
// moment before showing the reason it reported, and longer before showing a
// reason that may still clear by itself.
inline constexpr uint64_t kReasonDelayMs = 3000;
inline constexpr uint64_t kGenericDelayMs = 10000;
// How often a viewer asks a capable host for its capture state while waiting.
inline constexpr uint64_t kQueryIntervalMs = 1000;

// |reported| is the host's latest capture_status, or unknown when the host
// cannot report one or has not answered yet.
inline Hint Select(uint64_t waited_ms, CaptureState reported) {
  if (waited_ms >= kReasonDelayMs) {
    if (reported == CaptureState::permission_denied) return Hint::permission_denied;
    if (reported == CaptureState::display_unavailable) return Hint::display_unavailable;
  }
  if (waited_ms < kGenericDelayMs) return Hint::receiving;
  switch (reported) {
    case CaptureState::failed:
      return Hint::capture_failed;
    case CaptureState::capturing:
      return Hint::no_video;
    default:
      return Hint::no_picture;
  }
}

}  // namespace crossdesk::frame_wait

#endif
