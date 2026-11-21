# Biên dịch và tích hợp CrossDesk MCP

Tài liệu này giới thiệu biên dịch từ nguồn, tích hợp ứng dụng chủ, giao diện công cụ, truyền media và đóng gói/phát hành. Cài đặt và thao tác thường ngày xem [hướng dẫn MCP](README.md).

Điều hướng: [biên dịch nguồn](#chạy-từ-mã-nguồn) · [cấu hình phát triển DSH](#harness-development) · [API công cụ](#công-cụ-mô-hình) · [thao tác từ xa](#thao-tác-từ-xa) · [video trực tiếp](#video-trực-tiếp-và-ảnh-theo-nhu-cầu) · [đóng gói](#đóng-gói)

## Phiên bản và phụ thuộc

CLI, gói MCP `@chunkdash/crossdesk-mcp` và plugin DSH tích hợp dùng chung phiên bản, hiện đều là 0.2.0; phụ thuộc gốc của MCP cũng cố định ở 0.2.0. Dùng [script phiên bản](../../docs/VERSIONING.md) sửa phiên bản một công cụ sẽ đồng bộ cả nhóm. Phiên bản nguồn và gói tạo cục bộ không có nghĩa phiên bản đã phát hành trên npm; gói nền tảng chứa chương trình gốc có thể phân phối độc lập.

Hiện có 31 công cụ dành cho mô hình và 11 API riêng cho bảng MCP Apps. MCP cục bộ dùng stdio, không cung cấp MCP Streamable HTTP hoặc công cụ shell từ xa dùng chung. CLI gốc bên điều khiển, máy được điều khiển và máy chủ tín hiệu cần hỗ trợ giao thức tự động hóa tương ứng.

## Chạy từ mã nguồn

Trước tiên clone nguồn có submodule, rồi biên dịch chương trình gốc tại gốc kho; workspace hiện có có thể giữ tùy chọn biên dịch máy tính trước đó:

```sh
git clone --recurse-submodules https://github.com/kunkundi/crossdesk.git
cd crossdesk
```

```sh
sh apps/cli/scripts/prepare-deps.sh
xmake f -y --CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y --USE_CUDA=n -m release
xmake build crossdesk-cli
cd apps/mcp
npm ci
npm run build
```

Môi trường biên dịch đầy đủ xem [hướng dẫn biên dịch nguồn](../../docs/BUILD.md). `npm run build` biên dịch đồng thời MCP, trang giao diện và plugin DSH đi kèm. `npm ci` trong thư mục nguồn chỉ cài phụ thuộc phát triển; **chạy từ nguồn bắt buộc chỉ rõ đường dẫn chương trình gốc**. Chỉ gói phát hành được sinh ra mới khai phụ thuộc nền tảng cố định.

Ví dụ, sau khi biên dịch Release macOS arm64, bạn có thể kiểm trước:

```sh
/absolute/path/crossdesk/build/macosx/arm64/release/crossdesk-cli doctor --json
```

Windows, Linux, x64 hoặc bản Debug dùng đường dẫn sản phẩm thực tế tương ứng. `--native-binary` bắt buộc trỏ tới tệp thực thi, không nhận lệnh shell; bạn cũng có thể dùng `CROSSDESK_NATIVE_BINARY`. Chương trình gốc cần cung cấp các giao thức automation, terminal và video mà MCP yêu cầu. Chỉ chạy được CLI tương tác không có nghĩa phiên bản tương thích.

Hai cách cài đặt dùng đường dẫn khác nhau trong các phần dưới:

| Mục | Gói cài cục bộ | Biên dịch nguồn |
| --- | --- | --- |
| Điểm vào MCP | `<thu-muc-cai-dat>/node_modules/@chunkdash/crossdesk-mcp/dist/main.js` | `<thu-muc-du-an>/apps/mcp/dist/main.js` |
| Thư mục plugin DSH | `<thu-muc-cai-dat>/node_modules/@chunkdash/crossdesk-mcp` (hoặc cài `.tgz` trực tiếp) | `<thu-muc-du-an>/apps/mcp` |
| Chương trình gốc | Tích hợp trong gói nền tảng, gói chung dùng phụ thuộc cố định hoặc chỉ đường rõ | Chỉ đường sản phẩm rõ |

Dịch vụ không kết nối từ xa hay mở trình duyệt khi khởi động; chỉ bắt đầu quy trình cấp quyền/kết nối sau khi Agent gọi công cụ mở hoặc kết nối.

### Cấu hình mã nguồn Codex

Khi cấu hình dịch vụ theo [hướng dẫn cài Codex](README.md#install-codex), người dùng nguồn thay `args` bằng nội dung sau. Ví dụ đường dẫn gốc ứng với Release macOS arm64; các bản biên dịch khác thay bằng đường dẫn sản phẩm của mình:

```toml
args = [
  "/absolute/path/crossdesk/apps/mcp/dist/main.js",
  "serve", "--stdio",
  "--panel=browser", "--browser-launch=host",
  "--native-binary", "/absolute/path/crossdesk/build/macosx/arm64/release/crossdesk-cli"
]
```

<a id="harness-development"></a>

## Phát triển và cấu hình plugin DSH

Người dùng nguồn cần hoàn tất [biên dịch từ nguồn](#chạy-từ-mã-nguồn) ở trên, rồi cài **thư mục gốc MCP**:

```sh
dsh plugin --profile desktop add /absolute/path/crossdesk/apps/mcp
```

Thư mục nguồn không chứa chương trình gốc. Bạn cần thêm cấu hình cho `crossdesk` trong `~/.dsh/profiles/desktop/cordis.patch.yml`; nếu dùng `DSH_HOME` hoặc profile riêng, đổi sang đường dẫn tương ứng. Hợp mục vào danh sách hiện có và thay đường dẫn gốc bằng sản phẩm thực tế trên máy:

```yaml
- id: crossdesk
  config:
    nativeBinary: /absolute/path/crossdesk/build/macosx/arm64/release/crossdesk-cli
```

Bạn chỉ cần điền cấu hình plugin khi thay đổi mặc định:

| Trường | Mặc định | Mục đích |
| --- | --- | --- |
| `nativeBinary` | Gốc trong gói hoặc phụ thuộc nền tảng cố định | Phát triển nguồn/dùng CLI tương thích rõ, bắt buộc tuyệt đối |
| `configDir` | `~/.config/crossdesk/mcp/` | Chỉ thư mục cấu hình thiết bị, bắt buộc tuyệt đối |
| `serverName` | `crossdesk` | Namespace công cụ MCP; thanh bên dùng cùng tên |

Khi cấu hình người dùng ghi đè toàn bộ `config`, cần giữ các trường vẫn còn dùng. Windows có thể dùng đường dẫn `C:/.../crossdesk-cli.exe`. Cơ chế tích hợp tham khảo [Harness bundle](https://github.com/deepseek-ai/deepseek-harness/tree/master/packages/boot/app-boot#profiles) và [MCP Client](https://github.com/deepseek-ai/deepseek-harness/tree/master/packages/mcp/mcp-client).

Nguồn DSH ở `integrations/deepseek-harness/src/`: `plugin.js` quản lý kiểm tra chương trình gốc và vòng đời MCP, `host.js` bàn giao kết quả công cụ, `client.js` mở thanh bên của hội thoại tương ứng. Điểm vào biên dịch sinh `dist/harness.js` và `dist/harness-client.js`; `cordis.patch.yml` ở gốc gói đăng ký thành phần `crossdesk`. Host cung cấp MCP Client, dùng lại quyền công cụ, giữ chỗ namespace, xử lý ảnh và dọn kết nối của nó.

## Gói chung và phụ thuộc gốc

Tải/cài gói nền tảng xem [hướng dẫn cài đặt](README.md#install-package). Nếu dùng gói chung không chứa chương trình gốc, như `chunkdash-crossdesk-mcp-0.2.0.tgz`, hãy cài ở thư mục cố định. Bạn có thể tìm sản phẩm `mcp-package` trong lần chạy [Build MCP](https://github.com/kunkundi/crossdesk/actions/workflows/build-mcp.yml) thành công. Tệp tải từ Actions là ZIP bọc ngoài; giải nén trước rồi đưa `.tgz` bên trong cho npm, không đưa ZIP trực tiếp. Nếu chưa có gói cài phù hợp, dùng [biên dịch từ nguồn](#chạy-từ-mã-nguồn).

macOS / Linux：

```sh
mkdir -p "$HOME/crossdesk-mcp"
cd "$HOME/crossdesk-mcp"
npm init -y
npm install /absolute/path/chunkdash-crossdesk-mcp-0.2.0.tgz
node node_modules/@chunkdash/crossdesk-mcp/dist/main.js --help
```

Windows PowerShell：

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE/crossdesk-mcp"
Set-Location "$env:USERPROFILE/crossdesk-mcp"
npm init -y
npm install "C:/Downloads/chunkdash-crossdesk-mcp-0.2.0.tgz"
node node_modules/@chunkdash/crossdesk-mcp/dist/main.js --help
```

Gói cài đã chứa trang giao diện và plugin DSH đã biên dịch, không cần chạy lại `npm run build`. Sản phẩm `crossdesk-mcp-<nen-tang>` từ quy trình CLI còn chứa CLI gốc tương ứng, nên không cần cài riêng CLI. Các bước bổ sung dưới chỉ dành cho gói chung `mcp-package` không có chương trình gốc. Khi cài gói chung, hãy giữ optional dependencies: gói sẽ thử cài phụ thuộc nền tảng cố định, hỗ trợ macOS arm64/x64, glibc Linux arm64/x64 và Windows x64. MCP và gói gốc dùng cùng phiên bản, ví dụ dưới là 0.2.0. Nếu gói gốc tương ứng chưa phát hành hoặc tải thất bại, bạn cần cài gói nền tảng do nhà duy trì cung cấp vào **cùng thư mục cài đặt**, ví dụ macOS arm64:

```sh
npm install /absolute/path/chunkdash-crossdesk-cli-darwin-arm64-0.2.0.tgz
```

Bạn cũng có thể thêm `--native-binary` vào cấu hình host bên dưới, trỏ tới tệp `crossdesk-cli` tương thích đã biên dịch, trên Windows là `.exe`. MCP không tự tìm CLI cài toàn cục: khi không chỉ đường dẫn, nó ưu tiên chương trình gốc đúng nền tảng trong gói, nếu không thì phân giải phụ thuộc nền tảng cố định, không tìm PATH. `--help` chỉ kiểm điểm vào Node; dịch vụ stdio thông thường kiểm giao thức gốc khi kết nối, còn plugin DSH hợp nhất kiểm trước lúc khởi động.

Nếu dùng npm registry, trước hết xác nhận phiên bản cần dùng đã phát hành, rồi thay lệnh cài trên bằng `npm install @chunkdash/crossdesk-mcp@<ban-da-phat-hanh>`. Số phiên bản trong kho không có nghĩa npm đã có gói đó.

## Quy trình kết nối người dùng

1. Nói với Agent “Mở CrossDesk”. `crossdesk_open` tạo `view_id`, trả về lựa chọn bảng và trạng thái chờ người dùng.
2. Chọn thiết bị đã lưu hoặc nhập ID thiết bị 9 chữ số trong bảng; mật khẩu nhập ở bảng. Bạn có thể chọn chỉ xem hoặc cho phép Agent điều khiển.
3. “Lưu thiết bị”, “Nhớ mật khẩu” và “Cho phép tự kết nối sau” lần lượt điều khiển sổ thiết bị, kho xác thực hệ thống và quyền bền vững. Hai mục sau cần lưu thiết bị và mặc định đều tắt. Lưu mật khẩu không tự cấp quyền điều khiển.
4. Khi còn tác vụ từ xa chờ thực hiện, Agent gọi ngay `crossdesk_get_session({view_id, wait_ms: 25000})` sau khi mở bảng để đợi kết nối. Khi sẵn sàng, công cụ trả trực tiếp ID phiên và quyền để Agent tiếp tục. Bạn không cần trở về hội thoại gửi xác nhận kết nối hoặc nhấn nút tiếp tục. Bảng trình duyệt và Apps dùng cùng quy trình.
5. Agent lấy ảnh chụp riêng, dùng `observation_id` và tọa độ ảnh để gửi thao tác. Xem trước trong bảng không tiêu thụ chuỗi ảnh của mô hình.

`crossdesk_open` trả ngay để host Apps có cơ hội kết xuất bảng; chờ kết nối được thực hiện trong lần gọi `crossdesk_get_session` tiếp theo. Truy vấn theo `view_id` mặc định đợi tối đa 25 giây, còn `wait_ms: 0` trả ngay. Nếu chưa sẵn sàng, công cụ trả `wait_timed_out: true`; Agent tiếp tục đợi nếu tác vụ ban đầu vẫn còn. Theo `session_id`, mặc định trả ngay; `wait_ms` dương cho đợi quá trình khôi phục đang chạy, có `unlock_request_id` thì đợi yêu cầu mở khóa tương ứng. Hủy chờ chỉ hủy truy vấn hiện tại, không ngắt kết nối người dùng đang tạo hoặc sử dụng. Sau khi người dùng chủ động ngắt, kết quả là `closed`, Agent dừng đợi và không tự kết nối lại.

Nếu kết nối từ bảng thất bại, chờ theo `view_id` cũng trả `failed` và mã lỗi, tránh để Agent mãi chờ người dùng. Ngắt kết nối đang tạo sẽ hủy toàn bộ lần thử. Nhiều bảng và Agent hỏi trạng thái dùng chung một truy vấn gốc; hủy một truy vấn hoặc bận tạm thời không ngắt phiên. Nếu kết nối thành công nhưng khởi tạo bảng thất bại, `crossdesk_connect` vẫn trả `session_id` hợp lệ kèm `panel_error`.

Đăng ký tín hiệu lần đầu dùng thời hạn tổng 30 giây của kết nối gốc; trong đó MiniRTC được phép phục hồi lỗi truyền tạm theo backoff. Thời hạn bao phủ mọi lần thử ban đầu. Xác minh chứng chỉ thất bại, đăng nhập bị từ chối hoặc mất tín hiệu sau khi gửi yêu cầu kết nối thiết bị đều kết thúc lần thử hiện tại. `error.code` chỉ loại lỗi; `error.reason` cung cấp nguyên nhân gốc đã nhận diện, như `signaling_timeout`, `signaling_disconnected` hoặc `device_offline_or_unavailable`. Bảng hiển thị giải thích tương ứng, lỗi terminal chứa mã nguyên nhân. `logs/<session_id>/transport.log` ghi giai đoạn kết nối, phân loại/mã số lỗi truyền tải được kiểm soát, mã đóng và thông tin backoff.

Tự tiếp tục phụ thuộc host giữ tác vụ Agent và luồng gọi công cụ hiện tại. Cơ chế này không giả tin nhắn người dùng qua trang web, cũng không hứa chủ động đánh thức Agent từ hội thoại đã kết thúc; host chịu trách nhiệm cho kiểu đánh thức đó.

Tạm dừng thủ công được khóa ở lớp gốc; mô hình không thể khôi phục qua `crossdesk_set_control({session_id, enabled: true})`, chỉ nút tiếp tục của bảng được cấp quyền hiện tại mới làm được. Bảng trình duyệt và Apps cũng cho người dùng tiếp quản chuột/phím rồi trả lại Agent. Kết nối chỉ xem không thể nâng thành điều khiển; cần kết nối lại và cấp quyền. Đóng/ẩn bảng sẽ dừng xem trước, nhả phím do bảng giữ nhưng giữ phiên từ xa. Chỉ nút ngắt hoặc `crossdesk_disconnect` mới kết thúc phiên.

Một tiến trình MCP hỗ trợ tối đa tám kết nối thiết bị song song; bảng gắn phiên theo thiết bị. Các tiến trình cùng thư mục cấu hình dùng khóa thiết bị để tránh cùng nối một đích. Khóa không phối hợp với người dùng hệ thống khác, thư mục cấu hình khác hoặc ứng dụng máy tính độc lập.

## Chính sách bảng điều khiển

`--panel` ưu tiên hơn `panel` trong cấu hình thiết bị, mặc định `auto`; `--browser-launch` cũng mặc định `auto`. MCP tích hợp chính sách mặc định cho Codex và DeepSeek Harness theo nhận diện client, không phụ thuộc cấu hình Codex cá nhân hay `AGENTS.md`. Chế độ bảng và cách khởi động được chỉ rõ trong cấu hình cũ vẫn hiệu lực.

| Chế độ | Hành vi |
| --- | --- |
| `auto` | Codex/DeepSeek Harness mặc định thanh bên host; khác có GUI dùng hệ thống, không GUI có terminal cho điểm CLI, thiếu cả hai `off` |
| `apps` | Chỉ MCP Apps; khả năng lệch `PANEL_UNSUPPORTED` |
| `browser` | Trang cục bộ; khởi `auto` Codex/Harness dùng host, khác hệ thống; chọn rõ `--browser-launch=host`/`--browser-launch=system`, không thêm siêu dữ liệu tài nguyên Apps |
| `terminal` | Lệnh `view` terminal riêng chung phiên hiện tại, không web service |
| `off` | Không web service, giữ ảnh/thao tác, xác thực cấu hình `setup` riêng |

`auto` không chọn MCP Apps. Nếu cần bảng nhúng trong host, chỉ rõ `--panel=apps`. Apps dò khả năng qua `text/html;profile=mcp-app` trong `extensions["io.modelcontextprotocol/ui"].mimeTypes`. Giao thức cũ đọc khả năng initialize, giao thức 2026-07-28 đọc khả năng yêu cầu hiện tại, không đoán theo tên sản phẩm host. Host quyết định Apps xuất hiện dạng thẻ, thanh bên hay cửa sổ.

Ngoài Codex và DeepSeek Harness, khi client dùng `auto`, Linux dò GUI qua `DISPLAY` / `WAYLAND_DISPLAY`; macOS kiểm phiên đăng nhập desktop của người dùng hiện tại, không tự mở trình duyệt Mac từ xa trong phiên SSH; Windows loại phiên SSH và Services. Dò terminal bao gồm TTY, `SSH_TTY` và `TERM` không phải `dumb`, vì stdin/stdout MCP thường là pipe. Cần chạy terminal hoặc không bảng thì chỉ rõ `--panel=terminal` hoặc `--panel=off`.

Ở chế độ terminal, `crossdesk_open` / `crossdesk_connect` trả `terminal_command`. Bạn chạy lệnh trong terminal tương tác khác trên cùng máy; nếu chạy nguồn, thay `crossdesk-mcp` bằng `node /absolute/path/apps/mcp/dist/main.js`:

```sh
crossdesk-mcp view --view <view_id> --config-dir <cung-thu-muc-cau-hinh>
```

Lệnh cho chọn thiết bị, nhập mật khẩu ẩn và cấp quyền điều khiển lần này; nếu phiên đã có thì hiển thị ảnh ngay. Terminal dùng descriptor chạy trong thư mục cấu hình hạn chế và IPC cục bộ xác thực để tái dùng kết nối gốc của MCP, không tạo kết nối từ xa thứ hai, không chiếm terminal Agent hay stdio MCP. Agent nên hiển thị lệnh để người dùng thực hiện rồi đợi qua `crossdesk_get_session`; không được chạy `view` trong kênh bắt đầu ra công cụ. Nếu không có terminal tương tác, `view` trả `VIEW_REQUIRES_TERMINAL`.

Hình terminal dùng bộ kết xuất ANSI phần tư của CLI gốc, mục tiêu 8 FPS, tối đa 160 cột × 60 dòng, dùng màu 8/256/truecolor tùy môi trường. Tỷ lệ rộng/cao ký tự mặc định 0.5, chỉnh bằng `--cell-aspect` (0.2–2). Điểm vào này chưa dùng Kitty/Sixel. `p` dừng Agent, `r` tiếp tục điều khiển đã cấp, `d` ngắt, `q` / Ctrl+C chỉ đóng hình và khôi phục terminal, giữ phiên. Tạm dừng vẫn khóa ở lớp gốc, Agent không tự tiếp tục được.

Trình duyệt/Apps và terminal đều vẽ vị trí chuột cuối Agent gửi thành công, hỗ trợ di chuyển, nhấn, cuộn và kéo; tọa độ từ ánh xạ tỷ lệ/cắt ảnh của lớp gốc. Tạm dừng hoặc tắt điều khiển sẽ ẩn, đổi màn hình hoặc cỡ ảnh sẽ xóa. Con trỏ chỉ gợi ý vị trí thao tác Agent, không phải ACK thực thi nhập từ xa và không theo chuột của người dùng từ xa. Nó không được ghi vào ảnh mô hình hay video nguồn.

Con trỏ video trực tiếp trong trình duyệt/Apps theo tọa độ mới bằng easing ngắn 60–140 mili giây, độc lập với tốc độ khung video. Cập nhật liên tiếp nối từ vị trí đang hiển thị, không xếp hàng phát lại đường cũ. Lần đầu hiện hoặc đổi thế hệ kết nối/hình học sẽ đặt thẳng vị trí; ẩn hoặc dừng phát sẽ hủy hoạt ảnh, đồng thời tuân “Giảm chuyển động” hệ thống. Hiệu ứng mượt chỉ áp cho con trỏ gợi ý trên trang, không ảnh hưởng nhập chuột gốc hoặc tọa độ nhấp, nên không có nghĩa chuột từ xa thực đi qua đường nội suy.

Con trỏ video là mũi tên bo góc màu chính CrossDesk (`#3564e3`), viền trắng và quầng sáng mềm cùng màu, đồng nhất với ứng dụng và trang. Khi di chuyển, đầu mũi tên hướng theo chiều thực trên trang và làm tâm quay; hướng tính theo pixel ảnh để thích ứng màn rộng/dọc. Dừng thì giữ hướng cuối, bỏ ảnh hưởng của dịch chuyển dưới nửa CSS pixel. Lần đầu hiện hoặc đổi kết nối/hình học sẽ về hướng mặc định.

Tài nguyên Apps là `ui://crossdesk/viewer.html`; công cụ chỉ lộ siêu dữ liệu UI và API riêng khi Apps được cho phép và yêu cầu hiện tại hỗ trợ. Chỉ đánh dấu sẵn sàng sau khi trang báo `crossdesk_ui_ready`; quá 10 giây không báo thì hiện `not_confirmed`, không tự mở trình duyệt thứ hai. Khi người dùng yêu cầu rõ phương án dự phòng, gọi `crossdesk_open_browser`. Công cụ này không kèm siêu dữ liệu Apps; chính sách `apps`/`off` từ chối ghi đè.

Bảng trình duyệt chỉ nghe cổng ngẫu nhiên trên `127.0.0.1`, cùng tiến trình MCP. Đây không phải MCP Streamable HTTP và không cung cấp MCP trong LAN. MCP, CLI và trình duyệt người dùng nên chạy trên cùng máy điều khiển; chuyển tiếp trình duyệt từ container SSH hoặc host xa không thuộc chế độ truy cập cục bộ này.

Trang tập trung giám sát phiên Agent: lần đầu nối hiện thẻ cấp quyền, sau đó ảnh từ xa chiếm vùng khả dụng, còn thiết bị, quyền và khôi phục nằm trong ngăn “Chi tiết phiên”. Thanh công cụ hiện thiết bị và quyền thực, chỉ giữ “Tạm dừng Agent / Tiếp tục Agent” khi đã cấp điều khiển; khi bảng này tiếp quản thì đổi thành “Trả quyền cho Agent”. Phóng ảnh, tiếp quản, âm thanh/âm lượng và ngắt nằm trong “Tùy chọn phiên”; phiên chỉ xem không hiện tiếp quản. Đợi mở khóa hệ thống có thông báo và mục nhập mật khẩu riêng; đóng hộp mở khóa vẫn mở lại được. Trạng thái phân biệt chỉ xem, điều khiển bật, tạm dừng, tiếp quản và khôi phục, không diễn giải có quyền điều khiển là Agent đang thực hiện tác vụ.

Trang có tiêu đề CrossDesk MCP và dùng logo/tên chính thức. Màu theo WorkbenchStyle / ButtonStyle desktop: nền xanh xám nhạt, thao tác chính màu xanh, điều khiển bo 6 pixel và thẻ bo 12 pixel. Nút thanh công cụ cao thống nhất 32 pixel; nút biểu tượng giữ tên trợ năng và tooltip. Bố trí lấp đầy vùng rộng/cao host hoặc trình duyệt, ảnh giữ tỷ lệ, phía dưới tập trung trạng thái video, độ phân giải và FPS. Thiếu chiều cao thì thẻ cấp quyền cuộn riêng; thanh bên hẹp giữ cùng bố trí giám sát. “Phóng to hình ảnh” ẩn header và giảm lề. Thoát menu, chi tiết hoặc chế độ phóng khi không tiếp quản đều hỗ trợ Esc. Tài nguyên dùng ngoại tuyến được; lưu thiết bị và quyền bền vững vẫn do người dùng chọn riêng.

Lần đầu mở dùng bootstrap ngắn hạn, một lần trong URL fragment; trang xóa fragment ngay rồi đổi cookie HttpOnly/SameSite và token CSRF. Bootstrap thô không vào kết quả công cụ. `browser_url` không có xác thực ghép, còn `browser_launch_url` là điểm ghép ngắn hạn chỉ giao host điều hướng, không nên đọc hay công bố. Sao địa chỉ không xác thực sang trình duyệt khác không cấp quyền điều khiển phiên.

Nếu không tự mở được trình duyệt, hãy chạy lệnh và tham số `pairing_command` từ kết quả công cụ trong terminal riêng. Ví dụ:

```sh
crossdesk-mcp setup --open-panel --view <view_id> --config-dir <cung-thu-muc-cau-hinh>
```

Khi chạy từ nguồn, thay `crossdesk-mcp` bằng `node /absolute/path/apps/mcp/dist/main.js`. Ghép qua descriptor chạy giới hạn trong thư mục cấu hình người dùng và IPC cục bộ; không cho yêu cầu HTTP ẩn danh cấp xác thực điều khiển. `setup` dùng môi trường desktop của terminal người dùng để mở trình duyệt. Nếu vẫn không mở được, chỉ terminal tương tác đó hiển thị địa chỉ một lần hạn 60 giây, không trả cho MCP hay gửi vào chat. Bảng giữ tối đa 32 view; mở lại sẽ dọn view không dùng quá 30 phút. Phiên trình duyệt tối đa 12 giờ, hết hạn cần mở lại.

### Trình duyệt thanh bên mặc định Codex

Cài đặt, ví dụ cấu hình và lần đầu sử dụng xem [cài và dùng Codex](README.md#install-codex). Bất kỳ ai khởi MCP trực tiếp trong Codex đều mặc định nhận điểm vào trình duyệt thanh bên; dưới đây giải thích nhận diện host và cơ chế ghép.

MCP nhận tên client Codex `codex-mcp-client` và DeepSeek Harness `dsh-mcp-client`: giao thức cũ đọc `clientInfo` trong initialize, giao thức 2026-07-28 đọc `io.modelcontextprotocol/clientInfo` của yêu cầu hiện tại. Dấu nhận diện chỉ chọn cách hiển thị, không cấp bất kỳ quyền thiết bị nào. `--panel=auto` chọn `browser` và `--browser-launch=auto` chọn `host` trong hai host này; client khác vẫn dò môi trường và dùng trình duyệt hệ thống như trước.

`host` không khởi trình duyệt hệ thống hay tạo thẻ MCP App, đồng thời yêu cầu trình duyệt host và MCP cùng máy. Codex dùng trang khởi `file://` cục bộ; DeepSeek Harness dùng điểm khởi HTTP bên dưới. Chỉ rõ `--panel=browser --browser-launch=host` để ép trình duyệt host hoặc `--panel=browser --browser-launch=system` để ép trình duyệt hệ thống. Kết hợp tường minh `--browser-launch=host` với `apps`, `terminal` hoặc `off` sẽ báo lỗi; dùng riêng các chế độ này không bị ảnh hưởng bởi mặc định trình duyệt.

Chỉ dẫn dịch vụ, mô tả và kết quả công cụ đều tích hợp quy trình mở của Codex: nhận `browser_launch_url` thì Agent mở trình duyệt host ngay, kể cả khi kết nối đã sẵn sàng. Người dùng không cần yêu cầu mở thanh bên lần nữa hoặc thêm `AGENTS.md` cá nhân. Mở thực vẫn do công cụ trình duyệt host; giao thức MCP không thể ép mọi client hiển thị thanh bên. Nếu thiếu công cụ đó, như Codex CLI không có trình duyệt thanh bên, Agent nên nói rõ giới hạn và hiện lệnh ghép cục bộ.


Trong chế độ này, `crossdesk_open`, `crossdesk_connect` và `crossdesk_open_browser` trả `browser_launch_url`, `browser_url`, `browser_host`, thời gian còn hiệu lực và thời điểm hết hạn tuyệt đối. Agent nên giao trực tiếp `browser_launch_url` cho công cụ mở trình duyệt host. Trong Codex, gọi `open_in_codex`, đặt `target.type` là `browser`, `target.url` là địa chỉ đó và `placement` là `right`. Không đọc, dán hoặc tải lên nội dung trang khởi.

Trang khởi Codex nằm trong thư mục riêng dưới thư mục tạm hệ thống, quyền POSIX là `0700` cho thư mục và `0600` cho tệp, chứa địa chỉ ghép dùng một lần trong 60 giây. Trình duyệt chuyển cục bộ tới player để đổi cookie HttpOnly/SameSite và token CSRF riêng; kết quả công cụ không chứa vé bootstrap thô. Vé gắn view/thế hệ; khi dùng, hết hạn hoặc đóng dịch vụ bình thường, trang khởi được dọn. Nếu địa chỉ khởi hết hạn, gọi `crossdesk_open_browser` với `view_id` hiện có để cấp lại, không kết nối từ xa lại.

`browser_url` là địa chỉ HTTP cục bộ không có xác thực ghép, dùng mở lại trình duyệt đã ghép; sao trực tiếp sang trình duyệt khác không có quyền phiên hiện tại. Với host không có trình duyệt tích hợp, người dùng có thể chạy `pairing_command` cũ để ghép bằng trình duyệt hệ thống.

Trang trong thanh bên vẫn dùng WebSocket cục bộ → WebCodecs/VideoFrame; media không qua kết quả công cụ MCP và không cần relay công cộng mới. Đây là môi trường khác iframe MCP Apps; khả năng phát và FPS thực cần kiểm trên host đích.

Ngày 2026-11-11 đã đo thực trong trình duyệt thanh bên Codex cục bộ: trang khởi trên kết nối máy macOS từ xa, trang hiện “Trực tiếp”, 1920 × 1080, khoảng 26 FPS và số khung canvas tăng liên tục. Phía gốc xác nhận `h264-annexb/1` qua `loopback-websocket/1`, 1 người xem hoạt động, hàng gửi không tồn đọng. Trong kiểm chứng chỉ xem, không gửi nhập từ xa. Kết quả xác minh phát trực tiếp trong môi trường hiện tại; FPS thực vẫn tùy máy từ xa, mạng và giải mã.

Ngày 2026-11-12, cấu hình tạm mới hoàn toàn ngoài dự án đã qua 16 kiểm tra stdio: giao thức Codex cũ/mới đều trả trang khởi host khi không đặt tham số bảng; client khác giữ hành vi cũ; ghi đè bảng/cách khởi tường minh có hiệu lực; cấp lại và dọn khi thoát bình thường. Phiên mô phỏng còn kiểm định tuyến Codex cho kết nối đã sẵn sàng và bảng mở khóa. Các kiểm tra không đọc cấu hình Codex cá nhân hay nối máy từ xa thật.

### Thanh bên DeepSeek Harness desktop

Sidebar Browser của Harness desktop chỉ nhận HTTP(S), không mở trang khởi `file://` của Codex; thanh bên tích hợp cũng không có công cụ mở cho Agent gọi. Sau khi cài plugin CrossDesk MCP hợp nhất, gọi công cụ sẽ tự mở thanh bên trình duyệt của phiên Harness hiện tại, không cần nhấn liên kết thủ công. Cấu trúc mã xem [tích hợp DSH](integrations/deepseek-harness/README.md).

Cài đặt và cấu hình đầy đủ xem [cài và dùng DeepSeek Harness](README.md#install-dsh). Plugin hợp nhất quản lý tiến trình MCP và hiển thị trang, không thêm quyền thiết bị hay tạo kết nối từ xa thứ hai.

`browser_launch_url` của Harness là `/launch/<ma-ngau-nhien>` cục bộ, chỉ cho điều hướng một lần trong 60 giây rồi chuyển sang luồng ghép cũ; token bootstrap thô không vào kết quả công cụ. Bản thân địa chỉ là điểm ghép ngắn hạn, không nên công bố, lấy hoặc ghi nội dung. Nó không tái dùng được sau khi tiêu thụ, hết hạn, đổi thế hệ view hoặc đóng dịch vụ; hết hạn thì cấp lại cho `view_id` hiện có. Kiểm Host/Origin, cookie HttpOnly/SameSite, CSRF và cấm nhúng iframe vẫn giữ hiệu lực.

Thích ứng thanh bên chỉ bật ở Harness desktop; iframe Harness web không hỗ trợ đường này. Không mở nhúng tùy miền hoặc chuyển tiếp công cộng để khắc phục. Khi chưa cài plugin, Agent nên chỉ cách cài hoặc đưa liên kết để người dùng chọn “Mở trong thanh bên”, không được tuyên bố đã tự mở. Thành công điều hướng thanh bên và ghép/video sẵn sàng cần xác nhận riêng; Agent vẫn phải đợi `crossdesk_get_session`.

Ngày 2026-11-12 đã đo trên DeepSeek Harness 0.2.2-rc.1 macOS: sau khi cài plugin đi kèm, `crossdesk_connect` nối Mac mini đã lưu ở chế độ chỉ xem; plugin tự mở trình duyệt bên phải của phiên hiện tại và ghép xong. Trang hiển thị hình trực tiếp, 1920 × 1080, khoảng 27–29 FPS. Không gửi phím/chuột từ xa trong kiểm chứng. Ngoài ra, thư mục tạm ngoài dự án đã qua 23 kiểm giao thức/ghép/cầu host và 6 kiểm vòng đời client, gồm giao thức Codex/Harness cũ/mới, dùng một lần, hết hạn, vô hiệu theo thế hệ, cách ly phiên, nhiều cửa sổ nhận và tái dùng thẻ.

Cùng ngày đã kiểm gói hợp nhất: dùng CLI kèm Harness trong profile mới hoàn toàn ngoài dự án, chỉ cài một gói nền tảng macOS arm64 và xác nhận bundle/schema cấu hình nạp được. Runtime Harness thật, Cordis và MCP Client kiểm tự đăng ký 31 công cụ, bàn giao yêu cầu thanh bên, xung đột namespace trùng, dọn khi tắt, bật lại và tên tùy chỉnh. Cũng đã kiểm cài/gỡ, từ chối chương trình gốc không hợp lệ, giao thức đóng gói và vòng đời client thanh bên sau biên dịch; hồi quy định tuyến Codex đạt. Lần kiểm gói hợp nhất này không nối máy từ xa, nền tảng khác vẫn cần CI và máy thật nghiệm thu.

## Cấu hình không bảng và thông tin xác thực

Chạy trong terminal tương tác của chính bạn, không nhập mật khẩu tương tác qua kênh stdio của host:

```sh
crossdesk-mcp setup
```

Cấu hình mặc định ở `~/.config/crossdesk/mcp/`, ghi đè được bằng `--config-dir` hoặc `CROSSDESK_MCP_CONFIG_DIR`. `devices.json` chứa thiết bị, tùy chọn bảng/video và tham chiếu xác thực; mật khẩu lưu trong macOS Keychain, Windows Credential Manager hoặc Linux Secret Service. Kho xác thực không khả dụng sẽ báo lỗi rõ, không chuyển về tệp văn bản rõ. Kết nối tạm trên bảng vẫn dùng mật khẩu người dùng nhập tại chỗ. Lưu bền vững thất bại sẽ giữ kết nối đã tạo và hiện lỗi trong bảng.

Bạn cũng có thể chỉ tên biến môi trường mật khẩu bên ngoài trong `setup`; dịch vụ đọc biến đó làm provider xác thực. Ví dụ cấu hình không có giá trị mật khẩu:

```json
{
  "version": 1,
  "panel": "off",
  "server": "api.crossdesk.cn",
  "port": 9099,
  "devices": [{
    "alias": "work-pc",
    "device_id": "123456789",
    "server": "api.crossdesk.cn",
    "port": 9099,
    "authorization": "control",
    "credential": "environment",
    "environment_variable": "MY_CROSSDESK_PASSWORD"
  }]
}
```

Cần có cả phạm vi cấp quyền và xác thực khả dụng. Mô hình không thể đưa `device_id` tùy ý để bỏ xác nhận bảng; `crossdesk_connect` phía mô hình chỉ nhận bí danh thiết bị đã cấp quyền. Không gửi mật khẩu cho Agent hay ghi lệnh chứa mật khẩu vào cấu hình host.

Apps phụ thuộc host tuân quy ước hiển thị app-only và `_meta` không thấy bởi mô hình. Dịch vụ kiểm thêm token năng lực UI ngẫu nhiên và generation của từng view; riêng dấu app-only không phải thông tin xác thực đáng tin cậy. Agent có thể đọc tiến trình/tệp cùng người dùng hoặc chạy mã cục bộ tùy ý thì không được cách ly bởi ranh giới này.

## Công cụ mô hình

31 công cụ sau dành cho Agent, định nghĩa trong [src/server.ts](src/server.ts). Khả năng từ xa hỏi qua `capabilities` trong trạng thái phiên; khả năng nền tảng cụ thể cho cửa sổ/phần tử hỏi qua `crossdesk_desktop({session_id, operation: "capabilities"})`.

| Công cụ | Chức năng |
| --- | --- |
| `crossdesk_open` | Mở chọn thiết bị/kết nối, trả `view_id` |
| `crossdesk_open_browser` | Điểm vào/dự phòng trình duyệt người dùng yêu cầu rõ |
| `crossdesk_list_devices` | Sổ cục bộ/tóm tắt quyền, không tìm đám mây |
| `crossdesk_connect` | Bí danh nối thiết bị đã cấp/có xác thực |
| `crossdesk_get_session` | Chọn đúng một `session_id`, `view_id`; hỏi/đợi kết nối, khôi phục, mở khóa cụ thể, `wait_ms` 0–25000 |
| `crossdesk_list_sessions` | Phiên hoạt động, thiết bị/màn hình/điều khiển, tối đa tám nối song song |
| `crossdesk_set_recovery` | Bật/tắt khôi phục giới hạn, 1–5 lần, mặc định 3; cần quyền bền/xác thực |
| `crossdesk_disconnect` | Ngắt/nhả nhập/tiến trình con/khóa thiết bị |
| `crossdesk_screenshot` | PNG image content/siêu dữ liệu quan sát |
| `crossdesk_wait` | Theo quan sát cũ đợi vùng đổi/ổn định, đủ trả ảnh mới |
| `crossdesk_action` | Nhập cấu trúc, tùy chọn ảnh sau thao tác |
| `crossdesk_release_inputs` | Nhả mọi phím/nút phiên giữ, hủy nhập hiện tại |
| `crossdesk_set_control` | Bật/tắt nhập Agent trong quyền đã cấp |
| `crossdesk_select_display` | Đổi màn, vô hiệu quan sát cũ |
| `crossdesk_capture_displays` | Chụp tuần tự tối đa tám màn, trả ảnh/`capture_id`, về màn gốc |
| `crossdesk_drag_across` | Tọa độ ảnh nhiều màn kéo xuyên màn |
| `crossdesk_video_settings` | Chỉnh FPS/phân giải/chất lượng/bitrate xem trước khi chạy |
| `crossdesk_desktop` | Hỏi khả năng/cửa sổ/cây trợ năng, đọc cây con theo tiến trình/phần tử |
| `crossdesk_desktop_action` | Kích cửa sổ, nhấn/tiêu điểm phần tử, đặt giá trị, khởi ứng dụng/mở HTTP(S) |
| `crossdesk_wait_ui` | Theo chữ/vai trò đợi cửa sổ/phần tử trợ năng xuất hiện/mất |
| `crossdesk_audio` | Bật/tắt nhận tiếng desktop, mặc định tắt |
| `crossdesk_audio_capture` | Ghi 100–10000 mili giây, trả khối WAV đơn kênh 48 kHz |
| `crossdesk_clipboard_get` | Đọc bảng nhớ chữ từ xa, cần phiên cấp điều khiển |
| `crossdesk_clipboard_set` | Ghi/xóa bảng nhớ chữ từ xa |
| `crossdesk_files_list` | Phân trang thư mục/gốc hệ thống tệp |
| `crossdesk_files_transfer` | Tải lên/xuống tệp/thư mục, trả tác vụ hỏi được |
| `crossdesk_files_status` | Hỏi tiến độ/kết quả từng tệp/trạng thái xong-lỗi-hủy |
| `crossdesk_files_cancel` | Hủy/dọn tệp tạm chưa xong, giữ đã xong |
| `crossdesk_system_status` | Hỏi trạng thái khóa hệ thống/dịch vụ desktop an toàn từ xa |
| `crossdesk_system_command` | Khóa hệ thống hoặc gửi chuỗi chú ý an toàn `send_sas` qua dịch vụ Windows |
| `crossdesk_unlock` | Mở khóa bằng xác thực khóa riêng, hoặc yêu cầu mật khẩu qua bảng cục bộ |

## Giao diện riêng bảng MCP Apps

11 API sau chỉ được công bố dưới dạng công cụ app-only khi `--panel=apps` và yêu cầu hiện tại hỗ trợ MCP Apps, do bảng đã xác minh token view gọi. Bảng trình duyệt và terminal dùng chức năng phiên qua kênh cục bộ xác thực riêng của mình.

| API | Chức năng |
| --- | --- |
| `crossdesk_ui_ready` | Báo bảng khởi tạo xong |
| `crossdesk_ui_list_devices` | Lấy sổ thiết bị/cấu hình máy chủ mặc định |
| `crossdesk_ui_get_session` | Hỏi phiên gắn bảng hiện tại |
| `crossdesk_ui_connect` | Gửi thiết bị/xác thực kết nối/tùy chọn quyền người dùng chọn |
| `crossdesk_video_open` | Lấy vé nối video trực tiếp view hiện tại |
| `crossdesk_ui_preview` | PNG giới hạn view hiện tại khi video hạn chế, chỉ trả qua siêu dữ liệu bảng |
| `crossdesk_viewer_control` | Người dùng dừng/tiếp tục/tiếp quản/ngắt |
| `crossdesk_ui_input` | Gửi chuột/phím bảng khi người dùng tiếp quản |
| `crossdesk_ui_unlock` | Gửi/hủy nhập mật khẩu khóa, tùy chọn lưu xác thực |
| `crossdesk_ui_recovery` | Đặt bật/tắt khôi phục/số lần thử |
| `crossdesk_ui_audio` | Bật/tắt nhận âm thanh phiên bảng |

## Thao tác từ xa

### Nhập bàn phím và chuột

`crossdesk_action` hỗ trợ `move`, `click`, `double_click`, `scroll`, `drag`, `key_press`, `hotkey`, `type_text`, cùng `key_down`, `key_up`, `mouse_down`, `mouse_up` có thể giữ trạng thái qua nhiều thao tác. Phím dùng tên chuẩn tương ứng Windows VK, gồm `A`–`Z`, `0`–`9`, `F1`–`F24`, bàn phím số, `CTRL`, `ALT`, `SHIFT`, `META`, `ENTER`, `ESC`, mũi tên, menu, phím khóa và media. Phím tắt ứng dụng và hiệu quả thực tế vẫn cần chọn theo hệ thống từ xa.

`move`, `click`, `double_click`, `scroll`, `drag` và `key_press` hỗ trợ tùy chọn `modifiers: ["CTRL", "ALT", "SHIFT", "META"]`, chỉ chọn phím cần và không được trùng, để thực hiện nhấn kết hợp, chọn, kéo và phím tắt. Phím bổ trợ chỉ giữ trong thao tác hiện tại, nhả khi kết thúc, hủy hoặc người dùng tạm dừng. `drag` có thể chỉ `button: "left" | "right" | "middle"`, mặc định trái. Phím dấu câu hỗ trợ `SEMICOLON`, `EQUAL`, `COMMA`, `MINUS`, `PERIOD`, `SLASH`, `BACKQUOTE`, `BRACKET_LEFT`, `BACKSLASH`, `BRACKET_RIGHT`, `QUOTE`. Đây là tên vị trí VK; ký tự thực tùy bố cục bàn phím từ xa, nên dùng `type_text` để nhập văn bản. Ví dụ:

```json
{"session_id":"…","operation_id":"select-item-1","observation_id":"…","action":{"type":"click","x":120,"y":80,"modifiers":["SHIFT"]}}
```

Trạng thái nhấn của `key_down`, `key_up`, `mouse_down` và `mouse_up` có thể giữ qua nhiều thao tác, ví dụ giữ Ctrl rồi di chuyển hoặc kéo chuột. Sau khi thao tác cuối kết thúc, trạng thái giữ có hiệu lực mười lăm giây, hết hạn tự nhả. `crossdesk_release_inputs` nhả ngay mọi phím/nút của phiên và hủy thao tác nhập hiện tại. Tạm dừng thủ công, ngắt và lỗi nhập cũng nhả trạng thái giữ.

`key_press`, `hotkey` hỗ trợ `duration_ms` tối đa ba mươi giây; `drag` cũng tối đa ba mươi giây. Trạng thái trả `held_key_codes` và `held_mouse_flags`. Trước khi nhập văn bản, nên nhả trạng thái chuột/phím đang giữ liên tục.

### Giảm lượt gọi thao tác Agent

`crossdesk_action` hỗ trợ `action.type: "sequence"`, thực hiện tuần tự 1–8 thao tác cố định trong một lần gọi và chỉ trả một ảnh cuối. Ví dụ khi trình duyệt từ xa đã có tiêu điểm, bạn có thể gộp đặt tiêu điểm thanh địa chỉ, nhập URL và Enter:

```json
{
  "session_id": "current_session",
  "operation_id": "open_bilibili",
  "observation_id": "current_observation",
  "action": {
    "type": "sequence",
    "actions": [
      { "type": "hotkey", "keys": ["META", "L"] },
      { "type": "type_text", "text": "https://www.bilibili.com" },
      { "type": "key_press", "key": "ENTER" }
    ]
  },
  "wait": { "condition": "changed", "timeout_ms": 1500 }
}
```

Ví dụ này dành cho macOS; hệ khác chọn phím bổ trợ theo phím tắt thực tế. Chỉ gộp thao tác không cần quan sát/phán đoán giữa chừng; mọi tọa độ vẫn theo quan sát ban đầu. Chuỗi không cho lồng hoặc giữ nhấn xuyên thao tác, tổng văn bản không quá 16 KiB, thời gian nhập dự kiến kể cả khoảng cách không quá 8 giây, thời hạn thực thi tổng là 25 giây. `interval_ms` là khoảng tối thiểu giữa thao tác, mặc định 100 mili giây, đặt được 0–500, không có nghĩa ứng dụng tải xong. Lớp gốc kiểm cả nhóm trước lần nhập đầu và mỗi bước đợi biên nhận nhập nếu hỗ trợ. Biên nhận thất bại, quá hạn, tạm dừng thủ công, hủy hoặc đổi hình học đều dừng các bước còn lại. Máy cũ không có biên nhận giữ `remote_execution: "unsupported"`, vẫn cần kiểm hình cuối. `sequence.finished_actions` là số thao tác đã hết giai đoạn nhập; `stopped_at` khi lỗi là vị trí dừng bắt đầu từ 0, không đại diện kết quả nghiệp vụ của ứng dụng.

`automation` trong kết quả kết nối/trạng thái tổng hợp hỗ trợ chuỗi cục bộ, biên nhận nhập từ xa và khả năng tự động hóa desktop. Agent nên dùng lại phiên hiện tại và quan sát hợp lệ. Khi có tự động hóa desktop, ưu tiên `crossdesk_desktop_action` để mở URL, ứng dụng hoặc thao tác phần tử trợ năng; nếu chưa khai hỗ trợ, dùng trực tiếp đường ảnh chụp/nhập. CLI cũ không hỗ trợ chuỗi sẽ trả `NATIVE_SEQUENCE_UNSUPPORTED` trước mọi nhập, cần cập nhật CLI cục bộ. Khi khả năng từ xa chưa được khai, dịch vụ đưa gợi ý tương thích, không tự ngắt hoặc thử thao tác không hỗ trợ.

`timing_ms` của kết quả gồm giai đoạn nhập gốc, khứ hồi nhập MCP–CLI, giai đoạn chờ/chụp và tổng thời gian trong công cụ. Bạn có thể dùng để phân biệt thời gian thực thi công cụ với độ trễ host, mô hình hoặc giọng nói bên ngoài lần gọi.

### Ảnh chụp và chờ điều kiện

Ảnh chụp mặc định có cạnh dài tối đa 1280, giới hạn lớn nhất 2048, cho phép chỉ vùng pixel của khung nguồn. Kết quả trả `source_size`, `source_rect`, `image_size`, `generation`, `geometry_epoch`, `frame_id`, `frame_age_ms` và `observation_id`. Tọa độ thao tác theo ảnh trả về, không dùng pixel CSS trình duyệt. Giữ tối đa 32 quan sát, hết hạn sau 30 giây; đổi màn hình, thế hệ kết nối hoặc hình học ảnh sẽ vô hiệu ngay. Khung dùng để chụp không quá 2 giây, PNG không quá 2 MiB; nếu cần sẽ thu nhỏ tiếp và trả kích thước thực.

`timeout_ms` giới hạn thời gian đợi khung khả dụng, còn mã hóa PNG thực hiện độc lập. `0` nghĩa chỉ dùng khung mới đã sẵn; không có khung trả `FRAME_TIMEOUT`, encoder bận trả `BUSY`. Yêu cầu chụp xử lý theo thứ tự đến và ưu tiên hơn xem trước. Video quá 2 giây không cập nhật sẽ từ chối nhập chuột/phím mới; hình mới trở lại thì tiếp tục được. Truy vấn loại trùng cho thao tác đã gửi vẫn đọc được kết quả cũ.

`operation_id` của nhập chuột/phím đệm 128 thao tác gần nhất trong phiên gốc hiện tại. Trong cửa sổ 5 phút, ít nhất khi chưa bị loại do dung lượng, có thể hỏi lại bằng cùng tham số. Dùng lại ID với tham số khác trả xung đột; cả chuỗi cũng tuân quy tắc này. `submitted` nghĩa đầu vào đã gửi; trạng thái xử lý nhập gốc ở xa xem `remote_execution`, còn kết quả giao diện thực vẫn cần quan sát sau đó. Quá hạn, mất mạng, hủy hoặc gửi lỗi có thể trả `outcome_unknown`/`side_effect_possible`, cấm tự phát lại.

`observe_after` mặc định bật: cùng lần gọi đợi hình thay đổi tối đa 1000 mili giây rồi trả ảnh cuối cùng cùng `wait.matched` / `wait.wait_timed_out`. Chỉ di chuyển hoặc nhấn/nhả riêng mặc định lấy khung mới, tránh đợi pixel vốn không đổi; vẫn có thể chỉ `wait` tường minh. `wait` chọn `changed`, `stable`, hoặc `condition: "ui"` với `text` / `role` để đợi phần tử trợ năng từ xa xuất hiện, tối đa 5000 mili giây. Chờ hình có thể dùng quan sát cắt trước của cùng phiên qua `wait.observation_id`, tránh vùng video, hoạt ảnh hay đồng hồ không liên quan. CLI mới chỉ nhận khung sau khung kết thúc thao tác; CLI cũ trả `wait.frame_bound: false`. Chờ quá hạn hoặc chụp lỗi không ghi đè kết quả nhập đã gửi và không phát lại thao tác. Hình đổi hoặc ổn định không có nghĩa tác vụ ứng dụng thành công. `observe_after: false` chỉ trả kết quả nhập, không được đồng thời chỉ `wait`.

Máy được điều khiển mới khai biên nhận xử lý nhập qua `capabilities.input_ack`. `remote_execution` trong kết quả là trạng thái xử lý nhập gốc ở xa; kết quả giao diện thực được xác nhận qua quan sát sau. Máy cũ trả `unsupported`. Chờ biên nhận có thời hạn rõ; quá hạn thì giữ trạng thái kết quả không chắc chắn.

`crossdesk_wait` dùng `observation_id` của ảnh hiện có để đợi vùng tương ứng thay đổi (`changed`) hoặc ổn định liên tục (`stable`). Có thể đặt `stable_ms` và `timeout_ms` tối đa 25000 mili giây. Đủ điều kiện sẽ trả ảnh mới; đổi màn hình, ngắt hoặc hủy đều kết thúc chờ.

### Mở khóa hệ thống và lưu mật khẩu

Khi nhận ra hệ thống từ xa đang khóa, Agent đặt tiêu điểm vào ô mật khẩu trên ảnh và chụp lại, rồi gọi `crossdesk_unlock` với `session_id`, `operation_id` và `observation_id` ảnh. Mặc định `use_saved: true` đọc xác thực khóa được lưu riêng cho thiết bị. Thiếu xác thực thì bảng trình duyệt/Apps hiện ô nhập ẩn trong hộp nổi, còn trình xem terminal hỏi ẩn. Hộp đóng được và mở lại qua “Nhập mật khẩu” trong thông tin kết nối bên trái. Kết quả gửi hiển thị bằng gợi ý nổi thu sau sáu giây; thông tin kết nối tiếp tục hiện trạng thái mở khóa và cấu hình xác thực khóa.

Người dùng có thể chọn “Lưu mật khẩu và cho phép tự mở khóa sau” trong ô mở khóa. “Lưu và cấp quyền” trên form kết nối và `crossdesk-mcp setup` cục bộ cũng cung cấp cấu hình mật khẩu khóa. Lưu bền vững cần thiết bị đã có trong sổ cục bộ; kho hệ thống dùng tài khoản riêng cho mật khẩu truy cập và mật khẩu khóa. Danh sách thiết bị trả tóm tắt `unlock_credential_provider`, tệp cấu hình lưu tham chiếu `unlock_credential`. Cách dùng biến môi trường đặt `unlock_credential: "environment"` và `unlock_environment_variable`; cấu hình thiết bị cũ vẫn hiệu lực.

Trong lúc chờ nhập, Agent dùng `crossdesk_get_session` với `session_id` hoặc `view_id`, `unlock_request_id` và `wait_ms: 25000`. Sau `wait_timed_out` có thể tiếp tục đợi; hủy chờ chỉ kết thúc truy vấn hiện tại. Yêu cầu mở khóa hạn năm phút và vẫn giữ hình trực tiếp khi chờ; nhập Agent thường hoặc đổi màn hình sẽ trả `UNLOCK_PENDING`. Người dùng hủy, tạm dừng thủ công, kết thúc kết nối hoặc đổi thế hệ hình đều kết thúc yêu cầu.

Trước khi gửi, dịch vụ lấy lại ảnh của cùng thế hệ màn hình; phía gốc chọn nội dung ô mật khẩu, nhập văn bản và Enter. Với máy hỗ trợ `capabilities.system`, dịch vụ hỏi trạng thái khóa trước khi gửi mật khẩu. Nếu đã mở, kết quả là `unlock.state: "verified"` và `unlock.result.status: "already_unlocked"`, không nhập nữa. Sau khi gửi, trạng thái sang `verifying` và hỏi hệ thống trong cửa sổ tám giây. Xác nhận mở được thì trả `verified`; không xác nhận được thì giữ `submitted` kèm thông tin xác minh. Máy cũ trả `submitted` sau khi gửi thành công. Agent lấy ảnh mới, xác nhận trạng thái desktop rồi tiếp tục tác vụ ban đầu.

Khi người dùng chọn lưu trong hộp mở khóa, máy hỗ trợ trạng thái hệ thống chỉ ghi kho xác thực sau khi xác minh đã mở; chưa xác nhận trả `persistence_error: "UNLOCK_NOT_CONFIRMED"`. Máy cũ vẫn lưu sau khi gửi thành công, cần kiểm hình để xác nhận mật khẩu đúng. Nếu mật khẩu bị từ chối, dùng yêu cầu mới với `use_saved: false` để người dùng sửa; nếu kết quả chưa chắc, quan sát hình trước. Cùng `operation_id` và tham số sẽ truy vấn kết quả yêu cầu cũ.

Mật khẩu khóa đi qua bảng cục bộ đã xác thực hoặc IPC terminal vào kênh nhập gốc. Tham số/kết quả công cụ phía mô hình chỉ chứa trạng thái yêu cầu; đệm thao tác gốc chỉ lưu ID và kết quả yêu cầu mở khóa. Mở khóa tuân quyền điều khiển và trạng thái tạm dừng thủ công hiện có, phụ thuộc khả năng nhập văn bản từ xa. Chế độ `off` dùng được xác thực khóa cấu hình trước, thiếu thì trả `UNLOCK_CREDENTIAL_REQUIRED`. CLI gốc khai hỗ trợ qua `unlock_protocol: "password/1"`; cập nhật nguồn cần biên dịch lại CLI/MCP và khởi động lại dịch vụ.

### Bảng nhớ tạm từ xa

`crossdesk_clipboard_get` đọc bảng nhớ tạm văn bản từ xa; `crossdesk_clipboard_set` đặt văn bản, chuỗi rỗng sẽ xóa. Đọc tối đa 128 KiB, một lần ghi tối đa 16 KiB UTF-8. Ghi cần `operation_id`, cùng yêu cầu hỏi kết quả cũ. Công cụ dùng phiên điều khiển đã cấp quyền và tuân tạm dừng thủ công/chờ mở khóa. Máy mới khai qua `capabilities.clipboard`; ghi thành công được xác nhận bởi biên nhận cập nhật bảng nhớ hệ thống.

### Duyệt và truyền tệp

`crossdesk_files_list` duyệt thư mục từ xa: đường rỗng chọn thư mục chính người dùng, `@roots` trả gốc hệ thống tệp. Mỗi trang tối đa 256 mục, tiếp tục bằng `request_id` và `next_offset`. Quá hạn trả ID yêu cầu có thể truy vấn tiếp.

`crossdesk_files_transfer` hỗ trợ tải lên/xuống tệp và thư mục. Đường nguồn tuyệt đối, thư mục đích cần tồn tại. `operation_id` loại trùng; `crossdesk_files_status` hỏi tiến độ byte và trạng thái cuối từng tệp; `crossdesk_files_cancel` hủy và dọn tệp chưa xong. Mỗi kết nối xử lý một tác vụ truyền đồng thời, nhiều tệp xếp hàng theo bộ máy hiện có. Giữ bốn bản ghi lịch sử đã xong; danh sách tệp trong trạng thái được phân trang. Tệp đích trùng tên lưu riêng theo quy tắc xử lý xung đột hiện có.

Duyệt thư mục và bắt đầu truyền cần phiên điều khiển đã cấp quyền, chưa tạm dừng, và máy được điều khiển khai `capabilities.files`. Yêu cầu tải xuống gắn thư mục đích/ID truyền; bên nhận chấp nhận tệp đã dành trước cho yêu cầu đó. Sau khi tạm dừng thủ công, vẫn có thể truy vấn hoặc hủy tác vụ hiện có.

### Trạng thái hệ thống và desktop an toàn

`crossdesk_system_status` hỏi trạng thái khóa hệ thống; Windows đồng thời báo dịch vụ khả dụng và giai đoạn desktop tương tác. `crossdesk_system_command` hỗ trợ `lock`; Windows có thể dùng dịch vụ đang chạy để thực hiện `send_sas`. Lệnh cần quyền điều khiển và `operation_id` riêng; cùng yêu cầu hỏi kết quả cũ. Sau khi hệ thống nhận lệnh, quan sát trạng thái và hình để xác nhận giao diện thực tế.

### Quản lý nhiều phiên và màn hình

Một MCP hỗ trợ tối đa tám kết nối thiết bị song song. `crossdesk_list_sessions` trả thiết bị, màn hình và trạng thái điều khiển từng kết nối. Bảng gắn phiên theo thiết bị; ngắt một kết nối sẽ giải phóng tài nguyên của nó. Nhật ký chẩn đoán mỗi kết nối nằm ở `logs/<session_id>/transport.log`.

`crossdesk_capture_displays` chụp tuần tự tối đa tám màn hình rồi khôi phục màn hình ban đầu, hiển thị ảnh nhiều màn trong một kết quả công cụ. `capture_id` có hiệu lực ba mươi giây để `crossdesk_drag_across` dùng tọa độ ảnh từng màn kéo xuyên màn hình. Thao tác kéo giữ nhấn qua lúc đổi màn và nhả khi xong, hủy hoặc lỗi. Nhập thông thường dùng ảnh mới lấy sau khi chọn lại màn hình.

`crossdesk_video_settings` chỉnh tốc độ khung, độ phân giải, chất lượng và bitrate xem trước lúc chạy, trả trạng thái chấp nhận từ xa. Thiết lập bị giới hạn bởi khả năng đối tác. Trong lúc chụp nhiều màn hoặc kéo xuyên màn hình, các yêu cầu nhập khác cùng phiên trả trạng thái bận.

### Khôi phục kết nối có giới hạn

`crossdesk_set_recovery` và công tắc “Tự khôi phục khi mất kết nối” trên bảng dùng để bật khôi phục. Mặc định tắt; bật cần thiết bị đã lưu xác thực kết nối và quyền bền vững. Giám sát nền bao phủ mất tín hiệu tạm, mất kết nối và tiến trình gốc bất thường, dùng backoff để nối lại tối đa số lần cấu hình trong ngân sách sáu mươi giây. Mỗi lần nối lại kiểm danh tính và quyền thiết bị; tạm dừng thủ công, tắt khôi phục hoặc chủ động ngắt sẽ dừng khôi phục.

Trong khôi phục, trạng thái là `reconnecting`; có thể đợi qua `crossdesk_get_session` với `session_id` và `wait_ms`. Thành công trả `session_id` mới; bảng giám sát hiện có theo kết nối mới. ID phiên cũ có thể hỏi kết nối thay thế, nhưng thao tác tiếp theo dùng phiên và ảnh mới. ID ảnh chứa phiên sở hữu để tránh nhầm số quan sát giữa tiến trình gốc. Nhập, mở khóa và tác vụ tệp trước đó giữ ranh giới kết quả cũ; khôi phục chỉ dựng lại kết nối và thiết lập xem.

### Người dùng tiếp quản qua bảng

Bảng trình duyệt/Apps có “Tiếp quản thủ công” và “Trả quyền cho Agent”. Sau khi tiếp quản, người dùng thao tác chuột, con lăn, phím tắt và bộ gõ tiếng Trung; thông tin kết nối hiển thị quyền hiện tại. Nhập gắn với bảng, đợt điều khiển và kích thước hình; bàn giao nhả trạng thái giữ và vô hiệu đợt cũ. Hình quá hai giây không cập nhật thì dừng gửi nhập mới, cửa sổ mất tiêu điểm hoặc ẩn thì nhả phím. Lớp gốc giữ thời hạn thuê nhập mười lăm giây làm bảo đảm dọn khi mất kết nối.

Agent vẫn có thể quan sát khi người dùng tiếp quản. Nhập chuột/phím, thiết lập màn hình và thao tác khác cần quyền điều khiển phải đợi người dùng trả quyền trong bảng. Kết nối cần được cấp quyền điều khiển trước và dùng CLI gốc hỗ trợ `events/1`.

### Âm thanh từ xa

`crossdesk_audio` bật/tắt nhận âm thanh desktop từ xa, mặc định tắt. Nút “Phát âm thanh” trong bảng bật cả nhận và phát cục bộ, âm lượng chỉnh riêng trong bảng. Phát dùng kênh WebSocket video đã cấp quyền; hàng PCM và độ trễ phát trình duyệt đều có trần, nghẽn sẽ bỏ âm thanh cũ. Lần phát đầu trên trình duyệt cần người dùng nhấn nút.

`crossdesk_audio_capture` ghi 100 tới 10000 mili giây âm thanh tiếp theo, trả WAV đơn kênh 48 kHz qua khối nội dung âm MCP. Trong lúc ghi tạm bật nhận, kết thúc hoặc hủy thì khôi phục thiết lập cũ; thiếu âm thanh trả `AUDIO_UNAVAILABLE`. Hệ thống cần cho máy được điều khiển ghi âm desktop. Tự khôi phục kết nối sẽ phục hồi lựa chọn nhận âm thanh của người dùng.

### Cửa sổ, ứng dụng và phần tử trợ năng

`crossdesk_desktop` hỏi khả năng nền tảng, danh sách cửa sổ và cây trợ năng. Có thể đọc tiếp cây con theo tiến trình hoặc phần tử snapshot trước; nút có vai trò, tên, giá trị đọc được và tọa độ desktop gốc hệ thống. Mỗi snapshot tối đa 128 nút, phản hồi khoảng 48 KiB, vượt thì trả `truncated`. Tham chiếu phần tử gắn kết nối, có hiệu lực ba mươi giây, giữ tối đa tám snapshot.

`crossdesk_desktop_action` hỗ trợ kích hoạt cửa sổ, nhấn/đặt tiêu điểm phần tử, đặt giá trị văn bản, khởi ứng dụng và mở HTTP(S). Thao tác phần tử dùng snapshot và `element_id`, làm xong thì snapshot mất hiệu lực. Mọi thay đổi cần quyền điều khiển và `operation_id`; cùng yêu cầu hỏi kết quả cũ. Trường mật khẩu an toàn ẩn giá trị, từ chối sửa qua API cấu trúc; mở khóa hệ thống dùng luồng mật khẩu cục bộ. Kết quả biểu thị API hệ thống có chấp nhận hay không; sau đó dùng hình hoặc cây mới xác nhận thay đổi UI.

`crossdesk_wait_ui` đợi phần tử xuất hiện hoặc biến mất theo văn bản/vai trò. Nếu phạm vi đọc chưa đầy đủ thì không xác nhận biến mất được và trả `search_incomplete` trong kết quả quá hạn.

macOS dùng AX, cần quyền trợ năng hệ thống. Windows dùng UI Automation; kích hoạt cửa sổ tuân giới hạn cửa sổ tiền cảnh và quyền hệ thống. Linux bật truy cập phần tử AT-SPI nếu phát hiện gói phát triển `atspi-2` lúc biên dịch; nếu không, chỉ hỏi/kích hoạt cửa sổ X11. Trên Wayland, truy cập phần tử phụ thuộc desktop và ứng dụng cung cấp AT-SPI. Tham số khởi ứng dụng nhận bundle ID macOS hoặc đường `.app` tuyệt đối, Windows nhận `.exe` tuyệt đối, Linux nhận tệp thực thi tuyệt đối. Quyền sở hữu và xử lý quá hạn API Linux theo tài liệu chính thức. [Tài liệu AT-SPI](https://docs.gtk.org/atspi2/)

## Video trực tiếp và ảnh theo nhu cầu

Video trực tiếp có hai đường, mặc định `browser`; cả hai được WebSocket cục bộ của CLI gốc gửi trực tiếp tới trình duyệt. Node chỉ xử lý phiên, vé quyền và công cụ MCP, không đọc/chuyển byte video. Không còn tạo pipe video, không qua stdio MCP hay ngữ cảnh mô hình, không chuyển từng khung thành PNG hoặc mã hóa lại video.

| `video.decode_path` | Đường hình ảnh |
| --- | --- |
| `browser` | MiniRTC xuất H.264 Annex B gốc → WebCodecs trình duyệt giải → canvas |
| `minirtc` | MiniRTC giải → khung NV12 gọn thô → VideoFrame / canvas trình duyệt; trình duyệt không giải H.264 nữa |

`video.decoder` mặc định `auto`: ưu tiên bộ giải mã phần cứng khả dụng, không hỗ trợ thì dùng phần mềm. `software` không cho chương trình chọn hoặc chuyển dự phòng sang phần cứng. Đường trình duyệt dùng WebCodecs `prefer-hardware` / `prefer-software`; đây là ưu tiên API mà trình duyệt có thể bỏ qua, không được coi là xác nhận backend phần cứng vật lý hay phần mềm. Đường gốc do factory giải mã MiniRTC chọn, macOS này dùng VideoToolbox hoặc OpenH264; nền tảng khác dùng backend MiniRTC đã hỗ trợ. `native_video_decoder` trả tên backend MiniRTC báo, rỗng trước khi có thống kê, không đại diện backend trình duyệt.

Thêm vào đối tượng gốc của `~/.config/crossdesk/mcp/devices.json`:

```json
"video": {
  "decode_path": "minirtc",
  "decoder": "auto"
}
```

Bạn cũng có thể cấu hình qua tham số khởi MCP trong host Agent; dòng lệnh ưu tiên hơn tệp:

```sh
crossdesk-mcp serve --stdio --video-path browser --video-decoder auto
crossdesk-mcp serve --stdio --video-path minirtc --video-decoder software
```

Sau khi sửa, khởi động lại MCP rồi tạo kết nối. Tùy chọn áp dụng toàn tiến trình MCP; cả hai đường đều gửi lựa chọn giải mã sang phía gốc. Giải mã gốc trong đường trình duyệt phục vụ đệm ảnh chụp. Mã hóa PNG chỉ kích hoạt theo yêu cầu chụp, gồm một màn, nhiều màn và ảnh sau thao tác/chờ; video WebSocket không kích PNG, xem trước tương thích Apps chỉ kích theo nhu cầu. Đây là cấu hình bộ giải mã bên điều khiển, không đổi bộ mã hóa từ xa. Tệp cấu hình cũ tiếp tục dùng được mà không thêm trường.

Chế độ Agent ban đầu yêu cầu mức tốc độ khung cao, giới hạn độ phân giải 1920 × 1080. Sau khi nối, dùng `crossdesk_video_settings` chỉnh tốc độ khung, độ phân giải, chất lượng và bitrate. Công cụ nhận tối đa 3840 × 2160; độ phân giải/FPS thực tùy khả năng từ xa, mạng và giải mã cục bộ. Bảng hiển thị kích thước và FPS trình bày thực. Đường trình duyệt cần WebCodecs giải H.264; đường MiniRTC chỉ cần trình duyệt tạo VideoFrame từ NV12 và vẽ. Bảng trình duyệt thiếu khả năng sẽ báo rõ; Apps không nối được kênh trực tiếp hoặc không giải mã được sẽ về xem trước PNG giới hạn dưới đây. NV12 không nén: 1080p / 30 FPS cần khoảng 93 MB/s truyền cục bộ, 4K / 30 FPS khoảng 373 MB/s, không tăng bitrate H.264 từ xa tới bên điều khiển. Bỏ Node chuyển tiếp không bỏ chi phí truyền NV12 và xử lý pixel trình duyệt; đây không phải GPU zero-copy.

WebSocket video chạy trong luồng I/O riêng của CLI gốc, chỉ nghe cổng ngẫu nhiên `127.0.0.1`, khác cổng HTTP Node. Bảng trình duyệt lấy vé qua cookie/CSRF; Apps qua app-only `crossdesk_video_open` và token view. Vé có hiệu lực một lần trong 15 giây, gắn view, phiên và thế hệ, gửi trong tin đầu WebSocket, không ghi URL, nhật ký hay kết quả công cụ thấy bởi mô hình. WebSocket cung cấp hình, PCM đã bật, ACK nhận khung và yêu cầu phục hồi khung khóa, không thực hiện nhập từ xa. Tối đa 8 người xem hoạt động. H.264 mất khung đợi khung khóa; NV12 độc lập từng khung, mỗi người xem tối đa một khung chưa xác nhận, hàng gửi gốc chỉ giữ hình mới nhất. H.264 tối đa 8 khung chưa xác nhận và chịu trần byte. Quá 10 giây chưa xác nhận khung sớm nhất sẽ đóng kênh video đó, không chặn người xem khác hay điều khiển MCP. Rút view, đổi màn/thế hệ kết nối, ngắt hoặc thoát sẽ đóng kênh tương ứng; tạm dừng Agent vẫn tiếp tục phát. Trang ẩn/gỡ sẽ nhả đăng ký video, hiện lại thì đăng ký lại.

CSP của trang trình duyệt và tài nguyên MCP Apps cho `ws://127.0.0.1:*` mà dịch vụ gốc dùng, chỉ cổng động; HTTP điều khiển vẫn giới hạn đúng địa chỉ Node. Dịch vụ gốc còn kiểm Host, Origin chính xác và vé một lần. Host vẫn có thể hạn loopback, WebSocket, WebCodecs hoặc quyền iframe. Đã thấy host nhúng Codex lọc whitelist HTTP/WS và cuối cùng dùng `connect-src 'self'` chặn video gốc; thêm trường CSP đồng nghĩa không giải quyết giới hạn giao thức. Player phân biệt lỗi CSP và cung cấp xem trước tương thích qua kênh MCP đã cấp quyền khi video Apps thất bại.

Xem trước tương thích hiển thị “Xem trước” và FPS thực, tối đa 1 khung/giây, cạnh dài 960 pixel, PNG tối đa 256 KiB/khung, không âm thanh. `crossdesk_ui_preview` chỉ cho bảng, xác minh token view, thế hệ và phiên gắn; mỗi view tối đa một yêu cầu đang chạy, trước phản hồi kiểm lại phiên có đổi không. Ảnh chỉ ở `_meta`, không vào nội dung mô hình thấy và không chiếm lịch sử ảnh thao tác Agent. Frontend giải trực tiếp byte PNG, không cần whitelist mạng mới. Ẩn/gỡ bảng thì dừng làm mới, phản hồi trễ không vẽ. Cần video mượt và tiếng thì dùng `--panel=browser`; `--panel=apps` vẫn cấm tự mở trình duyệt ngoài.

Ngày 2026-11-11, Codex MCP Apps cục bộ nối macOS từ xa ở chế độ chỉ xem, xác nhận WebSocket bị CSP chặn rồi tự hiện xem trước tương thích. Số khung canvas tăng liên tục, đo khoảng 0.5–1 FPS. Trong trần 256 KiB, ảnh tự co thành 540 × 303. Kết quả xác minh xem trước nhúng, không đại diện host đã hỗ trợ video WebSocket cục bộ trực tiếp.

## Đóng gói

```sh
npm run build
npm run pack:npm -- --out-dir /absolute/path/artifacts
```

Mặc định sinh tarball chung gồm MCP, điểm vào plugin DSH và khai báo bundle, bảng, mã nguồn, giấy phép và năm phụ thuộc nền tảng tùy chọn cố định phiên bản. Quy trình CI `MCP package` tải lên gói chung `mcp-package`, không phát hành npm; `Publish MCP to npm` tái dùng để biên dịch/phát hành, xem [phát hành tự động](#phát-hành-lên-npm-qua-github-actions) dưới đây. Trước khi dùng gói chung, phụ thuộc npm gốc đúng phiên bản cần có sẵn.

Để sinh gói nền tảng DSH chỉ cần cài một lần, giải nén `.tgz` gốc do quy trình CLI sinh ra ngoài dự án, rồi truyền thư mục `package` của nó:

```sh
native_package_dir=$(mktemp -d)
tar -xzf /absolute/path/chunkdash-crossdesk-cli-darwin-arm64-0.2.0.tgz -C "$native_package_dir"
npm run pack:npm -- --native-package-dir "$native_package_dir/package" --out-dir /absolute/path/platform-artifacts
rm -rf "$native_package_dir"
```

Gói nền tảng giữ chương trình, nguồn, giấy phép và thông tin biên dịch của gói gốc, đặt giới hạn OS / CPU / libc tương ứng và không tải phụ thuộc nền tảng gốc nữa; phụ thuộc JavaScript khác vẫn do trình quản lý gói cài. Quy trình CLI sinh sản phẩm `crossdesk-mcp-<nen-tang>` cho từng nền tảng. Phiên bản CLI, MCP, DSH và phụ thuộc gốc không khớp sẽ dừng biên dịch hoặc đóng gói ngay. Gói chung và gói nền tảng cùng tên/phiên bản nên đặt ở thư mục đầu ra khác nhau; không phát hành gói nền tảng lên npm như bản chung.

`doctor` gốc bắt buộc khai đồng thời giao thức tự động hóa, `automation_video_protocols: ["h264-annexb/1", "nv12/1"]`, `automation_video_decoders: ["auto", "software"]`, `automation_video_transport: "loopback-websocket/1"` và `automation_terminal_protocol: "ansi/1"`. Phân phối npm chung cần phát hành gói nền tảng 0.2.0 tương ứng trước rồi mới MCP; gói nền tảng chứa chương trình gốc không chịu thứ tự này. Biên dịch thành công không có nghĩa tích hợp trên từng host/thiết bị đã hoàn tất.

Khi biên dịch lại TypeScript/bảng từ tarball phát hành, sao `build-package.json` trong gói thành `package.json` rồi chạy `npm ci && npm run build`. Nguồn gốc và nguồn phụ thuộc kèm gói nền tảng, xem [nguồn gốc mã nguồn](../cli/docs/source-provenance.md).

## Phát hành lên npm qua GitHub Actions

[`release-mcp.yml`](../../.github/workflows/release-mcp.yml) phát hành gói chung `@chunkdash/crossdesk-mcp`, đồng thời cung cấp stdio MCP và plugin DSH. Không cần phát hành gói plugin DSH riêng trong quy trình này.

| Thao tác | Kết quả |
| --- | --- |
| Commit/PR thường | `MCP package` biên dịch/tải `mcp-package`, không phát hành npm |
| Actions → Publish MCP to npm → Run workflow, giữ chọn `dry_run` | Biên dịch/kiểm gói, không dùng thông tin xác thực npm, không cần duyệt environment GitHub `npm` |
| Đẩy `mcp-v0.2.1` | Kiểm phiên bản nguồn chung `0.2.1`, biên dịch/kiểm phụ thuộc gốc/phát hành `latest` |
| Đẩy `mcp-v0.3.0-rc.1` | Kiểm nguồn chung `0.3.0-rc.1`, phát hành `next` |
| Chạy thủ công ở nhãn `mcp-v*` khớp, bỏ `dry_run` | Phát hành thật; không cho phát hành thật trên nhánh |

Nhãn MCP không kích hoạt biên dịch máy tính, CLI độc lập hay biên dịch MCP lặp. Điểm chạy thủ công cần quy trình đã vào nhánh mặc định; chạy thử có thể chọn nhánh, nhưng chạy thử trên nhãn cũng bắt buộc khớp phiên bản nguồn.

### Cấu hình quyền phát hành npm

Trên npm, mở **Settings → Trusted Publisher** của `@chunkdash/crossdesk-mcp` và chọn **GitHub Actions**:

| Trường | Giá trị |
| --- | --- |
| Organization or user | `kunkundi` |
| Repository | `crossdesk` |
| Workflow filename | `release-mcp.yml` |
| Environment name | `npm` |
| Allowed actions | Chọn **Allow npm publish** |

Quy trình dùng GitHub-hosted runner, Node.js 24, npm 12.2.1 và OIDC, không cần đặt `NPM_TOKEN`. Chỉ job phát hành thật được `id-token: write`; tải lên kèm provenance. Các trường cấu hình cần khớp hoàn toàn quy trình; xem [npm Trusted Publishing](https://docs.npmjs.com/trusted-publishers/).

Nếu **Settings → Environments → npm** của kho GitHub đang chỉ cho `cli-v*`, cần cho cả `mcp-v*`. Chạy thử không vào môi trường này; phát hành thật tuân quy tắc duyệt môi trường hiện có. Cấu hình tin cậy npm của MCP và CLI lần lượt ứng với `release-mcp.yml` và `release-cli.yml`, không hoán đổi.

### Phát hành bản mới

1. Tại gốc kho, đặt chung phiên bản CLI, MCP, DSH và phụ thuộc gốc. Ví dụ chuẩn bị `0.2.1`:

   ```sh
   python3 scripts/version.py set mcp 0.2.1
   python3 scripts/version.py check
   ```

2. Commit và đẩy thay đổi rồi chạy một `dry_run` của quy trình phát hành MCP. Script phiên bản cũng đồng bộ cấu hình CLI, tệp khóa MCP và siêu dữ liệu tích hợp DSH; xem [quản lý phiên bản thành phần](../../docs/VERSIONING.md). Chạy thử kiểm tên, phiên bản, SHA-512, điểm vào thực thi, trang, bundle DSH và khai báo phụ thuộc gốc cố định trong tarball; không kiểm quyền phát hành npm hay gói gốc đã có trên registry chưa.
3. Theo [phát hành CLI](../cli/docs/npm-distribution.md#ci-và-phát-hành), đẩy `cli-v0.2.1` từ commit đó để phát hành năm gói nền tảng gốc và điểm vào CLI trước. Xác nhận truy vấn được `0.2.1` của cả năm gói trong [`platforms.json`](../cli/npm/platforms.json) rồi mới MCP. Dù chỉ sửa MCP, vẫn cần phát hành CLI cùng phiên bản; bản đã phát hành không được ghi đè bằng nội dung khác.
4. Sau khi xác nhận cấu hình tin cậy npm MCP khả dụng, tạo và đẩy nhãn MCP từ cùng commit:

   ```sh
   git tag mcp-v0.2.1
   git push origin mcp-v0.2.1
   ```

Trước khi phát hành thật, quy trình kiểm phiên bản chính xác, trường nền tảng và thông tin phân phối của năm gói gốc. Thiếu bất kỳ gói nào hoặc registry lỗi quyền/mạng đều dừng phát hành. Phiên bản gốc không phụ thuộc dist-tag `latest` hay `next`, chỉ cần đúng phiên bản MCP tham chiếu có sẵn. Gói cài `crossdesk-mcp-<nen-tang>` bị từ chối; npm chỉ phát hành gói chung trong `mcp-package`.

### Kiểm kết quả và phục hồi lỗi

Job phát hành chỉ tải artifact `mcp-package` của quy trình hiện tại. Sau khi tải lên, cứ 15 giây kiểm phiên bản, SHA-512 và dist-tag đích, tối đa 15 phút; npm còn xử lý thì không tải lên lặp. Thành công sẽ hiện phiên bản phát hành, phiên bản phụ thuộc gốc và checksum tarball trong Actions Summary.

Nếu phát hành thất bại, ưu tiên **Re-run failed jobs** để dùng lại artifact cũ. Gói đã phát hành cùng phiên bản, checksum và dist-tag khớp sẽ bỏ tải lên. Nếu cùng phiên bản khác nội dung, dist-tag trỏ bản mới hơn, dist-tag bản đã có không khớp hoặc registry chỉ có bản giữ chỗ `0.0.0-stage`, quy trình sẽ dừng và nêu nguyên nhân. Đợi xử lý xong hoặc sửa phụ thuộc/quyền rồi chạy lại, không dựng lại toàn bộ hay ghi đè bản cũ để khôi phục tải. Quy trình không tự sửa dist-tag của phiên bản hiện có.

Sau khi phát hành thành công, kiểm ngoài dự án để tránh `npx` khớp gói cùng tên trong thư mục nguồn. Ví dụ `0.2.1`:

```sh
cd "$(mktemp -d)"
npm view @chunkdash/crossdesk-mcp@0.2.1 version optionalDependencies --registry=https://registry.npmjs.org/
npx --yes --registry=https://registry.npmjs.org/ @chunkdash/crossdesk-mcp@0.2.1 --help
```

`--help` chỉ kiểm điểm vào lệnh. Nghiệm thu đầy đủ vẫn cần theo [hướng dẫn người dùng](README.md), kết nối thiết bị đã cấp quyền trong Codex hoặc DSH.

## Kiểm chứng rà soát cục bộ (2026-11-03)

Kiểm tham số thao tác và sinh bước nhập tập trung trong `apps/cli/automation_protocol.*`; vòng lặp chính điều phối, hủy và kiểm quyền. Ảnh căn theo lưới sắc độ rồi chỉ chuyển vùng cần; PNG nén một lần trong đệm giới hạn dung lượng. Khung gốc chỉ tái dùng bộ nhớ khi không có người đọc khác. Node tích lũy byte phản hồi theo dòng, giải mã khi đủ phản hồi để tránh quét và sao ảnh lớn lặp lại.

macOS arm64 cục bộ đã hoàn tất biên dịch TypeScript/bảng, CLI gốc, kiểm wire/keyboard-state hiện có và hồi quy MCP, phiên, thao tác gốc ngoài dự án. ASan/UBSan kiểm điều phối thao tác và dọn nhập. 140 nhóm ảnh ngẫu nhiên bao phủ nguyên khung, biên cắt chẵn/lẻ, tỷ lệ và giảm kích thước khi chạm trần; PNG trước/sau tối ưu giống nhau từng byte.

Benchmark cục bộ `-O2` với NV12 tổng hợp 1920 × 1080, cạnh dài tối đa 1280, mỗi mục 6 lần: mã hóa nguyên ảnh khoảng 22.5 → 11.5 ms, cắt 321 × 241 khoảng 2.1 → 0.9 ms. Các số chỉ đo chuyển ảnh/mã hóa, không gồm mạng, giải mã, thực thi từ xa hay kết xuất host. Chưa nối máy từ xa thật hoặc nghiệm thu Windows/Linux. Mã, script, dữ liệu và sản phẩm kiểm bổ sung đều ở thư mục tạm hệ thống ngoài dự án.
