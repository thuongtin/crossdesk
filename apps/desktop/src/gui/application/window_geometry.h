/*
 * @Author: DI JUNKUN
 * @Date: 2026-09-04
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _WINDOW_GEOMETRY_H_
#define _WINDOW_GEOMETRY_H_

#include <algorithm>
#include <cmath>

namespace crossdesk::window_geometry {

struct PhysicalRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

struct PhysicalSize {
  int width = 0;
  int height = 0;
};

struct PhysicalPosition {
  int x = 0;
  int y = 0;
};

constexpr PhysicalPosition CenteredPosition(const PhysicalRect& bounds,
                                            const PhysicalSize& size) {
  return {
      bounds.x + (bounds.width - std::min(bounds.width, size.width)) / 2,
      bounds.y + (bounds.height - std::min(bounds.height, size.height)) / 2,
  };
}

constexpr PhysicalPosition BottomRightPosition(const PhysicalRect& bounds,
                                               const PhysicalSize& size) {
  return {
      bounds.x + std::max(0, bounds.width - size.width),
      bounds.y + std::max(0, bounds.height - size.height),
  };
}

// Do fork sở hữu: tỷ lệ (0.5 đến 1) để canvas Postcard cố định vừa trong vùng
// làm việc của màn hình. Mọi giá trị là đơn vị logic (không phụ thuộc DPI);
// frame_* là chỗ khung cửa sổ và thanh tiêu đề gốc chiếm.
inline float FitScale(float usable_width, float usable_height, float frame_width,
                      float frame_height, float design_width,
                      float design_height, float min_scale = 0.5f) {
  if (!std::isfinite(usable_width) || !std::isfinite(usable_height) ||
      usable_width <= 0.0f || usable_height <= 0.0f || design_width <= 0.0f ||
      design_height <= 0.0f) {
    return 1.0f;
  }
  const float fit =
      std::min((usable_width - frame_width) / design_width,
               (usable_height - frame_height) / design_height);
  return std::clamp(fit, min_scale, 1.0f);
}

}  // namespace crossdesk::window_geometry

#endif