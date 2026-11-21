# Cập nhật hình minh họa tài liệu

## Trang chủ trang web chính thức

Dùng hồ sơ trình duyệt trống để mở riêng trang web tiếng Trung và tiếng Anh, đặt cửa sổ 1280 × 720 pixel logic. Chờ ảnh thiết bị đầu trang tải xong, đóng lớp nổi trình duyệt rồi chụp toàn bộ cửa sổ gồm thẻ và thanh địa chỉ. Lưu bản tiếng Trung thành `fixtures/website.png`, bản tiếng Anh thành `fixtures/website-en.png`. [Trang web tiếng Trung](https://www.crossdesk.cn/) · [English website](https://www.crossdesk.cn/en/)

Office PC dùng trang chủ trang web trong thiết bị gần đây và DeskHub; cài đặt hình ảnh phiên cũng dùng nó làm nền. Sau khi cập nhật tư liệu, tạo lại ảnh chụp máy tính bên dưới để đồng bộ các hình này.

## Ứng dụng Web

Khi cập nhật trang chủ Web, dùng hồ sơ trình duyệt trống mở ứng dụng trực tuyến, đặt cửa sổ 1280 × 720 pixel logic. Chờ biểu mẫu và số phiên bản hiện, xác nhận ID thiết bị và mật khẩu trống, lời nhắc trạng thái kết nối đã biến mất. Chụp toàn bộ cửa sổ gồm thẻ và thanh địa chỉ, xuất thành `web-client.png`. Cập nhật ảnh mà chương trình chụp đọc trước, rồi tạo lại ảnh chụp máy tính có thiết bị gần đây hoặc ô DeskHub.

## Ứng dụng máy tính

Hàng thứ hai Bức tường màn hình lần lượt hiện video Kitty của Studio Mac, terminal biên dịch Build Server và tải CPU Test Device. Có thể chụp tư liệu terminal `fixtures/build-terminal.png` sau khi chạy `xmake build crossdesk_wire`. Đường CPU từ “Window → CPU History” của Activity Monitor macOS; lưu toàn bộ cửa sổ riêng thành `fixtures/cpu-history.png` và `fixtures/cpu-history-en.png`. Sau khi cập nhật tư liệu, tạo lại `gallery` và `gallery-focus`.

Thực hiện các lệnh sau ở thư mục gốc dự án macOS đã cấu hình phụ thuộc biên dịch và Python Pillow. Khi chụp dùng màn hình 2× và cho terminal quyền ghi màn hình. Nguồn chương trình chụp, tệp thực thi và ảnh cửa sổ gốc đều lưu trong thư mục tạm ngoài dự án.

Kiểm tra `/tmp/crossdesk-ui-capture-page` và `/tmp/crossdesk-ui-capture-language` trước: tệp ghi đè cũ tồn tại và không rỗng ưu tiên hơn biến môi trường, cần xác nhận và chuyển đi rồi mới chụp. `CROSSDESK_UI_CAPTURE_DEMO_ASSETS` trỏ đến thư mục này để dùng cho hình thu nhỏ thiết bị gần đây.

### Tạo chương trình chụp

Tái dùng hàm trợ giúp chụp trong tệp nguồn hiện có, cắt bỏ điểm vào kiểm thử, thêm trạng thái trình diễn và điểm vào riêng trong thư mục tạm; giữ nguyên tệp kiểm thử trong dự án. Khi cấu trúc nguồn đổi, cần rà soát lại vị trí thay thế.

```bash
set -e
xmake b -y crossdesk_ui
capture_dir=$(mktemp -d "${TMPDIR:-/tmp}/crossdesk-readme-capture.XXXXXX")
export capture_dir
python3 - <<'PY'
from pathlib import Path
import os
root = Path.cwd()
out = Path(os.environ['capture_dir'])
source = (root / 'apps/desktop/tests/slint_ui_smoke_test.cpp').read_text()
source = source[:source.index('// Sample the native window while')]
old = '  // Preview DeskHub with the same demo images, without opening any sessions.'
new = '''  // Documentation-only state; no runtime, listeners, or remote sessions.
  window->set_local_password("Ab12Cd");
  window->set_ip_direct_enabled(true);
  window->set_ip_direct_enabled_draft(true);
  if (options.page == "settings-network") window->set_settings_open(true);
  if (options.page == "lan" || options.page == "lan-password") {
    window->set_lan_mode(true);
    window->set_lan_listener_ip("192.168.1.10");
    window->set_lan_listener_port("52888");
    window->set_lan_local_port("52888");
    window->set_lan_interfaces(std::make_shared<slint::VectorModel<slint::SharedString>>(
        std::vector<slint::SharedString>{"192.168.1.10", "192.168.2.10"}));
    window->set_lan_new_password("Cd34Ef");
    window->set_lan_listening(true);
    window->set_remote_id_input("192.168.1.20:52888");
    const auto recent = window->get_recent_connections();
    for (size_t i = 0; i < recent->row_count(); ++i) {
      auto device = recent->row_data(i).value();
      device.lan = true;
      device.online = false;
      device.remote_id = slint::SharedString(("lan-demo-" + std::to_string(i)).c_str());
      device.endpoint = slint::SharedString(("192.168.1." + std::to_string(20 + i) + ":52888").c_str());
      recent->set_row_data(i, device);
    }
    if (options.page == "lan-password") {
      window->set_lan_remote_address("192.168.1.20");
      window->set_lan_remote_port("52888");
      window->set_lan_open(true);
      window->set_lan_remember_password(true);
    }
  }
''' + old
assert source.count(old) == 1
source = source.replace(old,new)
old = '      // Let settings scroll animations settle as well as hover/focus feedback.'
new = '''      if (options.page == "settings-deskhub") window->set_settings_tab(1);
      if (options.page == "settings-network") {
        window->window().dispatch_pointer_scroll_event(
            slint::LogicalPosition({400.0f, 250.0f}), 0.0f, -148.0f);
        window->window().dispatch_pointer_move_event(slint::LogicalPosition({2.0f, 2.0f}));
      }
''' + old
assert source.count(old) == 1
source = source.replace(old,new)
# Preserve the six-device DeskHub composition used in the README.
for device, old_image, new_image in [
    ('Studio Mac', 'fixtures/source.png', 'cli-kitty-overview.png'),
    ('Build Server', 'fixtures/website.png', 'fixtures/build-terminal.png'),
    ('Test Device', 'web-client.png', 'fixtures/cpu-history.png'),
]:
    old = f'"{device}", "{old_image}"'
    assert source.count(old) == 1
    source = source.replace(old, f'"{device}", "{new_image}"')
old = 'if (connections.size() == 3 && options.page != "main-recent-overflow" &&'
new = 'if (connections.size() == 3 && !options.page.starts_with("gallery") && options.page != "main-recent-overflow" &&'
assert source.count(old) == 1
source = source.replace(old,new)
old = '    window->set_gallery_mode(true);'
assert source.count(old) == 1
source = source.replace(old,old + '\n    window->set_gallery_tile_width(300);')
old = '        698.0f;'
assert source.count(old) == 1
source = source.replace(old,'        options.page == "gallery" ? 500.0f : 698.0f;')
source += '''} // namespace
int main() {
  const auto options = LoadCaptureOptions();
  if (!options.enabled || options.snapshot_path.empty()) return 1;
  auto window = crossdesk::ui::MainWindow::create();
  auto stream = crossdesk::ui::StreamWindow::create();
  auto server = crossdesk::ui::ServerWindow::create();
  if (!RegisterFontAwesome(window->window()) || !RegisterFontAwesome(stream->window()) ||
      !RegisterFontAwesome(server->window())) return 2;
  window->set_local_id("123 456 789");
  window->set_local_password("Ab12Cd");
  window->window().set_size(slint::LogicalSize({640.0f, 472.0f}));
  return RunCaptureMode(options, window, stream, server);
}
'''
# Documentation-only additions for the current UI.
source = source.replace('window->set_current_version("v1.5.2");',
    'window->set_current_version("development");\n'
    '    window->set_update_check_status(crossdesk::ui::UpdateCheckStatus::Idle);')
source = source.replace('  // Preview DeskHub with the same demo images, without opening any sessions.',
    '  if (options.page == "feedback") window->set_feedback_open(true);\n'
    '  // Preview DeskHub with the same demo images, without opening any sessions.')
source = source.replace('      stream->set_video_menu_open(true);', r'''      stream->set_video_menu_open(true);
      stream->set_remote_resolution_visible(true);
      stream->set_remote_resolution_enabled(true);
      stream->set_remote_resolutions(std::make_shared<slint::VectorModel<slint::SharedString>>(
          std::vector<slint::SharedString>{"1280 × 720", "1920 × 1080", "2560 × 1440"}));
      stream->set_remote_resolution_index(1);''')
source = source.replace('      // Let settings scroll animations settle as well as hover/focus feedback.', r'''      if (options.page == "stream-video-settings") {
        stream->set_video_menu_open(false);
        const slint::LogicalPosition logo({19.0f, 19.0f});
        stream->window().dispatch_pointer_move_event(logo);
        stream->window().dispatch_pointer_press_event(logo, slint::PointerEventButton::Left);
        stream->window().dispatch_pointer_release_event(logo, slint::PointerEventButton::Left);
        stream->window().dispatch_pointer_move_event(slint::LogicalPosition({950.0f, 590.0f}));
        stream->set_video_menu_open(true);
      }
      // Let settings scroll animations settle as well as hover/focus feedback.''')
source = source.replace('options.page.rfind("settings", 0) == 0 ? 350 : 150',
    'options.page == "stream-video-settings" ? 650 : options.page.rfind("settings", 0) == 0 ? 350 : 150')
source = source.replace('int RunCaptureMode(', r'''int CaptureFileBrowser(const CaptureOptions &options) {
  auto files = crossdesk::ui::FileBrowserWindow::create();
  if (!RegisterFontAwesome(files->window())) return 2;
  const bool zh = options.language == 0;
  files->set_chinese(zh);
  files->set_peer_name("Office PC");
  files->set_local_role(zh ? "控制端" : "Controller");
  files->set_remote_role(zh ? "被控端" : "Host");
  files->set_local_path("/Users/demo/Documents");
  files->set_local_current_path("/Users/demo/Documents");
  files->set_remote_path("/home/demo/Documents");
  files->set_remote_current_path("/home/demo/Documents");
  files->set_local_up(true);
  files->set_remote_up(true);
  files->set_local_status(zh ? "4 个项目" : "4 items");
  files->set_remote_status(zh ? "4 个项目" : "4 items");
  std::vector<crossdesk::ui::BrowserEntry> local, remote;
  for (int pane = 0; pane < 2; ++pane) {
    const char *names[] = {"Design assets", "Release notes.md", "Roadmap.pdf", "Screenshots.zip"};
    const char *sizes[] = {"—", "12 KB", "248 KB", "8.4 MB"};
    auto &entries = pane == 0 ? local : remote;
    for (int i = 0; i < 4; ++i) {
      crossdesk::ui::BrowserEntry entry{};
      entry.name = names[i];
      entry.path = slint::SharedString((std::string(pane == 0 ? "/Users/demo/Documents/" : "/home/demo/Documents/") + names[i]).c_str());
      entry.directory = i == 0;
      entry.selected = pane == 0 && i == 3;
      entry.size = sizes[i];
      entry.modified = "2026-11-11 10:30";
      entries.push_back(entry);
    }
  }
  files->set_local_entries(std::make_shared<slint::VectorModel<crossdesk::ui::BrowserEntry>>(local));
  files->set_remote_entries(std::make_shared<slint::VectorModel<crossdesk::ui::BrowserEntry>>(remote));
  for (int pane = 0; pane < 2; ++pane) {
    std::vector<crossdesk::ui::BrowserTreeEntry> tree;
    const char *names[] = {"demo", "Desktop", "Documents", "Downloads"};
    for (int i = 0; i < 4; ++i) {
      crossdesk::ui::BrowserTreeEntry entry{};
      entry.name = names[i]; entry.depth = i == 0 ? 0 : 1;
      entry.path = slint::SharedString((std::string(pane == 0 ? "/Users/demo" : "/home/demo") + (i == 0 ? "" : std::string("/") + names[i])).c_str());
      entry.expanded = i == 0; entry.selected = i == 2;
      tree.push_back(entry);
    }
    auto model = std::make_shared<slint::VectorModel<crossdesk::ui::BrowserTreeEntry>>(tree);
    if (pane == 0) files->set_local_tree(model); else files->set_remote_tree(model);
  }
  files->set_local_selected(1);
  files->set_transfer_ready(true);
  files->set_can_upload(true);
  files->set_tasks_expanded(true);
  crossdesk::ui::BrowserTask task{};
  task.id = 1; task.name = "Roadmap.pdf"; task.size = "248 KB";
  task.detail = zh ? "发送到远端" : "Send to remote";
  task.status = zh ? "已完成" : "Completed";
  task.progress = 1.0f;
  files->set_tasks(std::make_shared<slint::VectorModel<crossdesk::ui::BrowserTask>>(std::vector{task}));
  files->set_task_summary(zh ? "1 个任务 · 已完成 1" : "1 task · 1 completed");
  files->window().set_size(slint::LogicalSize({960.0f, 620.0f}));
  files->show();
  bool saved = false;
  slint::Timer timer;
  timer.start(slint::TimerMode::SingleShot, std::chrono::milliseconds(600), [&] {
    saved = CaptureTitledWindow(options.snapshot_path.c_str());
    slint::quit_event_loop();
  });
  slint::run_event_loop();
  return saved ? 0 : 7;
}

''' + 'int RunCaptureMode(')
source = source.replace('  auto window = crossdesk::ui::MainWindow::create();',
    '  if (options.page == "files") return CaptureFileBrowser(options);\n'
    '  auto window = crossdesk::ui::MainWindow::create();')
# Capture the native frame after the demo state has settled.
source = source.replace('namespace {', 'bool CaptureTitledWindow(const char *path);\n\nnamespace {', 1)
start = source.index('bool WriteWindowSnapshot(')
end = source.index('bool RegisterFontAwesome(', start)
source = source[:start] + source[end:]
source = source.replace('WriteWindowSnapshot(*capture_window, ', 'CaptureTitledWindow(')
(out / 'readme_capture.cpp').write_text(source)
print(out / 'readme_capture.cpp')
PY

cat > "$capture_dir/window_capture.mm" <<'OBJC'
#import <AppKit/AppKit.h>

bool CaptureTitledWindow(const char *path) {
  NSWindow *window = NSApp.keyWindow;
  if (!window || !window.visible || !(window.styleMask & NSWindowStyleMaskTitled))
    return false;
  NSTask *capture = [[NSTask alloc] init];
  capture.executableURL = [NSURL fileURLWithPath:@"/usr/sbin/screencapture"];
  capture.arguments = @[@"-x", @"-o", @"-l", @(window.windowNumber).stringValue,
                        [NSString stringWithUTF8String:path]];
  if (![capture launchAndReturnError:nil]) return false;
  [capture waitUntilExit];
  return capture.terminationStatus == 0;
}
OBJC
```

### Biên dịch và chụp

Dùng AppKit lấy số cửa sổ hiện tại và lưu toàn bộ cửa sổ thành PNG. Các đường dẫn sau tương ứng cấu hình `macosx / arm64 / debug`; cấu hình khác dùng `xmake show -t slint_ui_smoke_test` xem đường dẫn header sinh và thư viện trước rồi điều chỉnh. Chụp tuần tự để tránh các cửa sổ tranh tiêu điểm.

```bash
slint_prefix=$(xmake lua -c 'import("core.project.config"); config.load(); import("core.project.project"); print(project.target("slint_ui_smoke_test"):pkg("slint"):installdir())')
xcrun clang++ -std=c++20 \
  -Iapps/desktop/src/gui \
  -Iapps/desktop/src/gui/assets/fonts \
  -Iapps/desktop/src/gui/assets/localization \
  -Ibuild/.gens/crossdesk_ui/macosx/arm64/debug/rules/slint \
  -isystem "$slint_prefix/include/slint" \
  "$capture_dir/readme_capture.cpp" "$capture_dir/window_capture.mm" \
  build/macosx/arm64/debug/libcrossdesk_ui.a \
  -L"$slint_prefix/lib" -lslint_cpp -Wl,-rpath,"$slint_prefix/lib" \
  -framework CoreFoundation -framework AppKit \
  -o "$capture_dir/readme_capture"

images="$PWD/docs/images"
english_assets="$capture_dir/assets-en"
mkdir -p "$capture_dir/raw" "$english_assets/fixtures"
ln -sf "$images/web-client.png" "$english_assets/web-client.png"
ln -sf "$images/cli-kitty-overview.png" "$english_assets/cli-kitty-overview.png"
ln -sf "$images/fixtures/cpu-history-en.png" "$english_assets/fixtures/cpu-history.png"
ln -sf "$images/fixtures/build-terminal.png" "$english_assets/fixtures/build-terminal.png"
ln -sf "$images/fixtures/source.png" "$english_assets/fixtures/source.png"
ln -sf "$images/fixtures/website-en.png" "$english_assets/fixtures/website.png"

for locale in zh en; do
  language=0
  assets="$images"
  if [ "$locale" = en ]; then
    language=1
    assets="$english_assets"
  fi
  for page in main lan gallery lan-password settings settings-network settings-deskhub self-hosted about gallery-focus stream-video-settings files feedback; do
    image_page="$page"
    if [ "$page" = gallery ]; then image_page=deskhub; fi
    if [ "$page" = gallery-focus ]; then image_page=deskhub-focus; fi
    snapshot="$capture_dir/raw/desktop-$image_page-$locale.png"
    SLINT_BACKEND=winit-software SLINT_SCALE_FACTOR=2 \
      CROSSDESK_UI_CAPTURE=1 \
      CROSSDESK_UI_CAPTURE_DEMO_ASSETS="$assets" \
      CROSSDESK_UI_CAPTURE_PAGE="$page" \
      CROSSDESK_UI_CAPTURE_LANGUAGE="$language" \
      CROSSDESK_UI_CAPTURE_SNAPSHOT="$snapshot" \
      "$capture_dir/readme_capture"
  done
done

python3 - "$capture_dir" <<'PY'
import io, sys
from pathlib import Path
from PIL import Image, ImageCms

out = Path(sys.argv[1])
for raw in (out / 'raw').glob('desktop-*.png'):
    with Image.open(raw) as window:
        rgb = ImageCms.profileToProfile(window.convert('RGB'),
            ImageCms.ImageCmsProfile(io.BytesIO(window.info['icc_profile'])),
            ImageCms.createProfile('sRGB'))
        background = Image.new('RGB', window.size, '#f1f2f3')
        background.paste(rgb, mask=window.convert('RGBA').getchannel('A'))
        background.save(out / raw.name)
PY
```

Sau khi kiểm tra tiêu đề cửa sổ tiếng Trung và tiếng Anh, hình thu nhỏ và viền chữ, sao chép PNG đã tạo vào `docs/images`.

## CLI

Chạy riêng các lệnh sau trong terminal hỗ trợ chế độ kết xuất tương ứng, thay ID ví dụ `123456789` bằng ID máy được điều khiển. Nhập mật khẩu theo lời nhắc. Chờ thanh trên hiện `Displaying` rồi chụp toàn bộ khung nhìn gồm thanh trạng thái và phím tắt phía dưới.

```sh
crossdesk-cli 123456789 --renderer ansi --quality high
crossdesk-cli 123456789 --renderer kitty --quality high
```
