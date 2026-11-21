# Thông tin mã nguồn và giấy phép CLI

[Về hướng dẫn CLI](../README.md)

Mã nguồn CrossDesk CLI nằm trong [kho mã CrossDesk](https://github.com/kunkundi/crossdesk). Sau khi tải gói cài đặt một phiên bản, hãy dùng phiên bản nguồn và thông tin biên dịch ghi trong gói để tìm đúng mã nguồn; không coi nhánh hiện tại là nguồn chính xác của gói đó.

CLI dùng [MiniRTC](https://github.com/kunkundi/minirtc); phiên bản cụ thể do commit submodule `deps/submodules/minirtc` của kho chính ghi lại. Dùng `--recurse-submodules` khi clone hoặc chạy `git submodule update --init --recursive` trong kho hiện có. Sau khi cập nhật phụ thuộc, cần biên dịch lại CLI.

Gói gốc npm kèm ảnh chụp mã nguồn `source/`, giữ cấu trúc thư mục CLI, thư viện chung, cấu hình biên dịch và MiniRTC. Nếu cần tự biên dịch, cấu hình `--CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y` trong thư mục nguồn theo [các bước biên dịch](terminal-client.md#biên-dịch-và-cài-đặt).

CrossDesk dùng [GPL-3.0-only](../../../LICENSE); MiniRTC giữ LGPL-3.0 và thông báo bên thứ ba của mình. Các phụ thuộc khác theo giấy phép riêng; tài liệu cụ thể theo giấy phép và thông tin nguồn kèm phiên bản đang dùng.

## Nguồn tích hợp và thay đổi phụ thuộc

CLI được tích hợp vào CrossDesk từ thư mục phát triển độc lập; điểm vào, script đóng gói, trình khởi chạy npm và tiện ích Pi nằm ở `apps/cli`, triển khai phiên và kết xuất terminal lần lượt ở `libs/controller`, `libs/terminal`.

- Kho CrossDesk: `https://github.com/kunkundi/crossdesk.git`. Mã giao thức CLI độc lập ban đầu lấy từ `becd1c0437f6c4fad757052c70db1eba71cf312f`; khi tích hợp, kho đích dựa trên `2f57d0ab`, nguồn wire hai bên giống nhau, tái dùng trực tiếp `libs/wire` hiện có.
- MiniRTC: `https://github.com/kunkundi/minirtc.git`, phiên bản cố định bởi commit submodule `deps/submodules/minirtc` ghi trong kho chính. Khi tích hợp dùng `b9ee1fa7fe038fdf771f5dec1b1d561d78273f15`, dựa trên `cdcd0d8fb0e9b688f3bf6416fe52b634cf20d9ca` trước tích hợp, chỉ thêm bốn thay đổi che thông tin xác thực trong nhật ký dưới đây.
- Tên luồng, HostInformation và nhập liệu của `ControllerSession` tham khảo `rtc_bridge.cpp` Android; danh tính dùng giao thức đăng ký tạm `cli` một Peer; thích ứng terminal không phụ thuộc JNI, Surface hay GuiRuntime.
- Bộ đệm khung tuân thủ hợp đồng mượn dữ liệu callback `MiniRtcVideoFrame`, sao chép trực tiếp vào bộ nhớ CPU có giới hạn, không giữ con trỏ native qua các callback.
- PreviewSubscription tái dùng triển khai wire và trường capability của đối tác, không thêm giao thức máy được điều khiển.
- Bảng trực tiếp MCP thêm callback `on_receive_encoded_video` của MiniRTC, chuyển tiếp khung H.264 Annex B hoàn chỉnh trước giải mã; callback và yêu cầu khung khóa do nguồn submodule hiện tại cung cấp. Trường mới của `MiniRtcParams` đổi kích thước ABI, CLI và MiniRTC bắt buộc biên dịch lại cùng nhau; khi phát hành, ghi đồng bộ commit submodule chứa thay đổi này.
- Mã hóa ảnh tự động hóa tái dùng libyuv 2025.8.14 có sẵn của MiniRTC, cố định libpng tĩnh v1.6.59 qua Xmake; phụ thuộc nén libpng tiếp tục dùng zlib nền tảng. Giấy phép và mã nguồn tương ứng của phụ thuộc mới cũng nên đưa vào tài liệu phụ thuộc bản phát hành chính thức.

MiniRTC đã hợp nhất che thông tin xác thực trong nhật ký tại commit `b9ee1fa7fe038fdf771f5dec1b1d561d78273f15`: bốn điểm Join, Join xếp hàng, hủy Join, Leave chỉ ghi ID trước `@`, không ghi mật khẩu. `apps/cli/scripts/prepare-deps.sh` chỉ khởi tạo submodule theo kho chính, không áp dụng bản vá riêng nữa; khi cập nhật MiniRTC, chỉ cần đổi commit submodule. Che thông tin trong submodule chung cũng áp dụng cho chương trình máy tính; thay đổi này không đổi xác thực, TLS, TURN hay giao thức đa phương tiện.

Gói gốc npm mang ảnh chụp nguồn CLI, thư viện chung, cấu hình biên dịch và MiniRTC, không sao chép máy tính, di động, kiểm thử hay bộ nhớ đệm biên dịch cục bộ. Gói nguồn giữ cấu trúc kho mã; cấu hình `--CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y` trong `source/` rồi biên dịch.

LICENSE giữ GPL-3.0 của CrossDesk; submodule MiniRTC giữ LGPL-3.0 và thông báo bên thứ ba. Giấy phép, mã nguồn tương ứng và tài liệu liên kết lại của phụ thuộc Xmake ngoài vẫn cần cung cấp cùng bản chính thức.
