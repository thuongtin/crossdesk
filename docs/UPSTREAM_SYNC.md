# Đồng bộ fork với upstream

Tài liệu này mô tả cách đưa thay đổi mới của upstream `kunkundi/crossdesk` (nhánh `mcp`) vào fork `thuongtin/crossdesk` mà vẫn giữ bản Việt hóa: catalog có cột `vi` đầy đủ, tiếng Việt chọn được trong cài đặt (bản cài mới của desktop mặc định English), tài liệu chính viết bằng tiếng Việt.

[Danh mục tài liệu](README.md) · [Bảo trì bản địa hóa](../localization/README.md#bảo-trì-tài-nguyên)

## Remote và quy tắc an toàn

| Remote | Kho | Quyền |
| --- | --- | --- |
| `origin` | `thuongtin/crossdesk` | Đọc và ghi. Mọi nhánh đồng bộ và PR đều nằm ở đây. |
| `upstream` | `kunkundi/crossdesk` | Chỉ fetch. Push URL được đặt thành `DISABLED_no_push_to_upstream`. |

Thiết lập một lần cho bản clone mới:

```bash
git remote add upstream https://github.com/kunkundi/crossdesk.git
git remote set-url --push upstream DISABLED_no_push_to_upstream
gh repo set-default thuongtin/crossdesk
```

Những việc không được làm:

- Không push lên `upstream`, không đổi push URL của `upstream`, không thêm remote khác trỏ tới `kunkundi`.
- Không dùng nút "Sync fork" trên GitHub hay `gh repo sync`: hai cách này có thể bỏ commit riêng của fork. Luôn merge ở máy local.
- Không force-push lên `origin/mcp`. Làm trên nhánh mới và chỉ push nhánh đó.
- Mọi lệnh `gh` có ghi dữ liệu (PR, issue, comment) phải truyền rõ `-R thuongtin/crossdesk`.
- Không dùng GitHub Actions trên fork (tốn phí cho chủ dự án): các workflow đã bị tắt trong cài đặt repo. Không bật lại, không `gh workflow run`, không sửa tệp workflow để lách.
- Chỉ merge PR đồng bộ khi chủ dự án đã giao việc merge và PR đã qua cổng ở [Quy tắc PR](#quy-tắc-pr).

## Xem trước bằng script

`scripts/sync-upstream.sh` fetch `upstream/mcp` rồi merge thử bằng `git merge-tree`, không đụng tới working tree, index hay ref của fork, và không bao giờ push:

```bash
scripts/sync-upstream.sh            # so HEAD với upstream/mcp
scripts/sync-upstream.sh origin/mcp # so một ref khác của fork
SKIP_FETCH=1 scripts/sync-upstream.sh
```

Báo cáo gồm: merge base, số commit upstream còn thiếu, danh sách commit, các tệp sẽ xung đột chia theo nhóm (catalog, tài nguyên sinh tự động, tài liệu, code), các tệp Markdown upstream đã sửa và kết quả chạy thử `scripts/merge_localization_catalog.py --dry-run`. Mã thoát là `0` khi merge sạch hoặc không có gì để đồng bộ, `1` khi có xung đột, `2` khi thiếu remote. Đổi remote hay nhánh bằng `UPSTREAM_REMOTE` và `UPSTREAM_BRANCH`.

## Các bước đồng bộ

1. Bắt đầu từ `mcp` mới nhất của fork và tạo nhánh theo ngày:

   ```bash
   git fetch origin
   git switch -c sync/upstream-YYYY-MM-DD origin/mcp
   git fetch --no-tags upstream mcp
   scripts/sync-upstream.sh
   ```

2. Tạo một merge commit thật (không rebase, không squash) để lịch sử giữ được điểm nối với upstream:

   ```bash
   git merge --no-ff --no-commit upstream/mcp
   git submodule update --init --recursive
   ```

3. Giải quyết xung đột theo từng nhóm ở mục dưới. Không chạy `git add -u` khi vẫn còn tệp chưa xử lý xong, vì lệnh này đánh dấu cả tệp còn dấu xung đột là đã giải quyết. Thêm từng tệp sau khi đã kiểm tra.
4. Chạy toàn bộ [danh sách kiểm tra](#danh-sách-kiểm-tra).
5. Commit merge với tiêu đề `Merge upstream kunkundi/crossdesk mcp (<base>..<upstream>)` và phần thân tóm tắt cách xử lý từng nhóm. Sửa lỗi phát sinh sau merge (ví dụ test hỏng) nằm ở commit riêng.
6. Push nhánh và mở PR theo [quy tắc PR](#quy-tắc-pr).

## Quy tắc giải quyết xung đột

### Code

Lấy hành vi của upstream, rồi chỉ áp lại phần Việt hóa: chuỗi hiển thị đi qua catalog, giá trị ngôn ngữ mặc định và các chỉ số ngôn ngữ của fork. Trên desktop, tiếng Việt luôn là cột cuối của catalog: `kVietnameseLanguage = 12` và `kSystemLanguage = 13`. Giá trị lưu trong `config.ini` thì không đổi theo cột: 0–9 giữ nguyên, 10 là “Theo hệ thống”, 11 là tiếng Việt (tương thích cấu hình cũ của fork), 12 là tiếng Thổ Nhĩ Kỳ, 13 là tiếng Hebrew; `ConfigCenter` đổi qua lại giữa hai cách đánh số này. Không giữ lại logic cũ của fork chỉ vì nó từng nằm ở đó.

Với tệp merge sạch, vẫn rà chuỗi hiển thị mới mà upstream viết thẳng trong code. Chuỗi nào người dùng nhìn thấy thì phải đi qua `L10n.tr` hoặc cơ chế tương đương của nền tảng, và có trong catalog.

Test riêng của fork (ví dụ `apps/android/test/localization_test.py`) có thể hỏng khi upstream đổi cấu trúc helper. Sửa assertion theo cấu trúc mới, giữ nguyên ý định kiểm tra.

### Phần do fork sở hữu

Các tệp và đoạn dưới đây là của fork CrossDesk Postcard. Khi upstream sửa cùng chỗ, lấy logic mới của upstream rồi áp lại phần của fork; tuyệt đối không để máy chủ của upstream (`api.crossdesk.cn`, `version.crossdesk.cn`, `downloads.crossdesk.cn`) hay kênh update của upstream quay lại bản build desktop.

| Tệp | Phần fork giữ |
| --- | --- |
| `AGENTS.md` | Toàn bộ tệp: quy tắc cho agent làm việc trên fork. |
| `apps/desktop/src/updater/update_source.h` | Toàn bộ tệp: manifest trên nhánh `desktop-updates`, trang release và URL asset `github.com/thuongtin/crossdesk/releases/download/v<version>/`, danh sách host redirect được phép. |
| `apps/desktop/src/version_checker/version_checker.cpp`, `version_checker.h` | Đọc manifest qua `update_source.h`, không gọi `version.crossdesk.cn`. Bản portable (Linux portable thiếu dấu `package-name` dùng mục gói thường cùng kiến trúc, `ManualUpdatePlatform`) và giao diện máy tính trên HarmonyOS (mục `harmony-arm64`, `kHarmonyUpdatePlatform`) chỉ đọc phiên bản trong `downloads` của đúng nền tảng (`ParsePlatformUpdateManifest`), không dùng phiên bản cấp cao nhất. Bản cài đặt có khóa công khai chỉ nhận mục đã ký (`ParseSignedUpdateManifest`); bản tự biên dịch không có khóa, hoặc bản Linux thiếu dấu `package-name` (dùng mục gói thường cùng kiến trúc), cũng chỉ đọc mục của nền tảng đó và chỉ tải thủ công (`ParseDesktopUpdateManifest`). |
| `apps/desktop/src/updater/update_core.cpp`, `update_core.h` | Chỉ tải gói từ release của fork, không dùng `downloads.crossdesk.cn`. Giữ thời gian tải tối đa 30 phút mỗi lượt như upstream. |
| `apps/desktop/src/version_checker/linux_update_download.cpp` | Tên asset `.deb` lấy qua `updater::ReleaseAssetUrl`, khớp tên asset do `scripts/release` phát hành. |
| `apps/desktop/src/config_center/config_center.h`, `config_center.cpp` | Signaling mặc định `crossdesk.ecom.io.vn` (hằng `kDefaultSignalServerHost` trong `.cpp`, giữ con trỏ `volatile` để chuỗi nằm liền trong binary Linux); cài đặt mới và giá trị ngôn ngữ hỏng mặc định English. |
| `apps/desktop/src/gui/runtime/runtime_state.h` | Không có giá trị mặc định `api.crossdesk.cn` / `9099`. |
| `apps/desktop/scripts/macos/pkg_arm64.sh` | Khối chuẩn hóa phiên bản: tag `v<MAJOR>.<MINOR>.<PATCH>-<BUILD>-<YYYYMMDD>` thành `CFBundleVersion` và `pkg-ref` dạng `<MAJOR>.<MINOR>.<PATCH*1000+BUILD>` (tối đa ba số nguyên, ví dụ `1.7.7003`), `CFBundleShortVersionString` là `<MAJOR>.<MINOR>.<PATCH>`. `scripts/release/build_macos.sh`, `make_release_feed.py` và `make_fork_update_manifest.py` kiểm cùng giá trị. |
| `apps/desktop/scripts/windows/nsis_script.nsi`, `installer_strings.nsh` | Bộ cài Windows: English đứng đầu và là ngôn ngữ dự phòng; bộ cài tự chọn theo ngôn ngữ giao diện Windows (macro `CROSSDESK_SELECT_LANGUAGE`, không có hộp chọn), zh-HK/zh-MO dùng tiếng Trung phồn thể. Danh sách `MUI_LANGUAGE` gồm mọi ngôn ngữ của app, thêm Turkish và Hebrew mà app chưa dịch (trên Windows dùng hai ngôn ngữ này, bộ cài hiện đúng ngôn ngữ còn app dùng ngôn ngữ dự phòng của nó); chuỗi tự viết là `LangString CD_*` trong `installer_strings.nsh`, khóa mới phải có đủ mọi ngôn ngữ. Bản cài mới vào `$PROGRAMFILES64\CrossDesk`; bản đã cài (kể cả `Program Files (x86)` của v1.7.7-3 trở về trước) được nâng cấp hoặc sửa tại chỗ, không hỏi thư mục (`LocateExistingInstall`, `SkipDirectoryPageOnUpgrade`). Thư mục truyền bằng `/D` (updater luôn truyền) luôn được dùng, kể cả khi trùng thư mục mặc định: NSIS cắt `$CMDLINE` tại ` /D=`, nên `$CMDLINE` ngắn hơn dòng lệnh của tiến trình nghĩa là có `/D`. Còn mục uninstall thì giữ thư mục đó kể cả khi `CrossDesk.exe` đã bị xoá; `InstallDir` trong HKCU chỉ được dùng khi `CrossDesk.exe` vẫn còn. Vì nâng cấp tương tác bỏ qua trang thư mục, `SkipDirectoryPageOnUpgrade` tự kiểm tra dung lượng trống thay cho trang đó, hỏi theo chính thư mục cài chứ không theo gốc ổ, vì thư mục có thể là điểm mount của một volume khác. Chỉ thư mục chưa tồn tại mới được hỏi qua thư mục cha gần nhất còn tồn tại, và đường dẫn luôn có dấu `\` ở cuối, vì `D:` trần là thư mục hiện hành trên ổ D: (có thể là điểm mount của volume khác) chứ không phải gốc ổ; lỗi khác (ví dụ access denied trên một điểm mount) thì bỏ qua kiểm tra, vì thư mục cha có thể nằm trên volume khác. Volume không đủ chỗ thì bộ cài báo lỗi và thoát trước khi đóng CrossDesk, dừng service hay ghi đè tệp. Mục uninstall vẫn ở view registry 32-bit vì `SupportedInstallation()` của updater đọc ở đó. Liên kết hỗ trợ (`PRODUCT_WEB_SITE`) trỏ về repo fork, chân trang là `BrandingText`. Khi cài và khi nâng cấp (kể cả bản cập nhật im lặng của updater), `AddFirewallRules` thêm hai quy tắc inbound Allow cho `$INSTDIR\CrossDesk.exe` (TCP và UDP, mọi profile Domain/Private/Public, tên hiển thị và group `CrossDesk`, ID bắt đầu bằng `CrossDesk Installer `), để Windows Defender Firewall không bao giờ hỏi có cho phép CrossDesk không; hàm chạy trước `RegisterInstalledService` vì service khởi động lại CrossDesk.exe khi truy cập không giám sát đang bật. Mỗi lần cài, hàm xoá trước các quy tắc có tiền tố đó cho cùng đường dẫn, nên nâng cấp vẫn chỉ còn đúng một cặp, và xoá luôn các quy tắc Allow `Query User` mà Windows thêm cho đường dẫn đó (cặp mới đã bao trùm chúng). Các quy tắc Block `Query User` thì được giữ, dù Block thắng Allow: Windows tạo chúng khi người dùng bấm Cancel ở hộp thoại tường lửa hoặc bỏ chọn một loại mạng, và bản cập nhật im lặng không được đảo ngược lựa chọn đó. Quy tắc của admin hay policy cũng được giữ. Thêm thất bại (ví dụ máy không có module NetSecurity) thì bộ cài chỉ ghi `CD_FirewallRulesNotAdded` vào danh sách chi tiết, không dừng cài đặt. Bộ cài không thêm exclusion nào cho Microsoft Defender Antivirus. Gỡ cài đặt (`un.RemoveMachineData`) dọn cả phần nằm ngoài thư mục cài: khóa `HKLM\SOFTWARE\CrossDesk` (view 64-bit, xoá sau `--service-uninstall` vì lệnh này ghi lại `Unattended\Enabled=0`; chỉ giữ dấu `UsbmmiddDriverInstalled` khi driver có thể vẫn còn, vì app không đặt lại dấu cho driver đã có sẵn, nên mất dấu thì không lần gỡ nào sau đó gỡ driver nữa; `deviceinstaller64.exe` là mẫu devcon: `remove` in một dòng cho mỗi thiết bị, kết thúc bằng "Removed", "Removed on reboot" hoặc "Remove failed", trả về 1 khi có thiết bị chờ khởi động lại kể cả khi thiết bị khác gỡ thất bại, và trả về 0 trong mọi trường hợp còn lại, kể cả khi mọi lần gỡ đều thất bại; vì vậy uninstaller chạy `remove` bằng `ExecToStack`, in lại từng dòng vào danh sách chi tiết (`un.PrintDriverRemoveOutput`) và giữ dấu khi có dòng "Remove failed"; sau mã 0, uninstaller chạy thêm `find usbmmidd` và chỉ bỏ dấu khi kết quả là "No matching devices found."; mã 1 không kèm dòng "Remove failed" nghĩa là Windows đã nhận lệnh gỡ mọi thiết bị còn lại và hoàn tất khi khởi động lại, nên dấu cũng được bỏ (driver cài sau lần khởi động lại đó không phải của CrossDesk); với mã 1, uninstaller ghi `CD_DriverRemovedOnRestart`, đặt reboot flag, và sau khi người dùng đóng uninstaller, `un.onUninstSuccess` nhắc người dùng tự khởi động lại bằng `CD_RestartToFinishUninstall`; uninstaller không bao giờ tự khởi động lại, vì lệnh `Reboot` của NSIS buộc đăng xuất mọi người dùng khác), `%ProgramData%\CrossDeskUnattended`, các thư mục `%ProgramData%\CrossDesk-update-*` mà lần cập nhật có quyền admin bị ngắt giữa chừng để lại, dữ liệu CrossDesk trong hồ sơ SYSTEM (`systemprofile\AppData\Local|Roaming\CrossDesk`, tắt WOW64 redirection để tới System32) và các quy tắc tường lửa cho `$INSTDIR\CrossDesk.exe`: quy tắc `AddFirewallRules` tạo và quy tắc Windows tự thêm khi người dùng trả lời hộp thoại tường lửa (ID bắt đầu bằng `TCP Query User{` hoặc `UDP Query User{`, có ở các bản cài trước khi có `AddFirewallRules` hoặc khi hàm đó thất bại), xoá bằng `Get-NetFirewallRule`/`Remove-NetFirewallRule` vì `netsh` không chọn được theo ID; quy tắc do admin hay policy tạo cho cùng đường dẫn được giữ, đường dẫn truyền qua biến môi trường `CROSSDESK_FIREWALL_PROGRAM` để không vỡ quoting). Các thư mục trong ProgramData chỉ bị xoá khi qua cùng phép kiểm tra như `PrivateDirectory` trong `unattended_config.cpp` (`un.RemovePrivateMachineDirectory`): thư mục thật, không phải junction, chủ sở hữu là SYSTEM hoặc Administrators, DACL chỉ cấp quyền cho hai tài khoản đó; vì user thường tạo được thư mục trong ProgramData và `RMDir /r` đi theo junction bên trong. Service và host không giám sát chạy bằng SYSTEM nên tạo thư mục có DACL chỉ cấp SYSTEM (ví dụ `CrossDeskUnattended\host`), uninstaller chạy bằng quyền admin không xoá được; vì vậy `un.SetBackupPrivileges` bật tạm `SeBackupPrivilege` và `SeRestorePrivilege` (admin có sẵn nhưng tắt) chỉ trong đoạn xoá các thư mục ProgramData đã qua kiểm tra và thư mục trong hồ sơ SYSTEM, rồi tắt lại; hàm trả về 0 khi `AdjustTokenPrivileges` báo `ERROR_NOT_ALL_ASSIGNED` (policy đã tước quyền) và uninstaller ghi `CD_BackupPrivilegesUnavailable`. Thư mục nào còn sau `RMDir /r` (`CD_MachineDataNotRemoved`), thư mục bị bỏ qua (`CD_MachineDataKept`), driver hay quy tắc tường lửa chưa gỡ được đều đặt `$UninstallIncomplete`; cuối phần gỡ, uninstaller mở danh sách chi tiết, báo `CD_UninstallIncomplete` (bỏ qua khi chạy `/S`) và đổi tiêu đề trang cuối thành `CD_UninstallIncompleteTitle`/`CD_UninstallIncompleteSubtitle` thay cho "Uninstallation Complete" của MUI (`MUI_INSTFILESPAGE_FINISHHEADER_TEXT` trỏ tới biến `$UninstallFinishTitle`, gán ở cuối section, nên các biến này phải khai báo trước `MUI_UNPAGE_INSTFILES`). Bộ cài nâng cấp không bao giờ chạy uninstaller nên cấu hình không giám sát được giữ khi cập nhật. |
| `apps/desktop/scripts/macos/make_fork_update_manifest.py`, `apps/desktop/docs/fork-self-update.md` | Toàn bộ tệp: manifest ký Ed25519 và quy tắc kênh update của fork. |
| Giao diện Postcard: `apps/desktop/src/gui/ui/postcard_controls.slint`, `main_window.slint`, `group_filter.slint`, `gallery_menu_button.slint`, `workbench_controls.slint` | Phong cách Postcard (`PostcardStyle`), tiêu đề "Kết nối với thiết bị", bố cục danh sách thiết bị. |
| Cửa sổ kết nối vừa màn hình nhỏ: `apps/desktop/src/gui/ui/main_window.slint` (`fit-scale`, `ui-scale`, `ui-width`, `ui-height`, bọc `ui-scaler`), `apps/desktop/src/gui/application/window_geometry.h` (`FitScale`), `gui_application.cpp` (`UpdateMainWindowFitScale`, nhân tọa độ kéo cửa sổ với `ui_scale`), `apps/desktop/tests/window_fit_scale_test.cpp` | Cửa sổ Postcard vốn cố định 960x650 nên bị cắt trên màn hình nhỏ hoặc scale cao. Máy chủ tính `fit-scale` (0.5 đến 1) từ vùng làm việc (trừ taskbar, thanh menu) của màn hình cửa sổ đang mở, giá trị này thu nhỏ kích thước tối thiểu và tối đa của cửa sổ; `ui-scaler` thu nhỏ toàn bộ canvas (cả hộp thoại, menu) theo kích thước thật. Mọi `root.width`/`root.height` nằm trong `ui-scaler` phải là `root.ui-width`/`root.ui-height`. Khi merge upstream mà `main_window.slint` xung đột, giữ wrapper này, không thụt lề lại phần bên trong để diff sau dễ merge. Chế độ DeskHub (`gallery-mode`) luôn có `ui-scale` bằng 1. |
| Lý do màn hình đen khi kết nối: `libs/wire/include/remote_action.h`, `libs/wire/src/remote_action.cpp` (`capture_status_request` = 1000, `capture_status` = 1001, `CaptureState`, cờ `supports_capture_status` trong `host_info`), `apps/desktop/src/screen_capturer/screen_capturer.h` (`ScreenCaptureHealth`, `GetCaptureHealth` thêm ở cuối interface), `apps/desktop/src/platform/macos/screen_capturer/screen_capturer_sck*`, `builtin_panel_wait.h` (`PlanCaptureDisplays`), `apps/desktop/tests/builtin_panel_wait_test.cpp`, `apps/desktop/src/gui/features/devices/session_device_manager.*` (`UpdateCaptureState`), `screen_sharing_power.*` (`WakeDisplay`), `apps/desktop/src/gui/runtime/peer_data_callbacks.cpp`, `peer_event_handler.cpp`, `remote_session.h`, `apps/desktop/src/gui/application/frame_wait_hint.h`, `gui_application.cpp` (`FrameWaitText`, `frame_wait_since`), `stream_window.slint` (chữ xuống dòng), `apps/desktop/tests/frame_wait_hint_test.cpp`, năm key `frame_wait_*` trong `localization/catalog.json` | Máy chủ theo dõi trạng thái ghi màn hình (thiếu quyền, màn hình ngủ, lỗi) và cứ 10 giây đánh thức lại màn hình khi nó đang ngủ. Máy xem chỉ hỏi trạng thái (mỗi giây, khi chưa có hình) nếu `host_info` báo `supports_capture_status`, vì bản desktop cũ ghi ERROR khi gặp loại tin lạ; mã 1000 và 1001 nằm xa dãy của upstream. Sau 3 giây máy xem hiện lý do máy chủ báo, sau 10 giây hiện lời nhắc chung thay vì "Đang nhận màn hình..." mãi mãi. Khi upstream thêm loại `ControlType` mới, giữ hai mã này và kiểm lại test `wire_test`. `Init` của `screen_capturer_sck_impl.mm` chọn slot màn hình qua `PlanCaptureDisplays`, cùng hành vi với `Init` trong upstream `bd41b106` nhưng có test; khi merge upstream mà chỗ này xung đột, giữ lời gọi `PlanCaptureDisplays`. `OnShareableContentCreated` chọn màn hình qua `ChooseCaptureDisplay`: màn hình đang mirror màn hình khác thì ghi nguồn mirror (hành vi của upstream `bd41b106`, nay có test), và chỉ coi màn hình là đang bật khi `CGDisplayIsOnline(...) == 1`, vì WindowServer trả `UINT32_MAX` cho màn hình đã bị gỡ (upstream `4b843226`). |
| Bố cục Settings và các hộp thoại mở từ Settings: `apps/desktop/src/gui/ui/main_window.slint` (nút "Edit" máy chủ tự host, "Manage" thiết bị tin cậy và quyền hệ thống, nút dịch vụ portable, đường dẫn lưu tệp, nút làm mới trạng thái tự khởi động, thẻ logo và nút OK trong About, cột trạng thái của hộp thoại cấp quyền, bề rộng hộp thoại cài đặt máy chủ tự host, `UiStrings.manage`, `UiStrings.manage-system-permissions`), `trusted_devices.slint` (nút sao chép thông tin ghép đôi), `ui_localization.h`, `apps/desktop/tests/slint_settings_layout_test.cpp`, target `slint_settings_layout_test` trong `apps/desktop/xmake/targets.lua` | Nút trong hàng cao 28px và căn giữa theo chiều dọc trong hàng 36px (`y: (parent.height - self.height) / 2`), không tràn qua đường kẻ hàng. Đợt làm mới Postcard từng nâng chúng lên 38px với `y: 4px` nên nút tràn 6px ra dưới hàng. Hàng có nút đặt `label-reserve` bằng bề rộng nút cộng 36px để nhãn dài bị rút gọn trước nút. Hai hàng "Trusted devices" và "System permissions" dùng nút ngắn `UiStrings.manage` (key `管理`); upstream dùng "Manage trusted devices" cho cả hai hàng, kể cả hàng quyền hệ thống. Trình đọc màn hình vẫn nghe đủ "Manage trusted devices" và "Manage system permissions" (key `管理系统权限`) qua `accessible-label`; chữ trên nút phải nằm trong tên đọc đó ở mọi ngôn ngữ để điều khiển bằng giọng nói vẫn tìm được nút (vì vậy bản ru của `管理` là «Управление», không phải «Управлять»). Nút có chữ dịch (làm mới trạng thái, OK trong About, sao chép thông tin ghép đôi) nở theo `label-width`. Thẻ logo About cao ít nhất 88px. Khi quyền đã cấp, cột trạng thái chỉ rộng bằng chữ "Granted". Hộp thoại cài đặt máy chủ tự host rộng tối thiểu 360px và nở theo tiêu đề (`self-host-title-metrics`), không vượt cửa sổ; tiêu đề quá dài thì elide (bản es, pt-BR từng bị cắt ở mép phải). Khi merge upstream, giữ các giá trị này. Test render `MainWindow` thật bằng software renderer ở 100%, 125%, 150% và vùng làm việc nhỏ (fit-scale gộp vào hệ số scale thiết bị vì software renderer bỏ qua `transform-scale`), với en, vi, de, và đo nút của cả năm hàng (Edit, thiết bị tin cậy, quyền hệ thống, dịch vụ portable, đường dẫn lưu tệp); test cũng kiểm ở cả 11 ngôn ngữ rằng chữ trên nút Manage (`UiStrings.manage`) nằm trong hai tên đọc (`TrustStrings.manage`, `UiStrings.manage-system-permissions`), không phân biệt hoa thường ASCII; đặt `CROSSDESK_SETTINGS_EVIDENCE_DIR` để xuất ảnh soát giao diện (thêm thư mục `all-languages` cho cả 11 ngôn ngữ, luôn ở 100%); `CROSSDESK_SETTINGS_DISPLAY` giới hạn test vào một cấu hình, tên không có trong danh sách thì test báo lỗi thay vì đạt mà không kiểm gì. |
| `apps/desktop/src/gui/ui/about_panel.slint`, `crossdesk_ui.slint`, `ui_localization.h`, `apps/desktop/src/gui/application/gui_application.cpp` | Không có dải tài trợ của upstream (`sponsor_strip.slint` đã xóa); liên kết "website" và tải thủ công mở trang release của fork. |
| `apps/desktop/src/gui/ui/feedback_panel.slint`, `main_window.slint`, `ui_localization.h`, `gui_application.cpp` | Bảng phản hồi không có nút GitHub (upstream mở `kunkundi/crossdesk/issues`), chỉ giữ xuất nhật ký chẩn đoán; lời nhắc và thông báo xuất xong bảo người dùng gửi tệp ZIP cho quản trị viên CrossDesk (hai key mới trong `localization/catalog.json`). Cổng conformance trượt nếu binary còn đường dẫn issue của upstream. |
| `README.md`, `README_EN.md`, `README_ZH.md`, `README_JA.md`, `docs/BUILD.md`, `docs/BUILD_EN.md`, `docs/FAQ.md`, `docs/UPDATER.md`, `localization/README.md`, `apps/mcp/README.md` | Liên kết tải bản máy tính và badge Release trỏ về Releases của fork; lệnh `git clone` trong phần biên dịch của README và `docs/BUILD*.md` lấy mã nguồn của fork (hướng dẫn iOS và MCP vẫn clone upstream vì fork không phát hành hai ứng dụng này); báo lỗi và gửi nhật ký cho quản trị viên CrossDesk, không có liên kết `kunkundi/crossdesk/issues`; `docs/UPDATER.md` mô tả nguồn cập nhật và origin tải gói của fork. |
| `apps/cli/package.json` | Trường `bugs` trỏ tới mục tài liệu và phản hồi trong `README_EN.md` của fork (báo lỗi cho quản trị viên CrossDesk), không trỏ tới `kunkundi/crossdesk/issues`; `repository` và `homepage` vẫn ghi nguồn upstream. Gói npm của CLI chỉ phát hành từ upstream. |
| `apps/desktop/src/network/lan/lan_hosts_file.h`, `apps/desktop/src/network/lan/lan_transport.cpp` (`NameLookup`, `Resolved`, `DialableAddress`), `apps/desktop/tests/lan_hosts_file_test.cpp` | Đọc `/etc/hosts` trước cho đúng tên đã nhập (giữ thứ tự “files trước dns” của nsswitch); chỉ khi file không có địa chỉ quay số được (cùng điều kiện `DialableAddress` với `QueueAddresses`, nên dòng chỉ có địa chỉ IPv4-mapped không chặn DNS) mới hỏi resolver, và luôn bằng tên tuyệt đối (`name.`): không ghép search suffix, không hỏi DNS bằng tên tương đối. Backend `files` của glibc không khớp `name.` với hosts file; hỏi lại DNS bằng tên tương đối thì search suffix có thể chuyển hướng một kết nối chưa ghim khóa máy chủ. Pool luồng tra cứu được cấp phát và cố ý không huỷ (`*new asio::thread_pool`), để một `getaddrinfo` đang treo không giữ tiến trình sống sau khi thoát (https://github.com/thuongtin/crossdesk/pull/7). |
| `apps/desktop/src/thumbnail/thumbnail.cpp`, `thumbnail.h` | Ảnh xem trước giữ trọn khung hình (fit), không cắt (cover). |
| `deps/thirdparty/slint/xmake.lua` (config `system_fontconfig` và patch Linux `1.17.1-fontconfig-dlopen.patch`), `deps/thirdparty/slint/patches/1.17.1-fontconfig-dlopen.patch` | Runtime Slint trên Linux dlopen `libfontconfig.so.1` của hệ thống (feature `fontconfig-dlopen` của `fontique`, cùng cách `api/rs/build` của Slint làm). Upstream link tĩnh fontconfig riêng do xmake build, bản đó đọc `fonts.conf` dưới thư mục cài trên máy build; máy người dùng không có tệp đó nên sans-serif rơi về phông serif (C059) ở mọi ngôn ngữ. Phụ thuộc `fontconfig` riêng của upstream vẫn giữ cho `slint-compiler` lúc build. Khi upstream nâng Slint, làm lại patch cho phiên bản mới (Cargo.toml của `api/cpp` và mục `slint-cpp` trong `Cargo.lock`), giữ config `system_fontconfig` để không dùng lại gói Slint cũ trong cache. Cổng `check_conformance.py` trượt khi `libslint_cpp.so` trong gói `.deb` không chứa `libfontconfig.so.1` hoặc còn đường dẫn `/packages/f/fontconfig/`. |
| `apps/desktop/scripts/linux/package_deb.sh` (`GUI_RUNTIME_DEPS` trong `Depends`) | Thư viện mà runtime Slint và SDL nạp bằng `dlopen` (winit X11 `libx11-xcb1`, `xkbcommon-dl` cần `libxkbcommon0` và `libxkbcommon-x11-0`, EGL của glutin, các backend Wayland, fontconfig hệ thống của `fontique`), `zlib1g` mà `crossdesk-bin` liên kết và `ca-certificates` cho TLS được khai báo trong `Depends` của cả hai gói. `ldd` và dpkg-shlibdeps không thấy thư viện nạp động; thiếu `libxkbcommon-x11-0` thì bản `v1.7.7-4-20261128` dừng ngay lúc mở trên Ubuntu tối giản. Khi upstream sửa dòng `Depends`, giữ `$GUI_RUNTIME_DEPS`; cổng `check_conformance.py` (`LINUX_RUNTIME_DEPENDS`) trượt khi gói thiếu một trong các tên này. |
| Hộp thoại Release Notes hiện ghi chú của fork: `apps/desktop/xmake/options.lua` (option `CROSSDESK_RELEASE_NOTES`), `apps/desktop/xmake/rules/about_resources.lua` (mảng `kInstalledReleaseNotes`, bỏ `releaseNotes` khỏi `kAboutResources`), `apps/desktop/src/gui/features/about/about_catalog.cpp`, `about_catalog.h` (`InstalledReleaseNotes`, `HasInstalledReleaseNotes`, `SetInstalledReleaseNotes`), `main_window.slint` (`installed-release-notes` kiểu `[ReleaseNoteBlock]`, khối `StyledText` trong hộp thoại), `gui_application.cpp` (chỉ đặt `release-welcome-pending` khi bản build có notes), `apps/desktop/tests/slint_popup_render_test.cpp` | Hộp thoại mở lần đầu sau khi cài hoặc nâng cấp hiện mục `## Changes` của release notes fork mà `scripts/release/release.sh` nhúng lúc build (`--CROSSDESK_RELEASE_NOTES`, xem bước 3 của `docs/RELEASE.md`), hiển thị Markdown bằng `release_notes_text::Parse` như lời nhắc cập nhật, không dùng `apps/desktop/resources/release-notes.json` của upstream. Bản build không có notes không mở hộp thoại. Khi upstream đổi hộp thoại hay `release-notes.json`, giữ nguồn notes này; cổng `check_conformance.py --notes` trượt khi file thực thi chính không chứa nguyên văn mục đó. |
| `apps/desktop/tests/slint_popup_render_test.cpp`, `slint_ui_smoke_test.cpp`, `version_checker_test.cpp` | Test cho giao diện Postcard, việc không còn dải tài trợ và kênh update của fork. |
| `localization/catalog.json` | Key `连接到设备`, `管理`, `管理系统权限`, bản `en` "This device" (`本桌面`) và "Recent Devices" (`近期连接`). Bản dịch fork sửa so với upstream: `关于` (de "Über"), `已授权` (de "Erteilt"), `确定` (ko "확인"), nút chế độ `ID 连接` / `IP 连接` rút gọn cho vi, fr, es để không bị cắt, khoảng trắng quanh "GNU GPL v3" và "CeCILL-C 1.0" trong chuỗi giấy phép, tiêu đề nhóm `启动与后台` (ja, ko, es, pt-BR từng là bản dịch máy sai nghĩa "khởi nghiệp", "bối cảnh", "tiền sử"), dấu phẩy thiếu khoảng trắng trong lời giải thích dịch vụ khóa màn hình (fr, es, pt-BR) và khoảng trắng quanh "CrossDesk" trong các chuỗi ko. Khi script merge báo xung đột ở các key này, giữ bản của fork trừ khi upstream đã tự sửa đúng. Cột `vi` phải đầy đủ; English vẫn là mặc định cho bản cài mới của desktop. |
| `scripts/release/*`, `docs/RELEASE.md` | Quy trình phát hành ba nền tảng, cổng `check_conformance.py` và public key ký manifest; `installed_release_notes.py` tách mục `## Changes` để nhúng vào hộp thoại Release Notes. |
| `scripts/sync-upstream.sh`, `scripts/merge_localization_catalog.py`, tài liệu này | Công cụ và quy trình đồng bộ. |

Sau mỗi lần đồng bộ, build desktop rồi chạy cổng `scripts/release/check_conformance.py` trên binary (xem `docs/RELEASE.md`): cổng này báo lỗi khi binary thiếu đường dẫn manifest hay signaling của fork, hoặc còn host hay trang issue của upstream.

### Catalog bản địa hóa

`localization/catalog.json` là nguồn duy nhất cho mọi chuỗi giao diện. Không sửa tay phần xung đột; dùng script merge ba chiều:

```bash
python3 scripts/merge_localization_catalog.py --dry-run   # chỉ báo cáo
python3 scripts/merge_localization_catalog.py             # ghi catalog.json
```

Script đọc ba phiên bản từ git (merge base, `HEAD`, `upstream/mcp`) và:

- giữ thứ tự key và mọi thay đổi của upstream; key chỉ có ở fork được chèn sau key đứng trước nó trong bản fork;
- merge riêng từng ngôn ngữ của một chuỗi; hai bên cùng đổi khác nhau thì báo xung đột và tạm giữ bản upstream;
- giữ nguyên danh sách `slint` của upstream (đây là bảng tra theo chỉ số), nối literal chỉ có ở fork vào cuối, rồi đánh lại chỉ số `GalleryStrings.texts[N]` trong các tệp `.slint` chỉ fork dùng;
- in danh sách chuỗi còn thiếu bản dịch `vi`.

Mã thoát `1` nghĩa là còn xung đột giá trị, key bị tham chiếu nhưng không tồn tại, hoặc một tệp `.slint` mà cả hai bên cùng thêm chỉ số mới; các trường hợp này phải xử lý tay.

Sau đó dịch mọi chuỗi còn thiếu `vi` trong catalog và sinh lại tài nguyên:

```bash
python3 localization/generate.py
python3 localization/generate.py --check
```

Tài nguyên sinh tự động (Android `strings.xml`, `LocalizationKeys.java`, `locales_config.xml`, iOS `Localizable.strings` và `InfoPlist.strings`, desktop `localization_data.h`, HarmonyOS `LocalizationData.ets`) không bao giờ sửa tay: khi xung đột, chọn bên nào cũng được rồi chạy lại trình sinh.

Ghi chú cập nhật trong `apps/desktop/resources/release-notes.json` không đi qua catalog và bản fork không còn hiển thị tệp này: hộp thoại Release Notes hiện mục `## Changes` của release notes fork (bảng phần do fork sở hữu ở trên). Khi merge, cứ lấy bản của upstream, không cần dịch mục `vi`.

### Tài liệu

| Loại tệp | Cách xử lý |
| --- | --- |
| `README.md`, `docs/*.md` không có hậu tố | Bản tiếng Việt. Dịch nội dung mới của upstream sang tiếng Việt, giữ cấu trúc và anchor của upstream. |
| `README_EN.md`, `docs/*_EN.md` | Khớp với bản tiếng Anh của upstream (`README_EN.md` của fork là `README.md` của upstream). |
| `README_ZH.md` | Bản tiếng Trung của upstream. |
| `README_JA.md` | Bản tiếng Nhật của upstream; liên kết tải, badge Release và phần phản hồi theo fork, nhãn liên kết tới `docs/*.md` ghi là bản tiếng Việt; đường dẫn ghép đôi ghi **信頼済みデバイス → 管理** như hàng Settings của fork (PR 9). |
| README trong `apps/*` | Bản tiếng Việt; dịch nội dung mới của upstream. |

Quy tắc liên kết:

- Tài liệu tiếng Việt liên kết tới `../README.md`; tài liệu `_EN` liên kết tới `../README_EN.md`.
- README trong `apps/*` giữ đích `../../README.md`.
- Nhãn và tên mục trong giao diện được viết theo đúng bản dịch trong catalog, để người đọc tìm thấy đúng nút trên ứng dụng.
- Sau khi dịch, kiểm tra mọi liên kết tương đối và anchor vẫn trỏ tới tệp và tiêu đề có thật.

## Danh sách kiểm tra

- Không còn dấu xung đột và tệp chưa merge:

  ```bash
  git diff --name-only --diff-filter=U
  rg -n '^(<<<<<<<|>>>>>>>|=======$)' --glob '!*.png'
  ```

- Tệp JSON, XML và plist đã đổi vẫn đọc được (`python3 -m json.tool`, `xmllint --noout`, `plutil -lint`).
- `python3 localization/generate.py --check` báo tài nguyên đã khớp.
- Toàn bộ test Python chạy được trên máy không có SDK nền tảng:

  ```bash
  for test in $(git ls-files '*_test.py' '*/test_*.py' 'localization/test_generate.py' | sort -u); do
    python3 "$test" || echo "FAILED: $test"
  done
  ```

- Test `.cjs` của HarmonyOS. Không có DevEco Studio thì cài TypeScript vào một thư mục tạm và truyền đường dẫn `typescript.js`:

  ```bash
  npm install --prefix "$TMPDIR/ts" typescript@5.6
  ts="$TMPDIR/ts/node_modules/typescript/lib/typescript.js"
  for test in localization privacy_policy mobile_localization mobile_models mobile_lifecycle mobile_platform mobile_store; do
    node "apps/harmony/tests/${test}_test.cjs" "$ts" apps/harmony/entry/src/main/ets/mobile
  done
  node apps/harmony/tests/text_input_host_test.cjs "$ts" apps/harmony/desktop-port/TextInputHost.ets \
    apps/harmony/desktop-port/KeyboardHost.ets apps/harmony/desktop-port/DesktopHost.ets apps/harmony/desktop-port/DesktopAccess.ets
  node apps/harmony/tests/desktop_window_host_test.cjs "$ts" apps/harmony/desktop-port/DesktopHost.ets
  node apps/harmony/tests/autostart_host_test.cjs "$ts" apps/harmony/desktop-port/AutostartHost.ets
  ```

- Quét ký tự Han trong các tệp đã đổi: mọi dòng còn chữ Han phải là key catalog, dữ liệu locale, fixture test hoặc tên thương hiệu, không phải chuỗi giao diện hay bình luận chưa dịch.
- Kiểm tra liên kết và anchor Markdown trong các tệp tài liệu đã đổi.
- Build desktop đầy đủ trên cả ba máy của dự án (macOS arm64, Windows x64, Linux x64 trong WSL), ví dụ bằng chế độ dry-run của `scripts/release/release.sh <version> --ref <commit>`, hoặc `xmake` theo [Hướng dẫn biên dịch](BUILD.md).

## Quy tắc PR

- Mở PR từ nhánh đồng bộ vào `mcp` của fork, và kiểm tra lệnh trước khi chạy:

  ```bash
  git push origin sync/upstream-YYYY-MM-DD
  gh pr create -R thuongtin/crossdesk --base mcp --head sync/upstream-YYYY-MM-DD --title "Sync upstream kunkundi/crossdesk mcp (<base>..<upstream>)"
  gh pr view -R thuongtin/crossdesk --json url,baseRepository
  ```

  `baseRepository` phải là `thuongtin/crossdesk`. PR mở nhầm lên `kunkundi/crossdesk` là lỗi nghiêm trọng, cần đóng ngay.
- Tiêu đề viết bằng tiếng Anh. Phần mô tả nêu dải commit upstream, cách giải quyết từng nhóm xung đột, các chuỗi mới đã dịch, kết quả build và test, và những điểm còn chưa chắc chắn.
- Cổng merge thay cho CI (check CI bị hủy là bình thường):
  - Codex review sạch trên đúng head, không còn thread review chưa giải quyết;
  - build desktop đầy đủ trên macOS arm64, Windows x64 và Linux x64 (WSL) cho đúng head sẽ merge;
  - một comment trên PR liệt kê từng máy, commit và kết quả pass/fail, không kèm tên người, Team ID hay bí mật.
- Merge bằng `gh pr merge <số PR> -R thuongtin/crossdesk --merge` (không squash), chỉ thêm `--admin` khi một check bắt buộc chặn merge.

## Lịch sử đồng bộ

| Ngày | Dải upstream | Số commit | Ghi chú |
| --- | --- | --- | --- |
| 2026-11-26 | `ff369cd0..2c046f51` | 106 | 34 tệp xung đột: catalog, 3 tài nguyên sinh, 24 tài liệu, 6 tệp code. Dịch 16 chuỗi mới, thêm mục `vi` cho ghi chú cập nhật. Thêm script merge catalog và script xem trước này. |
| 2026-11-28 | `2c046f51..26b56503` | 59 | 40 tệp xung đột: catalog và `generate.py`, 15 tài nguyên sinh, 10 tài liệu, 13 tệp code. Upstream thêm tiếng Thổ Nhĩ Kỳ và tiếng Hebrew: tiếng Việt dời sang cột 12, “Theo hệ thống” sang 13, giá trị cấu hình cũ giữ nguyên. Dịch 24 chuỗi mới; thêm `README_JA.md` theo chính sách tài liệu của fork. Sau đó merge lại `mcp` (PR 9, PR 10): script merge catalog dừng vì danh sách `slint` đã đổi thứ tự, nên áp tay phần thay đổi của `mcp` lên catalog đã đồng bộ; nút chế độ tiếng Việt giữ "Theo ID" / "Theo IP" (ngắn hơn "Chế độ ID", cùng kiểu với tr, he), thêm tr và he cho `管理`, `管理系统权限`; `slint_settings_layout_test` tra chỉ số ngôn ngữ theo mã (số 11 cũ của tiếng Việt nay là tiếng Hebrew) và phủ thêm tr, he. Merge thêm `mcp` sau PR 8 (lý do màn hình đen): 7 tệp xung đột; `screen_capturer_sck_impl.mm` giữ mô hình `PlanCaptureDisplays` / `ChooseCaptureDisplay` của PR 8 và đưa nguồn mirror của upstream vào `ChooseCaptureDisplay`; `screen_capturer.h`, `session_device_manager.h`, `targets.lua` giữ cả hai bên; dịch tr, he cho năm chuỗi `frame_wait_*`. |
