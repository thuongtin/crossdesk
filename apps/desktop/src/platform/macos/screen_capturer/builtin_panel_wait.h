#ifndef _BUILTIN_PANEL_WAIT_H_
#define _BUILTIN_PANEL_WAIT_H_

#include <chrono>
#include <optional>

namespace crossdesk {

// The built-in panel can briefly disappear from ScreenCaptureKit on wake while
// CoreGraphics still reports it online. A stream bound to that panel waits a
// short grace period for it to return, then falls back to a listed display so
// viewers are not left without a picture if the panel never comes back.
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

}  // namespace crossdesk

#endif
