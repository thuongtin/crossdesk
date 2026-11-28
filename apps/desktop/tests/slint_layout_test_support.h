// Do fork sở hữu: phần dựng chung cho các test bố cục render MainWindow thật bằng
// software renderer ở nhiều mức scale DPI, vùng làm việc nhỏ và mọi ngôn ngữ.
#pragma once

#include "crossdesk_ui.h"
#include "../src/gui/assets/fonts/fa_solid_900.h"
#include "application/window_geometry.h"
#include "ui/ui_localization.h"

#include <slint-platform.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace crossdesk::layout_test {

// Bộ chuyển cửa sổ có hệ số scale: buffer theo pixel vật lý, còn Slint bố cục
// theo đơn vị logic, giống cửa sổ thật trên màn hình 125% hay 150%.
class ScaledWindow : public slint::platform::WindowAdapter {
 public:
  slint::platform::SoftwareRenderer software{
      slint::platform::SoftwareRenderer::RepaintBufferType::NewBuffer};
  slint::PhysicalSize extent{{960, 650}};
  std::vector<slint::Rgb8Pixel> pixels =
      std::vector<slint::Rgb8Pixel>(std::size_t(960) * 650);
  float scale = 1.0f;

  slint::PhysicalSize size() override { return extent; }
  slint::platform::AbstractRenderer& renderer() override { return software; }
  void request_redraw() override {}
  void set_size(slint::PhysicalSize value) override {
    extent = value;
    pixels.assign(std::size_t(extent.width) * extent.height, {});
    window().dispatch_resize_event(slint::LogicalSize(
        {float(extent.width) / scale, float(extent.height) / scale}));
  }
  void SetScale(float value) {
    scale = value;
    window().dispatch_scale_factor_change_event(value);
  }
  // Mỗi lần chụp đều vẽ lại toàn bộ khung hình sau khi animation đã chạy xong.
  void Settle(int frames = 16) {
    for (int i = 0; i < frames; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(16));
      slint::platform::update_timers_and_animations();
    }
    software.render(pixels, extent.width);
  }
  slint::Rgb8Pixel At(int x, int y) const {
    return pixels.at(std::size_t(y) * extent.width + x);
  }
};

class ScaledPlatform : public slint::platform::Platform {
 public:
  ScaledWindow* adapter = nullptr;
  std::unique_ptr<slint::platform::WindowAdapter> create_window_adapter() override {
    auto result = std::make_unique<ScaledWindow>();
    adapter = result.get();
    return result;
  }
};

inline bool Check(bool value, const std::string& message) {
  if (!value) std::cerr << message << '\n';
  return value;
}

// Một cấu hình màn hình: hệ số scale và vùng làm việc logic (0 nghĩa là đủ chỗ
// cho kích thước thiết kế). Thanh tiêu đề tự vẽ giống bản Windows.
struct DisplayCase {
  const char* name;
  float scale;
  float usable_width;
  float usable_height;
  bool custom_titlebar;
};

constexpr float kTaskbarHeight = 48.0f;

inline const std::vector<DisplayCase>& AllDisplayCases() {
  static const std::vector<DisplayCase> cases = {
      {"100-default", 1.0f, 0.0f, 0.0f, false},
      {"125-default", 1.25f, 0.0f, 0.0f, true},
      {"150-default", 1.5f, 0.0f, 0.0f, true},
      {"125-1366x768", 1.25f, 1366.0f / 1.25f, (768.0f - kTaskbarHeight) / 1.25f, true},
      {"150-1280x800", 1.5f, 1280.0f / 1.5f, (800.0f - kTaskbarHeight) / 1.5f, true},
  };
  return cases;
}

// Biến môi trường filter_env (nếu đặt) giới hạn test vào một cấu hình khi cần
// soát nhanh. Tên gõ sai cho danh sách rỗng để main() báo lỗi.
inline std::vector<DisplayCase> SelectDisplayCases(const char* filter_env) {
  const char* only = std::getenv(filter_env);
  std::vector<DisplayCase> result;
  for (const auto& display : AllDisplayCases())
    if (!only || !*only || std::string(only) == display.name) result.push_back(display);
  return result;
}

inline void ReportUnknownDisplay(const char* filter_env) {
  std::cerr << "Unknown " << filter_env << "; expected one of:";
  for (const auto& display : AllDisplayCases()) std::cerr << ' ' << display.name;
  std::cerr << '\n';
}

struct LanguageCase {
  const char* name;
  int index;
};

// Tra chỉ số theo mã trong danh sách ngôn ngữ của ứng dụng thay vì ghi số cứng:
// đợt đồng bộ upstream chèn tr và he trước vi, khiến số 11 cũ của tiếng Việt
// trỏ sang tiếng Hebrew mà test vẫn qua.
inline int LanguageIndex(std::string_view code) {
  const auto& languages = crossdesk::localization::GetSupportedLanguages();
  for (std::size_t i = 0; i < languages.size(); ++i)
    if (languages[i].code == code) return static_cast<int>(i);
  std::cerr << "Unknown language code " << code << "\n";
  std::exit(9);
}

inline const std::vector<LanguageCase>& AllLanguages() {
  static const std::vector<LanguageCase> languages = [] {
    // Mọi ngôn ngữ ứng dụng hỗ trợ, đúng thứ tự của danh sách chọn ngôn ngữ.
    static const std::pair<const char*, const char*> kNamesAndCodes[] = {
        {"zh-Hans", "zh-CN"}, {"en", "en-US"}, {"ru", "ru-RU"}, {"zh-Hant", "zh-Hant"},
        {"ja", "ja"},         {"ko", "ko"},    {"de", "de"},    {"fr", "fr"},
        {"es", "es"},         {"pt-BR", "pt-BR"}, {"tr", "tr"}, {"he", "he"},
        {"vi", "vi"}};
    std::vector<LanguageCase> result;
    for (const auto& [name, code] : kNamesAndCodes)
      result.push_back({name, LanguageIndex(code)});
    if (result.size() != crossdesk::localization::GetSupportedLanguages().size()) {
      std::cerr << "AllLanguages must list every supported language\n";
      std::exit(9);
    }
    return result;
  }();
  return languages;
}

// Dựng MainWindow giống GuiApplication trên màn hình của cấu hình đã chọn: font
// icon, thanh tiêu đề, chuỗi theo ngôn ngữ, fit-scale và kích thước cửa sổ. Bên
// gọi nạp dữ liệu mẫu rồi show(). Trả về fit-scale đã gộp vào hệ số scale.
inline float PrepareMainWindow(const slint::ComponentHandle<crossdesk::ui::MainWindow>& main,
                               ScaledWindow& screen, const DisplayCase& display, int language) {
  if (main->window().window_handle().register_font_from_data(
          fa_solid_900_ttf, fa_solid_900_ttf_len).has_value()) {
    std::cerr << "Fixture FontAwesome registration failed\n";
    std::exit(8);
  }
  main->set_custom_titlebar(display.custom_titlebar);
  crossdesk::ui_localization::ApplyMainWindowStrings(main, language);
  main->set_language_index(language);
  float fit_scale = 1.0f;
  if (display.usable_width > 0.0f) {
    // Cùng công thức với GuiApplication::UpdateMainWindowFitScale.
    const float frame = display.custom_titlebar ? 8.0f : 16.0f;
    const float frame_height = display.custom_titlebar ? 8.0f : 40.0f;
    fit_scale = crossdesk::window_geometry::FitScale(
        display.usable_width, display.usable_height, frame, frame_height,
        main->get_design_width(), main->get_design_height());
  }
  // Software renderer bỏ qua transform-scale của ui-scaler. Phép thu nhỏ đó
  // đồng đều nên gộp vào hệ số scale của thiết bị cho ra đúng kích thước pixel
  // vật lý của cửa sổ thật: canvas giữ kích thước thiết kế, fit-scale giữ 1.
  screen.SetScale(display.scale * fit_scale);
  main->window().set_size(slint::LogicalSize(
      {main->get_minimum_window_width(), main->get_minimum_window_height()}));
  return fit_scale;
}

inline bool WritePpm(const ScaledWindow& screen, const std::filesystem::path& path) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  output << "P6\n" << screen.extent.width << ' ' << screen.extent.height << "\n255\n";
  for (const auto& pixel : screen.pixels) {
    output.put(static_cast<char>(pixel.r));
    output.put(static_cast<char>(pixel.g));
    output.put(static_cast<char>(pixel.b));
  }
  return Check(output.good(), "Layout evidence must be written completely: " + path.string());
}

// Lưu khung hình lúc kiểm tra thất bại để dễ xem lỗi, chỉ khi có thư mục evidence.
inline void WriteFailureFrame(const ScaledWindow& screen, const char* evidence_env,
                              const std::string& label) {
  const char* directory = std::getenv(evidence_env);
  if (!directory || !*directory) return;
  std::string name = "failure-" + label;
  for (auto& ch : name)
    if (ch == '/' || ch == ' ') ch = '-';
  WritePpm(screen, std::filesystem::path(directory) / (name + ".ppm"));
}

}  // namespace crossdesk::layout_test
