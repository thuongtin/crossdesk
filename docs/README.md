# Danh mục tài liệu CrossDesk

[Trang chủ dự án](../README.md) · [English user guide](USER_GUIDE_EN.md)

## Sử dụng lần đầu

1. [Tải xuống và chọn ứng dụng](../README.md#download).
2. [Cài đặt và cấp quyền hệ thống](USER_GUIDE.md#installation).
3. [Kết nối bằng ID thiết bị](USER_GUIDE.md#quick-start), hoặc [kết nối IP trực tiếp](USER_GUIDE.md#lan-direct) trong mạng LAN.
4. Khi không kết nối được, xem [câu hỏi thường gặp](FAQ.md).

## Tìm theo tình huống sử dụng

| Tình huống | Tài liệu |
| --- | --- |
| Tìm vị trí thao tác trong ứng dụng máy tính | [Hướng dẫn sử dụng máy tính](USER_GUIDE.md) |
| Điều chỉnh hình ảnh, âm thanh, gửi nhận tệp | [Thao tác phiên](USER_GUIDE.md#session) |
| Quản lý tập trung nhiều máy tính | [Bức tường màn hình (DeskHub)](deskhub.md) |
| Giới hạn thiết bị được phép kết nối | [Thiết bị đáng tin cậy và ghép đôi](CONNECTIONS.md#trusted-devices) |
| Cài đặt lại hoặc di chuyển danh tính máy này | [Sao lưu và di chuyển](CONNECTIONS.md#backup) |
| Cập nhật ứng dụng | [Cập nhật và xử lý sự cố](UPDATER.md) |
| Dùng điện thoại hoặc máy tính bảng điều khiển máy tính | [iPhone / iPad](../apps/ios/README.md) · [Android](../apps/android/README.md) · [HarmonyOS](../apps/harmony/MOBILE_PARITY.md) |
| Kết nối bằng terminal | [Hướng dẫn sử dụng CLI](../apps/cli/README.md) |
| Cho Agent thao tác máy tính và giám sát | [Cài đặt và sử dụng MCP](../apps/mcp/README.md) |
| Truy cập Windows trước khi đăng nhập | [Truy cập không giám sát](USER_GUIDE.md#windows-service) |
| Chạy Linux không có màn hình | [Chạy không màn hình](HEADLESS.md) |
| Sử dụng máy chủ riêng | [Tự lưu trữ (self-host)](SELF_HOSTING.md) |
| Tự biên dịch ứng dụng | [Hướng dẫn biên dịch](BUILD.md) |
| Chuyển ngôn ngữ giao diện | [Cài đặt ngôn ngữ](../localization/README.md) |

## Biên dịch và tích hợp

| Chủ đề | Tài liệu |
| --- | --- |
| Biên dịch, đóng gói và ký ứng dụng máy tính | [Tiếng Trung](BUILD.md) · [English](BUILD_EN.md) · [Ảnh môi trường biên dịch Linux](../docker/linux-build/README.md) |
| Phân lớp GUI, trạng thái và phân công mã nguồn | [Kiến trúc GUI](gui-architecture.md) |
| Định địa chỉ ID / IP, xác thực và vòng đời phiên | [Kiến trúc kết nối](connection-architecture.md) |
| Giao diện đăng ký và băng thông Bức tường màn hình (DeskHub) | [Giao thức xem trước và truyền tải](deskhub.md#传输实现与接口) |
| Kết xuất CLI, nhập liệu và hỗ trợ terminal | [Ứng dụng terminal](../apps/cli/docs/terminal-client.md) · [Biên bản kiểm chứng](../apps/cli/docs/validation.md#验证记录) |
| Tích hợp terminal và tự động hóa | [Bridge](../apps/cli/docs/bridge-protocol.md) · [Giao diện tự động hóa gốc](../apps/cli/docs/automation-protocol.md) |
| Công cụ MCP và tích hợp ứng dụng chủ | [Biên dịch và tích hợp](../apps/mcp/DEVELOPMENT.md) |
| Biên dịch ứng dụng di động và hành vi nền tảng | [Android](../apps/android/README.md#构建) · [iOS](../apps/ios/README.md#build) · [HarmonyOS](../apps/harmony/README.md#构建) |
| Hỗ trợ máy tính HarmonyOS và quyền | [Tài liệu nền tảng PC](../apps/harmony/desktop-port/README.md) |
| Phiên bản thành phần và phát hành npm | [Quản lý phiên bản](VERSIONING.md) · [Phân phối CLI](../apps/cli/docs/npm-distribution.md#生成包) · [Phát hành MCP](../apps/mcp/DEVELOPMENT.md#github-actions-发布到-npm) |
| Chữ ký cập nhật, bản kê và quy trình nâng cấp | [Biên dịch và phát hành trình nâng cấp](UPDATER.md#升级器构建与发布) |
| Tài nguyên dịch và ảnh chụp màn hình | [Bảo trì bản địa hóa](../localization/README.md#维护资源) · [Tạo ảnh chụp màn hình](images/REGENERATE.md) |

Các tính năng bản phát triển có thể chưa nằm trong gói đã phát hành; hãy căn cứ vào tài liệu và giao diện của phiên bản bạn dùng. Khi báo cáo sự cố, cung cấp hệ điều hành hai bên, phiên bản, phương thức kết nối và các bước tái hiện; trên máy tính, bạn có thể xuất nhật ký chẩn đoán qua **Menu chính → Phản hồi và góp ý**.
