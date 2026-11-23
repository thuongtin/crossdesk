# Giao diện cầu nối terminal (Bridge v1)

Trang này mô tả yêu cầu, phản hồi và sự kiện khung cần để nhúng hình ảnh CLI vào giao diện terminal. Kết nối thông thường xem [hướng dẫn terminal](terminal-client.md), dùng qua Agent xem [hướng dẫn MCP](../../mcp/README.md). Đây là giao thức riêng của CrossDesk, không phải MCP.

Khởi động: `crossdesk-cli bridge --stdio [--password-fd 3] [--server host] [--port 9099] [--quality low|balanced|high|ultra]`.
Khởi động qua mảng tham số, không qua shell. stdin/stdout là UTF-8 NDJSON, mỗi dòng một JSON hoàn chỉnh. stdout dùng mô tả dành riêng, stdout tầng dưới chuyển sang stderr. Ứng dụng tích hợp nên đọc/cách ly stderr, không được hiển thị thành dòng video hay thông điệp mô hình. bridge không yêu cầu TTY.

Mỗi yêu cầu có số nguyên `id` và chuỗi `method`. Phản hồi có cùng `id`, boolean `ok`; thành công còn có `version:1`, `status` đầy đủ; thất bại có `error` cố định (yêu cầu, phiên bản không hợp lệ hoặc thao tác không khả dụng...), không trả lại đầu vào/thông tin xác thực gốc. Lỗi không có id hợp lệ dùng null. Sự kiện không có id. Gửi hello trước rồi thao tác khác.

| method | Trường | Hành vi |
|---|---|---|
| hello | version:1 | Bắt tay phiên bản giao thức |
| viewport | columns:1..240, rows:1..100, colors:8/24/256, utf8:boolean, fps:1..30; tùy chọn cell_aspect:0.2..2, detail:quadrant/half, quality:low/balanced/high/ultra, zoom:1..32, native:boolean | Đặt số dòng hình ảnh (không gồm thanh trạng thái), tỷ lệ rộng/cao ký tự, mục tiêu cục bộ và mức xem trước; mặc định cell_aspect=0.5, quadrant/high/1, native=false, giữ cell_aspect/detail/quality/zoom/native không chỉ định tường minh; không dò TTY ứng dụng tích hợp |
| connect | device:chuỗi 9 chữ số, display:0..63 (mặc định 0) | Đọc từ password fd riêng đến xuống dòng/EOF (tối đa 256 byte, chờ 5 giây), bắt đầu đăng ký/kết nối bất đồng bộ; ok không có nghĩa đã kết nối máy từ xa |
| status | Không | Trả trạng thái hiện tại |
| control | enabled:boolean | Bật/tắt điều khiển tường minh; tắt thì nhả nhập liệu |
| input | data:string, tối đa 16KiB | Phím thô terminal/UTF-8/bracketed paste/chuột SGR; chỉ đọc thì c bật điều khiển, q/Ctrl+C thoát, nhấn chuột vào chi tiết, +/-/con lăn thu phóng tại chuột, mũi tên hoặc i lên, j trái, k xuống, l phải di chuyển, f khôi phục toàn cảnh và đặt lại tỷ lệ/vị trí; Ctrl+] mở lệnh cục bộ, sau đó c chọn trạng thái điều khiển khi quay lại, q thoát hoặc phím khung nhìn chỉnh liên tục, Esc/Ctrl+C quay lại thủ công; trong điều khiển Ctrl+T/Ctrl+C gửi đến máy từ xa, ở chế độ cục bộ nhấn lại Ctrl+] để quay lại thủ công, và gửi Ctrl+] từ xa khi quay về điều khiển |
| display | index:0..63 | Chọn màn hình đã công bố, chỉ số bắt đầu từ không, tăng generation |
| ack | frame_id, generation: số nguyên không âm | Xác nhận ứng dụng tích hợp đã dùng khung; chỉ khớp khung đang chờ mới mở khung tiếp |
| disconnect | Không | Nhả nhập liệu, kết thúc phiên đa phương tiện hiện tại |
| exit | Không | Ngắt kết nối và kết thúc tiến trình |

Không được đặt thông tin xác thực trong JSON connect. Mô tả riêng có thể do pipe thứ tư của Node `spawn` cung cấp. Sau khi đọc thành công, phiên không giữ mật khẩu người dùng cho tầng trên truy cập; MiniRTC giữ thông tin cần cho Join/kết nối lại hiện có đến khi hủy Peer. Lần connect tiếp bắt buộc có nguồn xác thực đọc được; Pi kết thúc thẳng tiến trình sau khi đóng khung nhìn.

`viewport.fps` là mục tiêu đầu ra cục bộ. Nếu máy từ xa hỗ trợ đăng ký xem trước, mục tiêu trên 15 dùng chế độ tương tác hiện có (giao thức `fps=60`, mã hóa `frame_rate=30`), không quy đổi thành 15 FPS nữa; mục tiêu thấp vẫn chọn 1/8/15 theo khả năng. bridge luôn xuất dòng ký tự và yêu cầu ACK từ ứng dụng tích hợp, không dùng truyền ảnh bộ nhớ chung của Kitty tương tác.

Sự kiện khung:

```json
{"event":"frame","frame_id":42,"decoded_frame_id":38,"generation":4,"source_width":1280,"source_height":720,"zoom":2,"native":false,"source_rect":[320,180,640,360],"detail":"quadrant","columns":80,"rows":23,"cell_aspect":0.45,"lines":["..."],"queue_ms":3.2}
```

Số lines thực bằng rows, chỉ chứa chuỗi màu SGR sinh cục bộ, dấu cách và ký tự khối U+2580～U+259F, không có tên từ xa hay lệnh terminal tùy ý. `frame_id` là số thứ tự đầu ra tăng trong tiến trình, vẽ lại hình tĩnh vẫn sinh số mới; `decoded_frame_id` định danh khung nguồn đã giải mã, vẽ lại cùng nguồn có thể giống nhau. `source_width/height` là kích thước giải mã thực, source_rect là hình chữ nhật cắt nguồn thực [x,y,width,height]. native=true chỉ chế độ chi tiết nguyên bản, zoom là tỷ lệ chế độ hiện tại; generation đổi khi đóng, kết nối lại và chuyển màn hình. Đổi viewport có thể xen khung kích thước cũ đã ghi; ứng dụng tích hợp nên bỏ khung sai số hàng/cột／cell_aspect hoặc generation cũ và gửi ACK tương ứng. ACK ứng dụng tích hợp trả chỉ việc dùng/bố trí, không chỉ hiển thị vật lý xong.

`cell_aspect` là rộng／cao một ký tự, bắt buộc là số JSON hữu hạn trong 0.2～2; ví dụ ký tự 9×20 pixel nên truyền 0.45. Ứng dụng tích hợp chịu trách nhiệm đo hoặc cung cấp cài thủ công; bridge tính bố trí ảnh và ánh xạ chuột theo tỷ lệ này, `status.cell_aspect` trong phản hồi và sự kiện trạng thái trả giá trị hiện tại. Đây là mở rộng tùy chọn v1; bỏ qua thì giữ giá trị trước, lần đầu mặc định 0.5. Khung bridge cũ có thể thiếu trường này. Đổi tỷ lệ cũng kích hoạt vẽ lại hình tĩnh; ánh xạ chuột mới vẫn có hiệu lực sau ACK khung.

Sự kiện trạng thái tối đa một lần mỗi giây: `{"event":"state","status":{...},"output_bytes":123,"submitted_frames":3,"elapsed_seconds":1.5}`. status có registering/connecting/connected/failed/closed/idle, client_id (`cli-xxxxxxxxx` của phiên hiện tại hoặc vừa kết thúc, rỗng trước đăng ký), error, control, generation, màn hình, khả năng văn bản/xem trước, trạng thái xác nhận xem trước, preview_width/height/quality/bitrate_bps yêu cầu, số khung và thống kê mạng. FPS nhận/gửi tính bằng chênh lệch bộ đếm tương ứng chia chênh lệch thời gian.

`status.command_mode` trong trạng thái và phản hồi thành công chỉ chế độ lệnh cục bộ liên tục; khi đó `control=false`, nhập từ xa tạm dừng; `status.resume_control` chỉ có bật điều khiển sau quay lại thủ công hay không. Thực thi lệnh, phím không rõ hoặc dán không thoát chế độ này; mất tiêu điểm hay tràn đầu vào chỉ đặt chế độ quay lại thành chỉ đọc. `Esc` / `Ctrl+C` hoặc nhấn lại `Ctrl+]` để quay lại thủ công; `status.exit_requested` chỉ người dùng yêu cầu thoát qua phím, ứng dụng tích hợp có thể đóng thành phần theo đó. Ứng dụng tích hợp nên cập nhật gợi ý lệnh ngay từ phản hồi `input`, không cần chờ sự kiện trạng thái tiếp; không tự chặn `Ctrl+]` để thoát. Yêu cầu `control` tường minh chỉ cập nhật trạng thái điều khiển chọn, vẫn dừng nhập từ xa trong chế độ cục bộ. Tạo kết nối hoặc ngắt đặt lại chế độ cục bộ. Xem phím tắt trong [ứng dụng terminal](terminal-client.md).

Giới hạn: mỗi đầu vào và bộ đệm phân tích hiện tại tối đa 64KiB, lồng 32 tầng; hàng đợi phản hồi 64 mục; một thông điệp đầu ra đang gửi tối đa 2MiB; 1MiB/s; tối đa một khung chờ ACK; một NV12 mới nhất chờ render cộng một bộ đệm vẽ lại hình gần nhất. Lỗi giao thức trả rõ ràng; kết thúc sau khi bộ đệm vượt giới hạn. EOF dọn phiên, xử lý yêu cầu nhận đủ theo thứ tự, báo lỗi phần cuối chưa đủ; thoát sau tối đa hai giây xả đầu ra. Ứng dụng tích hợp nên đọc phản hồi liên tục, không được gửi vô hạn mà không đọc. Sau mất kết nối, gửi lệnh nhả chỉ có thể cố gắng hết sức.

Không có phương thức `screenshot`, `audio`, tệp, clipboard, hành động từ xa tùy ý hay lệnh shell; phương thức không rõ báo lỗi. Bên tích hợp mô hình bắt buộc xin quyền chụp màn hình/thao tác tự động riêng, không được đính kèm sự kiện khung liên tục vào lịch sử trò chuyện.
