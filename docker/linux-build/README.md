# Biên dịch ứng dụng Linux bằng container

[Về hướng dẫn biên dịch](../../docs/BUILD.md#linux)

Khi cần tự biên dịch trong môi trường Ubuntu 20.04 / glibc 2.31, bạn có thể dùng Dockerfile trong thư mục này. Cài đặt thông thường không cần Docker, chỉ tải DEB đúng kiến trúc.

Ảnh cung cấp GCC 10, CMake 3.31.6, Xmake 3.1.1, Rust 1.92.0 và phụ thuộc biên dịch Linux. Lần đầu tạo ảnh cần mạng để tải và biên dịch phụ thuộc, có thể tốn nhiều thời gian và dung lượng đĩa.

## Tạo ảnh môi trường biên dịch cục bộ

Cài Docker trước và tải mã nguồn đầy đủ gồm submodule. Chạy một lệnh đúng kiến trúc máy này ở thư mục gốc kho mã.

x64 / amd64：

```sh
git submodule update --init --recursive
docker build -f docker/linux-build/Dockerfile -t crossdesk-build:local .
```

arm64 dùng ảnh nền Ubuntu, không chứa CUDA SDK:

```sh
git submodule update --init --recursive
docker build -f docker/linux-build/Dockerfile \
  --build-arg BASE_IMAGE=ubuntu:20.04 -t crossdesk-build:local .
```

Các lệnh trên biên dịch trên kiến trúc hiện tại, không tự tạo gói cho cả hai kiến trúc. Nếu cần kiến trúc khác, biên dịch trong môi trường tương ứng.

## Vào container và biên dịch

Khởi động từ thư mục gốc kho mã:

```sh
docker run --rm -it -v "$PWD:/workspace" -w /workspace crossdesk-build:local
```

Thực hiện trong container:

```sh
verify-crossdesk-build-image
xmake f -c -m release --USE_CUDA=false --toolchain=gcc-10 -y
xmake b crossdesk
```

Thư mục nguồn được gắn vào container, đầu ra biên dịch giữ trong `build/` của máy chủ. Container mặc định chạy bằng root, sản phẩm trên máy Linux có thể thuộc root. Sau khi đổi kiến trúc hoặc chuỗi công cụ, cần cấu hình lại để tránh dùng sản phẩm cũ không khớp.

Xem lệnh tạo DEB trong [các bước đóng gói Linux](../../docs/BUILD.md#linux); phiên bản đóng gói cần khớp phiên bản biên dịch.

## Wayland và yêu cầu khi chạy

Ảnh cung cấp header PipeWire 0.3 / SPA để biên dịch, không đóng thư viện động PipeWire vào gói cài. Chạy ghi hình Wayland cần máy được điều khiển cung cấp PipeWire 0.3 và thành phần cấp quyền màn hình; nếu thiếu có thể dùng X11.

Container biên dịch không phải môi trường chạy máy tính từ xa. Để chạy CrossDesk trên máy Linux không màn hình, chuẩn bị màn hình hiện có hoặc màn hình ảo độc lập theo [hướng dẫn chạy không màn hình](../../docs/HEADLESS.md).

## Phát hành và bảo trì ảnh

Tag chuỗi công cụ ghi nhận môi trường biên dịch thực tế; ảnh ứng dụng dùng để biên dịch theo [quy trình biên dịch](../../.github/workflows/build.yml). Trước khi đổi tag nên xác nhận cả biến thể amd64 và arm64 đều khả dụng.



Khi push các tệp ảnh hưởng ảnh lên nhánh `ci/linux-build-image`,
`build-linux-image.yml` biên dịch và xác minh riêng trên runner GitHub gốc amd64/arm64,
sau khi tất cả thành công mới hợp nhất thành cùng một ảnh multi-arch:

```text
crossdesk/ubuntu20.04:buildenv-cuda12.6.3-gcc10-cmake3.31.6-pipewire0.3.48-xmake3.1.1-rust1.92.0
```

Docker tự chọn kiến trúc đúng theo máy đang chạy. Biến thể amd64 dựa trên
`nvidia/cuda:12.6.3-devel-ubuntu20.04`; biến thể arm64 không chứa CUDA SDK
không cần cho biên dịch hiện tại. Cả hai dùng Ubuntu 20.04/glibc 2.31, nên
cả gói thường và gói màn hình ảo Xfce được tạo đều chạy trên Ubuntu 20.04 trở lên.

Kho Ubuntu 20.04 chỉ cung cấp PipeWire 0.2. Ảnh dùng
`install-pipewire-sdk.sh` để cài header PipeWire 0.3.48/SPA phiên bản cố định, nhưng không
cài hay đóng gói thư viện động PipeWire. CrossDesk dùng `dlopen()` khi chạy để sử dụng
PipeWire 0.3 của hệ thống chủ: nếu Ubuntu 20.04 không có thư viện này vẫn dùng X11 (và DRM
nếu bật khi biên dịch); Ubuntu 22.04 trở lên có thể dùng ghi hình Wayland.

Mỗi lần phát hành thành công còn tạo `sha-<SHA-commit-đầy-đủ>` để quay lui chính xác; `latest` và tag
phiên bản chuỗi công cụ trên chỉ cập nhật khi xác minh cả hai kiến trúc thành công.

## Cấu hình GitHub

Cấu hình trong Actions secrets của kho mã:

- `DOCKERHUB_USERNAME`: Tên người dùng Docker Hub có quyền push `crossdesk/ubuntu20.04`
- `DOCKERHUB_TOKEN`: Access token Docker Hub của người dùng đó

Sau khi tệp quy trình vào nhánh mặc định, có thể chạy thủ công từ trang Actions. Để tránh lần đầu phát hành
tag mới mà biên dịch ứng dụng chạy trước khi ảnh hoàn tất, `build.yml` thường không phản hồi
push nhánh `ci/linux-build-image`. Lần đầu bật, nên push nhánh đó và chờ ảnh
phát hành thành công rồi mới hợp nhất thay đổi quy trình biên dịch ứng dụng vào `main`.

## Cập nhật phụ thuộc

Sau khi sửa Dockerfile, bản kê xmake, công thức gói cục bộ hoặc trong submodule, chỉ cần push nhánh này.
Khi nâng cấp chuỗi công cụ, nên sửa tham số phiên bản Dockerfile và `STACK_TAG` trong quy trình ảnh trước,
chờ ảnh cả hai kiến trúc phát hành thành công rồi mới đổi tag ảnh được quy trình biên dịch thường tham chiếu.
