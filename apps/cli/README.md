# CrossDesk CLI

Ứng dụng điều khiển terminal của CrossDesk, mã nguồn nằm trong `apps/cli` của kho mã này. Xem màn hình từ xa qua phiên ID MiniRTC thực; mặc định chỉ đọc, tắt tiếng. Có hình ảnh ký tự góc phần tư ANSI, phóng to cục bộ và di chuyển, backend đồ họa Kitty / Sixel, cùng tiện ích TUI tùy chỉnh cho Pi.

## Xem trước hình ảnh

| Hình ảnh ký tự ANSI | Hình ảnh pixel giao thức Kitty |
| --- | --- |
| <img src="../../docs/images/cli-ansi-overview.png" width="480" alt="Đầu ra toàn cảnh ANSI của CrossDesk CLI, gồm thanh trạng thái chỉ đọc và gợi ý phím tắt" /> | <img src="../../docs/images/cli-kitty-overview.png" width="480" alt="Đầu ra toàn cảnh pixel Kitty của CrossDesk CLI, hiển thị cùng màn hình từ xa" /> |

Hiệu quả hiển thị tùy terminal và kích thước cửa sổ. Mặc định `auto` lần lượt chọn Kitty, Sixel, ANSI; terminal thông thường có thể dùng `--renderer ansi`, terminal hỗ trợ giao thức tương ứng có thể dùng `--renderer kitty` hoặc `--renderer sixel`. Chế độ pixel chỉ định tường minh cần vượt qua kiểm tra khả năng; thêm đầu ra Sixel không có nghĩa mọi phiên bản Xshell đều hiển thị ảnh, bản thân terminal cũng phải hỗ trợ giao thức này. Toàn cảnh ANSI phù hợp xem bố cục màn hình; để đọc chữ nhỏ, cần vào chế độ chi tiết.

## Cài đặt và kết nối

### Cài qua npm

Cần **Node.js 22 trở lên** và npm. Gói đầu vào chung là [@chunkdash/crossdesk-cli](https://www.npmjs.com/package/@chunkdash/crossdesk-cli), phân phối cho macOS / Linux (x64, arm64) và Windows (x64). Cài đặt tự chọn chương trình native tương ứng, không cần Xmake hay chuỗi công cụ biên dịch C++:

```sh
npm install -g @chunkdash/crossdesk-cli@latest
crossdesk-cli doctor --json
crossdesk-cli 123456789
```

Thay ID ví dụ bằng ID 9 chữ số của máy được điều khiển, nhập mật khẩu ẩn theo lời nhắc. `doctor` chỉ kiểm tra môi trường cục bộ, không kết nối. Chạy lại lệnh cài đặt trên để cập nhật bản mới nhất.

Nếu không cần cài toàn cục, dùng `npx`; lần chạy đầu tải gói nền tảng tương ứng:

```sh
npx @chunkdash/crossdesk-cli@latest doctor --json
npx @chunkdash/crossdesk-cli@latest 123456789
```

Hãy cài gói đầu vào chung và giữ phụ thuộc tùy chọn npm, không dùng `--omit=optional`. Chỉ cài gói nền tảng không đăng ký lệnh. Xem yêu cầu chạy, chuyển đổi gói cũ và xử lý lỗi cài đặt trong [hướng dẫn cài đặt](docs/npm-distribution.md).

### Biên dịch từ mã nguồn

Chạy các lệnh sau ở thư mục gốc kho mã CrossDesk để biên dịch CLI:

```sh
./apps/cli/scripts/prepare-deps.sh
xmake f -y --CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y --USE_CUDA=n -m release
xmake build crossdesk-cli
xmake run crossdesk-cli doctor --json
xmake run crossdesk-cli 123456789 --quality high --zoom native
```

Sau khi biên dịch, bạn có thể chạy trực tiếp `build/<platform>/<arch>/release/crossdesk-cli` hoặc thêm thư mục tương ứng vào PATH; tên tệp trên Windows là `crossdesk-cli.exe`.

### Lệnh kết nối thường dùng

Các ví dụ sau dành cho cài npm toàn cục hoặc môi trường đã thêm chương trình native vào PATH. `crossdesk-cli <ID-thiết-bị> [tùy-chọn]` là cách kết nối mặc định, `view <ID-thiết-bị>` giữ làm bí danh tương thích. ID thiết bị chỉ minh họa định dạng; hãy thay bằng thiết bị bạn có quyền truy cập, nhập mật khẩu qua lời nhắc ẩn:

```sh
crossdesk-cli 123456789 --renderer kitty
crossdesk-cli 123456789 --renderer sixel --fps 8
crossdesk-cli 123456789 --renderer ansi --zoom native
crossdesk-cli 123456789 --quality balanced --fps 8
crossdesk-cli 123456789 --display 1
```

Mặc định yêu cầu chất lượng `high` (tối đa 1920 × 1080) và ưu tiên độ rõ nét; chỉ số `--display` bắt đầu từ `0`. CLI tương tác mặc định nhắm đầu ra cục bộ 30 FPS, nguồn khung từ xa thích ứng theo thời gian xuất; `--fps` tường minh cố định mục tiêu và yêu cầu xem trước tương ứng. Ở mọi tốc độ khung hình đều ưu tiên giữ độ phân giải; chất lượng và tốc độ thực tế vẫn tùy thiết bị, terminal và đường truyền. Xem thêm tham số trong [README chính](../../README.md#cli) và [hướng dẫn terminal](docs/terminal-client.md).

## Xem, điều khiển và thoát

| Thao tác | Phím |
| --- | --- |
| Bật điều khiển từ xa | Nhấn `c` ở chế độ chỉ đọc |
| Thu phóng | Dùng `+` / `=` / `-` quanh vị trí chuột ở chế độ chỉ đọc hoặc lệnh cục bộ; chế độ chỉ đọc cũng dùng được con lăn |
| Về toàn cảnh | Nhấn `f` ở chế độ chỉ đọc hoặc lệnh cục bộ, đồng thời đặt lại tỷ lệ, vị trí và chế độ chi tiết |
| Di chuyển khung nhìn | Dùng phím mũi tên hoặc `i` lên, `j` trái, `k` xuống, `l` phải ở chế độ chỉ đọc hoặc lệnh cục bộ |
| Chi tiết nguyên bản | Nhấn vị trí đích ở chế độ chỉ đọc hoặc khởi động với `--zoom native` |
| Tạm dừng nhập từ xa và vào chế độ lệnh cục bộ | `Ctrl+]` |
| Chọn trạng thái điều khiển khi quay lại | Nhấn `c` ở chế độ lệnh cục bộ, rồi `Esc` / `Ctrl+C` để quay lại |
| Thoát kết nối | Nhấn `q` / `Ctrl+C` ở chế độ chỉ đọc; ở chế độ điều khiển, nhấn `Ctrl+]` trước rồi `q` |

Thu phóng cố giữ nội dung dưới chuột tại chỗ; nếu không có vị trí chuột hợp lệ, dùng tâm hiện tại. Gợi ý dưới cùng đổi theo chế độ và chiều rộng cửa sổ; `Zoom at mouse` là thu phóng neo chuột, `Full view` / `Full` là khôi phục toàn cảnh.

Chế độ lệnh cục bộ tiếp tục sau thu phóng, di chuyển và nhấn `c` cho đến khi quay lại thủ công. Trong chế độ này chuột chỉ cập nhật điểm neo thu phóng; nhấn và lăn không thao tác khung nhìn. Ở chế độ điều khiển, `Ctrl+T`, `Ctrl+C` và phím thường đều gửi đến máy từ xa. Bạn cũng có thể nhấn lại `Ctrl+]` để rời chế độ lệnh cục bộ; nếu quay về chế độ điều khiển, tổ hợp này cũng được gửi đến máy từ xa.

## Phạm vi tính năng và trợ giúp thêm

CLI chỉ điều khiển máy từ xa, không phát âm thanh, truyền tệp, đồng bộ clipboard hay kết nối IP trực tiếp; cũng không kết nối được máy bật giới hạn thiết bị đáng tin cậy. Máy được điều khiển cần màn hình khả dụng; máy chủ self-host cần hỗ trợ danh tính tạm CLI.

- [Hướng dẫn terminal](docs/terminal-client.md): tham số, hiển thị, nhập liệu, tiện ích Pi và khắc phục sự cố.
- [Cài đặt và phân phối npm](docs/npm-distribution.md): yêu cầu nền tảng, chuyển đổi bản cũ, đóng gói và phát hành.
- [Tương thích và giới hạn](docs/validation.md): phạm vi nền tảng và terminal đã xác nhận.
- [Hướng dẫn MCP](../mcp/README.md): thao tác máy từ xa qua Agent và theo dõi trực tiếp.
- [Giao diện tự động hóa](docs/automation-protocol.md) / [giao diện cầu nối terminal](docs/bridge-protocol.md): tích hợp script, ứng dụng hoặc giao diện terminal tùy chỉnh.
- [Thông tin mã nguồn và giấy phép](docs/source-provenance.md).
