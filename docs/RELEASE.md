# Phát hành CrossDesk Postcard (macOS, Windows, Linux)

Tài liệu này mô tả quy trình phát hành lặp lại được cho fork `thuongtin/crossdesk`. Mọi bước
đều chạy bằng `scripts/release/release.sh` từ một máy Mac Apple Silicon; bản Windows và Linux
được build trên máy từ xa qua SSH rồi chép về bằng `scp`.

- Mặc định là **dry-run**: build, ký (nếu đã cấu hình), đóng gói, chạy fork-conformance gate và
  tạo checksum trong `dist/<version>/`. Không tạo tag, release, không sửa feed.
- Chỉ khi truyền `--publish` mới tạo GitHub release trên `thuongtin/crossdesk` và cập nhật feed
  `crossdesk-desktop-update.json` trên nhánh `desktop-updates`.
- Upstream `kunkundi/crossdesk` chỉ để đọc. Script không bao giờ ghi vào upstream; mã nguồn được
  clone từ `https://github.com/thuongtin/crossdesk` (submodule `minirtc` vẫn lấy theo URL ghi trong
  `.gitmodules`, chỉ đọc).

## 1. Thành phần

| File | Việc làm |
| --- | --- |
| `scripts/release/release.sh` | Điểm vào: chọn commit, build song song ba nền tảng, gate, checksum, publish |
| `scripts/release/build_macos.sh` | Build, ký, notarize và kiểm tra gói `.pkg` ngay trên máy Mac |
| `scripts/release/build_windows.sh` + `windows/build.ps1` | Build bộ cài NSIS trên máy Windows qua SSH |
| `scripts/release/build_linux.sh` + `linux/*` | Build hai gói `.deb` trong bubblewrap trên máy Linux qua SSH |
| `scripts/release/installed_release_notes.py` | Tách mục `## Changes` của release notes để nhúng vào hộp thoại Release Notes của app |
| `scripts/release/test_installed_release_notes.py` | Unit test của `installed_release_notes.py` (xem bước 3 của mục 4) |
| `scripts/release/check_conformance.py` | Fork-conformance gate trên file thực thi trong từng gói |
| `scripts/release/make_release_feed.py` | Ký payload Ed25519 và sinh feed mới, không push |
| `scripts/release/publish.sh` | Tạo release, đọc lại asset, công bố, rồi mới cập nhật feed |
| `scripts/release/release.env.example` | Mẫu cấu hình máy build và tên identity ký |

Asset của một release `v<VERSION>`:

| Nền tảng | Asset | Khoá trong feed |
| --- | --- | --- |
| macOS 14+ arm64 | `crossdesk-macos-arm64-v<VERSION>.pkg` | `macos-arm64` |
| Windows x64 | `crossdesk-win-x64-v<VERSION>.exe` | `windows-x64` |
| Linux amd64 | `crossdesk-linux-amd64-v<VERSION>.deb` | `linux-amd64` |
| Linux amd64 (desktop ảo XFCE) | `crossdesk-virtual-desktop-xfce-linux-amd64-v<VERSION>.deb` | `linux-amd64-virtual-desktop-xfce` |
| Checksum | `SHA256SUMS.txt` | |

Tên asset và khoá feed phải khớp `ExpectedFilename()` và `PlatformKey()` trong
`apps/desktop/src/updater/update_core.cpp`, nên Linux dùng `amd64` (không phải `x64`).

## 2. Chuẩn bị một lần

### 2.1. Máy Mac chạy release

- Xcode (có `xcrun`, `productbuild`, `notarytool`, `stapler`), Homebrew, `xmake`, `gh` đã
  `gh auth login` với quyền ghi vào `thuongtin/crossdesk`.
- `pkg-config` trên `PATH` (`brew install pkgconf`). Runner macOS của GitHub có sẵn, máy Mac
  thường thì không; thiếu nó thì meson của gói libsoup không tìm thấy glib và build dừng.
- OpenSSL 3 trở lên đứng trước trên `PATH` (`brew install openssl@3`). `/usr/bin/openssl` của
  macOS là LibreSSL, không có `pkeyutl -rawin` nên không ký feed được.
- Hai identity cùng Team ID trong một keychain thuộc search list của user (keychain đăng nhập
  hoặc keychain riêng):
  - "Developer ID Application" để ký app (`security find-identity -v -p codesigning`).
  - "Developer ID Installer" để ký gói `.pkg` (`security find-identity -v -p basic`). Thiếu
    identity này thì `--publish` dừng, dry-run chỉ build gói chưa ký.
- **Không ghi họ tên hay Team ID vào file commit nào** (script, tài liệu, CI, PR, commit message).
  Đặt `CROSSDESK_MACOS_SIGN_IDENTITY=auto` và `CROSSDESK_MACOS_INSTALLER_IDENTITY=auto` trong
  `release.env` để `build_macos.sh` tự lấy tên lúc chạy; nhiều identity cùng loại thì nó dừng và
  khi đó mới ghi tên đầy đủ vào `release.env` (file ngoài repo). Họ tên và Team ID được thay bằng
  `<redacted>` trong log; `gate/macos-signing.txt` chỉ ghi kết quả kiểm.
- Keychain riêng có thể khoá lại sau mỗi lần khởi động máy. Chủ máy tự mở bằng
  `security unlock-keychain <keychain>`; script không hỏi hay gõ mật khẩu. Trước khi ký,
  `build_macos.sh` kiểm trạng thái khoá mà không bật hộp thoại (trong phiên GUI, codesign trên
  keychain đang khoá sẽ bật hộp thoại xin mở khoá thay vì báo `errSecInternalComponent`), rồi thử
  ký một bản sao `/usr/bin/true` trước khi build.
- Notarize bằng **khoá API App Store Connect** (cách chính): file `.p8` nằm ngoài repo, cùng Key ID
  và Issuer ID, khai báo qua `CROSSDESK_MACOS_NOTARY_KEY`, `CROSSDESK_MACOS_NOTARY_KEY_ID`,
  `CROSSDESK_MACOS_NOTARY_ISSUER` trong `release.env`. Có thể nạp từ một file env riêng ngoài repo
  (xem `release.env.example`). Kiểm tra bằng
  `xcrun notarytool history --key <file.p8> --key-id <KEYID> --issuer <ISSUER_ID>`.
  Profile notarytool (`CROSSDESK_MACOS_NOTARY_PROFILE`) chỉ là phương án dự phòng khi không đặt
  khoá API, vì profile trong keychain từng mất sau khi khởi động lại máy.
- Trước khi gửi Apple, `build_macos.sh` kiểm mọi Mach-O trong app mang Team ID của identity và có
  Hardened Runtime (log chỉ ghi `ok`/`MISMATCH`). `sign_app.sh` ký từ trong ra ngoài, không dùng
  `codesign --deep` để ký.
- Khoá bí mật Ed25519 ký feed cập nhật cũng nằm trên máy này (xem mục 2.5).

### 2.2. Máy Windows x64

- OpenSSH Server, đăng nhập bằng khoá (script dùng `BatchMode=yes`).
- `git`, Visual Studio 2022 có workload C++, `xmake`, NSIS (`makensis` trên `PATH`), LLVM
  (`clang-cl`, cần cho `libyuv`), Windows PowerShell 5.1.
- Thư mục build riêng `C:\build\crossdesk` (không dùng chung với bản clone làm việc). Script tự
  clone fork vào `src`, đặt plugin NSIS (INetC ghim SHA-256) trong `tools` và nạp qua
  `NSISCONFDIR`, nên không cần quyền admin. CUDA 12.6.3 lấy qua gói xmake như CI upstream.
- Bộ cài hiện **chưa ký Authenticode** (fork chưa có chứng thư số). Windows SmartScreen sẽ cảnh
  báo khi cài tay. Trình cập nhật trong app vẫn kiểm chữ ký Ed25519, kích thước và SHA-256 của
  feed nên bản tải về qua app vẫn được xác thực. File `gate/windows-signing.txt` ghi trạng thái
  Authenticode của từng lần build.

### 2.3. Máy Linux x86_64

- WSL2 hoặc máy thật, có `git`, `python3`, `bubblewrap` (`bwrap`), `flock`, `sha256sum` và cho
  phép user namespace không đặc quyền (`unshare -r id` chạy được). Không cần Docker hay `sudo`.
- Lần đầu, `linux/fetch_image_rootfs.py` tải image build của upstream
  `crossdesk/ubuntu20.04:buildenv-cuda12.6.3-gcc10-cmake3.31.6-pipewire0.3.48-xmake3.1.1-rust1.92.0`
  theo đúng digest ghim trong `lib.sh`, kiểm SHA-256 từng layer rồi bung thành rootfs (khoảng 13 GB)
  trong `~/build/crossdesk/rootfs/<digest>`. Lần build đầu cũng cài bổ sung `libdbus-1-dev`,
  `libdrm-dev`, `libfuse-dev`, `libfuse3-dev` vào rootfs giống bước "Ensure Wayland and clipboard
  build dependencies" của CI. Dự trù khoảng 40 GB trống.
- Build chạy trong `bwrap` với uid 0 giả lập (user namespace), mạng của máy, mã nguồn ở
  `/workspace`. Gói `.deb` được kiểm mức glibc 2.31 và `apt-get install --simulate` trên Ubuntu
  20.04 như CI (kết quả trong `gate/linux/`).
- Muốn đổi image, cập nhật `LINUX_BUILD_IMAGE_*` trong `lib.sh` (lấy digest manifest
  `linux/amd64` mới) rồi xoá rootfs cũ.

### 2.4. File cấu hình

```bash
mkdir -p ~/.config/crossdesk
cp scripts/release/release.env.example ~/.config/crossdesk/release.env
chmod 600 ~/.config/crossdesk/release.env
# Sửa host SSH, thư mục, tên identity và đường dẫn khoá cho đúng máy của bạn.
```

File cấu hình chỉ chứa tên host, đường dẫn và tên identity. Không đưa mật khẩu, token hay nội
dung khoá vào đây. Có thể trỏ tới file khác bằng `CROSSDESK_RELEASE_CONFIG=<path>`.

### 2.5. Khoá ký feed cập nhật (Ed25519)

- Khoá bí mật ở `~/.private_keys/crossdesk-update-ed25519.pem` trên máy macOS phát hành,
  `chmod 600`, nằm ngoài mọi repo. Khai báo qua `CROSSDESK_UPDATE_PRIVATE_KEY_FILE` trong
  `release.env`. Không in, không chép, không commit file này. Chỉ `make_release_feed.py` đọc nó
  để ký và không bao giờ in nội dung.
- Khoá công khai (32 byte, hex) nằm ở `FORK_UPDATE_PUBLIC_KEY` trong `scripts/release/lib.sh`.
  Ba bản build nhúng nó qua `CROSSDESK_UPDATE_PUBLIC_KEY`, gate kiểm nó có mặt trong mọi file thực
  thi, và `make_release_feed.py` dùng nó để tự kiểm từng chữ ký vừa tạo.
- Kiểm khoá bí mật khớp khoá công khai mà không in khoá bí mật: lệnh sau phải in đúng giá trị
  trong `lib.sh`.

  ```bash
  openssl pkey -in ~/.private_keys/crossdesk-update-ed25519.pem -pubout -outform DER | tail -c 32 | xxd -p -c 64
  ```

- **Đổi khoá ngày 26.11.2026, có hiệu lực từ v1.7.7-3.** Khoá cũ ký feed của v1.7.7-1 và v1.7.7-2
  không còn trên máy phát hành nên đã tạo cặp khoá mới. Hệ quả:
  - Máy đang chạy v1.7.7-1 hoặc v1.7.7-2 nhúng khoá công khai cũ, nên không kiểm được chữ ký của
    feed mới và không tự cập nhật lên v1.7.7-3. Những máy đó phải tải và cài v1.7.7-3 bằng tay
    **một lần**. Từ v1.7.7-3 trở đi cập nhật qua app bình thường.
  - Release notes của v1.7.7-3 phải ghi rõ điều này. Bản nháp do `release.sh` sinh đã có sẵn câu
    nhắc trong phần `## Downloads`; giữ câu đó ở các bản sau.
  - Bản đầu tiên ký bằng khoá mới nên build đủ ba nền tảng để mọi mục trong feed mang chữ ký mới.
    Mục của nền tảng không build được giữ nguyên với chữ ký cũ, chỉ máy bản cũ đọc được mục đó.
- Nếu khoá bí mật bị mất hoặc lộ: tạo cặp mới bằng
  `( umask 077; set -C; openssl genpkey -algorithm ED25519 -out ~/.private_keys/crossdesk-update-ed25519.pem )`
  (đổi tên hoặc chuyển khoá cũ đi trước), cập nhật `FORK_UPDATE_PUBLIC_KEY`, rồi coi mọi bản đã cài
  như ở trên: phải cài tay một lần. Chủ máy tự quyết định việc sao lưu khoá bí mật ra nơi an toàn.

## 3. Quy ước phiên bản

- Tag và tên asset: `v<MAJOR.MINOR.PATCH>-<BUILD>-<YYYYMMDD>`, ví dụ `v1.7.7-3-20261126`.
- Giữ nguyên phiên bản ngữ nghĩa thì **tăng `BUILD`** mỗi lần phát hành; đổi ngày đơn thuần
  không làm bản mới "mới hơn" với trình cập nhật. Không bao giờ dùng lại tag hay ghi đè asset đã
  công bố, kể cả khi release lỗi: phát hành số build mới. `BUILD` tối đa 999 (xem
  `CFBundleVersion` bên dưới); `release.sh` từ chối số lớn hơn.
- Phiên bản nhúng trong app (`CROSSDESK_VERSION`) và phiên bản trong feed bỏ tiền tố `v`
  (`1.7.7-3-20261126`), giống các bản fork đã phát hành.
- Gói macOS phải có `CFBundleVersion` và `pkg-ref` dạng `<MAJOR>.<MINOR>.<PATCH*1000+BUILD>`
  (ví dụ `1.7.7003` của bản `v1.7.7-3-20261126`), còn `CFBundleShortVersionString` là
  `<MAJOR.MINOR.PATCH>`. Apple chỉ đọc tối đa ba số nguyên của `CFBundleVersion`, nên giá trị
  này mã hoá đúng thứ tự của trình cập nhật (số phiên bản rồi tới `BUILD`, không xét ngày) để
  macOS Installer nâng cấp đúng. `v1.7.7-1` và `v1.7.7-2` từng ghi `1.7.7.20261125.<BUILD>`,
  mà macOS chỉ đọc thành `1.7.7`, nhỏ hơn `1.7.7003`. `make_release_feed.py` và
  `make_fork_update_manifest.py` kiểm giá trị này; `--publish` dừng trước khi tạo release nếu
  giá trị sai.
- Dry-run có thể dùng chữ thay cho ngày, ví dụ `v1.7.7-3-dryrun`; dạng này không bao giờ được
  publish (trình cập nhật chỉ chấp nhận phiên bản gồm chữ số, dấu chấm và gạch ngang).

## 4. Quy trình phát hành

### Bước 1: Chọn phiên bản và chuẩn bị commit

1. Mọi thay đổi cần phát hành đã merge vào `mcp` của fork qua cổng merge của fork (Codex review
   sạch, build local trên ba máy, xem [Đồng bộ upstream](UPSTREAM_SYNC.md#quy-tắc-pr)). GitHub
   Actions trên fork đã tắt, nên không chờ CI.
2. Xem release gần nhất để chọn số build tiếp theo:

   ```bash
   gh release list -R thuongtin/crossdesk --limit 5
   ```

3. Release luôn build đúng commit đầu của `origin/mcp` tại thời điểm chạy (script hỏi GitHub,
   không dùng bản clone làm việc). Không push gì thêm vào `mcp` trong lúc đang phát hành.

### Bước 2: Dry-run

```bash
scripts/release/release.sh v1.7.7-3-20261126
# Hoặc chỉ một phần: --platforms macos,linux; build commit/nhánh khác: --ref <sha>
```

Ba nền tảng build song song. Kết quả trong `dist/v1.7.7-3-20261126/`:

- Các asset ở bảng mục 1 và `SHA256SUMS.txt`.
- `gate/CONFORMANCE.txt`: kết quả fork-conformance gate cho từng file thực thi. Gate kiểm cả chuỗi
  ASCII và UTF-16LE:
  - mọi file thực thi (app và `crossdesk_updater`) chứa khoá công khai cập nhật của fork và
    đường dẫn release `/thuongtin/crossdesk/releases` (trình cập nhật của fork chỉ nhận URL
    `https://github.com/thuongtin/crossdesk/releases/download/v<version>/<file>`, đúng dạng URL
    mà feed ký), và không chứa `https://version.crossdesk.cn` hay `https://downloads.crossdesk.cn`;
  - file thực thi chính chứa đường dẫn feed
    `/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json` và signaling mặc định
    `crossdesk.ecom.io.vn`, không chứa `api.crossdesk.cn` và trang issue `/kunkundi/crossdesk/issues`
    của upstream (nút phản hồi GitHub đã bỏ khỏi bản fork);
  - `usr/lib/crossdesk/libslint_cpp.so` trong mỗi gói `.deb` chứa `libfontconfig.so.1` (runtime
    Slint dlopen fontconfig của hệ thống) và không chứa `/packages/f/fontconfig/`. Fontconfig tĩnh
    do xmake build đọc `fonts.conf` dưới thư mục cài trên máy build, máy người dùng không có tệp đó
    nên chữ giao diện rơi về phông serif (bản `v1.7.7-3-20261126` bị lỗi này).
  - trường `Depends` trong `control` của mỗi gói `.deb` có đủ thư viện mà runtime Slint và SDL nạp
    bằng `dlopen` (dpkg-shlibdeps và `ldd` không thấy chúng): `libxkbcommon0`,
    `libxkbcommon-x11-0`, `libx11-xcb1`, `libfontconfig1`, `libegl1`, `libwayland-client0`,
    `libwayland-cursor0`, `libwayland-egl1`, cùng `zlib1g` và `ca-certificates` (TLS của signaling
    và kiểm tra cập nhật). Thiếu `libxkbcommon-x11-0` thì app dừng ngay lúc mở trên Ubuntu tối giản
    với lỗi `Library libxkbcommon-x11.so could not be loaded` (bản `v1.7.7-4-20261128` bị lỗi này).
    Danh sách nằm ở `GUI_RUNTIME_DEPS` trong `apps/desktop/scripts/linux/package_deb.sh` và
    `LINUX_RUNTIME_DEPENDS` trong gate; thêm thư viện nạp động mới thì sửa cả hai;
  - file thực thi chính chứa nguyên văn `INSTALLED_RELEASE_NOTES.md` (mục `## Changes` của release
    notes, xem bước 3), so trọn từ marker đứng trước tới byte NUL kết thúc, tức hộp thoại Release
    Notes của bản cài mới hiện đúng ghi chú của bản này, không thừa không thiếu dòng nào.

  Ở dry-run gate chỉ cảnh báo; ở `--publish` gate trượt là dừng. Gate cố ý trượt trên mã upstream
  thuần (updater và signaling mặc định của upstream): chỉ publish khi `mcp` đã mang logic fork
  Postcard (xem `docs/UPSTREAM_SYNC.md`). Bản `v1.7.7-2-20261125` đã qua gate strict với khoá cập
  nhật cũ; từ đợt đổi khoá (mục 2.5) gate chạy trên gói đó báo thiếu khoá công khai, đúng như mong
  đợi.
- `gate/macos-signing.txt`: trạng thái ký (kèm identity còn thiếu), kết quả kiểm Team ID,
  notarize và cách notarize, `CFBundleVersion`, `CFBundleShortVersionString` và kết quả kiểm
  bundle version (kiểm cả bản dry-run, vì hậu tố ngày hay chữ không ảnh hưởng tới giá trị mong
  đợi; trượt khi `--publish` thì dừng ngay sau khi đóng gói). Không chứa họ tên hay Team ID.
- `gate/windows-signing.txt`: trạng thái ký gói Windows.
- `gate/linux/apt-simulate-*.txt`: kết quả giả lập cài đặt trên Ubuntu 20.04.
- `logs/<platform>.log`: log đầy đủ từng nền tảng. `FAILED_PLATFORMS.txt` xuất hiện nếu có nền
  tảng build lỗi (các nền tảng còn lại vẫn được gate và tính checksum).
- `RELEASE_NOTES.md`: bản sao của `--notes`, hoặc bản nháp theo khuôn các bản Postcard trước khi
  không truyền `--notes`.
- `INSTALLED_RELEASE_NOTES.md`: mục `## Changes` của `RELEASE_NOTES.md`, được viết trước khi build
  và nhúng vào app của cả ba nền tảng (bước 3).

Nên cài thử từng gói trên máy thật trước khi publish. Với Linux, cài từng gói `.deb` bằng
`apt install --no-install-recommends ./<gói>.deb` trên Ubuntu 20.04, 22.04 và 24.04 tối giản
(container hay rootfs sạch, không cài thêm gói nào bằng tay) rồi mở app. Màn hình cho app phải đến
từ ngoài môi trường thử: chạy Xvfb trên máy chủ và chỉ chia sẻ socket X (`/tmp/.X11-unix/X<n>`) vào
container. Đừng tự cài `xvfb` hay gói khác vào chính container đó, vì nó kéo theo thư viện như
`zlib1g` (qua `libxfont2`) và che mất thư viện mà gói CrossDesk quên khai báo; gói headless tự kéo
`xvfb` qua `Depends` của nó thì không sao. Gate chỉ kiểm tên trong `Depends`, còn `apt-simulate`
chỉ giả lập cài trên 20.04; thư viện nạp bằng `dlopen` còn thiếu chỉ lộ ra khi app thật sự chạy
trên máy không có sẵn nó.

### Bước 3: Viết release notes

Chép `dist/<version>/RELEASE_NOTES.md` ra ngoài `dist/` (thư mục này bị xoá và tạo lại mỗi lần
build) rồi sửa. Giữ hai phần như các bản trước:

- `## Downloads`: từng nền tảng, yêu cầu hệ điều hành, tình trạng ký, nhắc `SHA256SUMS.txt`.
- `## Changes`: thay đổi chính so với bản trước, ghi rõ nền tảng mới (ví dụ bản Linux đầu tiên).

Giữ câu nhắc máy đang chạy v1.7.7-1 hoặc v1.7.7-2 phải cài tay một lần (mục 2.5).

Nội dung này vừa là mô tả GitHub release vừa là `releaseNotes` trong feed mà app hiển thị.

Mục `## Changes` (từ tiêu đề đó tới tiêu đề cấp 1 hoặc 2 kế tiếp, cả hai ở cột 0 và nằm ngoài
code fence hay khối HTML ở cấp cao nhất) còn được nhúng vào chính bản build: app mở hộp thoại
**Release Notes** một lần sau khi cài mới hoặc nâng cấp lên bản này và hiện đúng mục đó, hiển thị
Markdown giống lời nhắc cập nhật.
Tiêu đề con trong mục (ví dụ `### Linux`) hiện thành dòng in đậm, còn dòng trong code fence hiện
nguyên chữ. Vì vậy:

- Notes phải hoàn chỉnh trước lần build publish. `release.sh` tách mục này bằng
  `installed_release_notes.py` vào `dist/<version>/INSTALLED_RELEASE_NOTES.md` trước khi build và
  truyền nó cho xmake (`--CROSSDESK_RELEASE_NOTES=<file>`) trên cả ba nền tảng; gate kiểm từng file
  thực thi chính chứa nguyên văn mục đó.
- Mục phải có ít nhất một dòng; thiếu mục thì dừng trước khi build. Khi `--publish`, dòng giữ chỗ
  `- TODO` của bản nháp bị từ chối, kể cả khi có khoảng trắng thừa. Dry-run không có `--notes` nhúng bản nháp, nên muốn soát hộp
  thoại với notes thật thì dry-run kèm `--notes`.
- Tiêu đề `## Changes` và mọi tiêu đề cấp 1 hoặc 2 phải ở cột 0. `installed_release_notes.py` tìm
  tiêu đề theo cách GitHub (cmark-gfm) dựng khối: dòng trong code fence ở cấp cao nhất (kể cả fence
  thụt một tới ba dấu cách) hay trong khối HTML (ví dụ `<!-- ... -->`, `<details>`) chỉ là chữ, nên
  một `## Changes` làm ví dụ trong đó không bị chọn nhầm. Fence hay khối HTML chưa đóng nằm trước
  hoặc trong mục Changes nuốt hết phần sau nó trên GitHub, nên script dừng ở dòng mở của khối đó.
- Trong mục Changes, script dừng ở dòng vi phạm đầu tiên:
  - Code fence (```` ``` ```` hoặc `~~~`) phải mở ở cột 0, không thụt lề, không đứng sau dấu list
    hay `>` (kể cả dòng giống fence trong khối code thụt lề), vì theo CommonMark fence lồng kết thúc
    cùng mục list chứa nó, còn extractor và hộp thoại chỉ hiểu fence ở cột 0. Ví dụ lệnh trong một
    mục list thì đặt fence ra ngoài list.
  - Không có tiêu đề cấp 1 hoặc 2 thụt lề, và không có dòng toàn `=` hay `-` ngay dưới một dòng chữ
    (GitHub biến dòng chữ đó thành tiêu đề). Cần đường kẻ thì để một dòng trống phía trên nó. Dòng
    cuối của bảng GFM không phải dòng chữ, nên `---` ngay dưới bảng vẫn là đường kẻ.
  - Không có thứ GitHub đọc thành HTML, dạng khối hay nội dòng, kể cả trong ô bảng và comment
    `<!-- -->` mà GitHub giấu đi: hộp thoại hiện dòng có HTML thành chữ thô. Muốn hiện một thẻ thì
    đặt nó trong backtick (`` `<package>` ``). Script kiểm theo cú pháp nội dòng của cmark-gfm, nên
    autolink như `<https://example.com>`, đích link như `[guide](<docs/Upgrade Guide.md>)` và dấu `<`
    trong `a < b` hay `x<limit` vẫn dùng được.
- Bảng GFM trong mục Changes không bị chặn, nhưng hộp thoại đọc từng dòng riêng nên không dựng lại
  bảng: mỗi hàng, kể cả dòng phân cách `| --- |`, hiện thành một dòng chữ có dấu `|`. Nên viết mục
  Changes bằng list.
- Phần ngoài mục Changes (ví dụ Downloads) không bị kiểm, trừ fence hay khối HTML chưa đóng nằm
  trước mục. Sửa script này thì chạy
  `python3 -B -m unittest discover -s scripts/release -p test_installed_release_notes.py`.
- Sửa notes sau khi đã build thì phải build lại. `--skip-build` sinh lại
  `INSTALLED_RELEASE_NOTES.md` từ notes hiện tại, nên gate báo lệch nếu notes đã đổi.
- Viết bằng tiếng Anh như phần còn lại của release notes: hộp thoại hiện nguyên văn ở mọi ngôn ngữ
  giao diện, giống release notes trong lời nhắc cập nhật.
- Bản build không qua `release.sh` (build tay, build dev) không nhúng notes và không mở hộp thoại
  này. Hộp thoại không còn đọc `apps/desktop/resources/release-notes.json` của upstream.
- Dry-run `--ref` một commit cũ hơn option `CROSSDESK_RELEASE_NOTES` vẫn build được: script build
  ghi cảnh báo, không truyền option đó cho xmake, và gate báo file thực thi thiếu notes. `--publish`
  luôn build `mcp` nên luôn có option này.

### Bước 4: Publish

```bash
scripts/release/release.sh v1.7.7-3-20261126 --publish --notes ~/release-notes/v1.7.7-3-20261126.md
```

`--publish` chỉ chấp nhận `--ref mcp` và làm lần lượt, dừng ngay khi một bước lỗi:

1. Build lại sạch cả ba nền tảng từ đầu `origin/mcp`, nhúng mục `## Changes` của `--notes`.
   macOS bắt buộc ký Developer ID, notarize và staple; thiếu identity, keychain đang khoá hay
   thiếu thông tin notarize thì dừng trước khi build.
2. Chạy gate ở chế độ nghiêm ngặt và tính `SHA256SUMS.txt`.
3. `publish.sh` kiểm điều kiện: `gh` đã đăng nhập, `origin` trỏ đúng fork, remote `upstream` (nếu
   có) giữ push URL `DISABLED_no_push_to_upstream`, tag và release chưa tồn tại, checksum khớp,
   có `CROSSDESK_UPDATE_PRIVATE_KEY_FILE`, rồi chạy lại gate nghiêm ngặt với cùng tham số (kể cả
   `--notes`), nên `gate/CONFORMANCE.txt` cuối cùng vẫn có kết quả kiểm notes.
4. Sinh feed mới từ feed hiện tại của `origin/desktop-updates` (`make_release_feed.py`): kiểm
   OpenSSL 3, quyền file khoá (600), phiên bản mới hơn feed đang công bố, kiểm gói macOS
   (`spctl`, `stapler`, `codesign`, `lipo`, `CFBundleVersion`), ký payload
   `{schema, platform, version, filename, url, size, sha256}` cho từng nền tảng rồi tự kiểm lại
   chữ ký bằng khoá công khai. Mục của nền tảng không build lần này được giữ nguyên.
5. Tạo GitHub release dạng **draft** với tiêu đề `CrossDesk Postcard <version>` tại đúng commit,
   tải lên mọi asset và `SHA256SUMS.txt`.
6. Đọc lại tên và kích thước asset, tải toàn bộ draft về và so SHA-256. Lệch thì dừng, draft được
   giữ lại để kiểm tra.
7. Công bố release (`--latest`), kiểm tag trỏ đúng commit, tải từng asset qua URL công khai và
   so SHA-256.
8. Chỉ khi các bước trên qua hết: commit feed vào `desktop-updates` trong một worktree tạm và
   `git push origin HEAD:refs/heads/desktop-updates` (push thường, không force), rồi đọc lại blob
   qua GitHub API. Kết quả ghi vào `dist/<version>/PUBLISHED.txt`.

### Bước 5: Kiểm tra sau phát hành

```bash
V=v1.7.7-3-20261126
gh release view "$V" -R thuongtin/crossdesk --json assets --jq '.assets[] | "\(.name) \(.size)"'
mkdir -p /tmp/check-$V && gh release download "$V" -R thuongtin/crossdesk -D /tmp/check-$V
(cd /tmp/check-$V && shasum -a 256 -c SHA256SUMS.txt)
curl -fsSL https://raw.githubusercontent.com/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json \
  | python3 -m json.tool | head -40
```

Sau đó trên một máy đang chạy bản cũ của từng nền tảng: Check for updates, xem release notes, cài
bản mới, kiểm hộp thoại Release Notes mở lần đầu hiện đúng mục `## Changes` của bản này, đọc lại
phiên bản trong app và thử kết nối qua signaling mặc định.

## 5. Xử lý sự cố

- **Keychain ký đang khoá**: `build_macos.sh` dừng trước khi build và in lệnh
  `security unlock-keychain` cần chạy. Chủ máy tự mở rồi chạy lại. Nếu codesign vẫn chờ quyền
  truy cập, probe tự dừng sau 90 giây; đóng hộp thoại đang mở (nếu có) trước khi chạy lại.
- **Notarize trượt**: phản hồi và nhật ký của Apple nằm trong `<CROSSDESK_RELEASE_MACOS_DIR>/notary`.
  Đọc lý do trước khi gửi lại; hay gặp nhất là thiếu Hardened Runtime, thiếu timestamp hoặc có
  binary ad-hoc hay của team khác.
- **Build một nền tảng lỗi**: xem `dist/<version>/logs/<platform>.log`. Có thể dry-run lại riêng
  nền tảng đó bằng `--platforms <platform>`. `--publish` luôn build lại cả các nền tảng được chọn.
- **Đọc lại draft lỗi**: release vẫn là draft (chưa có tag công khai). Kiểm tra rồi xoá bằng
  `gh release delete <version> -R thuongtin/crossdesk` và chạy lại với cùng phiên bản.
- **Lỗi sau khi đã công bố release, trước khi cập nhật feed**: không ghi đè asset. Bản đó vẫn tải
  được bằng tay nhưng app chưa thấy cập nhật. Sửa nguyên nhân rồi phát hành số build mới.
- **Push feed bị từ chối** (nhánh `desktop-updates` vừa đổi): release đã công bố, feed chưa đổi.
  Sinh lại feed từ bản `desktop-updates` mới nhất bằng `make_release_feed.py` (tham số giống
  `publish.sh`), commit vào một worktree của `origin/desktop-updates` và push thường.
- **Ngừng cung cấp một bản cập nhật**: tạo commit trên `desktop-updates` khôi phục feed đã biết
  tốt (`git revert`, push thường). Trình cập nhật chặn hạ phiên bản nên máy đã cài vẫn giữ bản
  mới; cài bản phục hồi riêng khi cần.

## 6. Bảo mật

- Không in, xuất hay chép chứng thư, mật khẩu, token hay khoá bí mật (kể cả file `.p8` và file env
  chứa thông tin notarize). Script chỉ đọc tên identity trong keychain, đọc khoá Ed25519 từ file
  để ký trong thư mục tạm, và không đọc secret trong keychain hay đổi cấu hình keychain.
- Output của `codesign -dv`, `security find-identity` và `pkgutil --check-signature` chứa họ tên
  và Team ID: không dán vào repo, issue hay PR.
- Mọi lệnh `gh` ghi dữ liệu đều có `-R thuongtin/crossdesk`. Không tạo release, PR, issue hay
  comment trên `kunkundi/crossdesk`, không đổi push URL của remote `upstream`.
- Không force-push `mcp` hay `desktop-updates`.
