# Dùng CrossDesk trong DeepSeek Harness

Sau khi cài plugin CrossDesk, bạn có thể để Agent thao tác máy từ xa và theo dõi hình ảnh trong trình duyệt bên phải cuộc trò chuyện hiện tại.

## Cài đặt

1. Theo [tải gói cài đặt](../../README.md#install-package), chọn gói nền tảng khớp hệ thống và chip máy của bạn, giải nén ZIP và lấy tệp `.tgz`.
2. Cài tệp này trên trang plugin của Harness bản máy tính, hoặc chạy lệnh sau. Thay đường dẫn và số phiên bản ví dụ bằng tệp thực tế:

   ```sh
   dsh plugin --profile desktop add "/absolute/path/chunkdash-crossdesk-mcp-0.2.0.tgz"
   ```

3. Khởi động lại Harness, xác nhận plugin CrossDesk đã bật.

Gói nền tảng đã chứa chương trình gốc cần thiết, tự cung cấp công cụ MCP và hình ảnh ở thanh bên. Cài đặt cần mạng để tải phụ thuộc. Nếu terminal không tìm thấy `dsh`, bạn có thể dùng trực tiếp trang plugin, hoặc dùng lệnh đi kèm ứng dụng theo [hướng dẫn cài đặt đầy đủ](../../README.md#install-dsh).

## Kết nối và sử dụng

Gửi trong cuộc trò chuyện:

> Mở CrossDesk để tôi chọn máy từ xa.

Nhập ID thiết bị từ xa và mật khẩu truy cập trong bảng bên phải. Khi cần Agent thao tác máy từ xa, chọn **Cho phép Agent thao tác**, rồi nhấn **Cấp quyền và kết nối**. Sau khi thấy màn hình từ xa, bạn có thể giao nhiệm vụ, ví dụ:

> Tìm thông tin thời tiết trong trình duyệt trên máy từ xa.

Chỉ nhập mật khẩu trong bảng. Khi cần dừng thao tác, nhấn **Tạm dừng Agent**; khi dùng xong, chọn **Ngắt kết nối** trong **Tùy chọn phiên**. Chỉ đóng thanh bên không ngắt kết nối từ xa.

Xem cách lưu thiết bị, tự kết nối, tiếp quản thủ công và dùng âm thanh trong [hướng dẫn sử dụng](../../README.md#quy-trình-kết-nối-của-người-dùng). Nếu không có công cụ hoặc hình ảnh, xem [câu hỏi thường gặp](../../README.md#install-troubleshooting).

## Cập nhật và gỡ cài đặt

Cài gói nền tảng mới qua trang plugin rồi khởi động lại Harness. Bạn cũng có thể gỡ trên trang plugin hoặc chạy:

```sh
dsh plugin --profile desktop remove @chunkdash/crossdesk-mcp
```

Sau khi gỡ, cấu hình thiết bị và thông tin xác thực hệ thống đã lưu trên máy vẫn được giữ.

Xem biên dịch nguồn, cấu hình chương trình gốc và tích hợp plugin trong [cấu hình DSH](../../DEVELOPMENT.md#harness-development).
