# CrossDesk MCP

Cho phép Codex hoặc DeepSeek Harness xem và thao tác máy từ xa khi bạn cấp quyền, đồng thời hiển thị hình ảnh từ xa bên phải. Bạn có thể giao nhiệm vụ bằng ngôn ngữ tự nhiên, tạm dừng thao tác, tiếp quản chuột/bàn phím hoặc ngắt kết nối bất cứ lúc nào.

[Chuẩn bị](#install) · [Tải gói cài đặt](#install-package) · [Cài DSH](#install-dsh) · [Cài Codex](#install-codex) · [Kết nối thiết bị](#quy-trình-kết-nối-của-người-dùng) · [Sử dụng thường ngày](#ví-dụ-sử-dụng) · [Câu hỏi thường gặp](#install-troubleshooting)

<a id="install"></a>

## Chuẩn bị

Bạn cần hai máy tính:

- **Máy này**: chạy Codex bản máy tính hoặc DeepSeek Harness (DSH) bản máy tính, cài CrossDesk MCP.
- **Máy từ xa**: cài và chạy [CrossDesk](https://www.crossdesk.cn/), giữ trực tuyến, chuẩn bị ID thiết bị 9 chữ số và mật khẩu truy cập ứng dụng hiển thị. Cấp quyền ghi màn hình, trợ năng... theo lời nhắc hệ thống; xem [hướng dẫn ứng dụng](../../docs/USER_GUIDE.md).

| Ứng dụng bạn dùng | Cần chuẩn bị trên máy này | Hình ảnh ở đâu |
| --- | --- | --- |
| DeepSeek Harness bản máy tính | Một gói plugin nền tảng CrossDesk MCP | Trình duyệt bên phải cuộc trò chuyện hiện tại |
| Codex bản máy tính | Node.js 22 trở lên và gói cài nền tảng CrossDesk MCP | Trình duyệt tích hợp bên phải |
| Codex CLI | Node.js 22 trở lên và gói cài nền tảng CrossDesk MCP | Trình duyệt hệ thống hoặc terminal riêng |

Cài đặt cần mạng để tải phụ thuộc. MCP và ứng dụng hiển thị hình ảnh nên chạy trên cùng máy cục bộ. Nếu máy từ xa dùng máy chủ tự triển khai, nhập cùng máy chủ và cổng trong “Cài đặt nâng cao” của bảng kết nối.

Thanh bên hiện đã được kiểm chứng trên Codex macOS và DSH 0.2.2-rc.1; Harness bản web không hỗ trợ thanh bên này. Hình ảnh, nhập liệu và quyền hệ thống trên nền tảng khác cần theo môi trường thực tế.

<a id="install-package"></a>

## Tải gói cài đặt

DSH và Codex đều dùng **gói cài đặt nền tảng** trong bảng sau, đã chứa chương trình native cần thiết.

1. Đăng nhập GitHub, mở [trang biên dịch CrossDesk](https://github.com/kunkundi/crossdesk/actions/workflows/build-cli.yml).
2. Chọn một lần chạy thành công, tải tệp đúng máy của bạn trong vùng **Artifacts** trên trang.
3. Giải nén `.zip` đã tải, lấy gói `.tgz` bên trong, ví dụ `chunkdash-crossdesk-mcp-0.2.0.tgz`. Giữ `.tgz` để dùng trực tiếp khi cài.

GitHub yêu cầu đăng nhập mới tải được sản phẩm biên dịch; xem [tải sản phẩm biên dịch](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts).

| Máy chạy Codex / DSH | Tên sản phẩm tải xuống |
| --- | --- |
| Apple Silicon Mac (chip dòng M) | `crossdesk-mcp-darwin-arm64` |
| Intel Mac | `crossdesk-mcp-darwin-x64` |
| Linux x64（glibc） | `crossdesk-mcp-linux-x64` |
| Linux arm64（glibc） | `crossdesk-mcp-linux-arm64` |
| Windows x64 | `crossdesk-mcp-win32-x64` |

Mac xem chip trong “About This Mac”; Windows xem loại hệ thống trong “Settings → System → About”; Linux có thể chạy `uname -m` để xem kiến trúc, `x86_64` tương ứng x64, `aarch64` tương ứng arm64.

Tên `.tgz` sau giải nén giống nhau giữa các nền tảng, hãy phân biệt bằng tên sản phẩm khi tải. Nếu một lần chạy không có sản phẩm tương ứng trong bảng, chọn lần khác có cung cấp; cũng có thể tự biên dịch theo [hướng dẫn mã nguồn](DEVELOPMENT.md#chạy-từ-mã-nguồn).

Dưới đây dùng `0.2.0` làm ví dụ. **Mọi `/absolute/path/...` cần thay bằng đường dẫn đầy đủ trên máy bạn**, số phiên bản cũng nên khớp tệp tải thực tế. Giữ dấu nháy nếu đường dẫn có dấu cách.

<a id="install-dsh"></a>

## Cài trong DeepSeek Harness

### 1. Cài plugin

Chọn gói `.tgz` cục bộ trên trang plugin Harness, hoặc chạy trong terminal:

```sh
dsh plugin --profile desktop add "/absolute/path/chunkdash-crossdesk-mcp-0.2.0.tgz"
```

Gói plugin này tự cung cấp công cụ MCP và hình ảnh thanh bên, không cần cài riêng Node.js, CLI native hay điền cấu hình khởi động MCP.

Nếu terminal macOS báo không tìm thấy `dsh`, bạn có thể dùng lệnh kèm ứng dụng:

```sh
"/Applications/DeepSeek Harness.app/Contents/Resources/runtime/cli/bin/dsh" plugin --profile desktop add "/absolute/path/chunkdash-crossdesk-mcp-0.2.0.tgz"
```

Lệnh trên tương ứng vị trí cài mặc định và cấu hình `desktop`. Nếu dùng vị trí hoặc profile tùy chỉnh, thay bằng giá trị của bạn.

### 2. Mở CrossDesk

Khởi động lại Harness, xác nhận CrossDesk (`@chunkdash/crossdesk-mcp`) đã bật trên trang plugin. Sau đó gửi trong cuộc trò chuyện:

> Mở CrossDesk để tôi chọn máy từ xa.

Khi bảng kết nối thiết bị xuất hiện bên phải, tiếp tục [kết nối thiết bị](#quy-trình-kết-nối-của-người-dùng). Bản thân cài plugin không kết nối máy từ xa nào.

<a id="install-codex"></a>

## Cài trong Codex

### 1. Cài vào thư mục cố định

Cài Node.js 22 trở lên trước. Mở terminal, chạy lệnh theo hệ thống của bạn và thay đường dẫn gói cài.

macOS / Linux：

```sh
mkdir -p "$HOME/crossdesk-mcp"
cd "$HOME/crossdesk-mcp"
npm init -y
npm install "/absolute/path/chunkdash-crossdesk-mcp-0.2.0.tgz"
```

Windows PowerShell：

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE/crossdesk-mcp"
Set-Location "$env:USERPROFILE/crossdesk-mcp"
npm init -y
npm install "C:/Downloads/chunkdash-crossdesk-mcp-0.2.0.tgz"
```

Sau khi cài xong, chạy trong cùng thư mục:

```sh
node node_modules/@chunkdash/crossdesk-mcp/dist/main.js --help
```

Nếu xuất hiện trợ giúp lệnh thì điểm vào Node chạy được. Hãy giữ thư mục cài; Codex sẽ khởi động CrossDesk từ đây về sau.

### 2. Thêm cấu hình MCP

Mở `.codex/config.toml` trong thư mục người dùng (macOS / Linux là `~/.codex/config.toml`), thêm cấu hình sau. Nếu đã có `[mcp_servers.crossdesk]`, cập nhật phần đó, giữ cấu hình dịch vụ khác.

```toml
[mcp_servers.crossdesk]
command = "/absolute/path/node"
args = [
  "/absolute/path/crossdesk-mcp/node_modules/@chunkdash/crossdesk-mcp/dist/main.js",
  "serve", "--stdio",
  "--panel=browser", "--browser-launch=host"
]
enabled = true
startup_timeout_sec = 30
tool_timeout_sec = 90
```

Cần thay hai đường dẫn:

- `command`: đường dẫn đầy đủ tệp thực thi Node. macOS / Linux tra bằng `command -v node`; Windows PowerShell tra bằng `(Get-Command node).Source`.
- Mục đầu `args`: đường dẫn đầy đủ `node_modules/@chunkdash/crossdesk-mcp/dist/main.js` dưới thư mục cài ở bước trước.

Không dùng `~` trong đường dẫn cấu hình. Windows có thể dùng gạch chéo xuôi, ví dụ `C:/Program Files/nodejs/node.exe`. Giữ `--panel=browser --browser-launch=host` để Codex bản máy tính mở trình duyệt bên phải. Tham khảo định dạng trong [tài liệu Codex chính thức](https://learn.chatgpt.com/docs/extend/mcp?surface=cli).

### 3. Mở CrossDesk

Khởi động lại Codex, gửi trong cuộc trò chuyện:

> Mở CrossDesk để tôi chọn máy từ xa.

Sau khi bảng kết nối xuất hiện bên phải, cấp quyền theo các bước dưới. Lần cài đầu không có thiết bị đã lưu là bình thường.

**Nếu dùng Codex CLI**, đổi `--browser-launch=host` trong cấu hình thành `--browser-launch=system`, hình ảnh sẽ hiện trong trình duyệt hệ thống cục bộ. Terminal không có đồ họa xem [cấu hình hiển thị](DEVELOPMENT.md#chính-sách-bảng-điều-khiển).

## Quy trình kết nối của người dùng

1. Chọn thiết bị đã lưu trong bảng bên phải hoặc chọn “Kết nối thiết bị mới”.
2. Nhập **ID thiết bị** và **Mật khẩu truy cập** của máy từ xa. Chỉ điền mật khẩu trong bảng, không gửi vào cuộc trò chuyện.
3. Chọn phạm vi cấp quyền: **Chỉ xem màn hình** phù hợp quan sát và kiểm tra; nếu cần mở trang web, nhập văn bản, thao tác tệp, chọn **Cho phép Agent thao tác**.
4. Nhấn **Cấp quyền và kết nối**. Khi kết nối thành công, bảng hiển thị màn hình từ xa, độ phân giải và tốc độ khung hình.
5. Quay lại cuộc trò chuyện để giao nhiệm vụ. Nếu đã giao trước khi kết nối, Agent tiếp tục sau khi kết nối sẵn sàng.

### Cách kết nối trực tiếp lần sau

Mở **Lưu và cấp quyền** trong bảng kết nối, đặt theo nhu cầu:

| Tùy chọn | Tác dụng |
| --- | --- |
| Lưu thiết bị | Ghi nhớ thông tin thiết bị, có thể thêm ghi chú như “Máy tính công việc” |
| Nhớ mật khẩu | Lưu mật khẩu truy cập trong kho thông tin xác thực hệ thống cục bộ |
| Cho phép tự động kết nối lần sau | Cho phép Agent dùng phạm vi cấp quyền lần này để kết nối trong nhiệm vụ sau |

Hai mục sau cần chọn “Lưu thiết bị” trước. Nếu muốn kết nối trực tiếp qua ghi chú về sau, hãy lưu mật khẩu và cho tự kết nối; chỉ lưu thiết bị không tự cấp quyền điều khiển.

Ví dụ:

> Dùng CrossDesk kết nối “Máy tính công việc”, mở trình duyệt tìm thông tin thời tiết giúp tôi.

Nếu lần này chỉ cấp quyền xem mà sau đó cần thao tác, hãy ngắt rồi kết nối lại, chọn “Cho phép Agent thao tác”.

## Ví dụ sử dụng

Sau khi kết nối, mô tả thẳng việc cần làm cho Agent:

| Nhiệm vụ | Có thể nói |
| --- | --- |
| Xem màn hình | “Xem máy từ xa hiện đang hiển thị gì.” |
| Thao tác trình duyệt | “Tìm một con bò trong trình duyệt từ xa và mở video liên quan.” |
| Tìm tệp | “Xem giúp tôi thư mục tải xuống từ xa có những PDF nào.” |
| Tải tệp | “Tải báo cáo trên màn hình máy từ xa vào thư mục Downloads trên máy này.” |
| Chuyển màn hình | “Chuyển sang màn hình thứ hai của máy từ xa.” |
| Chỉnh hình ảnh | “Giảm độ phân giải xem trước từ xa một chút để hình ảnh mượt hơn.” |

Thao tác cụ thể tùy quyền thiết bị, quyền hệ thống từ xa và hỗ trợ ứng dụng. Hình trực tiếp dùng theo dõi; tốc độ thao tác Agent còn tùy thời gian quan sát và quyết định.

### Tạm dừng, tiếp quản và kết thúc nhiệm vụ

| Bạn muốn làm gì | Thao tác trong bảng |
| --- | --- |
| Tạm dừng thao tác Agent | Nhấn **Tạm dừng Agent**; khi cần tiếp tục, chính bạn nhấn **Tiếp tục Agent** |
| Tự thao tác máy từ xa | Mở **Tùy chọn phiên → Tiếp quản thủ công**, xong nhấn **Trả lại Agent** |
| Nghe âm thanh từ xa | Mở **Tùy chọn phiên → Phát âm thanh**, chỉnh âm lượng theo nhu cầu |
| Phóng to hình ảnh | Mở **Tùy chọn phiên → Phóng to hình ảnh** |
| Kết thúc phiên từ xa | Mở **Tùy chọn phiên → Ngắt kết nối** hoặc nhờ Agent ngắt |

Tiếp quản thủ công cần đã cấp quyền thao tác cho kết nối này. **Đóng thanh bên không ngắt kết nối từ xa**; khi dùng xong hãy chọn “Ngắt kết nối”.

## Cập nhật và gỡ cài đặt

Kết thúc nhiệm vụ đang chạy và ngắt kết nối từ xa trước khi cập nhật hoặc gỡ.

**DSH:** cài gói nền tảng mới trên trang plugin hoặc chạy lại lệnh cài `dsh plugin ... add`, rồi khởi động lại Harness. Gỡ trên trang plugin hoặc chạy:

```sh
dsh plugin --profile desktop remove @chunkdash/crossdesk-mcp
```

**Codex:** chạy `npm install "/absolute/path/goi-cai-dat-moi.tgz"` trong thư mục cài `crossdesk-mcp` cũ rồi khởi động lại Codex. Khi tắt, đổi `enabled` thành `false` trong cấu hình; khi gỡ, bỏ phần cấu hình tương ứng và chạy `npm uninstall @chunkdash/crossdesk-mcp` tại thư mục cài.

Gỡ cài đặt không xóa cấu hình thiết bị và thông tin xác thực hệ thống đã lưu trên máy.

<a id="install-troubleshooting"></a>

## Câu hỏi thường gặp

| Vấn đề gặp phải | Cách xử lý |
| --- | --- |
| Không tìm thấy tệp tải hoặc nút tải | Đăng nhập GitHub, xác nhận Artifacts của lần chạy đã chọn có tệp `crossdesk-mcp-...` đúng nền tảng; nếu sản phẩm hết hạn, chọn lần chạy khác khả dụng |
| DSH báo không tìm thấy `dsh` | Cài qua trang plugin; macOS cũng có thể dùng lệnh đầy đủ trong ứng dụng ở trên |
| Không có công cụ CrossDesk sau khi cài DSH | Xác nhận plugin đã bật và khởi động lại Harness; xem lỗi trên trang plugin. Nếu báo thiếu chương trình native, cài lại gói đúng nền tảng |
| Codex không có công cụ CrossDesk | Kiểm tra phiên bản Node và hai đường dẫn đầy đủ trong cấu hình, chạy `--help` trước rồi khởi động lại Codex. Nếu có Codex CLI, có thể xem bằng `codex mcp list` |
| Có công cụ nhưng không có hình bên phải | Xác nhận dùng bản máy tính; Codex giữ hai tham số hiển thị ở trên. Nhờ Agent “mở lại hình ảnh kết nối CrossDesk hiện tại” |
| Trang mở nhưng không kết nối được thiết bị | Xác nhận máy từ xa trực tuyến, ID và mật khẩu truy cập đúng; nếu tự triển khai máy chủ, kiểm tra “Cài đặt nâng cao” trong bảng |
| Đã kết nối nhưng đen màn hình hoặc không thao tác được | Kiểm tra quyền ghi màn hình, trợ năng... của CrossDesk từ xa; nếu chỉ có quyền xem, kết nối lại và cấp quyền thao tác |
| Kết nối được nhưng một tính năng báo không hỗ trợ | Kiểm tra phiên bản ứng dụng từ xa, cập nhật bản tương thích theo lời nhắc; thao tác cửa sổ/ứng dụng còn tùy hệ thống và hỗ trợ ứng dụng từ xa |
| Không lưu được mật khẩu | Kiểm tra kho thông tin xác thực cục bộ khả dụng; hoặc bỏ “Nhớ mật khẩu”, nhập thủ công trong bảng mỗi lần kết nối |
| Hình ảnh giật | Nhờ Agent giảm độ phân giải hoặc chất lượng xem trước, kiểm tra mạng hai bên. Tốc độ khung hình và tốc độ quyết định Agent là hai chỉ số khác nhau |
| Không có âm thanh | Nhấn “Phát âm thanh” trong bảng, xác nhận máy từ xa cho ghi âm thanh máy tính |
| Máy từ xa khóa màn hình | Nhập mật khẩu khóa hệ thống theo lời nhắc mở khóa; nhập riêng với mật khẩu truy cập CrossDesk |
| Báo kết nối hoặc điểm vào trang hết hạn | Nhờ Agent mở lại hình ảnh kết nối hiện tại, không cần tạo lại kết nối từ xa |
| Báo `DEVICE_BUSY` | Ngắt kết nối của Agent hoặc tiến trình MCP khác với cùng thiết bị trước |
| Báo `serverName ... already in use` | Kiểm tra cài đặt/cấu hình dịch vụ CrossDesk trùng, chỉ bật một phiên bản |
| Báo `NATIVE_PACKAGE_MISSING` hoặc `NATIVE_PROTOCOL_MISMATCH` | Cài lại gói đầy đủ đúng nền tảng trong bảng; chạy nguồn thì kiểm tra [cấu hình chương trình native](DEVELOPMENT.md#chạy-từ-mã-nguồn) |
| Vì sao không tự kết nối lại | Bật “Tự khôi phục khi mất kết nối” theo nhu cầu trong **Chi tiết phiên**; cần quyền tự kết nối đã lưu và thông tin xác thực khả dụng, chủ động ngắt không kích hoạt khôi phục |

Nếu vẫn lỗi, bạn có thể [gửi phản hồi](https://github.com/kunkundi/crossdesk/issues), nêu hệ thống cục bộ và từ xa, phiên bản Codex / DSH, nguồn gói cài, lỗi và bước tái hiện. Không đính kèm mật khẩu thiết bị hoặc địa chỉ trang có thông tin ghép nối.

<a id="从源码运行"></a>

## Tài liệu khác

- [Biên dịch nguồn và cấu hình](DEVELOPMENT.md#chạy-từ-mã-nguồn)
- [Tích hợp MCP host, giao diện công cụ và đóng gói phát hành](DEVELOPMENT.md)
- [Hướng dẫn cài DSH ngắn gọn](integrations/deepseek-harness/README.md)
