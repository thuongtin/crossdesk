# Biên dịch từ mã nguồn

[Về README](../README.md) · [English](BUILD_EN.md)

Dưới đây là phụ thuộc, tham số biên dịch, đóng gói và ký theo từng nền tảng. Xem gói cài đặt ở [trang tải xuống](../README.md#download), đồng bộ phiên bản và số bản phát hành trong [quản lý phiên bản](VERSIONING.md).

Sau khi clone kho mã, thực hiện lệnh biên dịch và đóng gói tại thư mục gốc kho mã. Phụ thuộc và tham số theo [cấu hình Xmake](../apps/desktop/xmake/options.lua) và [quy trình CI](../.github/workflows/build.yml). `1.4.3` chỉ là số phiên bản ví dụ khi biên dịch cục bộ; khi điều chỉnh, hãy đổi đồng bộ lệnh cấu hình và tham số đóng gói để tránh phiên bản trong chương trình khác gói cài đặt.

## Phụ thuộc biên dịch

Mặc định chỉ biên dịch ứng dụng máy tính. Có thể biên dịch riêng ứng dụng điều khiển terminal qua `--CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y`; bật cả hai tùy chọn để biên dịch cả hai ứng dụng. Xem chuẩn bị phụ thuộc, chạy và cài CLI trong [ứng dụng terminal](../apps/cli/README.md).

- Git, [Xmake](https://xmake.io/guide/quick-start.html) (cả máy tính và di động đều cần 3.1.1 hoặc mới hơn) và chuỗi công cụ biên dịch tương ứng từng nền tảng bên dưới.
- Script biên dịch quản lý phụ thuộc như **CMake 3.x (≥ 3.21, < 4.0)**, **Rust 1.92.0** qua Xmake; lần đầu cần tải công cụ và mã nguồn. Xem ràng buộc cụ thể trong [cấu hình gói Slint](../deps/thirdparty/slint/xmake.lua).

Bản chuyển Slint cho HarmonyOS PC biên dịch trực tiếp giao diện và mã nghiệp vụ chung của máy tính, xem [cài đặt và quyền HarmonyOS PC](../apps/harmony/desktop-port/README.md) (API 26). Dự án ArkUI điện thoại / máy tính bảng riêng nằm trong [biên dịch và cài HarmonyOS](../apps/harmony/README.md). Hãy dùng script trong tài liệu từng bản, không đổi cấu hình HarmonyOS qua dự án gốc máy tính.

## Windows

Cài workload “Desktop development with C++”, Windows SDK và thành phần “C++ Clang Compiler for Windows” của Visual Studio 2022, cùng Xmake. libyuv Windows x64 dùng clang-cl để bật chuyển đổi và co giãn hình ảnh SIMD; chương trình chính vẫn dùng MSVC. Thực hiện trong Developer PowerShell:

```powershell
git clone --recurse-submodules https://github.com/kunkundi/crossdesk.git
cd crossdesk
xmake f -c -p windows -a x64 -m release --CROSSDESK_VERSION=1.4.3 --USE_CUDA=false -y
xmake b -vy crossdesk
xmake r crossdesk
```

Nên dùng `release` khi đo lại hiệu năng; `debug` giữ thông tin gỡ lỗi và tắt tối ưu hóa biên dịch chương trình chính.

Gói cài đặt còn dùng NSIS; cần đặt [nsProcess.dll](../apps/desktop/scripts/windows/nsProcess.dll) của kho mã vào thư mục `Plugins/x86-unicode` của NSIS và bảo đảm `makensis` trong `PATH`. Sau khi biên dịch Release, chạy:

```powershell
.\apps\desktop\scripts\windows\pkg_x64.ps1 1.4.3
```

Xem quy trình đầy đủ trong [script Windows](../apps/desktop/scripts/windows/pkg_x64.ps1) và [quy trình CI](../.github/workflows/build.yml). Triển khai thủ công cần giữ thư viện thời gian chạy, hai trình trợ giúp dịch vụ và thư mục trình điều khiển `usbmmidd_v2` (xmake sao chép vào thư mục đầu ra sau biên dịch), xem [tài liệu dịch vụ Windows](../README.md#windows-service).

## macOS

Cài Xcode cùng công cụ dòng lệnh và Xmake, rồi thực hiện:

```bash
git clone --recurse-submodules https://github.com/kunkundi/crossdesk.git
cd crossdesk
xmake f -c -m release --target_minver=14.0 --CROSSDESK_VERSION=1.4.3 --USE_CUDA=false -y
xmake b -vy crossdesk
xmake r crossdesk
```

CI hiện đặt macOS 14.0 làm phiên bản đích tối thiểu cho cả Intel và Apple Silicon; xem phiên bản Xcode/SDK trong [quy trình CI](../.github/workflows/build.yml). Khi đóng gói, chạy một script theo kiến trúc máy này:

```bash
# Apple Silicon
./apps/desktop/scripts/macos/pkg_arm64.sh 1.4.3
# Intel
./apps/desktop/scripts/macos/pkg_x64.sh 1.4.3
```

Script đóng gói tạo `.app` và `.pkg` chứa thư viện thời gian chạy Slint. Khi các bản chính thức liên tục dùng chữ ký Developer ID cùng nhóm, cùng Bundle ID và yêu cầu chữ ký tương thích, nâng cấp bình thường thường giữ được quyền ghi màn hình, trợ năng. Khi kiểm thử khả năng được điều khiển, hãy dùng gói ứng dụng ở đường dẫn cố định.

Gói cài Developer ID kiểm tra ứng dụng cũ trước khi ghi đè qua `preinstall`. Khi phát hiện CrossDesk chưa ký hoặc ký ad-hoc, lưu nguyên tử dấu chuyển đổi vào `/Library/Application Support/CrossDesk/permission-migration.json` trên ổ đích. Sau khi cài, thoát bản cũ rồi chạy bản mới; bản mới ký chính thức chuyển đổi riêng quyền trợ năng và ghi màn hình cho người dùng hiện tại trong nền. Dấu chuyển đổi hỗ trợ cài trên ổ ngoại tuyến và người dùng khác đăng nhập sau.

Tiến độ xử lý từng quyền lưu trong `~/Library/Application Support/CrossDesk/PermissionMigration/progress.json` của người dùng hiện tại. Mục đã xử lý hoặc đã có quyền hợp lệ được bỏ qua; khi thất bại hoặc gián đoạn, có thể nhấn “Thử chuyển đổi quyền lại” trong “Cài đặt → Quyền hệ thống”. Khi đang chuyển đổi hoặc thất bại đều có thể chọn “Để sau”; tính năng được điều khiển bị ảnh hưởng khả dụng sau khi cấp quyền. Nếu trình cài không ghi được trạng thái chuyển đổi, lỗi được ghi vào nhật ký cài đặt và việc cài vẫn tiếp tục. Với quyền bất thường còn lại sau cài hoặc thay danh tính chứng chỉ ký, xem [khắc phục quyền macOS](FAQ.md#macos-đã-kết-nối-nhưng-màn-hình-đen-hoặc-không-thao-tác-được-bàn-phím-và-chuột).

### Ký Developer ID và công chứng

Đóng gói mặc định ký ad-hoc cho ứng dụng, không ký gói cài đặt, phù hợp phát triển cục bộ. Để phân phối ra ngoài, cần cài chứng chỉ `Developer ID Application`, `Developer ID Installer` cùng nhóm và khóa riêng tương ứng vào chuỗi khóa, rồi đặt biến môi trường sau (thay tên chứng chỉ và ID nhóm):

```bash
export CROSSDESK_MACOS_SIGN_IDENTITY='Developer ID Application: Your Organization (TEAMID)'
export CROSSDESK_MACOS_INSTALLER_IDENTITY='Developer ID Installer: Your Organization (TEAMID)'

# Chỉ cấu hình lần đầu: nhập Apple ID và mật khẩu dành riêng cho ứng dụng theo lời nhắc, lưu vào chuỗi khóa máy này.
xcrun notarytool store-credentials crossdesk-notary --team-id TEAMID
export CROSSDESK_MACOS_NOTARY_PROFILE=crossdesk-notary

# Hoàn tất biên dịch release ở trên trước; với Intel, đổi sang pkg_x64.sh.
./apps/desktop/scripts/macos/pkg_arm64.sh 1.4.3
```

Script lần lượt ký thư viện động Slint, trình cập nhật và ứng dụng, bật Hardened Runtime, dấu thời gian bảo mật và quyền tài nguyên cần thiết, rồi ký `.pkg` bằng chứng chỉ Installer. Sau khi cấu hình Profile công chứng, script gửi đến Apple để công chứng, chỉ đính kèm vé và kiểm tra Gatekeeper khi nhận `Accepted`. Khi chưa cấu hình Profile, chỉ tạo gói đã ký nhưng chưa công chứng; ký và công chứng không thay thế việc người dùng cấp quyền ghi màn hình, trợ năng.

Gói cài đã ký hiện có có thể gửi công chứng riêng:

```bash
CROSSDESK_MACOS_NOTARY_PROFILE=crossdesk-notary \
  bash apps/desktop/scripts/macos/notarize_pkg.sh crossdesk-macos-arm64-1.4.3.pkg
```

Sau khi tải lên thành công, ID lần gửi hiện ngay và được lưu trong `.notary-submit.json`; kết quả và nhật ký công chứng lưu cạnh gói cài trong `.notary.json`, `.notary-log.json`. Thất bại hoặc hết thời gian chờ dừng đóng gói, giữ sản phẩm và ID lần gửi. Mặc định chờ 30 phút, có thể điều chỉnh bằng `CROSSDESK_MACOS_NOTARY_TIMEOUT`. Khi hết thời gian, dùng `xcrun notarytool wait <ID-lần-gửi> --keychain-profile crossdesk-notary` để chờ tiếp; sau khi xác nhận `Accepted`, thực hiện `xcrun stapler staple <gói-cài-đặt>`, `xcrun stapler validate <gói-cài-đặt>` và `spctl --assess --type install --verbose=2 <gói-cài-đặt>`.

Nếu tải lên tăng tốc bị hết thời gian kết nối, đặt `CROSSDESK_MACOS_NOTARY_S3_ACCELERATION=false` để thử lại qua kênh S3 thường; mặc định là `true`. Tải lên thất bại vẫn có thể đã tạo bản ghi ở Apple; trước khi thử lại, dùng `xcrun notarytool history --keychain-profile crossdesk-notary` kiểm tra lịch sử gửi.

Có thể chỉ định chuỗi khóa tùy chỉnh bằng `CROSSDESK_MACOS_KEYCHAIN`; khóa riêng chứng chỉ và Profile công chứng đều cần lưu trong chuỗi khóa đó. Không commit khóa riêng, tệp P12 hoặc mật khẩu vào kho mã. Tài liệu Apple: [chứng chỉ Developer ID](https://developer.apple.com/help/account/certificates/create-developer-id-certificates/), [yêu cầu công chứng](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

### Tự ký và công chứng bằng GitHub Actions

[Quy trình Build and Release](../.github/workflows/build.yml) tự bật ký, công chứng và đính kèm vé macOS cho tag phát hành `v*`; khi chạy thủ công có thể chọn `sign_macos`. Push nhánh thông thường và biên dịch thủ công không chọn tùy chọn này tiếp tục ký ad-hoc. Mỗi kiến trúc xử lý trên runner riêng; chỉ bản ký có công chứng và kiểm tra Gatekeeper thành công mới tải sản phẩm chính thức lên; thiếu Secrets hoặc công chứng thất bại ngăn phát hành.

Trước hết, trong “Truy cập chuỗi khóa → Chứng chỉ của tôi” trên máy này, xuất riêng `Developer ID Application` và `Developer ID Installer` **cùng khóa riêng tương ứng** thành hai tệp `.p12`, đặt cùng mật khẩu xuất cho hai tệp. Chỉ có tệp chứng chỉ `.cer` thì không thể ký CI. Lưu ngoài dự án, ví dụ `~/Downloads/crossdesk-application.p12` và `~/Downloads/crossdesk-installer.p12`.

Cấu hình trong **Settings → Secrets and variables → Actions → New repository secret** của kho GitHub:

| Secret | Nội dung |
| --- | --- |
| `MACOS_APP_CERTIFICATE_P12_BASE64` | Base64 của tệp `.p12` chứa chứng chỉ Application và khóa riêng |
| `MACOS_INSTALLER_CERTIFICATE_P12_BASE64` | Base64 của tệp `.p12` chứa chứng chỉ Installer và khóa riêng |
| `MACOS_P12_PASSWORD` | Mật khẩu chung đã đặt khi xuất hai `.p12` |
| `APPLE_ID` | Email tài khoản Apple có quyền với nhóm phát triển đó |
| `APPLE_APP_SPECIFIC_PASSWORD` | Mật khẩu dành riêng cho ứng dụng được tạo để công chứng |
| `APPLE_TEAM_ID` | ID nhóm gồm 10 ký tự của hai chứng chỉ |

Thực hiện lần lượt các lệnh sau trên máy này, dán nội dung clipboard vào Base64 Secret tương ứng. Lưu mục đầu rồi mới thực hiện lệnh thứ hai:

```bash
base64 -i "$HOME/Downloads/crossdesk-application.p12" | pbcopy
base64 -i "$HOME/Downloads/crossdesk-installer.p12" | pbcopy
```

Profile chuỗi khóa `crossdesk-notary` lưu trên máy này không đồng bộ sang runner GitHub. Quy trình dùng các Secrets trên để tạo lại Profile trong chuỗi khóa tạm, tự chọn chứng chỉ hợp lệ khớp ID nhóm, cho phép công cụ ký truy cập khóa riêng tạm không cần tương tác, rồi xóa chứng chỉ và thông tin công chứng khi tác vụ kết thúc. Không cần thêm Secret mật khẩu chuỗi khóa, cũng không cần cấu hình provisioning profile cho quy trình phân phối Developer ID hiện tại.

Sau cấu hình, chạy quy trình thủ công trước: nhập `desktop_version`, chọn `sign_macos`, xác nhận bước `Notarize and staple macOS installer` trên ARM64 và Intel đều thành công, rồi mới push tag phát hành. Cần commit và push quy trình cùng script đi kèm lên GitHub trước thì công tắc và bước mới mới có hiệu lực.

CI chờ công chứng tối đa 60 phút; nếu Apple vẫn xử lý, tác vụ thất bại và ngăn phát hành nhưng giữ gói cài `macos-pending-notarization-*` và sản phẩm chẩn đoán JSON `macos-notarization-*` trong 14 ngày. Sau khi tải sản phẩm, dùng ID lần gửi đã lưu để truy vấn và đính kèm vé theo phần trên, không cần tải lại cùng gói lên. Không coi sản phẩm pending là gói phát hành đã công chứng xong.

## Linux

Biên dịch Linux hỗ trợ amd64 và arm64 trên Ubuntu 20.04 trở lên. Gói cài đặt phát hành lấy
Ubuntu 20.04/glibc 2.31 làm mức tương thích cơ sở. Các lệnh phụ thuộc sau lấy Ubuntu 20.04 làm ví dụ; bản phân phối khác hoặc phiên bản mới hơn cần điều chỉnh tên gói và chuỗi công cụ theo kho phần mềm. Có thể dùng ảnh môi trường biên dịch bên dưới để tái hiện CI. Trước hết cài phụ thuộc biên dịch cơ bản:

```bash
sudo apt-get update
sudo apt-get install -y \
  git curl unzip build-essential gcc-10 g++-10 python3-pip \
  pkg-config binutils dpkg-dev \
  libx11-dev libxext-dev libxrender-dev libxft-dev libfribidi-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libxi-dev libxfixes-dev libxv-dev \
  libxtst-dev libxcb-randr0-dev libxcb-xtest0-dev libxcb-xinerama0-dev \
  libxcb-shape0-dev libxcb-xkb-dev libxcb-xfixes0-dev libxcb-shm0-dev \
  libasound2-dev libsndio-dev libpulse-dev libfuse-dev libfuse3-dev \
  libgl1-mesa-dev
```

CMake 3.16 đi kèm Ubuntu 20.04 không đáp ứng yêu cầu; Xmake tải
CMake 3.x trong phạm vi trên, không cần thay CMake hệ thống thủ công.

GCC 9 mặc định của Ubuntu 20.04 thiếu một phần thư viện chuẩn C++20 mà Slint C++ API cần,
vì vậy lệnh trên cũng cài GCC 10 và chỉ định nó khi cấu hình sau đó.

Tải submodule và biên dịch bản Release:

```bash
git clone --recurse-submodules https://github.com/kunkundi/crossdesk.git
cd crossdesk

# Chạy dòng này nếu đã clone kho mã
git submodule update --init --recursive

xmake f -c -m release --CROSSDESK_VERSION=1.4.3 --USE_CUDA=false --toolchain=gcc-10 -y
xmake b -vy crossdesk
```

Ở đây nên dùng `--toolchain=gcc-10` để bảo đảm CrossDesk và các gói bên thứ ba như
libyuv do xmake biên dịch ở bước cấu hình đều dùng GCC 10. Nếu chỉ đặt `--cc`/`--cxx`, cài phụ thuộc có thể vẫn quay về
`/bin/cc` và `/bin/c++` mặc định hệ thống. `-c` xóa cấu hình cũ, bắt buộc giữ khi đổi trình biên dịch.

Kết quả biên dịch amd64 ở `build/linux/x86_64/release/crossdesk`, arm64 ở `build/linux/arm64/release/crossdesk`.

Khi bật Wayland và DRM, cài thêm phụ thuộc phát triển D-Bus, DRM và PipeWire. Ubuntu 20.04 dùng SDK header PipeWire do kho mã cung cấp; Ubuntu 22.04 trở lên có thể dùng gói hệ thống `libpipewire-0.3-dev` và `libspa-0.2-dev`:

```bash
sudo apt-get install -y libdbus-1-dev libdrm-dev
# Bỏ qua dòng này nếu đã cài gói phát triển PipeWire 0.3 hệ thống
sudo ./docker/linux-build/install-pipewire-sdk.sh

xmake f -c -m release --USE_WAYLAND=true --USE_DRM=true --USE_CUDA=false \
  --CROSSDESK_VERSION=1.4.3 --toolchain=gcc-10 -y
xmake b -vy crossdesk
```

Tạo gói cài Debian (bắt buộc hoàn tất biên dịch Release trên kiến trúc tương ứng trước):

```bash
# amd64
./apps/desktop/scripts/linux/pkg_amd64.sh 1.4.3

# arm64
./apps/desktop/scripts/linux/pkg_arm64.sh 1.4.3
```

Mỗi lần đóng gói tạo bản thường `crossdesk-linux-<arch>-<version>.deb` và bản màn hình ảo `crossdesk-virtual-desktop-xfce-linux-<arch>-<version>.deb` từ cùng sản phẩm biên dịch. Cả hai chứa chương trình đầy đủ, chỉ bản sau bắt buộc phụ thuộc Xvfb, Xfce và công cụ phiên D-Bus; bản thân biên dịch và đóng gói không cần cài các thành phần này. Nếu khi chạy từ nguồn cần tạo màn hình ảo đầy đủ, chuẩn bị phụ thuộc tùy chọn theo [hướng dẫn chạy headless](HEADLESS.md). Ảnh môi trường biên dịch dự án và CI có thể giữ các thành phần này để kiểm thử.

Script đóng gói cài thư viện thời gian chạy dùng chung Slint vào thư mục riêng `/usr/lib/crossdesk` của gói, người dùng không cần cài thêm `libslint_cpp.so`. Dấu `package-name` trong cùng thư mục do gói cài quản lý để chọn bản tải cập nhật, không ghi vào cấu hình người dùng.
PipeWire không phải phụ thuộc thời gian chạy bắt buộc: chương trình phát hiện PipeWire 0.3 của hệ thống chủ khi chạy,
nếu không có thư viện này vẫn dùng được ghi hình X11 (và DRM nếu bật khi biên dịch).

## Tùy chọn biên dịch chung

```text
--USE_CUDA=true/false: Bật mã hóa và giải mã phần cứng CUDA, mặc định tắt
--USE_WAYLAND=true/false: Bật ghi hình Wayland/PipeWire trên Linux, mặc định tắt
--USE_DRM=true/false: Bật ghi hình DRM trên Linux, mặc định tắt
--CROSSDESK_PORTABLE=true/false: Biên dịch bản portable, mặc định tắt
--CROSSDESK_VERSION=xxx: Chỉ định phiên bản CrossDesk
--CROSSDESK_UPDATE_PUBLIC_KEY=xxx: Khóa công khai Ed25519 tùy chọn để xác minh chữ ký cập nhật, 64 ký tự thập lục phân viết thường; mặc định rỗng, không bật tự cài bản cập nhật
--MINIRTC_ENABLE_AOM=true/false: Biên dịch backend libaom tùy chọn, mặc định tắt; AV1 vẫn dùng SVT-AV1 / dav1d

# Ví dụ
xmake f --CROSSDESK_VERSION=1.4.3 --USE_CUDA=true
```

Chạy:

```bash
xmake r crossdesk
```

<a id="update-public-key"></a>

### Khóa công khai cập nhật (tùy chọn)

`CROSSDESK_UPDATE_PUBLIC_KEY` được nhúng vào ứng dụng máy tính khi biên dịch để xác minh chữ ký Ed25519 của manifest cập nhật. Khi tự biên dịch có thể bỏ qua: chương trình vẫn biên dịch, chạy và kiểm tra phiên bản mới bình thường, nhưng chỉ cung cấp liên kết tải thủ công, không tự cài trong ứng dụng.

Nếu cần tự cài bản cập nhật trong ứng dụng, thêm tham số này vào lệnh cấu hình `xmake f` của nền tảng tương ứng rồi biên dịch. Ví dụ, sau khi cấu hình nền tảng, thực hiện (thay chỗ giữ chỗ bằng khóa công khai thực tế):

```sh
xmake f --CROSSDESK_UPDATE_PUBLIC_KEY="<khóa-công-khai-64-ký-tự-thập-lục-phân-viết-thường>"
xmake b crossdesk
```

Bạn cũng có thể đặt biến môi trường cùng tên trước khi biên dịch; giá trị cấu hình Xmake ưu tiên hơn biến môi trường. Khóa công khai phải là biểu diễn thập lục phân của khóa Ed25519 32 byte, tổng 64 ký tự viết thường; nếu không rỗng nhưng sai định dạng, cấu hình biên dịch báo lỗi. Manifest phát hành phải được ký bằng khóa riêng tương ứng và đáp ứng điều kiện nâng cấp như vị trí cài; bản portable luôn dùng tải thủ công.

GitHub Actions đọc khóa công khai từ biến kho mã `CROSSDESK_UPDATE_PUBLIC_KEY`; khóa riêng tương ứng chỉ dùng ký manifest phát hành. Xem tạo khóa, ký manifest và điều kiện cài trong [biên dịch và phát hành trình nâng cấp](UPDATER.md#biên-dịch-và-phát-hành-trình-nâng-cấp).

## Biên dịch khi không có CUDA

Với **lập trình viên Linux chưa cài môi trường CUDA nhưng muốn sản phẩm biên dịch có mã hóa và giải mã phần cứng**, dự án cung cấp [ảnh môi trường biên dịch tương thích Ubuntu 20.04](https://hub.docker.com/r/crossdesk/ubuntu20.04) đã cấu hình sẵn. Ảnh chứa phụ thuộc cần thiết để tạo hai loại gói Linux tương thích glibc 2.31.

Xem phiên bản ảnh và khác biệt amd64 / arm64 trong [tài liệu ảnh môi trường biên dịch](../docker/linux-build/README.md). Ví dụ CUDA sau áp dụng cho ảnh amd64; sau khi vào container và tải dự án, thực hiện:

```bash
export CUDA_PATH=/usr/local/cuda
export XMAKE_GLOBALDIR=/data

xmake f --root -c -m release --CROSSDESK_VERSION=1.4.3 --USE_CUDA=true --toolchain=gcc-10 -y
xmake b --root -vy crossdesk
```

Với **lập trình viên Windows chưa cài môi trường CUDA**, chạy lệnh dưới để cài môi trường biên dịch CUDA:

```powershell
xmake require -vy "cuda 12.6.3"
```

Sau khi cài, thực hiện:

```powershell
xmake require --info "cuda 12.6.3"
```

Tìm trường `installdir` trong đầu ra.

Dùng thư mục cài CUDA được `installdir` trỏ đến. Thêm CUDA_PATH vào biến môi trường hệ thống hoặc nhập trong terminal:

```powershell
$env:CUDA_PATH = "C:\path\to\cuda_installdir"
```

Thực hiện lại:

```powershell
xmake f --USE_CUDA=true
xmake b -vy crossdesk
```

### Sự cố kết nối khi chạy

“Chưa kết nối máy chủ” có thể do mạng, cấu hình máy chủ hoặc tin cậy chứng chỉ; không nên chỉ cài ứng dụng khác để kiểm tra. Hãy kiểm tra theo [câu hỏi thường gặp](FAQ.md) và [tài liệu chứng chỉ self-host](SELF_HOSTING.md).

## Về Xmake

### Cài Xmake

Dùng curl:

```bash
curl -fsSL https://xmake.io/shget.text | bash
```

Dùng wget:

```bash
wget https://xmake.io/shget.text -O - | bash
```

Dùng powershell:

```powershell
irm https://xmake.io/psget.text | iex
```

### Tùy chọn biên dịch

```text
# Chuyển sang Debug; để biên dịch Release, thay debug bằng release
xmake f -m debug

# Tham số biên dịch tùy chọn
-r : Biên dịch lại đích
-v : Hiện nhật ký biên dịch chi tiết
-y : Tự xác nhận lời nhắc

# Ví dụ
xmake b -vy crossdesk
```

### Tùy chọn chạy

```bash
# Chạy bản biên dịch hiện tại bằng trình gỡ lỗi; không tự chuyển sang chế độ biên dịch Debug
xmake r -d crossdesk
```

Tham khảo thêm cách dùng trong [tài liệu Xmake chính thức](https://xmake.io/guide/quick-start.html).

## iOS / iPadOS

Ứng dụng native dùng dự án Xcode riêng, cần thiết bị thật arm64 chạy iOS 16+. Xem các bước biên dịch và kiểm chứng âm thanh, video, truyền tệp trong [tài liệu phát triển iOS](../apps/ios/README.md). CI hiện xuất ứng dụng chưa ký, cần tự ký để cài thiết bị thật.

[CI iOS](../.github/workflows/build-ios.yml) có thể chạy thủ công riêng và được quy trình biên dịch chính gọi; Pull Request liên quan cũng kích hoạt. Kiểm tra chuỗi công cụ, giấy phép và quyền riêng tư, xác minh ứng dụng và đóng ZIP dùng quy trình hiện có; phát hành tag tiếp tục thu thập tệp lưu trữ ứng dụng chưa ký cùng tên.

## Cập nhật ảnh chụp giao diện

Ảnh máy tính trong README được render trực tiếp từ Slint UI hiện tại; xem lệnh tạo trong [cập nhật hình minh họa tài liệu](images/REGENERATE.md).

## Android

Ứng dụng điều khiển native nằm trong dự án Gradle riêng `apps/android`, cần thiết bị arm64 Android 8.0+, dùng SDK 36, NDK r28c và Xmake 3.1.1 hoặc mới hơn. Xem biên dịch, cài APK và kiểm thử trình giả lập trong [tài liệu phát triển Android](../apps/android/README.md).

[CI Android](../.github/workflows/build-android.yml) có thể chạy thủ công riêng và được quy trình biên dịch chính gọi. CI cung cấp APK Debug / kiểm thử thiết bị, APK Release chưa ký và báo cáo Lint, chạy kiểm thử giao thức native, kiểm tra căn chỉnh 16 KB và giấy phép; phát hành tag thu thập APK Release chưa ký, cần tự ký trước khi cài. Kiểm thử thiết bị cần thực hiện riêng trên điện thoại arm64 hoặc trình giả lập.
