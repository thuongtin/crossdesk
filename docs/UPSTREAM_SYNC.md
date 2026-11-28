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

Lấy hành vi của upstream, rồi chỉ áp lại phần Việt hóa: chuỗi hiển thị đi qua catalog, giá trị ngôn ngữ mặc định và các chỉ số ngôn ngữ của fork. Trên desktop, `kVietnameseLanguage = 11` và `kSystemLanguage = 10`. Không giữ lại logic cũ của fork chỉ vì nó từng nằm ở đó.

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
| `apps/desktop/scripts/macos/make_fork_update_manifest.py`, `apps/desktop/docs/fork-self-update.md` | Toàn bộ tệp: manifest ký Ed25519 và quy tắc kênh update của fork. |
| Giao diện Postcard: `apps/desktop/src/gui/ui/postcard_controls.slint`, `main_window.slint`, `group_filter.slint`, `gallery_menu_button.slint`, `workbench_controls.slint` | Phong cách Postcard (`PostcardStyle`), tiêu đề "Kết nối với thiết bị", bố cục danh sách thiết bị. |
| Cửa sổ kết nối vừa màn hình nhỏ: `apps/desktop/src/gui/ui/main_window.slint` (`fit-scale`, `ui-scale`, `ui-width`, `ui-height`, bọc `ui-scaler`), `apps/desktop/src/gui/application/window_geometry.h` (`FitScale`), `gui_application.cpp` (`UpdateMainWindowFitScale`, nhân tọa độ kéo cửa sổ với `ui_scale`), `apps/desktop/tests/window_fit_scale_test.cpp` | Cửa sổ Postcard vốn cố định 960x650 nên bị cắt trên màn hình nhỏ hoặc scale cao. Máy chủ tính `fit-scale` (0.5 đến 1) từ vùng làm việc (trừ taskbar, thanh menu) của màn hình cửa sổ đang mở, giá trị này thu nhỏ kích thước tối thiểu và tối đa của cửa sổ; `ui-scaler` thu nhỏ toàn bộ canvas (cả hộp thoại, menu) theo kích thước thật. Mọi `root.width`/`root.height` nằm trong `ui-scaler` phải là `root.ui-width`/`root.ui-height`. Khi merge upstream mà `main_window.slint` xung đột, giữ wrapper này, không thụt lề lại phần bên trong để diff sau dễ merge. Chế độ DeskHub (`gallery-mode`) luôn có `ui-scale` bằng 1. |
| Lý do màn hình đen khi kết nối: `libs/wire/include/remote_action.h`, `libs/wire/src/remote_action.cpp` (`capture_status_request` = 1000, `capture_status` = 1001, `CaptureState`, cờ `supports_capture_status` trong `host_info`), `apps/desktop/src/screen_capturer/screen_capturer.h` (`ScreenCaptureHealth`, `GetCaptureHealth` thêm ở cuối interface), `apps/desktop/src/platform/macos/screen_capturer/screen_capturer_sck*`, `apps/desktop/src/gui/features/devices/session_device_manager.*` (`UpdateCaptureState`), `screen_sharing_power.*` (`WakeDisplay`), `apps/desktop/src/gui/runtime/peer_data_callbacks.cpp`, `peer_event_handler.cpp`, `remote_session.h`, `apps/desktop/src/gui/application/frame_wait_hint.h`, `gui_application.cpp` (`FrameWaitText`, `frame_wait_since`), `stream_window.slint` (chữ xuống dòng), `apps/desktop/tests/frame_wait_hint_test.cpp`, năm key `frame_wait_*` trong `localization/catalog.json` | Máy chủ theo dõi trạng thái ghi màn hình (thiếu quyền, màn hình ngủ, lỗi) và cứ 10 giây đánh thức lại màn hình khi nó đang ngủ. Máy xem chỉ hỏi trạng thái (mỗi giây, khi chưa có hình) nếu `host_info` báo `supports_capture_status`, vì bản desktop cũ ghi ERROR khi gặp loại tin lạ; mã 1000 và 1001 nằm xa dãy của upstream. Sau 3 giây máy xem hiện lý do máy chủ báo, sau 10 giây hiện lời nhắc chung thay vì "Đang nhận màn hình..." mãi mãi. Khi upstream thêm loại `ControlType` mới, giữ hai mã này và kiểm lại test `wire_test`. |
| `apps/desktop/src/gui/ui/about_panel.slint`, `crossdesk_ui.slint`, `ui_localization.h`, `apps/desktop/src/gui/application/gui_application.cpp` | Không có dải tài trợ của upstream (`sponsor_strip.slint` đã xóa); liên kết "website" và tải thủ công mở trang release của fork. |
| `apps/desktop/src/gui/ui/feedback_panel.slint`, `main_window.slint`, `ui_localization.h`, `gui_application.cpp` | Bảng phản hồi không có nút GitHub (upstream mở `kunkundi/crossdesk/issues`), chỉ giữ xuất nhật ký chẩn đoán; lời nhắc và thông báo xuất xong bảo người dùng gửi tệp ZIP cho quản trị viên CrossDesk (hai key mới trong `localization/catalog.json`). Cổng conformance trượt nếu binary còn đường dẫn issue của upstream. |
| `README.md`, `README_EN.md`, `README_ZH.md`, `docs/FAQ.md`, `docs/UPDATER.md`, `localization/README.md`, `apps/mcp/README.md` | Liên kết tải bản máy tính và badge Release trỏ về Releases của fork; báo lỗi và gửi nhật ký cho quản trị viên CrossDesk, không có liên kết `kunkundi/crossdesk/issues`; `docs/UPDATER.md` mô tả nguồn cập nhật và origin tải gói của fork. |
| `apps/cli/package.json` | Trường `bugs` trỏ tới mục tài liệu và phản hồi trong `README_EN.md` của fork (báo lỗi cho quản trị viên CrossDesk), không trỏ tới `kunkundi/crossdesk/issues`; `repository` và `homepage` vẫn ghi nguồn upstream. Gói npm của CLI chỉ phát hành từ upstream. |
| `apps/desktop/src/thumbnail/thumbnail.cpp`, `thumbnail.h` | Ảnh xem trước giữ trọn khung hình (fit), không cắt (cover). |
| `apps/desktop/tests/slint_popup_render_test.cpp`, `slint_ui_smoke_test.cpp`, `version_checker_test.cpp` | Test cho giao diện Postcard, việc không còn dải tài trợ và kênh update của fork. |
| `localization/catalog.json` | Key `连接到设备`, bản `en` "This device" (`本桌面`) và "Recent Devices" (`近期连接`). Cột `vi` phải đầy đủ; English vẫn là mặc định cho bản cài mới của desktop. |
| `scripts/release/*`, `docs/RELEASE.md` | Quy trình phát hành ba nền tảng, cổng `check_conformance.py` và public key ký manifest. |
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

Ghi chú cập nhật trong `apps/desktop/resources/release-notes.json` không đi qua catalog. Khi upstream đổi nội dung, viết lại mục `vi` cho khớp với bản mới.

### Tài liệu

| Loại tệp | Cách xử lý |
| --- | --- |
| `README.md`, `docs/*.md` không có hậu tố | Bản tiếng Việt. Dịch nội dung mới của upstream sang tiếng Việt, giữ cấu trúc và anchor của upstream. |
| `README_EN.md`, `docs/*_EN.md` | Khớp với bản tiếng Anh của upstream (`README_EN.md` của fork là `README.md` của upstream). |
| `README_ZH.md` | Bản tiếng Trung của upstream. |
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
