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
- Trong keychain đăng nhập (hoặc keychain riêng, khai báo qua `CROSSDESK_MACOS_KEYCHAIN`):
  - `Developer ID Application: <Tên> (<TEAMID>)` để ký app.
  - `Developer ID Installer: <Tên> (<TEAMID>)` để ký gói `.pkg`. Hai identity phải cùng Team ID.
- Profile notarytool, do chủ tài khoản tự tạo (lệnh hỏi mật khẩu ứng dụng, không lưu vào repo):

  ```bash
  xcrun notarytool store-credentials crossdesk-notary --apple-id <apple-id> --team-id <TEAMID>
  ```

- Khoá bí mật Ed25519 của kênh cập nhật fork (PEM), khớp khoá công khai nhúng trong
  `scripts/release/lib.sh`. Đặt ngoài repo, `chmod 600`. Mất hoặc đổi khoá nghĩa là mọi bản đã cài
  không nhận cập nhật nữa và phải cài lại bằng tay.

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

## 3. Quy ước phiên bản

- Tag và tên asset: `v<MAJOR.MINOR.PATCH>-<BUILD>-<YYYYMMDD>`, ví dụ `v1.7.7-3-20261126`.
- Giữ nguyên phiên bản ngữ nghĩa thì **tăng `BUILD`** mỗi lần phát hành; đổi ngày đơn thuần
  không làm bản mới "mới hơn" với trình cập nhật. Không bao giờ dùng lại tag hay ghi đè asset đã
  công bố, kể cả khi release lỗi: phát hành số build mới.
- Phiên bản nhúng trong app (`CROSSDESK_VERSION`) và phiên bản trong feed bỏ tiền tố `v`
  (`1.7.7-3-20261126`), giống các bản fork đã phát hành.
- Gói macOS phải có `CFBundleVersion` dạng `<MAJOR.MINOR.PATCH>.<YYYYMMDD>.<BUILD>` (ví dụ
  `1.7.7.20261125.2` của bản `v1.7.7-2-20261125`) để macOS Installer nâng cấp đúng và để
  `make_release_feed.py` chấp nhận. `--publish` dừng trước khi tạo release nếu giá trị này sai.
- Dry-run có thể dùng chữ thay cho ngày, ví dụ `v1.7.7-3-dryrun`; dạng này không bao giờ được
  publish (trình cập nhật chỉ chấp nhận phiên bản gồm chữ số, dấu chấm và gạch ngang).

## 4. Quy trình phát hành

### Bước 1: Chọn phiên bản và chuẩn bị commit

1. Mọi thay đổi cần phát hành đã merge vào `mcp` của fork, CI trên `mcp` xanh.
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
    `crossdesk.ecom.io.vn`, không chứa `api.crossdesk.cn`.

  Ở dry-run gate chỉ cảnh báo; ở `--publish` gate trượt là dừng. Gate cố ý trượt trên mã upstream
  thuần (updater và signaling mặc định của upstream): chỉ publish khi `mcp` đã mang logic fork
  Postcard (xem `docs/UPSTREAM_SYNC.md`). Bản `v1.7.7-2-20261125` đã phát hành qua gate strict.
- `gate/macos-signing.txt`: trạng thái ký, `CFBundleVersion`, `CFBundleShortVersionString` và kết
  quả kiểm bundle version (bỏ qua với phiên bản dry-run không có ngày; trượt khi `--publish` thì
  dừng ngay sau khi đóng gói).
- `gate/windows-signing.txt`: trạng thái ký gói Windows.
- `gate/linux/apt-simulate-*.txt`: kết quả giả lập cài đặt trên Ubuntu 20.04.
- `logs/<platform>.log`: log đầy đủ từng nền tảng. `FAILED_PLATFORMS.txt` xuất hiện nếu có nền
  tảng build lỗi (các nền tảng còn lại vẫn được gate và tính checksum).
- `RELEASE_NOTES.md`: bản nháp release notes theo khuôn các bản Postcard trước.

Nên cài thử từng gói trên máy thật trước khi publish.

### Bước 3: Viết release notes

Chép `dist/<version>/RELEASE_NOTES.md` ra ngoài `dist/` (thư mục này bị xoá và tạo lại mỗi lần
build) rồi sửa. Giữ hai phần như các bản trước:

- `## Downloads`: từng nền tảng, yêu cầu hệ điều hành, tình trạng ký, nhắc `SHA256SUMS.txt`.
- `## Changes`: thay đổi chính so với bản trước, ghi rõ nền tảng mới (ví dụ bản Linux đầu tiên).

Nội dung này vừa là mô tả GitHub release vừa là `releaseNotes` trong feed mà app hiển thị.

### Bước 4: Publish

```bash
scripts/release/release.sh v1.7.7-3-20261126 --publish --notes ~/release-notes/v1.7.7-3-20261126.md
```

`--publish` chỉ chấp nhận `--ref mcp` và làm lần lượt, dừng ngay khi một bước lỗi:

1. Build lại sạch cả ba nền tảng từ đầu `origin/mcp`. macOS bắt buộc ký Developer ID, notarize
   và staple; thiếu identity hay profile notarytool thì dừng.
2. Chạy gate ở chế độ nghiêm ngặt và tính `SHA256SUMS.txt`.
3. `publish.sh` kiểm điều kiện: `gh` đã đăng nhập, `origin` trỏ đúng fork, remote `upstream` (nếu
   có) giữ push URL `DISABLED_no_push_to_upstream`, tag và release chưa tồn tại, checksum khớp,
   có `CROSSDESK_UPDATE_PRIVATE_KEY_FILE`.
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
bản mới, đọc lại phiên bản trong app và thử kết nối qua signaling mặc định.

## 5. Xử lý sự cố

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

- Không in, xuất hay chép chứng thư, mật khẩu, token hay khoá bí mật. Script chỉ đọc tên
  identity trong keychain và đọc khoá Ed25519 từ file để ký trong thư mục tạm.
- Mọi lệnh `gh` ghi dữ liệu đều có `-R thuongtin/crossdesk`. Không tạo release, PR, issue hay
  comment trên `kunkundi/crossdesk`, không đổi push URL của remote `upstream`.
- Không force-push `mcp` hay `desktop-updates`.
