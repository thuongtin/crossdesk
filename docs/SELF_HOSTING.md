# Máy chủ tự lưu trữ (self-host)

[Về README](../README.md) · [English](SELF_HOSTING_EN.md)

Dịch vụ báo hiệu và chuyển tiếp TURN của CrossDesk được duy trì trong dự án riêng [CrossDesk Server](https://github.com/kunkundi/crossdesk-server). Trang này trình bày lối vào triển khai và cách cấu hình ứng dụng hiện tại; hãy dùng các tệp máy chủ đồng bộ trong cùng một Release.

Khi dùng dịch vụ công cộng, bạn không cần tự triển khai. Chỉ làm theo trang này khi cần kết nối thiết bị đến máy chủ riêng. Trước khi nâng cấp lên ứng dụng mới yêu cầu xác thực danh tính thiết bị, hãy cập nhật máy chủ trước, rồi cho cả hai bên dùng cùng địa chỉ và cổng máy chủ.

## 1. Triển khai máy chủ

Các bước sau dành cho máy chủ Linux đã cài Docker Engine và plugin Compose; trước hết xác nhận `docker compose version` hoạt động. Compose hiện dùng mạng máy chủ, chạy riêng dịch vụ báo hiệu và Coturn.

Theo [tài liệu triển khai máy chủ](https://github.com/kunkundi/crossdesk-server#运行服务), tải `compose.yaml` và `env.example` từ [Releases](https://github.com/kunkundi/crossdesk-server/releases) của dự án đó, đặt vào cùng thư mục. Thực hiện trong thư mục này:

```bash
cp env.example .env
```

Dùng trình soạn thảo văn bản sửa `.env`, kiểm tra ít nhất các mục sau, nhập xong rồi mới khởi động. Tham số đầy đủ theo tệp môi trường đi kèm; bạn cũng có thể tham khảo [ví dụ biến môi trường](https://github.com/kunkundi/crossdesk-server/blob/admin-dashboard/.env.example) trong kho máy chủ.

| Cấu hình | Nội dung cần nhập |
| --- | --- |
| `CROSSDESK_IMAGE` | Tag phát hành cố định khớp cấu hình đã tải |
| `EXTERNAL_IP` / `INTERNAL_IP` | IP công cộng / nội bộ thực tế của máy chủ; không dùng trực tiếp địa chỉ ví dụ |
| `CROSSDESK_SERVER_PORT` | Cổng báo hiệu, ví dụ `9099` |
| `COTURN_PORT` | Cổng chuyển tiếp, ví dụ `3478` |
| `MIN_PORT` / `MAX_PORT` | Phạm vi cổng đa phương tiện TURN |
| `COTURN_AUTH_SECRET` | Khóa bí mật dùng chung của dịch vụ báo hiệu và Coturn; không nhập vào ứng dụng |
| `COTURN_STATELESS_NONCE_SECRET` | Khóa bí mật nonce được tạo riêng và lưu bền vững |
| `CROSSDESK_DATA_DIR` / `CROSSDESK_LOG_DIR` | Vị trí lưu bền vững dữ liệu, chứng chỉ và nhật ký |

Tạo riêng hai khóa bí mật bằng `openssl rand -hex 32`. Tường lửa và nhóm bảo mật đám mây cần cho phép cổng báo hiệu TCP, cổng chuyển tiếp TCP/UDP và phạm vi cổng đa phương tiện UDP đã chọn. Xem cấu hình gắn kết container và mạng trong [tệp Compose chính thức](https://github.com/kunkundi/crossdesk-server/blob/admin-dashboard/compose.yaml).

Sau khi lưu cấu hình, kiểm tra và khởi động:

```bash
sudo docker compose config -q
sudo docker compose pull
sudo docker compose up -d
sudo docker compose ps
```

## 2. Cấu hình ứng dụng máy tính

Khi dùng chứng chỉ tự ký, trước hết nhập chứng chỉ gốc trên cả hai bên theo [các bước tin cậy chứng chỉ](#3-cấu-hình-tin-cậy-chứng-chỉ), rồi kiểm tra trạng thái kết nối.

1. Ngắt các phiên hiện có, nhấn **☰ → Cài đặt** ở góc trên bên phải cửa sổ chính.
2. Cuộn xuống trong danh sách cài đặt, nhấn nút **Chỉnh sửa** ở hàng **Cấu hình máy chủ tự lưu trữ (self-host)**.
3. Nhập **Địa chỉ máy chủ**, **Cổng dịch vụ báo hiệu**, nhấn “Xác nhận”. Địa chỉ hỗ trợ tên máy hoặc IP (như `my-server.example.com`), địa chỉ báo hiệu có đường dẫn (như `my-server.example.com/signal-path`) hoặc URL `https://` / `wss://`. Cổng được ghi rõ trong URL được ưu tiên; nếu không, dùng cổng dịch vụ báo hiệu nhập riêng. Mọi kết nối đều dùng WSS; địa chỉ có đường dẫn cần proxy ngược chuyển tiếp đường dẫn đó đến dịch vụ WebSocket báo hiệu, địa chỉ trang web thông thường không thể dùng trực tiếp cho kết nối báo hiệu.
4. Trở về cửa sổ cài đặt, bật công tắc bên phải **Cấu hình máy chủ tự lưu trữ (self-host)** rồi nhấn “Lưu”.
5. Cấu hình giống nhau trên máy điều khiển và máy được điều khiển. Chờ cả hai bên hiển thị “Đã kết nối máy chủ”, rồi dùng ID hiện hiển thị trên máy được điều khiển để kết nối.

Địa chỉ và cổng được lưu cùng nhau khi xác nhận cài đặt cuối cùng; nếu định dạng không hợp lệ hoặc lưu thất bại, nội dung nhập được giữ và lỗi được hiển thị.

<p align="center">
  <img src="images/desktop-self-hosted-zh.png" width="720" alt="Cài đặt tự lưu trữ của ứng dụng Slint hiện tại: địa chỉ máy chủ và cổng báo hiệu" />
</p>

Sau khi chuyển máy chủ, hãy kiểm tra lại ID máy này; danh tính thiết bị gắn với máy chủ, không nên áp dụng trực tiếp ID của dịch vụ cũ.

## 3. Cấu hình tin cậy chứng chỉ

Ứng dụng kết nối dịch vụ báo hiệu qua TLS. Chứng chỉ phải bao gồm tên miền / IP đã nhập và được hệ thống ứng dụng tin cậy. Khi dùng chứng chỉ do CA đáng tin cậy cấp, cấu hình chuỗi chứng chỉ theo hướng dẫn máy chủ; khi dùng chứng chỉ tự ký, cần tin cậy chứng chỉ gốc do bạn triển khai.

[Script tạo chứng chỉ](https://github.com/kunkundi/crossdesk-server/blob/admin-dashboard/docker/generate_certs.sh) mặc định chỉ ghi `EXTERNAL_IP` vào phạm vi địa chỉ của chứng chỉ; khi dùng chứng chỉ này, bạn nên nhập IP đó trong ứng dụng. Chuyển sang tên miền cần cấu hình chứng chỉ chứa tên miền đó. Khi đã có chứng chỉ, [script khởi động](https://github.com/kunkundi/crossdesk-server/blob/admin-dashboard/docker/start.sh) dùng lại chúng; đổi IP trong `.env` không tự cập nhật chứng chỉ.

Tệp chứng chỉ mặc định ở `certs/` trong thư mục dữ liệu máy chủ. Tải `api.crossdesk.cn_root.crt` từ máy chủ của bạn về máy chạy ứng dụng, rồi thực hiện lệnh cho nền tảng tương ứng; không nhập chứng chỉ gốc không rõ nguồn gốc.

**Windows (PowerShell với quyền quản trị viên):**

```powershell
certutil -addstore "Root" "C:\path\to\api.crossdesk.cn_root.crt"
```

**Ubuntu / Debian：**

```bash
sudo cp /path/to/api.crossdesk.cn_root.crt /usr/local/share/ca-certificates/
sudo update-ca-certificates
```

**macOS：**

```bash
sudo security add-trusted-cert -d -r trustRoot \
  -k /Library/Keychains/System.keychain /path/to/api.crossdesk.cn_root.crt
```

Sau khi xong, mở lại ứng dụng. Chứng chỉ tự ký không có nghĩa là không mã hóa, nhưng bắt buộc cấu hình đúng tin cậy chứng chỉ gốc và xác minh tên máy.

## 4. Web và iOS

- **Web:** Cấu hình kết nối báo hiệu riêng theo [dự án ứng dụng Web](https://github.com/kunkundi/crossdesk-web-client). Trình duyệt phải tin cậy chứng chỉ của dịch vụ này; không khắc phục lỗi chứng chỉ bằng cách tắt xác minh TLS.
- **iOS:** Nhập máy chủ báo hiệu, cổng báo hiệu và cổng STUN/TURN trong “Cài đặt → Máy chủ”, rồi nhấn “Áp dụng”. Chứng chỉ tự ký cần cài chứng chỉ gốc tương ứng trên thiết bị và bật tin cậy.

## Thứ tự khắc phục sự cố

- **Chưa kết nối máy chủ / lỗi TLS:** Kiểm tra địa chỉ, cổng báo hiệu, thời hạn chứng chỉ, tên máy trong chứng chỉ và tin cậy chứng chỉ gốc trước.
- **Đã kết nối máy chủ nhưng thiết bị ngoại tuyến:** Xác nhận hai bên kết nối cùng dịch vụ, máy được điều khiển vẫn chạy và sao chép lại ID hiện tại.
- **Kết nối P2P thất bại:** Mặc định đã cho phép chuyển sang chuyển tiếp khi cần; có thể bật “Buộc kết nối chuyển tiếp” để kiểm tra sự cố kết nối trực tiếp. Kiểm tra trạng thái Coturn, cổng TURN và phạm vi cổng đa phương tiện, đồng thời đối chiếu tương thích phiên bản ứng dụng và máy chủ.
- **Thay đổi cấu hình chưa có hiệu lực:** Chạy lại `sudo docker compose up -d` trong thư mục cấu hình; chỉ khởi động lại container không áp dụng biến môi trường đã thay đổi.

Về thao tác nhật ký, sao lưu và di chuyển, tham khảo [tài liệu bảo trì máy chủ](https://github.com/kunkundi/crossdesk-server).
