// Do fork sở hữu: kiểm tra bố cục hộp thoại Settings ở nhiều mức scale DPI và
// vùng làm việc nhỏ, render MainWindow thật bằng software renderer. Khi đặt
// CROSSDESK_SETTINGS_EVIDENCE_DIR, test xuất thêm ảnh PPM của từng trang Settings
// và các hộp thoại mở từ đó để soát giao diện bằng mắt.
#include "crossdesk_ui.h"
#include "../src/gui/assets/fonts/fa_solid_900.h"
#include "application/window_geometry.h"
#include "ui/ui_localization.h"

#include <slint-platform.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace {

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

bool Check(bool value, const std::string& message) {
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

const std::vector<DisplayCase>& AllDisplayCases() {
  static const std::vector<DisplayCase> cases = {
      {"100-default", 1.0f, 0.0f, 0.0f, false},
      {"125-default", 1.25f, 0.0f, 0.0f, true},
      {"150-default", 1.5f, 0.0f, 0.0f, true},
      {"125-1366x768", 1.25f, 1366.0f / 1.25f, (768.0f - kTaskbarHeight) / 1.25f, true},
      {"150-1280x800", 1.5f, 1280.0f / 1.5f, (800.0f - kTaskbarHeight) / 1.5f, true},
  };
  return cases;
}

// CROSSDESK_SETTINGS_DISPLAY giới hạn test vào một cấu hình khi cần soát nhanh.
const std::vector<DisplayCase>& DisplayCases() {
  static const std::vector<DisplayCase> cases = [] {
    const char* only = std::getenv("CROSSDESK_SETTINGS_DISPLAY");
    std::vector<DisplayCase> result;
    for (const auto& display : AllDisplayCases())
      if (!only || !*only || std::string(only) == display.name) result.push_back(display);
    return result;
  }();
  return cases;
}

struct LanguageCase {
  const char* name;
  int index;
};

// Chỉ số khớp danh sách ngôn ngữ của ứng dụng (10 là "theo hệ thống").
const std::vector<LanguageCase>& LayoutLanguages() {
  static const std::vector<LanguageCase> languages = {
      {"en", 1}, {"vi", 11}, {"de", 6}};
  return languages;
}

const std::vector<LanguageCase>& AllLanguages() {
  static const std::vector<LanguageCase> languages = {
      {"zh-Hans", 0}, {"en", 1}, {"ru", 2}, {"zh-Hant", 3}, {"ja", 4}, {"ko", 5},
      {"de", 6}, {"fr", 7}, {"es", 8}, {"pt-BR", 9}, {"vi", 11}};
  return languages;
}

// Hình học hộp thoại Settings trên canvas thiết kế (khớp main_window.slint).
constexpr float kDialogWidth = 400.0f;
constexpr float kDialogHeight = 446.0f;
constexpr float kContentInset = 26.0f;
constexpr float kScrollTop = 108.0f;
constexpr float kScrollBottomReserve = 174.0f;
constexpr float kRowWidth = kDialogWidth - 2 * kContentInset;
constexpr float kRowHeight = 36.0f;
constexpr float kToggleWidth = 36.0f;
constexpr float kToggleHeight = 20.0f;
constexpr float kToggleInset = 12.0f;

class Fixture {
 public:
  Fixture(ScaledPlatform& platform, const DisplayCase& display, int language)
      : display_(display), main_(crossdesk::ui::MainWindow::create()) {
    if (main_->window().window_handle().register_font_from_data(
            fa_solid_900_ttf, fa_solid_900_ttf_len).has_value()) {
      std::cerr << "Fixture FontAwesome registration failed\n";
      std::exit(8);
    }
    screen_ = platform.adapter;
    main_->set_custom_titlebar(display.custom_titlebar);
    crossdesk::ui_localization::ApplyMainWindowStrings(main_, language);
    main_->set_language_index(language);
    if (display.usable_width > 0.0f) {
      // Cùng công thức với GuiApplication::UpdateMainWindowFitScale.
      const float frame = display.custom_titlebar ? 8.0f : 16.0f;
      const float frame_height = display.custom_titlebar ? 8.0f : 40.0f;
      fit_scale_ = crossdesk::window_geometry::FitScale(
          display.usable_width, display.usable_height, frame, frame_height,
          main_->get_design_width(), main_->get_design_height());
    }
    // Software renderer bỏ qua transform-scale của ui-scaler. Phép thu nhỏ đó
    // đồng đều nên gộp vào hệ số scale của thiết bị cho ra đúng kích thước pixel
    // vật lý của cửa sổ thật: canvas giữ kích thước thiết kế, fit-scale giữ 1.
    screen_->SetScale(display.scale * fit_scale_);
    main_->window().set_size(slint::LogicalSize(
        {main_->get_minimum_window_width(), main_->get_minimum_window_height()}));
    ConfigureData();
    main_->show();
    screen_->Settle();
    ui_scale_ = main_->get_ui_scale();
    titlebar_ = main_->get_design_height() - 650.0f;
  }
  ~Fixture() { main_->hide(); }

  crossdesk::ui::MainWindow& main() { return *main_; }
  ScaledWindow& screen() { return *screen_; }

  // Đổi tọa độ canvas thiết kế sang tọa độ logic của cửa sổ và pixel vật lý.
  float Logical(float canvas) const { return canvas * ui_scale_; }
  float Physical(float canvas) const {
    return canvas * ui_scale_ * display_.scale * fit_scale_;
  }
  float DialogX() const { return (main_->get_design_width() - kDialogWidth) / 2; }
  float DialogY() const {
    return titlebar_ + (main_->get_design_height() - titlebar_ - kDialogHeight) / 2;
  }
  float ViewportTop() const { return DialogY() + kScrollTop; }
  float ViewportBottom() const {
    return DialogY() + kDialogHeight - kScrollBottomReserve;
  }
  float RowLeft() const { return DialogX() + kContentInset; }
  float RowRight() const { return RowLeft() + kRowWidth; }

  void MovePointer(float canvas_x, float canvas_y) {
    main_->window().dispatch_pointer_move_event(
        slint::LogicalPosition({Logical(canvas_x), Logical(canvas_y)}));
  }
  void Click(float canvas_x, float canvas_y) {
    const slint::LogicalPosition point({Logical(canvas_x), Logical(canvas_y)});
    main_->window().dispatch_pointer_move_event(point);
    main_->window().dispatch_pointer_press_event(point, slint::PointerEventButton::Left);
    main_->window().dispatch_pointer_release_event(point, slint::PointerEventButton::Left);
    screen_->Settle();
  }
  // Cuộn vùng Settings rồi đưa con trỏ ra ngoài hộp thoại để không dính hover
  // hay tooltip của hàng nằm dưới con trỏ.
  void Scroll(float delta, int frames = 16) {
    const slint::LogicalPosition point(
        {Logical(RowLeft() + 40.0f), Logical(ViewportTop() + 60.0f)});
    main_->window().dispatch_pointer_scroll_event(point, 0.0f, delta);
    MovePointer(8.0f, titlebar_ + 8.0f);
    screen_->Settle(frames);
  }
  void ScrollToTop() { Scroll(5000.0f); }

  void CloseAll() {
    main_->set_self_host_settings_open(false);
    main_->set_trusted_devices_open(false);
    main_->set_autostart_guide_open(false);
    main_->set_feedback_open(false);
    main_->set_about_open(false);
    main_->set_permission_dialog_open(false);
    main_->set_portable_service_dialog_open(false);
    main_->set_settings_open(false);
    main_->set_settings_tab(0);
    main_->set_settings_session_active(false);
    main_->set_settings_save_error("");
    main_->set_self_host_settings_error("");
    MovePointer(8.0f, titlebar_ + 8.0f);
    screen_->Settle();
  }

 private:
  void ConfigureData() {
    main_->set_local_id("123 456 789");
    main_->set_local_password("A1b2C3");
    main_->set_signal_connected(true);
    main_->set_current_version("1.7.7");
    main_->set_server_host("crossdesk.example.test");
    main_->set_server_port("9099");
    main_->set_file_save_path("/Users/fixture/Documents/CrossDesk received files");
    main_->set_device_fingerprint("AB12 CD34 EF56 7890 1A2B 3C4D 5E6F 7A8B");
    std::vector<crossdesk::ui::TrustedDeviceEntry> devices;
    for (const auto* name : {"Studio fixture", "Office fixture workstation"}) {
      crossdesk::ui::TrustedDeviceEntry entry{};
      entry.remote_id = "987654321"; entry.display_name = name;
      entry.device_id = "987654321";
      entry.fingerprint = "12AB 34CD 56EF 7890 AB12 CD34 EF56 7890";
      devices.push_back(entry);
    }
    main_->set_trusted_devices(
        std::make_shared<slint::VectorModel<crossdesk::ui::TrustedDeviceEntry>>(devices));
    main_->set_feedback_status("Diagnostic logs exported.");
    main_->set_feedback_succeeded(true);
    main_->set_feedback_saved_path(
        "/Users/fixture/Desktop/CrossDesk-diagnostics-20261128-101500.zip");
    main_->set_service_autostart_hint(
        "The lock-screen service starts with the system and keeps this device reachable.");
  }

  DisplayCase display_;
  slint::ComponentHandle<crossdesk::ui::MainWindow> main_;
  ScaledWindow* screen_ = nullptr;
  float fit_scale_ = 1.0f;
  float ui_scale_ = 1.0f;
  float titlebar_ = 0.0f;
};

bool WritePpm(const ScaledWindow& screen, const std::filesystem::path& path);

// Lưu khung hình lúc kiểm tra thất bại để dễ xem lỗi, chỉ khi có thư mục evidence.
void WriteFailureFrame(const ScaledWindow& screen, const std::string& label) {
  const char* directory = std::getenv("CROSSDESK_SETTINGS_EVIDENCE_DIR");
  if (!directory || !*directory) return;
  std::string name = "failure-" + label;
  for (auto& ch : name)
    if (ch == '/' || ch == ' ') ch = '-';
  WritePpm(screen, std::filesystem::path(directory) / (name + ".ppm"));
}

bool ToggleOn(const slint::Rgb8Pixel& pixel) {
  // ToggleSwitch đang bật có màu #2463c7; viền chống răng cưa thì nhạt hơn.
  return pixel.b > 150 && pixel.r < 90 && pixel.g > 60 && pixel.g < 140;
}

bool Ink(const slint::Rgb8Pixel& pixel) {
  // Nền hàng là trắng; viền nút (#b8c7db) và chữ đều tối hơn ngưỡng này.
  return pixel.r < 232 || pixel.g < 232 || pixel.b < 238;
}

struct RowProbe {
  const char* name;
  // Hàng chứa nút cách hàng có toggle mốc bao nhiêu hàng.
  int row_offset;
  // Mép phải của nút cách mép phải hàng bao nhiêu (đơn vị canvas).
  float button_right_inset;
  std::function<void(crossdesk::ui::MainWindow&)> arrange;
};

// Tìm toggle mốc (toggle duy nhất đang bật trong vùng cuộn), rồi đo nút trên
// hàng cần kiểm tra: nút phải nằm gọn trong hàng và cách đều mép trên, mép dưới.
bool VerifyRowButton(Fixture& fixture, const RowProbe& probe, const std::string& label) {
  auto& main = fixture.main();
  auto& screen = fixture.screen();
  main.set_ip_direct_enabled_draft(false);
  main.set_lan_listen_all_draft(false);
  main.set_force_relay_enabled(false);
  main.set_self_hosted_enabled(false);
  main.set_trusted_devices_only(false);
  main.set_save_remote_previews(false);
  main.set_desktop_realtime_rate_control(false);
  main.set_pause_video_when_minimized(false);
  main.set_autostart_enabled(false);
  main.set_daemon_enabled(false);
  probe.arrange(main);
  main.set_settings_open(true);
  screen.Settle();
  fixture.ScrollToTop();

  const float unit = fixture.Physical(1.0f);
  const int toggle_column =
      int(std::lround(fixture.Physical(fixture.RowRight() - kToggleInset - kToggleWidth / 2)));
  const int button_column = int(std::lround(
      fixture.Physical(fixture.RowRight() - probe.button_right_inset - 4.0f)));
  const int top_limit = int(std::ceil(fixture.Physical(fixture.ViewportTop()))) + 2;
  const int bottom_limit =
      std::min(int(std::floor(fixture.Physical(fixture.ViewportBottom()))) - 2,
               int(screen.extent.height) - 1);
  for (int step = 0; step < 60; ++step) {
    // Tìm đoạn pixel liên tiếp có màu toggle bật dài nhất trên cột toggle.
    int best_start = -1, best_length = 0;
    for (int y = top_limit; y <= bottom_limit;) {
      if (!ToggleOn(screen.At(toggle_column, y))) { ++y; continue; }
      const int start = y;
      while (y <= bottom_limit && ToggleOn(screen.At(toggle_column, y))) ++y;
      if (y - start > best_length) { best_length = y - start; best_start = start; }
    }
    const bool toggle_whole = best_length >= int(kToggleHeight * unit) - 3 &&
                              best_start > top_limit &&
                              best_start + best_length < bottom_limit;
    if (toggle_whole) {
      const float toggle_center = best_start + best_length / 2.0f;
      const float row_center = toggle_center + probe.row_offset * kRowHeight * unit;
      const float row_top = row_center - kRowHeight * unit / 2;
      const float row_bottom = row_center + kRowHeight * unit / 2;
      if (row_top > top_limit && row_bottom < bottom_limit) {
        // Quét trong lòng hàng, chừa mép trên (vạch chia của hàng trước) và mép dưới.
        const int scan_top = int(std::ceil(row_top + unit));
        const int scan_bottom = int(std::floor(row_bottom - 2 * unit));
        int ink_top = -1, ink_bottom = -1;
        for (int y = scan_top; y <= scan_bottom; ++y)
          if (Ink(screen.At(button_column, y))) { ink_top = y; break; }
        for (int y = scan_bottom; y >= scan_top; --y)
          if (Ink(screen.At(button_column, y))) { ink_bottom = y; break; }
        const float button_height = float(ink_bottom - ink_top + 1);
        const float gap_top = ink_top - row_top;
        const float gap_bottom = row_bottom - (ink_bottom + 1);
        const std::string where = label + " " + probe.name;
        if (!Check(ink_top >= 0 && button_height >= 20.0f * unit,
                   where + ": the row button must sit entirely inside its row (height " +
                       std::to_string(button_height / unit) + " of a 36px row)")) return false;
        if (!Check(std::fabs(gap_top - gap_bottom) <= 1.5f * unit + 1.0f,
                   where + ": the row button must be vertically centered (top gap " +
                       std::to_string(gap_top / unit) + ", bottom gap " +
                       std::to_string(gap_bottom / unit) + ")")) return false;
        if (!Check(std::fabs(button_height / unit - 28.0f) <= 2.5f,
                   where + ": the row button must match the 28px settings controls (got " +
                       std::to_string(button_height / unit) + ")")) return false;
        return true;
      }
    }
    fixture.Scroll(-24.0f, 4);
  }
  WriteFailureFrame(screen, label + " " + probe.name);
  return Check(false, label + " " + probe.name + ": the probed row never became visible");
}

const std::vector<RowProbe>& RowProbes() {
  static const std::vector<RowProbe> probes = {
      {"self-hosted Edit", 0, 60.0f,
       [](crossdesk::ui::MainWindow& main) { main.set_self_hosted_enabled(true); }},
      {"trusted devices Manage", 1, kToggleInset,
       [](crossdesk::ui::MainWindow& main) { main.set_trusted_devices_only(true); }},
      {"file save path", 1, kToggleInset,
       [](crossdesk::ui::MainWindow& main) { main.set_save_remote_previews(true); }},
  };
  return probes;
}

bool WritePpm(const ScaledWindow& screen, const std::filesystem::path& path) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  output << "P6\n" << screen.extent.width << ' ' << screen.extent.height << "\n255\n";
  for (const auto& pixel : screen.pixels) {
    output.put(static_cast<char>(pixel.r));
    output.put(static_cast<char>(pixel.g));
    output.put(static_cast<char>(pixel.b));
  }
  return Check(output.good(), "Settings evidence must be written completely: " + path.string());
}

struct EvidencePage {
  const char* name;
  std::function<void(Fixture&)> open;
};

// Các trang Settings và hộp thoại mở được từ Settings hoặc menu ứng dụng.
const std::vector<EvidencePage>& EvidencePages() {
  using Status = crossdesk::ui::AutostartStatus;
  static const std::vector<EvidencePage> pages = {
      {"settings-top", [](Fixture& f) { f.main().set_settings_open(true); }},
      {"settings-network", [](Fixture& f) {
         f.main().set_self_hosted_enabled(true);
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop(); f.Scroll(-200.0f);
       }},
      {"settings-security", [](Fixture& f) {
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop(); f.Scroll(-400.0f);
       }},
      {"settings-bottom", [](Fixture& f) {
         f.main().set_settings_open(true); f.screen().Settle();
         f.Scroll(-5000.0f);
       }},
      {"settings-all-rows-security", [](Fixture& f) {
         f.main().set_privacy_black_screen_available(true);
         f.main().set_privacy_setting_available(true);
         f.main().set_lock_on_disconnect_available(true);
         f.main().set_mac_permission_settings_visible(true);
         f.main().set_portable_service_settings_visible(true);
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop(); f.Scroll(-420.0f);
       }},
      {"settings-all-rows-bottom", [](Fixture& f) {
         f.main().set_privacy_black_screen_available(true);
         f.main().set_mac_permission_settings_visible(true);
         f.main().set_portable_service_settings_visible(true);
         f.main().set_portable_service_needs_repair(true);
         f.main().set_service_autostart_platform(true);
         f.main().set_autostart_system_managed(true);
         f.main().set_autostart_status(Status::Enabled);
         f.main().set_settings_open(true); f.screen().Settle();
         f.Scroll(-5000.0f);
       }},
      {"settings-session-active", [](Fixture& f) {
         f.main().set_settings_session_active(true);
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop(); f.Scroll(-200.0f);
       }},
      {"settings-save-error", [](Fixture& f) {
         f.main().set_settings_save_error(
             "The settings could not be saved because the configuration file is read only.");
         f.main().set_settings_open(true);
       }},
      {"settings-toggle-hint", [](Fixture& f) {
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop();
         // Hàng "low-load encoding" nằm ở nhóm thứ hai.
         f.MovePointer(f.RowLeft() + 120.0f, f.ViewportTop() + 74.0f + 24.0f + 36.0f + 18.0f);
       }},
      {"settings-language-dropdown", [](Fixture& f) {
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop();
         f.Click(f.DialogX() + 300.0f, f.DialogY() + 150.0f);
       }},
      {"settings-codec-dropdown", [](Fixture& f) {
         f.main().set_settings_open(true); f.screen().Settle();
         f.ScrollToTop();
         f.Click(f.DialogX() + 310.0f, f.DialogY() + 224.0f);
       }},
      {"settings-deskhub", [](Fixture& f) {
         // Mở Settings đưa tab về General, nên chỉ chuyển tab sau khi hộp thoại đã hiện.
         f.main().set_settings_open(true); f.screen().Settle();
         f.main().set_settings_tab(1);
       }},
      {"self-host-dialog", [](Fixture& f) {
         f.main().set_settings_open(true);
         f.main().set_self_host_settings_open(true);
       }},
      {"self-host-dialog-error", [](Fixture& f) {
         f.main().set_settings_open(true);
         f.main().set_self_host_settings_open(true);
         f.main().set_self_host_settings_error(
             "Enter a valid server host and a port between 1 and 65535 before saving.");
       }},
      {"trusted-devices", [](Fixture& f) {
         f.main().set_settings_open(true);
         f.main().set_trusted_devices_open(true);
       }},
      {"autostart-guide", [](Fixture& f) {
         f.main().set_autostart_system_managed(true);
         f.main().set_autostart_status(Status::Disabled);
         f.main().set_settings_open(true);
         f.main().set_autostart_guide_open(true);
       }},
      {"permission-dialog", [](Fixture& f) { f.main().set_permission_dialog_open(true); }},
      {"service-dialog", [](Fixture& f) { f.main().set_portable_service_dialog_open(true); }},
      {"feedback-log-export", [](Fixture& f) { f.main().set_feedback_open(true); }},
      {"about", [](Fixture& f) { f.main().set_about_open(true); }},
  };
  return pages;
}

bool WriteEvidence(ScaledPlatform& platform, const std::filesystem::path& root,
                   const DisplayCase& display, const LanguageCase& language) {
  for (const auto& page : EvidencePages()) {
    // Mỗi trang dùng một cửa sổ mới để trạng thái của trang trước không lọt sang.
    Fixture fixture(platform, display, language.index);
    page.open(fixture);
    fixture.screen().Settle(24);
    if (!WritePpm(fixture.screen(),
                  root / display.name / language.name / (std::string(page.name) + ".ppm")))
      return false;
  }
  return true;
}

}  // namespace

int main() {
  auto owned = std::make_unique<ScaledPlatform>();
  auto* platform = owned.get();
  slint::platform::set_platform(std::move(owned));

  // Tên cấu hình gõ sai thì danh sách rỗng: báo lỗi thay vì "đạt" mà không kiểm gì.
  if (DisplayCases().empty()) {
    std::cerr << "Unknown CROSSDESK_SETTINGS_DISPLAY; expected one of:";
    for (const auto& display : AllDisplayCases()) std::cerr << ' ' << display.name;
    std::cerr << '\n';
    return 1;
  }

  bool ok = true;
  for (const auto& display : DisplayCases()) {
    for (const auto& language : LayoutLanguages()) {
      for (const auto& probe : RowProbes()) {
        Fixture fixture(*platform, display, language.index);
        ok &= VerifyRowButton(fixture, probe,
                              std::string(display.name) + "/" + language.name);
      }
    }
  }
  if (ok) std::cout << "Settings row buttons fit and center at every scale and language.\n";

  if (const char* directory = std::getenv("CROSSDESK_SETTINGS_EVIDENCE_DIR");
      directory && *directory) {
    const std::filesystem::path root(directory);
    for (const auto& display : DisplayCases())
      for (const auto& language : LayoutLanguages())
        ok &= WriteEvidence(*platform, root, display, language);
    // Mọi ngôn ngữ ở cấu hình mặc định (100%) để soát chữ dài hay bị cắt, kể cả khi
    // CROSSDESK_SETTINGS_DISPLAY chọn cấu hình khác cho phần trên.
    for (const auto& language : AllLanguages())
      ok &= WriteEvidence(*platform, root / "all-languages", AllDisplayCases().front(), language);
  }
  return ok ? 0 : 1;
}
