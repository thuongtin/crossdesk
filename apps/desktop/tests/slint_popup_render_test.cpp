#include "crossdesk_ui.h"
#include "../src/gui/assets/fonts/fa_solid_900.h"
#include "slint_render_test_platform.h"
#include "ui/installed_release_notes_text.h"
#include "ui/model_update.h"
#include "ui/release_notes_text.h"
#include "ui/ui_localization.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace std::chrono_literals;

namespace {
bool Check(bool value, const char* message) {
  if (!value) std::cerr << message << '\n';
  return value;
}

template<typename Component>
void RegisterFixtureIcons(const Component& component) {
  if (component->window().window_handle().register_font_from_data(
          fa_solid_900_ttf, fa_solid_900_ttf_len).has_value()) {
    std::cerr << "Fixture FontAwesome registration failed\n";
    std::exit(8);
  }
}

auto CreateMainWindow() {
  auto main = crossdesk::ui::MainWindow::create();
  RegisterFixtureIcons(main);
  // Text and language options now come from the application. Load them before
  // measuring or clicking layouts whose geometry depends on their labels.
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 0);
  main->set_language_index(0);
  return main;
}

bool VerifyStreamPopups(RenderTestPlatform& platform) {
  auto stream = crossdesk::ui::StreamWindow::create();
  RegisterFixtureIcons(stream);
  auto& screen = *platform.adapter;
  stream->window().set_size(slint::LogicalSize({1280, 720}));
  stream->set_video_settings_enabled(true);
  // Exercise the Windows toolbar, including shortcuts and privacy controls.
  stream->set_secure_attention_supported(true);
  stream->set_privacy_visible(true);
  stream->show();
  const int top = int(stream->get_video_area_top());
  auto displays = std::make_shared<slint::VectorModel<slint::SharedString>>();
  auto transfers = std::make_shared<slint::VectorModel<crossdesk::ui::FileTransferEntry>>();
  stream->set_displays(displays);
  stream->set_file_transfers(transfers);
  bool file_visible = false;
  unsigned ticks = 0;
  slint::Timer sync;
  sync.start(slint::TimerMode::Repeated, 16ms, [&] {
    // Match the live session's model synchronization, including long clicks.
    crossdesk::ui::UpdateModelRows(*displays,
        std::vector<slint::SharedString>{"Display 1", "Display 2", "Display 3"});
    crossdesk::ui::FileTransferEntry entry{};
    entry.name = "test.txt"; entry.progress = (++ticks % 100) / 100.f;
    crossdesk::ui::UpdateModelRows(*transfers, std::vector(8, entry));
    stream->set_file_transfer_visible(file_visible);
  });
  int remote_presses = 0, remote_releases = 0, remote_scrolls = 0;
  stream->on_pointer_input([&](auto, auto kind, float, float) {
    if (kind == slint::PointerEventKind::Down) ++remote_presses;
    if (kind == slint::PointerEventKind::Up) ++remote_releases;
  });
  stream->on_scroll_input([&](float, float, float, float) { ++remote_scrolls; });
  int selected_display = -1;
  stream->on_switch_display([&](int index) { selected_display = index; });
  std::string shortcut;
  stream->on_send_shortcut([&](slint::SharedString text) { shortcut = text; });
  int video_changes = 0;
  stream->on_change_video_settings([&](int quality, int rate, int policy) {
    stream->set_video_quality_index(quality);
    stream->set_frame_rate_index(rate);
    stream->set_adaptation_policy_index(policy);
    ++video_changes;
  });
  stream->on_select_file([&] { file_visible = true; });
  bool close_synchronously = false;
  stream->on_close_file_transfer([&] {
    // The real application hides this on its next synchronization tick.
    file_visible = false;
    if (close_synchronously) stream->set_file_transfer_visible(false);
  });
  screen.Settle();
  // New sessions start with only the logo. Reveal the toolbar through its
  // normal gesture before testing the buttons to the right of the logo.
  screen.Click(19, top + 19);
  constexpr int display_x = 43, shortcut_x = 75, file_x = 203, video_x = 235;
  for (int repeat = 0; repeat < 2; ++repeat) {
    screen.Click(display_x + 14, top + 19);
    if (!Check(screen.Light(display_x + 5, top + 128), "Display menu must fully open without pointer movement")) return false;
    screen.Click(display_x + 53, top + 94, 4);
    if (!Check(selected_display == 1 && screen.Black(display_x + 5, top + 128),
               "Display selection must survive live refresh and close the menu")) return false;
    screen.Click(shortcut_x + 14, top + 19);
    if (!Check(screen.Light(shortcut_x + 5, top + 102), "Shortcut menu must open without pointer movement")) return false;
    screen.Click(shortcut_x + 31, top + 94, 4);
    if (!Check(shortcut == "Win+L" && screen.Black(shortcut_x + 5, top + 102),
               "Shortcut selection must close the menu immediately")) return false;
    screen.Click(video_x + 14, top + 19);
    if (!Check(stream->get_video_menu_open() && screen.Light(video_x + 4, top + 210),
               "Video settings menu must fully open without pointer movement")) return false;
    screen.Click(video_x + 41, top + 80, 4);
    screen.Click(900, top + 300);
    if (!Check(!stream->get_video_menu_open() && video_changes == repeat + 1,
               "Video selection and outside dismissal must respond immediately")) return false;
  }
  if (!Check(remote_presses == 0 && remote_releases == 0 && remote_scrolls == 0,
             "Menu gestures must not be forwarded to the peer")) return false;
  screen.Click(file_x + 14, top + 19);
  const int panel_y = top + int(stream->get_video_area_height()) - 210;
  if (!Check(file_visible && screen.Light(20, panel_y + 25),
             "File progress must appear without pointer movement")) return false;
  // Header/padding is outside the list's own input handler.
  screen.Click(25, panel_y + 20, 4);
  stream->window().dispatch_pointer_scroll_event(
      slint::LogicalPosition({25, float(panel_y + 20)}), 0, -60);
  screen.Settle();
  if (!Check(remote_presses == 0 && remote_releases == 0 && remote_scrolls == 0,
             "File progress header and padding must consume input locally")) return false;
  const slint::LogicalPosition panel_point({25, float(panel_y + 20)});
  const slint::LogicalPosition outside({900, float(top + 300)});
  stream->window().dispatch_pointer_press_event(panel_point, slint::PointerEventButton::Left);
  screen.Settle(4);
  stream->window().dispatch_pointer_move_event(outside);
  stream->window().dispatch_pointer_release_event(outside, slint::PointerEventButton::Left);
  screen.Settle();
  if (!Check(remote_presses == 0 && remote_releases == 0,
             "Dragging out of the file panel must not release a button on the peer")) return false;
  screen.Click(430, panel_y + 16, 4);
  if (!Check(!file_visible && screen.Black(20, panel_y + 25),
             "File progress close must clear its pixels without pointer movement")) return false;
  if (!Check(remote_presses == 0 && remote_releases == 0,
             "Closing file progress must not send an orphan pointer release")) return false;
  close_synchronously = true;
  screen.Click(file_x + 14, top + 19);
  screen.Click(430, panel_y + 16, 4);
  if (!Check(!file_visible && screen.Black(20, panel_y + 25) && remote_releases == 0,
             "Synchronous file panel dismissal must consume its closing release")) return false;
  screen.Click(900, top + 300);
  return Check(remote_presses == 1 && remote_releases == 1,
               "Closing local menus must restore remote input");
}

// Export the retained framebuffer, not take_snapshot(), so evidence cannot
// conceal a missing redraw. A caller-owned directory keeps artifacts disposable.
bool WritePopupEvidence(const RenderTestWindow& screen, const std::string& name) {
  const char* directory = std::getenv("CROSSDESK_POPUP_EVIDENCE_DIR");
  if (!directory || !*directory) return true;
  const auto path = std::filesystem::path(directory) / (name + ".ppm");
  std::filesystem::create_directories(path.parent_path());
  std::ofstream output(path, std::ios::binary);
  output << "P6\n" << screen.extent.width << ' ' << screen.extent.height << "\n255\n";
  for (const auto& pixel : screen.pixels) {
    output.put(static_cast<char>(pixel.r));
    output.put(static_cast<char>(pixel.g));
    output.put(static_cast<char>(pixel.b));
  }
  return Check(output.good(), "Retained popup framebuffer must be saved completely");
}

bool WriteEnglishEvidence(RenderTestWindow& screen,
                          const slint::ComponentHandle<crossdesk::ui::MainWindow>& main,
                          const std::string& name) {
  if (!std::getenv("CROSSDESK_POPUP_EVIDENCE_DIR")) return true;
  const int previous_language = main->get_language_index();
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 1);
  main->set_language_index(1);
  screen.Settle();
  const bool written = WritePopupEvidence(screen, name);
  // Existing interaction fixtures deliberately exercise Chinese geometry.
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 0);
  main->set_language_index(previous_language);
  screen.Settle();
  return written;
}

void ConfigureEvidenceData(const slint::ComponentHandle<crossdesk::ui::MainWindow>& main) {
  main->set_local_id("123 456 789");
  main->set_local_password("A1b2C3");
  main->set_password_visible(false);
  main->set_signal_connected(true);
  std::vector<crossdesk::ui::RecentConnection> recent;
  for (const auto& [id, name] : std::vector<std::pair<const char*, const char*>>{
           {"987654321", "Studio fixture"}, {"246813579", "Office fixture"}}) {
    crossdesk::ui::RecentConnection row{};
    row.remote_id = id; row.display_name = name; row.host_name = name;
    row.online = id[0] == '9';
    recent.push_back(row);
  }
  main->set_recent_connections(
      std::make_shared<slint::VectorModel<crossdesk::ui::RecentConnection>>(recent));
}

bool VerifyPasswordDialog(RenderTestPlatform& platform) {
  for (int language : {0, 1, crossdesk::localization::kVietnameseLanguage}) {
    auto main = CreateMainWindow();
    ConfigureEvidenceData(main);
    crossdesk::ui_localization::ApplyMainWindowStrings(main, language);
    main->set_language_index(language);
    auto& screen = *platform.adapter;
    const float width = main->get_minimum_window_width();
    const float height = main->get_minimum_window_height();
    main->window().set_size(slint::LogicalSize({width, height}));
    main->show();
    screen.Settle();
    if (!WritePopupEvidence(screen, "main-" + std::to_string(language))) return false;
    int saves = 0;
    bool accept = false;
    slint::SharedString submitted;
    main->on_reset_password([&](slint::SharedString text) {
      ++saves;
      submitted = text;
      return accept;
    });
    main->set_reset_password_open(true);
    screen.Settle();
    const auto geometry = [&] {
      const float x = main->get_reset_password_dialog_x();
      const float y = main->get_reset_password_dialog_y();
      const float w = main->get_reset_password_dialog_width();
      const float h = main->get_reset_password_dialog_height();
      const float input_x = main->get_reset_password_input_x();
      const float input_y = main->get_reset_password_input_y();
      const float input_w = main->get_reset_password_input_width();
      const float input_h = main->get_reset_password_input_height();
      return Check(x >= 0 && y >= 0 && x + w <= width && y + h <= height &&
                       std::abs(input_x - x - 24) < 1 &&
                       std::abs(input_w - w + 48) < 1 && input_h >= 38 &&
                       input_y > y && input_y + input_h < y + h &&
                       main->get_reset_password_save_y() >= input_y + input_h &&
                       main->get_reset_password_cancel_y() >= input_y + input_h,
                   "Password field and actions must fit the dialog with full padding");
    };
    if (!geometry()) return false;
    const std::string prefix = "password-" + std::to_string(language);
    if (!WritePopupEvidence(screen, prefix + "-empty")) return false;
    const auto input_pixels = [&] {
      return screen.Region(int(main->get_reset_password_input_x()) + 4,
                           int(main->get_reset_password_input_y()) + 4,
                           int(main->get_reset_password_input_width()) - 8,
                           int(main->get_reset_password_input_height()) - 8);
    };
    const auto empty_input_pixels = input_pixels();
    // Real key delivery reaches the focused editor, not a test-only property write.
    for (const char* character : {"A", "1", "b", "2", "C", "3"}) screen.Key(character);
    if (!Check(main->get_new_password_input() == "A1b2C3",
               "Password editor must receive all six fixture characters")) return false;
    screen.Key("7");
    if (!Check(main->get_new_password_input() == "A1b2C3",
               "Password editor must reject a seventh character")) return false;
    if (!Check(input_pixels() != empty_input_pixels,
               "All six password characters must repaint the retained input buffer")) return false;
    if (!WritePopupEvidence(screen, prefix + "-full")) return false;
    auto save = [&] {
      screen.Click(main->get_reset_password_save_x() + 56,
                   main->get_reset_password_save_y() + 18, 4);
    };
    save();
    if (!Check(saves == 1 && submitted == "A1b2C3" &&
                   main->get_reset_password_open() && main->get_reset_password_invalid(),
               "Rejected password save must preserve the draft and show validation")) return false;
    if (!geometry() || !WritePopupEvidence(screen, prefix + "-invalid")) return false;
    int error_pixels = 0;
    const int error_top = int(main->get_reset_password_input_y() + main->get_reset_password_input_height()) + 2;
    for (int y = error_top; y < int(main->get_reset_password_save_y()) - 8; ++y) {
      for (int x = int(main->get_reset_password_input_x());
           x < int(main->get_reset_password_input_x() + main->get_reset_password_input_width()); ++x) {
        const auto pixel = screen.pixels.at(y * screen.extent.width + x);
        if (pixel.r > 100 && pixel.r > 2 * pixel.g && pixel.r > 1.5f * pixel.b) ++error_pixels;
      }
    }
    if (!Check(error_pixels > 10,
               "Rejected password must visibly paint validation text below the input")) return false;
    main->set_password_change_pending(true);
    save();
    if (!Check(saves == 1 && main->get_reset_password_open(),
               "Pending password change must block repeated saves")) return false;
    main->set_password_change_pending(false);
    accept = true;
    save();
    if (!Check(saves == 2 && submitted == "A1b2C3" &&
                   !main->get_reset_password_open() &&
                   main->get_new_password_input() == "" && !main->get_reset_password_invalid(),
               "Accepted password save must close and clear the draft and validation")) return false;
    main->set_reset_password_open(true);
    screen.Settle();
    screen.Key("9");
    screen.Click(main->get_reset_password_cancel_x() + 56,
                 main->get_reset_password_cancel_y() + 18, 4);
    if (!Check(saves == 2 && !main->get_reset_password_open() &&
                   main->get_new_password_input() == "" && !main->get_reset_password_invalid(),
               "Password Cancel must clear its draft without calling save")) return false;
    if (!WritePopupEvidence(screen, prefix + "-closed")) return false;
  }
  std::cout << "Password popup EN/ZH/VI geometry, editing, validation, pending, Save and Cancel passed.\n";
  return true;
}

bool VerifyWorkbenchPopups(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  ConfigureEvidenceData(main);
  auto& screen = *platform.adapter;
  const float width = main->get_minimum_window_width();
  const float height = main->get_minimum_window_height();
  main->window().set_size(slint::LogicalSize({width, height}));
  main->show();
  auto groups = std::make_shared<slint::VectorModel<slint::SharedString>>(
      std::vector<slint::SharedString>{"All", "Ungrouped", "Lab"});
  main->set_recent_groups(groups);
  int deskhub_requests = 0, release_requests = 0, message_refreshes = 0;
  main->on_open_gallery([&] { ++deskhub_requests; });
  main->on_gallery_action([&](slint::SharedString action, slint::SharedString) {
    if (action == "release") ++release_requests;
  });
  main->on_refresh_announcements([&] { ++message_refreshes; });
  int selected_group = -1;
  main->on_recent_group_selected([&](int index) { selected_group = index; });
  screen.Settle();
  if (!WriteEnglishEvidence(screen, main, "main-postcard")) return false;
  // DeskHub is a separate-window callback, not an in-place gallery-mode toggle.
  screen.Click(main->get_ip_direct_enabled() ? 262 : 70, 28, 4);
  if (!Check(deskhub_requests == 1 && !main->get_gallery_mode(),
             "DeskHub entry must dispatch exactly once without replacing the main window")) return false;
  // Match recent-panel (x=8, width=window-380), its 155px filter,
  // and the shared picker (38px trigger, 4px gap, 36px rows, 6px divider).
  const float filter_x = 8 + (width - 380) - 155 - 20;
  constexpr float filter_y = 322, filter_height = 38;
  const float popup_y_filter = filter_y + filter_height + 4;
  const auto filter_pixels = [&] { return screen.Region(int(filter_x) + 4, int(popup_y_filter) + 4, 140, 70); };
  const auto filter_closed = filter_pixels();
  screen.Click(filter_x + 77, filter_y + filter_height / 2);
  if (!Check(filter_pixels() != filter_closed,
             "Group filter must reveal its contents while the pointer stays on the trigger")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(filter_pixels() == filter_closed,
             "Escape must dismiss the group filter")) return false;
  screen.Click(filter_x + 77, filter_y + filter_height / 2);
  screen.Click(filter_x + 77, popup_y_filter + 4 + 2 * 36 + 6 + 18, 4);
  if (!Check(selected_group == 2 && main->get_recent_group_index() == 2,
             "Group selection must survive a held click")) return false;
  const auto open_menu = [&] {
    const int before = release_requests;
    screen.Click(width - 46, 28);
    return Check(release_requests > before,
                 "Every application menu opening must release remote input");
  };
  if (!open_menu() || !WriteEnglishEvidence(screen, main, "main-menu")) return false;
  screen.Click(width - 100, 87, 4);
  if (!Check(main->get_settings_open(), "Application menu must open settings on click")) return false;
  const float settings_y = (height - 446) / 2;
  screen.Click(width / 2 + 87, settings_y + 76, 4);
  if (!Check(main->get_settings_tab() == 1, "Settings tabs must switch without pointer movement")) return false;
  screen.Click(width / 2 - 87, settings_y + 76, 4);
  if (!Check(main->get_settings_tab() == 0, "Settings tabs must switch back immediately")) return false;
  // The first row uses the same CompactComboBox as codec/capture settings.
  const int combo_x = int(width / 2 + 100);
  const int combo_y = int(settings_y + 150);
  const int popup_y = int(settings_y + 164);
  const auto combo_closed = screen.Region(combo_x - 35, popup_y + 2, 60, 60);
  screen.Click(combo_x, combo_y);
  if (!Check(screen.Region(combo_x - 35, popup_y + 2, 60, 60) != combo_closed,
             "Settings dropdown must animate open without pointer movement")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(screen.Region(combo_x - 35, popup_y + 2, 60, 60) == combo_closed,
             "Escape must dismiss the settings dropdown")) return false;
  screen.Click(combo_x, combo_y);
  screen.Click(combo_x, popup_y + 34, 4);
  if (!Check(main->get_language_index() == 1, "Settings dropdown must apply a selected row")) return false;
  // A session starting while a codec menu is open disables the setting.
  // Existing popup rows must not remain actionable after that transition.
  const int codec_y = int(settings_y + 224);
  screen.Click(combo_x, codec_y);
  main->set_settings_session_active(true);
  screen.Settle();
  screen.Click(combo_x, codec_y + 48, 4);
  if (!Check(main->get_codec_index() == 0,
             "A disabled codec dropdown must close and reject its stale options")) return false;
  main->set_settings_session_active(false);
  screen.Click(combo_x, codec_y);
  screen.Click(combo_x, codec_y + 79, 4);
  if (!Check(main->get_codec_index() == 2, "The codec dropdown must offer H.265")) return false;

  const bool low_load_before = main->get_desktop_realtime_rate_control();
  const bool pause_before = main->get_pause_video_when_minimized();
  screen.Click(width / 2 + 144, codec_y + 36, 4);
  if (!Check(main->get_desktop_realtime_rate_control() != low_load_before &&
                 main->get_pause_video_when_minimized() == pause_before,
             "Low-load encoding must toggle independently of video pause")) return false;
  if (!WriteEnglishEvidence(screen, main, "settings-h265-low-load")) return false;
  main->set_codec_index(0);
  main->set_desktop_realtime_rate_control(low_load_before);
  if (!WriteEnglishEvidence(screen, main, "settings")) return false;
  screen.Click(width / 2 + 32, settings_y + 412);
  if (!Check(!main->get_settings_open(), "Settings Cancel must close the dialog immediately")) return false;
  if (!open_menu()) return false;
  screen.Click(width - 100, 144, 4);
  if (!Check(main->get_messages_open() && message_refreshes == 1, "Messages must open directly from the application menu")) return false;
  if (!WriteEnglishEvidence(screen, main, "messages")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(!main->get_messages_open(), "Escape must dismiss Messages")) return false;
  if (!open_menu()) return false;
  screen.Click(width - 100, 201, 4);
  if (!Check(main->get_about_open(), "About must open directly from the application menu")) return false;
  if (!WriteEnglishEvidence(screen, main, "about-no-sponsor")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(!main->get_about_open(), "Escape must dismiss About")) return false;
  if (!open_menu()) return false;
  screen.Click(width - 100, 258, 4);
  if (!Check(main->get_feedback_open(), "Feedback must open directly from the application menu")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(!main->get_feedback_open(), "Escape must dismiss Feedback")) return false;
  if (!open_menu()) return false;
  screen.Key(slint::platform::key_codes::DownArrow);
  screen.Key(slint::platform::key_codes::Return);
  if (!Check(main->get_messages_open() && message_refreshes == 2,
             "Menu Down and Return must activate Messages and refresh once")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!open_menu()) return false;
  screen.Key(slint::platform::key_codes::Escape);
  screen.Key(slint::platform::key_codes::Return);
  return Check(!main->get_settings_open() && !main->get_messages_open() &&
                   !main->get_about_open() && !main->get_feedback_open() && message_refreshes == 2,
               "Menu Escape must close it and prevent Return from activating a stale row");
}

bool VerifyPostcardIpControls(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  ConfigureEvidenceData(main);
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 1);
  main->set_language_index(1);
  main->set_ip_direct_enabled(true);
  main->set_lan_mode(true);
  main->set_lan_listening(true);
  main->set_lan_listener_ip("192.0.2.24");
  main->set_lan_listener_port("52888");
  const std::vector<slint::SharedString> addresses{
      "192.0.2.24", "198.51.100.6", "[2001:db8:abcd:1234:9876:1234:1234:9abc]",
      "[2001:db8:1::1]", "[2001:db8:2::1]", "[2001:db8:3::1]",
      "[2001:db8:4::1]", "[2001:db8:5::1]"};
  main->set_lan_interfaces(
      std::make_shared<slint::VectorModel<slint::SharedString>>(addresses));
  auto& screen = *platform.adapter;
  const float width = main->get_minimum_window_width();
  main->window().set_size(slint::LogicalSize({width, main->get_minimum_window_height()}));
  main->show();
  screen.Settle();
  if (!WritePopupEvidence(screen, "postcard-ip")) return false;
  // The IP picker aligns with the right edge of the address field and opens
  // upward, with six visible 36px rows before scrolling.
  const int popup_x = int(width) - 44 - 450;
  constexpr int popup_y = 322 + 101 + 2 - 224 - 44;
  const auto popup_pixels = [&] { return screen.Region(popup_x + 2, popup_y + 2, 442, 216); };
  const auto closed = popup_pixels();
  int configured = 0, selected = 0, connects = 0;
  std::string picked;
  std::string port;
  main->on_lan_configure([&](slint::SharedString ip, slint::SharedString value) {
    ++configured;
    port = std::string(value);
    return ip == "192.0.2.24";
  });
  main->on_lan_select_address([&](slint::SharedString address) {
    ++selected;
    picked = std::string(address);
    return true;
  });
  main->on_lan_prepare_connect([&](slint::SharedString address) {
    if (address == "203.0.113.8:53210") ++connects;
    return slint::SharedString();
  });
  screen.Click(width - 176, 402);
  if (!Check(popup_pixels() != closed, "Switch IP must open a visible upward popup")) return false;
  const auto swatch = screen.pixels.at((popup_y + 19) * screen.extent.width + popup_x + 350);
  if (!Check(swatch.r > swatch.b && swatch.r >= 210,
             "Selected IP must have a warm Postcard fill")) return false;
  if (!WritePopupEvidence(screen, "postcard-ip-dropdown")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(selected == 0 && popup_pixels() == closed, "Dismiss must not select or leave popup pixels")) return false;
  screen.Click(width - 176, 402);
  screen.Click(popup_x + 124, popup_y + 4 + 2 * 36 + 18);
  if (!Check(picked == std::string(addresses[2]) && selected == 1 && configured == 0 &&
                 main->get_lan_interface_index() == 2,
             "Selecting IPv6 must preserve the full address without reconfiguring the listener")) return false;
  screen.Click(width - 82, 402);
  const std::string new_port = "53210";
  for (char key : new_port) screen.Key(slint::SharedString(std::string(1, key)));
  screen.Key(slint::platform::key_codes::Return);
  if (!Check(configured == 1 && port == new_port, "Larger edit-port button must focus, replace and save once")) return false;
  screen.Click(380, 226);
  const std::string endpoint = "203.0.113.8:53210";
  for (char key : endpoint) screen.Key(slint::SharedString(std::string(1, key)));
  screen.Click(width - 100, 226);
  if (!Check(connects == 1 && main->get_lan_open(),
             "Expanded remote editor and Connect must submit the exact endpoint once")) return false;
  std::cout << "Postcard IP picker, Escape, IPv6 selection, local port edit and remote endpoint passed.\n";
  return true;
}

bool VerifySquareRecentPreview(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  ConfigureEvidenceData(main);
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 1);
  main->set_language_index(1);
  main->set_ip_direct_enabled(true);
  auto& screen = *platform.adapter;
  main->window().set_size(slint::LogicalSize(
      {main->get_minimum_window_width(), main->get_minimum_window_height()}));
  std::vector<crossdesk::ui::RecentConnection> fixtures;
  const auto make_preview = [](int width, int height) {
    slint::SharedPixelBuffer<slint::Rgb8Pixel> buffer(width, height);
    for (int y = 0; y < height; ++y)
      for (int x = 0; x < width; ++x)
        buffer.begin()[y * width + x] =
            y < height / 5 ? slint::Rgb8Pixel{32, 163, 186} :
            y >= height * 4 / 5 ? slint::Rgb8Pixel{231, 168, 43} :
            x < width / 2 ? slint::Rgb8Pixel{46, 65, 96} :
                            slint::Rgb8Pixel{73, 99, 137};
    return slint::Image(std::move(buffer));
  };
  for (int i = 0; i < 4; ++i) {
    crossdesk::ui::RecentConnection connection{};
    connection.remote_id = slint::SharedString(std::to_string(123456780 + i));
    connection.display_name = i == 0 ? "Studio fixture / 16:9" :
                              i == 1 ? "Office fixture / 4:3" : "Scroll fixture";
    connection.online = true;
    connection.has_thumbnail = true;
    connection.thumbnail = i == 1 ? make_preview(160, 120) : make_preview(160, 90);
    fixtures.push_back(std::move(connection));
  }
  main->set_recent_connections(
      std::make_shared<slint::VectorModel<crossdesk::ui::RecentConnection>>(fixtures));
  main->show();
  screen.Settle();
  const float width = main->get_recent_preview_width();
  const float height = main->get_recent_preview_height();
  if (!Check(std::abs(width * 9 - height * 16) < 0.02f && height > 120,
             "Recent viewport must be 16:9, not a fixed-height strip")) return false;
  const auto pixel = [&](int x, int y) { return screen.pixels.at(y * screen.extent.width + x); };
  const auto color_near = [&](int x, int y, int r, int g, int b) {
    const auto p = pixel(x, y);
    return std::abs(int(p.r) - r) <= 4 && std::abs(int(p.g) - g) <= 4 &&
           std::abs(int(p.b) - b) <= 4;
  };
  constexpr int card_x = 34, card_y = 377;
  const int second_x = card_x + int(std::round(width)) + 12;
  if (!Check(color_near(30, 11, 189, 65, 24),
             "Active ID mode must fill its square inset corner")) return false;
  if (!Check(color_near(card_x + 100, card_y + 8, 32, 163, 186) &&
                 color_near(card_x + 100, card_y + int(height) - 8, 231, 168, 43),
             "16:9 preview must show both source edges within its square card")) return false;
  if (!Check(color_near(second_x + 10, card_y + 72, 36, 34, 30) &&
                 color_near(second_x + int(width / 2), card_y + 8, 32, 163, 186) &&
                 color_near(second_x + int(width / 2), card_y + int(height) - 8, 231, 168, 43),
             "4:3 preview must show both source edges with a neutral side matte")) return false;
  if (!WritePopupEvidence(screen, "postcard-recent-16x9")) return false;
  int connects = 0;
  main->on_recent_connect([&](slint::SharedString id) {
    if (id == "123456780") ++connects;
  });
  const int footer_y = card_y + int(height) + 22;
  screen.Click(card_x + int(width) - 26, footer_y);
  if (!Check(connects == 1, "Reconnect button must follow the preview without clipping")) return false;
  screen.Click(card_x + int(width) - 26, card_y + 26);
  screen.Key(slint::platform::key_codes::Return);
  if (!Check(main->get_alias_open() && connects == 1,
             "Recent overflow must open Rename without reconnecting")) return false;
  main->set_alias_open(false);
  screen.Click(157, 29);
  // Thanh chuyển chế độ nở theo nhãn dài nhất đo bằng font đang chạy (DejaVu Sans trên Linux
  // rộng hơn font Mac), nên nửa IP không luôn bắt đầu ở x 113. Tìm mép trái vùng tô ở hàng 15,
  // dưới góc và trên chữ, bắt đầu từ giữa nửa ID để tránh viền focus, rồi kiểm tra điểm sát góc
  // trên trái như trước: góc vuông thì điểm đó vẫn được tô.
  int ip_fill_left = -1;
  for (int x = 60; x < 28 + 400 && ip_fill_left < 0; ++x)
    if (color_near(x, 15, 189, 65, 24)) ip_fill_left = x;
  if (!Check(main->get_lan_mode() && ip_fill_left >= 28 + 85 &&
                 color_near(ip_fill_left + 1, 11, 189, 65, 24),
             "Active IP mode must also fill its square inset corner")) return false;
  return WritePopupEvidence(screen, "postcard-ip-square-selected");
}

bool VerifyAutostartSettings(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  auto& screen = *platform.adapter;
  using Status = crossdesk::ui::AutostartStatus;
  const float width = main->get_minimum_window_width();
  const float height = main->get_minimum_window_height();
  main->window().set_size(slint::LogicalSize({width, height}));
  main->set_autostart_system_managed(true);
  main->set_autostart_enabled(true); // Stale app preference must not label system state.
  main->set_autostart_status(Status::Disabled);
  int refreshes = 0;
  main->on_refresh_autostart_status([&] { ++refreshes; });
  main->show(); main->set_settings_open(true); screen.Settle();
  if (!Check(refreshes == 1, "Opening managed autostart settings must query the system")) return false;
  const float top = (height - 446) / 2;
  // Do fork sở hữu: cuộn bằng wheel đặt trên thanh cuộn (rộng 14px, ở x 382 đến
  // 396 của hộp thoại rộng 400px), không đặt trên nội dung. Wheel trên nội dung đi
  // qua hoạt cảnh giảm tốc của Flickable, và tuỳ nhịp khung hình có thể dừng lệch
  // biên một phần nhỏ của pixel. Khi đó vùng vẽ lại từng phần của SoftwareRenderer
  // cắt ngang dải khử răng cưa ở góc công tắc, nên ảnh chụp sau vài lần vẽ lại từng
  // phần khác ảnh sau một lần vẽ lại toàn bộ dù giao diện không đổi. Thanh cuộn của
  // style fluent kẹp thẳng giá trị về biên, nên vị trí cuộn không tuỳ nhịp khung hình.
  const auto scroll_settings_to_end = [&] {
    main->window().dispatch_pointer_scroll_event(
        slint::LogicalPosition({width / 2 + 189, top + 260}), 0, -1200);
    screen.Settle();
  };
  scroll_settings_to_end();
  const float toggle_x = width / 2 + 144, toggle_y = top + 322;
  const auto toggle_pixels = [&] {
    return screen.Region(int(toggle_x - 18), int(toggle_y - 10), 36, 20);
  };
  const auto save_snapshot = [&](const std::string& suffix) {
    if (const char* snapshot = std::getenv("CROSSDESK_AUTOSTART_UI_SNAPSHOT")) {
      std::ofstream out(std::string(snapshot) + suffix, std::ios::binary);
      out << "P6\n" << screen.extent.width << ' ' << screen.extent.height << "\n255\n";
      for (const auto pixel : screen.pixels) { out.put(pixel.r); out.put(pixel.g); out.put(pixel.b); }
    }
  };
  const auto disabled = toggle_pixels();
  save_snapshot(".settings.ppm");
  main->set_autostart_status(Status::Enabled); screen.Settle();
  const auto enabled = toggle_pixels();
  if (!Check(enabled != disabled,
             "The autostart switch must reflect system changes while settings stay open")) return false;
  main->set_autostart_status(Status::Disabled); screen.Settle();
  screen.Click(toggle_x, toggle_y);
  if (!Check(main->get_autostart_guide_open() && main->get_autostart_status() == Status::Disabled &&
                 main->get_autostart_enabled(),
             "Clicking the disabled system switch opens guidance without changing state or preference")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(toggle_pixels() == disabled, "The system switch must not flip when guidance closes")) return false;
  screen.Click(toggle_x, toggle_y);
  // Both actions stay below the scrollable instructions at minimum size.
  const auto button_y = [&]() {
    const int x = int(width / 2 + 120), y = int(height / 2 + 158);
    const auto p = screen.pixels.at(y * screen.extent.width + x);
    return p.b > 180 && p.r < 90 && p.g > 80 && p.g < 160 ? y : -1;
  };
  int refresh_y = button_y();
  if (!Check(refresh_y > 0, "Autostart guide buttons must fit inside the minimum window")) return false;
  save_snapshot("");
  screen.Click(width / 2 - 145, refresh_y);
  if (!Check(refreshes == 2, "Autostart guide Refresh must query the actual system")) return false;
  main->set_autostart_status(Status::Checking); screen.Settle();
  screen.Click(width / 2 - 145, button_y());
  if (!Check(refreshes == 2, "Refresh is disabled while the system query is pending")) return false;
  for (const auto status : {Status::Failed, Status::Unsupported, Status::Enabled}) {
    main->set_autostart_status(status); screen.Settle();
    if (!Check(button_y() > 0, "Error and unsupported states must remain readable and dismissible")) return false;
  }
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(!main->get_autostart_guide_open() && main->get_settings_open(),
             "Escape closes the guide without cancelling other settings")) return false;
  if (!Check(toggle_pixels() == enabled, "The switch must still follow system state after opening guidance")) return false;
  screen.Click(toggle_x, toggle_y);
  if (!Check(main->get_autostart_guide_open() && main->get_autostart_status() == Status::Enabled,
             "Clicking the enabled system switch must also open guidance without changing state")) return false;
  screen.Click(width / 2 + 160, button_y());
  if (!Check(!main->get_autostart_guide_open(), "The guide confirmation must close immediately")) return false;
  if (!Check(toggle_pixels() == enabled, "Dismissing guidance must keep the system switch enabled")) return false;
  main->set_autostart_guide_open(true); main->set_settings_open(false); screen.Settle();
  if (!Check(!main->get_autostart_guide_open(), "Closing settings must also dismiss its guide")) return false;
  main->set_autostart_system_managed(false);
  main->set_settings_open(true); screen.Settle();
  scroll_settings_to_end();
  screen.Click(toggle_x, toggle_y);
  return Check(refreshes == 2 && !main->get_autostart_enabled() && !main->get_autostart_guide_open(),
               "Other platforms toggle their autostart preference directly without querying Harmony or opening guidance");
}

bool VerifyGalleryPopups(RenderTestPlatform& platform) {
  auto wall = CreateMainWindow();
  auto& screen = *platform.adapter;
  wall->set_gallery_mode(true);
  wall->window().set_size(slint::LogicalSize({900, 600}));
  wall->set_gallery_count(8);
  wall->set_gallery_connected(8);
  // A deterministic CPU image gives DeskHub real tiles without remote sessions.
  slint::SharedPixelBuffer<slint::Rgb8Pixel> preview(160, 90);
  for (unsigned y = 0; y < 90; ++y) for (unsigned x = 0; x < 160; ++x)
    preview.begin()[y * 160 + x] = slint::Rgb8Pixel{
        static_cast<uint8_t>(30 + x), static_cast<uint8_t>(50 + y), 110};
  const slint::Image frame(std::move(preview));
  std::vector<crossdesk::ui::GalleryTile> tiles;
  for (int n = 0; n < 8; ++n) {
    crossdesk::ui::GalleryTile tile{};
    tile.id = slint::SharedString(std::to_string(123456780 + n));
    tile.name = slint::SharedString("Fixture workstation " + std::to_string(n + 1));
    tile.online = tile.presence_known = tile.connected = tile.has_frame = true;
    tile.frame = frame;
    tiles.push_back(tile);
  }
  wall->set_gallery_tiles(std::make_shared<slint::VectorModel<crossdesk::ui::GalleryTile>>(tiles));

  wall->set_gallery_groups(std::make_shared<slint::VectorModel<slint::SharedString>>(
      std::vector<slint::SharedString>{"All", "Lab"}));
  auto network = std::make_shared<slint::VectorModel<crossdesk::ui::GalleryNetworkDevice>>();
  wall->set_gallery_network_devices(network);
  wall->show();
  // Do fork sở hữu: nút "网络状态" nằm ngay sau chữ đếm kết nối, mà độ rộng chữ
  // này tùy font CJK của máy (rootfs build Linux không có font CJK nên chữ Hán
  // rộng 0). Lấy tâm nút từ mặt trắng ngay dưới viền trên của nó (chân trang ở
  // y 566 đến 590) thay vì hoành độ đo bằng font của macOS.
  const auto network_button_x = [&] {
    for (int x = 0, start = -1; x < 450; ++x) {
      const auto p = screen.pixels.at(568 * screen.extent.width + x);
      const bool face = p.r == 255 && p.g == 255 && p.b == 255;
      if (face && start < 0) start = x;
      if (!face && start >= 0) {
        if (x - start >= 48) return (start + x) / 2;
        start = -1;
      }
    }
    return -1;
  };
  unsigned ticks = 0;
  slint::Timer sync;
  sync.start(slint::TimerMode::Repeated, 16ms, [&] {
    crossdesk::ui::GalleryNetworkDevice row{};
    row.id = "123456789"; row.name = "Test device";
    row.inbound = slint::SharedString(std::to_string(++ticks) + " kbps");
    crossdesk::ui::UpdateModelRows(*network, std::vector(8, row));
  });
  screen.Settle();
  if (!WriteEnglishEvidence(screen, wall, "deskhub")) return false;
  const auto baseline = screen.Region(75, 100, 740, 400);
  const int network_x = network_button_x();
  if (!Check(network_x > 0, "DeskHub network button must be painted in the footer")) return false;
  for (int repeat = 0; repeat < 2; ++repeat) {
    screen.Click(network_x, 578);
    if (!Check(wall->get_gallery_network_open() && screen.Region(75, 100, 740, 400) != baseline,
               "DeskHub network dialog must paint immediately during live refresh")) return false;
    // Eight rows fill the available height, placing the dialog 16 px from the top.
    if (repeat == 0) screen.Click(780, 46, 4);
    else screen.Key(slint::platform::key_codes::Escape);
    if (!Check(!wall->get_gallery_network_open() && screen.Region(75, 100, 740, 400) == baseline,
               "DeskHub network dialog must close completely without pointer movement")) return false;
  }
  // Open the add menu, leave the pointer on it, then select its first row.
  const auto menu_closed = screen.Region(490, 42, 220, 60);
  screen.Click(680, 24);
  if (!Check(screen.Region(490, 42, 220, 60) != menu_closed,
             "DeskHub add menu must appear without pointer movement")) return false;
  screen.Click(600, 55, 4);
  if (!Check(wall->get_gallery_adding(), "Add menu must open the device dialog")) return false;
  screen.Click(422, 421, 4);
  return Check(!wall->get_gallery_adding(), "Add dialog Cancel must respond immediately");
}

bool VerifyUpdateRecovery(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  auto& screen = *platform.adapter;
  const float width = main->get_minimum_window_width();
  const float height = main->get_minimum_window_height();
  main->window().set_size(slint::LogicalSize({width, height}));
  main->set_update_available(true);
  main->set_update_install_available(true);
  main->set_latest_version("v2.0.0");
  main->set_current_version("1.7.6");
  // Do fork sở hữu: notes nhúng đi qua cùng đường dựng khối như app. Tiêu đề con phải thành
  // dòng in đậm, còn mọi dòng trong code fence ("# ...", "---", "- ...") phải giữ nguyên chữ
  // thay vì bị đọc thành tiêu đề, đường kẻ hay danh sách. Dòng đóng fence thụt tối đa ba dấu
  // cách; thụt bốn dấu cách hay khác ký tự fence vẫn là nội dung của khối code.
  struct ExpectedBlock {
    std::string_view text;
    bool markdown;
    bool section_gap;
  };
  const ExpectedBlock expected_blocks[] = {
      {"**Linux**", true, false},
      {"- Release highlights fixture", true, true},
      {"```sh", false, true},
      {"# Restart after updating", false, false},
      {"crossdesk", false, false},
      {"---", false, false},
      {"- not a list item", false, false},
      {"```", false, false},
      {"**Windows**", true, true},
      {"- C# tooling", true, true},
      {"**macOS**", true, true},
      {"~~~", false, true},
      {"``` not a closing fence", false, false},
      {"    ~~~", false, false},
      {"   ~~~", false, false},
      {"- Notarized", true, false},
  };
  if (!Check(*slint::StyledText::from_markdown("- not a list item") !=
                 slint::StyledText::from_plain_text("- not a list item"),
             "Markdown and literal list lines must differ for the fence check")) return false;
  const auto installed_blocks = crossdesk::release_notes_text::InstalledBlocks(
      "### Linux ###\n\n- Release highlights fixture\n\n```sh\n# Restart after updating\n"
      "crossdesk\n---\n- not a list item\n```\n\n## Windows\n\n- C# tooling\n\n## macOS\n\n"
      "~~~\n``` not a closing fence\n    ~~~\n   ~~~\n- Notarized");
  bool blocks_match = installed_blocks.size() == std::size(expected_blocks);
  for (size_t index = 0; blocks_match && index < installed_blocks.size(); ++index) {
    const auto& expected = expected_blocks[index];
    const auto content = expected.markdown
                             ? *slint::StyledText::from_markdown(expected.text)
                             : slint::StyledText::from_plain_text(expected.text);
    blocks_match = installed_blocks[index].content == content &&
                   installed_blocks[index].section_gap == expected.section_gap;
  }
  if (!Check(blocks_match,
             "Installed release notes must keep subheadings and every code fence line")) return false;
  // GitHub hiện chữ của dòng giữa code span nhiều dòng và của dòng "2. ..." nối tiếp đoạn văn, còn
  // Markdown đọc riêng từng dòng thấy định nghĩa link và bỏ mất chữ, nên hộp thoại giữ nguyên chữ.
  const struct {
    const char* text;
    bool markdown;
  } definition_like_blocks[] = {
      {"- Run `crossdesk", true},
      {"[notice]: https://example.com", false},
      {"--check`", true},
      {"Read the guide", true},
      {"2. [guide]: ./guide", false},
      {"- [x] Verified", true},
  };
  const auto definition_like = crossdesk::release_notes_text::InstalledBlocks(
      "- Run `crossdesk\n[notice]: https://example.com\n--check`\n\nRead the guide\n"
      "2. [guide]: ./guide\n\n- [x] Verified");
  blocks_match = definition_like.size() == std::size(definition_like_blocks);
  for (size_t index = 0; blocks_match && index < definition_like.size(); ++index) {
    const auto& expected = definition_like_blocks[index];
    blocks_match = definition_like[index].content ==
                   (expected.markdown ? *slint::StyledText::from_markdown(expected.text)
                                      : slint::StyledText::from_plain_text(expected.text));
  }
  if (!Check(blocks_match,
             "Installed release notes must keep lines that look like link definitions")) return false;
  // Markdown của Slint báo lỗi với trích dẫn, kể cả trích dẫn trong mục list, nên hộp thoại hiện cả
  // dòng thành chữ thô và link trong đó không bấm được. Vì vậy extractor từ chối trích dẫn.
  if (!Check(!slint::StyledText::from_markdown("> See [the guide](https://example.com)") &&
                 !slint::StyledText::from_markdown("- > See [the guide](https://example.com)"),
             "Slint Markdown must reject the block quotes that the extractor rejects")) return false;
  std::vector<crossdesk::ui::ReleaseNoteBlock> installed_notes;
  for (const auto& parsed : installed_blocks) {
    crossdesk::ui::ReleaseNoteBlock block{};
    block.content = parsed.content;
    block.section_gap = parsed.section_gap;
    installed_notes.push_back(std::move(block));
  }
  const auto installed_model =
      std::make_shared<slint::VectorModel<crossdesk::ui::ReleaseNoteBlock>>(
          std::move(installed_notes));
  main->set_installed_release_notes(installed_model);
  main->set_release_welcome_pending(true);
  int release_dismissals = 0;
  main->on_dismiss_release_welcome([&] {
    ++release_dismissals;
    main->set_release_welcome_open(false);
  });
  int install_requests = 0, manual_downloads = 0, release_channel_requests = 0;
  main->on_open_update_channel([&] { ++release_channel_requests; });
  main->on_open_download([&] { ++install_requests; main->set_update_confirm_open(true); });
  main->on_open_manual_download([&] { ++manual_downloads; });
  main->show();
  screen.Settle();
  const float button_y = height / 2 + std::min(488.f, height - 32) / 2 - 41;
  const float action_x = width / 2 + 161;
  const float manual_x = action_x - 118;
  const auto action_pixels = [&] {
    return screen.Region(int(action_x - 40), int(button_y - 12), 80, 24);
  };
  if (!Check(main->get_can_show_release_welcome(),
             "An available update must defer to pending installed release notes")) return false;
  main->set_release_welcome_open(true);
  main->set_release_welcome_pending(false);
  screen.Settle();
  screen.Click(action_x, button_y);
  if (!Check(install_requests == 0 && main->get_release_welcome_open(),
             "Update installation must not be clickable behind release notes")) return false;
  if (!WriteEnglishEvidence(screen, main, "release-notes-no-sponsor")) return false;
  // Link trong notes phải tới được callback mở trình duyệt, với cả hai cách viết link mà
  // extractor cho qua: autolink trong ngoặc nhọn và link nội dòng. Khối đi qua InstalledBlocks
  // như notes thật, rồi bấm vào chữ đầu tiên ở góc trên bên trái vùng cuộn của hộp thoại.
  std::vector<std::string> opened_links;
  main->on_open_about_link([&](slint::SharedString url) { opened_links.emplace_back(url); });
  const float notes_x = width / 2 - std::min(480.f, width - 40) / 2 + 24 + 12;
  const float notes_y = height / 2 - std::min(416.f, height - 32) / 2 + 64 + 10;
  for (const std::string_view link_markdown :
       {std::string_view{"<https://example.com/downloads>"},
        std::string_view{"[Open the Linux downloads page](https://example.com/downloads)"}}) {
    std::vector<crossdesk::ui::ReleaseNoteBlock> link_notes;
    for (const auto& parsed : crossdesk::release_notes_text::InstalledBlocks(link_markdown)) {
      crossdesk::ui::ReleaseNoteBlock block{};
      block.content = parsed.content;
      block.section_gap = parsed.section_gap;
      link_notes.push_back(std::move(block));
    }
    if (!Check(link_notes.size() == 1, "A one-line link must become one release notes block")) {
      return false;
    }
    opened_links.clear();
    main->set_installed_release_notes(
        std::make_shared<slint::VectorModel<crossdesk::ui::ReleaseNoteBlock>>(std::move(link_notes)));
    screen.Settle();
    screen.Click(notes_x + 16, notes_y + 8);
    if (!Check(opened_links == std::vector<std::string>{"https://example.com/downloads"} &&
                   main->get_release_welcome_open(),
               "A link in the installed release notes must open its URL")) return false;
  }
  main->set_installed_release_notes(installed_model);
  screen.Settle();
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(release_dismissals == 1 && !main->get_release_welcome_open(),
             "Escape must close release notes without suppressing the update")) return false;
  if (!WriteEnglishEvidence(screen, main, "update-no-sponsor")) return false;
  const float channel_y = height / 2 - std::min(488.f, height - 32) / 2 + 64;
  screen.Click(width / 2 + 163, channel_y);
  if (!Check(release_channel_requests == 1 && install_requests == 0 && manual_downloads == 0,
             "Update header must open the release channel without starting a download")) return false;
  // Do fork sở hữu: so nhãn bằng tiếng Anh, vì máy không có font CJK (rootfs build
  // Linux) vẽ cả "安装更新" lẫn "重试更新" thành khoảng trống như nhau. Phép so vẫn đòi
  // khung hình đổi nhãn; các cú bấm sau đó quay lại hình học tiếng Trung như cũ.
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 1);
  main->set_language_index(1);
  screen.Settle();
  const auto install_label = action_pixels();
  main->set_update_install_failed(true);
  main->set_update_install_status(crossdesk::ui_localization::Text(
      crossdesk::localization::update_install_failed[0] + "\nDownload failed"));
  screen.Settle();
  if (!Check(action_pixels() != install_label,
             "A failed update must visibly replace Install update with Retry update")) return false;
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 0);
  main->set_language_index(0);
  screen.Settle();
  screen.Click(manual_x, button_y);
  if (!Check(manual_downloads == 1 && install_requests == 0 && !main->get_update_confirm_open(),
             "Manual download must remain reachable after failure without starting automatic installation")) return false;
  screen.Click(action_x, button_y);
  if (!Check(install_requests == 1 && main->get_update_confirm_open(),
             "Retry must open the installation confirmation")) return false;
  screen.Key(slint::platform::key_codes::Escape);
  if (!Check(!main->get_update_confirm_open(), "Retry confirmation must remain cancellable")) return false;
  main->set_update_install_busy(true);
  screen.Settle();
  screen.Click(action_x, button_y);
  screen.Click(manual_x, button_y);
  if (!Check(install_requests == 1 && manual_downloads == 1,
             "Recovery buttons must reject repeated clicks while an update is running")) return false;
  main->set_update_install_busy(false);
  main->set_update_install_available(false);
  main->set_update_install_status(crossdesk::ui_localization::Text(
      crossdesk::localization::update_install_failed_manual[0]));
  // The application's primary action falls back to manual download when the
  // current installation or signed manifest no longer supports automatic updates.
  main->on_open_download([&] { ++manual_downloads; });
  screen.Settle();
  screen.Click(action_x, button_y);
  return Check(manual_downloads == 2 && !main->get_update_confirm_open(),
               "An unavailable automatic retry must retain the primary manual download action");
}

bool VerifyServerPanel(RenderTestPlatform& platform) {
  auto panel = crossdesk::ui::ServerWindow::create();
  RegisterFixtureIcons(panel);
  crossdesk::ui_localization::ApplyServerStrings(panel, 0);
  auto& screen = *platform.adapter;
  panel->set_controllers(std::make_shared<slint::VectorModel<crossdesk::ui::ControllerEntry>>(
      std::vector<crossdesk::ui::ControllerEntry>{{"1", "First"}, {"2", "Second"}}));
  panel->window().set_size(slint::LogicalSize({225, panel->get_content_height()}));
  panel->show();
  int selected = -1, disconnected = 0, picked = 0;
  panel->on_controller_selected([&](int index) { selected = index; });
  panel->on_disconnect_all([&] { ++disconnected; });
  panel->on_select_file([&] { ++picked; panel->set_file_transfer_visible(true); });
  slint::Timer sync;
  sync.start(slint::TimerMode::Repeated, 16ms, [&] {
    const auto height = unsigned(panel->get_content_height());
    if (screen.extent.height != height)
      panel->window().set_size(slint::LogicalSize({225, float(height)}));
    panel->set_file_progress(0.5);
  });
  screen.Settle();
  // Do fork sở hữu: hai nút chân bảng rộng theo nhãn, mà độ rộng chữ Hán tùy font CJK
  // của máy (rootfs build Linux không có font CJK). Lấy hai nút từ hàng y 123 của khung
  // hình thay vì hoành độ đo bằng font của macOS: nút "发送文件"/"取消" nằm giữa hai
  // cạnh viền đầu và cuối tính từ lề x 10, nút ngắt tất cả là đoạn đỏ #d92d20 bên phải.
  // Dò nút đỏ trước, rồi dò nút gửi tới trước cột liền kề nút đỏ, vì cột đó có thể là
  // mép khử răng cưa của nút đỏ chứ không thuộc nút gửi. Mỗi nút rộng ít nhất 20px (nhãn
  // cộng 20px): đoạn nút gửi tính cả cột viền khử răng cưa nên có ít nhất 20 cột, còn khi
  // nút đỏ đặt ở hoành độ lẻ thì hai cột mép là màu pha, nên chỉ chắc chắn còn 19 cột đỏ.
  const auto footer_pixel = [&](int x) { return screen.pixels.at(123 * screen.extent.width + x); };
  int send_left = -1, send_right = -1, danger_left = -1, danger_right = -1;
  for (int x = 10; x < int(screen.extent.width) - 10; ++x) {
    const auto p = footer_pixel(x);
    if (p.r == 0xd9 && p.g == 0x2d && p.b == 0x20) {
      if (danger_left < 0) danger_left = x;
      danger_right = x;
    }
  }
  for (int x = 10; x < danger_left - 1; ++x) {
    const auto p = footer_pixel(x);
    if (!(p.r == 255 && p.g == 255 && p.b == 255)) {
      if (send_left < 0) send_left = x;
      send_right = x;
    }
  }
  if (!Check(danger_left >= 0 && send_left >= 0 && send_right - send_left + 1 >= 20 &&
                 danger_right - danger_left + 1 >= 19,
             "Controlled-side footer buttons must be painted")) return false;
  const float send_x = (send_left + send_right) / 2.f;
  const float disconnect_all_x = (danger_left + danger_right) / 2.f;
  screen.Click(50, 84, 4);
  if (!Check(selected == 1, "Controlled-side controller selection must respond to a held click")) return false;
  const auto normal_height = screen.extent.height;
  screen.Click(disconnect_all_x, 123, 4);
  if (!Check(panel->get_confirming_disconnect() && screen.extent.height == normal_height,
             "Disconnect confirmation must use the space reserved below the buttons")) return false;
  screen.Click(send_x, 123, 4);
  if (!Check(!panel->get_confirming_disconnect() && disconnected == 0 && screen.extent.height == normal_height,
             "Cancel must clear the confirmation and preserve the panel height")) return false;
  screen.Click(disconnect_all_x, 123, 4);
  screen.Click(disconnect_all_x, 123, 4);
  if (!Check(disconnected == 1 && !panel->get_confirming_disconnect(),
             "Disconnect all must require and accept confirmation")) return false;
  screen.Click(send_x, 123, 4);
  return Check(picked == 1 && screen.extent.height == normal_height + 58,
               "Controlled-side file progress must resize the panel immediately");
}

// Do fork sở hữu: cửa sổ kết nối co theo fit-scale, còn tỷ lệ canvas suy ra
// từ kích thước cửa sổ thật nên không có gì bị cắt.
bool VerifyMainWindowFitScale(RenderTestPlatform&) {
  auto main = CreateMainWindow();
  const float design_width = main->get_design_width();
  const float design_height = main->get_design_height();
  if (!Check(main->get_minimum_window_width() == design_width &&
                 main->get_minimum_window_height() == design_height &&
                 main->get_ui_scale() == 1.0f,
             "Default fit-scale must keep the design size")) return false;
  const std::pair<float, float> cases[] = {{0.825f, 0.825f}, {0.5f, 0.5f}};
  for (const auto& [fit, expected] : cases) {
    main->set_fit_scale(fit);
    main->window().set_size(slint::LogicalSize(
        {main->get_minimum_window_width(), main->get_minimum_window_height()}));
    main->show();
    if (!Check(std::fabs(main->get_minimum_window_width() - design_width * fit) < 0.5f &&
                   std::fabs(main->get_ui_scale() - expected) < 0.01f &&
                   std::fabs(main->get_ui_width() - design_width) < 1.0f &&
                   std::fabs(main->get_ui_height() - design_height) < 1.0f,
               "Scaled window must expose the full design canvas")) return false;
  }
  main->set_gallery_mode(true);
  main->set_fit_scale(0.5f);
  main->window().set_size(slint::LogicalSize({960.0f, 720.0f}));
  return Check(main->get_ui_scale() == 1.0f && main->get_minimum_window_width() == 640.0f,
               "DeskHub mode must ignore fit-scale");
}

// Dòng link "GitHub Releases" trong hộp thoại cập nhật, theo toạ độ canvas thiết kế.
struct UpdateLinkRow {
  float y = 0;     // Tâm theo chiều dọc của dòng.
  float clip = 0;  // Mép phải vùng Text, nơi chữ bị cắt.
};

// Dải cột có mực của link, theo pixel vật lý; -1 khi không thấy mực.
struct InkSpan {
  int start = -1;
  int end = -1;
  int Width() const { return end - start + 1; }
};

// Mở hộp thoại cập nhật ở hệ số scale cửa sổ cho trước, ghi ảnh bằng chứng và trả
// về cột mực đầu và cuối của link (pixel vật lý, -1 nếu không thấy mực).
bool RenderUpdateLink(RenderTestPlatform& platform,
                      slint::ComponentHandle<crossdesk::ui::MainWindow> main,
                      int language, float scale, const std::string& evidence,
                      UpdateLinkRow& row, InkSpan& ink) {
  auto& screen = *platform.adapter;
  crossdesk::ui_localization::ApplyMainWindowStrings(main, language);
  main->set_language_index(language);
  screen.window().dispatch_scale_factor_change_event(scale);
  const float width = main->get_minimum_window_width();
  const float height = main->get_minimum_window_height();
  // RenderTestWindow::set_size coi kích thước vật lý là kích thước logic, nên ở
  // tỉ lệ khác 1 test tự cấp bộ đệm vật lý và báo kích thước logic đúng.
  screen.extent = slint::PhysicalSize({std::uint32_t(std::lround(width * scale)),
                                       std::uint32_t(std::lround(height * scale))});
  screen.pixels.resize(std::size_t(screen.extent.width) * screen.extent.height);
  screen.window().dispatch_resize_event(slint::LogicalSize({width, height}));
  screen.dirty = true;
  main->set_update_available(true);
  main->set_latest_version("v2.0.0");
  main->set_current_version("1.7.6");
  main->show();
  screen.Settle();
  if (!Check(main->get_ui_scale() == 1.0f,
             "Update link fixture must keep the design canvas")) return false;
  const float canvas_width = main->get_ui_width();
  const float canvas_height = main->get_ui_height();
  row.y = canvas_height / 2 - std::min(488.f, canvas_height - 32) / 2 + 64;
  // Hộp thoại rộng 480px, căn giữa, padding 24px; vùng Text rộng thêm 16px.
  row.clip = canvas_width / 2 + 240 - 24 + 16;
  const int top = int((row.y - 12) * scale);
  const int bottom = int((row.y + 12) * scale);
  // Quét từ mép trái vùng nội dung: nhãn dài (như tiếng Việt) bắt đầu bên trái
  // tâm hộp thoại, và chữ đầu bị cắt chỉ lộ ra khi đo được cả cột mực đầu.
  const int left = int((canvas_width / 2 - 240 + 24) * scale);
  const int right = std::min(int(std::ceil((row.clip + 4) * scale)),
                             int(screen.extent.width));
  ink = InkSpan{};
  for (int y = top; y <= bottom; ++y)
    for (int x = left; x < right; ++x) {
      const auto p = screen.pixels.at(std::size_t(y) * screen.extent.width + x);
      // Chữ của link là màu nhấn bão hoà; tiêu đề, số phiên bản và nền đều xám.
      if (std::max({p.r, p.g, p.b}) - std::min({p.r, p.g, p.b}) > 48) {
        ink.start = ink.start < 0 ? x : std::min(ink.start, x);
        ink.end = std::max(ink.end, x);
      }
    }
  return WritePopupEvidence(screen, evidence);
}

// Do fork sở hữu: ở 150% trên màn hình 1366x768 host thu cửa sổ về fit-scale
// khoảng 0.74, nên chữ được vẽ ở tỉ lệ thật khoảng 1.11 và có thể rộng hơn
// preferred-width vài pixel. Link "GitHub Releases" trong hộp thoại cập nhật phải
// vẽ đủ cả nhãn (không chạm mép cắt của vùng Text, không mất chữ cuối), và bấm vào
// cột mực cuối cùng vẫn phải mở kênh phát hành. Bộ vẽ phần mềm của test không áp
// transform-scale của ui-scaler, nên test đạt cùng cỡ chữ vật lý bằng hệ số scale
// cửa sổ 1.1104.
bool VerifyUpdateLinkAtFractionalScale(RenderTestPlatform& platform) {
  constexpr float kWindowScale = 1.1104f;
  // Chỉ số ngôn ngữ nội bộ: 1 là en-US, 10 là tr, 12 là vi.
  for (const int language : {1, 10, 12}) {
    const std::string suffix = std::to_string(language);
    // Bản tham chiếu ở tỉ lệ 1: preferred-width và chữ được đo cùng một tỉ lệ, nên
    // đây là chỗ mực của nhãn đầy đủ kết thúc với font của chính nền tảng này.
    InkSpan reference;
    {
      auto reference_window = CreateMainWindow();
      UpdateLinkRow reference_row;
      if (!RenderUpdateLink(platform, reference_window, language, 1.0f,
                            "update-link-100-" + suffix, reference_row, reference)) return false;
    }
    if (!Check(reference.end >= 0, "Update link must render at scale 1")) return false;

    auto main = CreateMainWindow();
    UpdateLinkRow row;
    InkSpan ink;
    if (!RenderUpdateLink(platform, main, language, kWindowScale, "update-link-150-" + suffix,
                          row, ink)) return false;
    if (!Check(ink.end >= 0, "Update link must render at a fractional scale")) return false;
    if (!Check(ink.end + 2 < int(row.clip * kWindowScale),
               "Update link must end before the edge that clips its text")) return false;
    // Chỉ kiểm mép cắt thì một nhãn bị mất phần đuôi vẫn lọt. Mất dù một chữ cuối
    // làm mực kết thúc sớm cỡ một bề rộng ký tự (khoảng 6px), còn dung sai 2px chỉ
    // chừa chỗ cho sai lệch làm tròn vị trí glyph giữa hai tỉ lệ.
    if (!Check((ink.end + 1) / kWindowScale >= float(reference.end + 1) - 2.f,
               "Update link must render its whole label at a fractional scale")) return false;
    // Mép phải đứng yên vẫn che được việc mất chữ đầu, nên so cả bề rộng mực với
    // bản tham chiếu, cùng dung sai làm tròn 2px.
    if (!Check(ink.Width() / kWindowScale >= float(reference.Width()) - 2.f,
               "Update link must keep its leading letters at a fractional scale")) return false;
    int channel_requests = 0;
    main->on_open_update_channel([&] { ++channel_requests; });
    platform.adapter->Click((ink.end + 0.5f) / kWindowScale, row.y);
    if (!Check(channel_requests == 1,
               "The last rendered letter of the update link must open the release channel")) return false;
    // Phần mực tràn ra tuỳ font nên có máy không tràn; bấm thẳng vào giữa dải
    // 16px mở rộng để chắc vùng bấm phủ hết vùng vẽ trên mọi nền tảng.
    platform.adapter->Click(row.clip - 8, row.y);
    if (!Check(channel_requests == 2,
               "The update link must respond across its whole drawing area")) return false;
  }
  return true;
}

bool VerifyConnectionDialogSurface(RenderTestPlatform& platform) {
  auto main = CreateMainWindow();
  ConfigureEvidenceData(main);
  crossdesk::ui_localization::ApplyMainWindowStrings(main, 1);
  main->set_language_index(1);
  auto& screen = *platform.adapter;
  const float width = main->get_minimum_window_width();
  const float height = main->get_minimum_window_height();
  main->window().set_size(slint::LogicalSize({width, height}));
  main->show();
  main->set_connection_status_text("Establishing P2P connection...");
  main->set_connection_pending(true);
  main->set_connection_dialog_open(true);
  screen.Settle();
  if (!WritePopupEvidence(screen, "connection-dialog")) return false;
  // Hộp thoại rộng 340px, cao 168px, căn giữa cửa sổ. Điểm lấy mẫu nằm trong
  // lề trái của thẻ, ngoài vùng chữ và nút, nên chỉ thấy nền của thẻ.
  const int card_x = int((width - 340.0f) / 2.0f);
  const int card_y = int((height - 168.0f) / 2.0f);
  const bool opaque = screen.Light(card_x + 10, card_y + 84) &&
                      screen.Light(card_x + 330, card_y + 84) &&
                      screen.Light(card_x + 170, card_y + 90);
  main->set_connection_dialog_open(false);
  main->set_connection_pending(false);
  return Check(opaque, "Connection progress dialog must paint an opaque surface over the scrim");
}
}  // namespace

int main() {
  auto platform = std::make_unique<RenderTestPlatform>();
  auto* backend = platform.get();
  slint::platform::set_platform(std::move(platform));
  // A bounded headless invocation can isolate this regression while the full
  // suite remains the default and retains every existing assertion.
  if (const char* test = std::getenv("CROSSDESK_POPUP_TEST");
      test && std::string(test) == "password") {
    return VerifyPasswordDialog(*backend) ? 0 : 7;
  }
  if (const char* test = std::getenv("CROSSDESK_POPUP_TEST");
      test && std::string(test) == "workbench") {
    return VerifyWorkbenchPopups(*backend) ? 0 : 2;
  }
  if (const char* test = std::getenv("CROSSDESK_POPUP_TEST");
      test && std::string(test) == "postcard-ip") {
    return VerifyPostcardIpControls(*backend) ? 0 : 8;
  }
  if (const char* test = std::getenv("CROSSDESK_POPUP_TEST");
      test && std::string(test) == "connection-dialog") {
    return VerifyConnectionDialogSurface(*backend) ? 0 : 11;
  }
  if (const char* test = std::getenv("CROSSDESK_POPUP_TEST");
      test && std::string(test) == "recent-16x9") {
    return VerifySquareRecentPreview(*backend) ? 0 : 9;
  }
  if (!VerifyStreamPopups(*backend)) return 1;
  if (!VerifyWorkbenchPopups(*backend)) return 2;
  if (!VerifyGalleryPopups(*backend)) return 3;
  if (!VerifyServerPanel(*backend)) return 4;
  if (!VerifyAutostartSettings(*backend)) return 5;
  if (!VerifyUpdateRecovery(*backend)) return 6;
  if (!VerifyPasswordDialog(*backend)) return 7;
  if (!VerifyPostcardIpControls(*backend)) return 8;
  if (!VerifySquareRecentPreview(*backend)) return 9;
  if (!VerifyMainWindowFitScale(*backend)) return 10;
  if (!VerifyConnectionDialogSurface(*backend)) return 11;
  if (!VerifyUpdateLinkAtFractionalScale(*backend)) return 12;
  std::cout << "Update link at a fractional scale passed.\n";
  std::cout << "Main window fit-scale passed.\n";
  std::cout << "Square mode controls and uncropped 16:9 recent preview passed.\n";
  std::cout << "Slint popup rendering and local input passed.\n";
}
