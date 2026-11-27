#include <cmath>
#include <iostream>

#include "application/window_geometry.h"

namespace {

bool Expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << '\n';
  return false;
}

bool Near(float actual, float expected) {
  return std::fabs(actual - expected) < 0.005f;
}

}  // namespace

int main() {
  using crossdesk::window_geometry::FitScale;
  // Chỗ cho khung gốc: 16 x 40 đơn vị logic. Canvas thiết kế: 960 x 650.
  constexpr float kFrameWidth = 16.0f;
  constexpr float kFrameHeight = 40.0f;
  constexpr float kDesignWidth = 960.0f;
  constexpr float kDesignHeight = 650.0f;

  bool ok = true;
  // 1920x1080 ở 100% với taskbar 40 px: vùng logic 1920x1040, vừa nguyên.
  ok &= Expect(FitScale(1920, 1040, kFrameWidth, kFrameHeight, kDesignWidth,
                        kDesignHeight) == 1.0f,
               "1080p at 100% must keep the design size");
  // 1366x768 ở 125% với taskbar 48 px: vùng logic 1092.8 x 576.
  ok &= Expect(Near(FitScale(1366 / 1.25f, (768 - 48) / 1.25f, kFrameWidth,
                             kFrameHeight, kDesignWidth, kDesignHeight),
                    0.825f),
               "1366x768 at 125% must scale to about 0.825");
  // 1920x1080 ở 150% với taskbar 48 px: vùng logic 1280 x 688.
  ok &= Expect(Near(FitScale(1920 / 1.5f, (1080 - 48) / 1.5f, kFrameWidth,
                             kFrameHeight, kDesignWidth, kDesignHeight),
                    0.997f),
               "1080p at 150% must be nearly unscaled");
  // Vùng làm việc quá nhỏ không bao giờ xuống dưới tỷ lệ tối thiểu.
  ok &= Expect(FitScale(300, 200, kFrameWidth, kFrameHeight, kDesignWidth,
                        kDesignHeight) == 0.5f,
               "scale must not drop below 0.5");
  // Đầu vào sai thì giữ kích thước thiết kế.
  ok &= Expect(FitScale(0, 0, kFrameWidth, kFrameHeight, kDesignWidth,
                        kDesignHeight) == 1.0f,
               "invalid work area must keep the design size");
  return ok ? 0 : 1;
}
