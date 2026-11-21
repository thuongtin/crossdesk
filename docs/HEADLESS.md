# Chạy Linux không màn hình

[Về README](../README.md) · [English](HEADLESS_EN.md)

Máy chủ Linux không có màn hình có thể tái sử dụng màn hình X11 hiện có hoặc tạo màn hình ảo độc lập qua Xvfb. Ghi hình, bàn phím, chuột, con trỏ và bảng nhớ tạm dùng cùng một phiên hiển thị.

Trước hết chọn cách theo màn hình bạn muốn thấy:

- **Màn hình và ứng dụng đã mở hiện có:** Khởi động bình thường theo phần sau.
- **Màn hình độc lập trên máy chủ:** Dùng [tạo màn hình ảo độc lập](#tạo-màn-hình-ảo-độc-lập).
- **Tiếp tục chạy sau khi đóng terminal:** Xem [chạy nền](#chạy-nền).

Xem chọn DEB thường hay bản màn hình ảo trong [hướng dẫn cài đặt](USER_GUIDE.md#installation). Chế độ không màn hình dùng để truy cập máy tính Linux này từ xa; để điều khiển máy khác trong terminal, dùng [CLI](../apps/cli/README.md).

## Giữ màn hình Ubuntu và ứng dụng ban đầu

Chỉ cần mở CrossDesk bình thường; chương trình tự phát hiện và chọn màn hình hiện có của người dùng hiện tại, giữ Dock, thanh tác vụ, hình nền và ứng dụng đã mở. Không cần biết số hiển thị như `:0`, `:10` hay đặt `DISPLAY`:

```bash
# Gói cài đặt
crossdesk
# Biên dịch từ mã nguồn
xmake r crossdesk
```

Khi có nhiều màn hình khả dụng, chương trình cũng tự chọn: ưu tiên màn hình trong môi trường khởi động, tiếp theo là phiên đang đăng nhập, màn hình chính hoạt động được hệ thống đánh dấu, màn hình hoạt động khác và màn hình vẫn trực tuyến. Khi không có thông tin phiên hệ thống hoặc cùng ưu tiên, chọn theo thứ tự hiển thị ổn định để tránh mỗi lần khởi động lại chuyển màn hình khác. Phiên mất hiệu lực và Xvfb trống không có trình quản lý cửa sổ không được chọn làm màn hình hiện có.

Tái sử dụng màn hình hiện có không phụ thuộc Xvfb và không kết thúc màn hình hay ứng dụng khác khi thoát CrossDesk. Nếu phiên gốc đã khóa, từ xa sẽ thấy màn hình khóa hệ thống trước, cần mở khóa bằng mật khẩu hệ thống của người dùng đó.

Tự phát hiện dùng thông tin phiên đăng nhập hệ thống và socket X11 của người dùng hiện tại, đồng thời xác minh trình quản lý cửa sổ thực sự khả dụng. Bản phân phối không có systemd/logind vẫn có thể tự phát hiện và chọn màn hình. Xác thực dùng `XAUTHORITY` hiện có (nếu chưa đặt thì dùng vị trí xác thực mặc định Xlib), khi cần thử tệp xác thực GDM của người dùng hiện tại. Không tự chọn màn hình người dùng khác hay giao diện chào đăng nhập.

`--headless` có thể kích hoạt rõ quy trình tự chọn này, ví dụ để bỏ qua biến môi trường Wayland mất hiệu lực trong terminal. `--headless-display` chỉ dành cho người dùng nâng cao cần ghi đè lựa chọn mặc định hoặc gỡ lỗi:

```bash
crossdesk --headless-display :10  # Số ví dụ, dùng theo phiên thực tế
```

Tùy chọn ghi đè này chỉ kết nối màn hình hiện có được chỉ định; nếu không kết nối được hoặc không có trình quản lý cửa sổ đang chạy sẽ báo lỗi, không âm thầm tạo màn hình khác.

**Không thêm `--headless-size` hoặc `--headless-session` để kết nối màn hình gốc**: chúng dùng tạo Xvfb riêng. Độ phân giải màn hình gốc do dịch vụ hiển thị ban đầu quản lý.

## Bảng điều khiển dòng lệnh

Ở chế độ không màn hình, terminal chỉ hiện ID thiết bị, mật khẩu kết nối, trạng thái trực tuyến và kết quả thao tác; nhật ký thông thường ghi vào tệp. Khởi động bình thường qua SSH / môi trường không đồ họa sẽ tự vào chế độ này; cũng có thể bật rõ:

```bash
xmake r crossdesk --headless
```

Sau khi chương trình chạy, nhập lệnh trong cùng terminal rồi nhấn Enter:

| Lệnh | Tác dụng |
| --- | --- |
| `status` | Hiện ID máy này, mật khẩu kết nối hiện tại và trạng thái trực tuyến |
| `settings` | Mở menu cài đặt, nhập số hoặc tên để sửa; `back` quay lại |
| `settings show` | Chỉ xem cài đặt |
| `settings set tên giá-trị` | Sửa trực tiếp một cài đặt |
| `password` | Nhắc nhập mật khẩu mới; 6 chữ cái tiếng Anh hoặc chữ số, nhập `cancel` để hủy |
| `password Ab12Cd` | Gửi trực tiếp mật khẩu mới (ví dụ, hãy tự chọn mật khẩu) |
| `random` | Tạo mật khẩu ngẫu nhiên và gửi thay đổi |
| `help` | Hiện trợ giúp |
| `quit` | Đóng CrossDesk; ở chế độ tái sử dụng không đóng màn hình gốc |

Menu cài đặt dùng chung `config.ini` với GUI; mỗi thay đổi được lưu ngay sau khi xác minh hợp lệ, không cần lệnh lưu riêng. Nhập `cancel` để bỏ giá trị đang sửa. Trạng thái bảng điều khiển, menu, lời nhắc mật khẩu và trợ giúp theo cài đặt ngôn ngữ, hỗ trợ cùng 11 ngôn ngữ với GUI, gồm tiếng Việt; tên lệnh không đổi.

H.264 tự ưu tiên mã hóa và giải mã phần cứng, quay về phần mềm nếu không khả dụng hoặc khởi tạo thất bại; AV1 dùng mã hóa và giải mã phần mềm. Tăng tốc phần cứng không còn công tắc riêng, cấu hình cũ `hardware_video_codec` được tự dọn.

| Tên cài đặt | Giá trị / định dạng | Cách có hiệu lực |
| --- | --- | --- |
| `language` | `system` / `zh-CN` / `en-US` / `ru-RU` / `zh-Hant` / `ja` / `ko` / `de` / `fr` / `es` / `pt-BR` / `vi` | Chuyển ngôn ngữ bảng điều khiển và GUI ngay, giữ ở lần khởi động sau |
| `codec` | `h264` / `av1` | Kết nối lại sau khi lưu |
| `turn` | `off` / `auto` / `udp` / `tcp` | Kết nối lại sau khi lưu; `udp` / `tcp` nghĩa là buộc chuyển tiếp |
| `server_host` | Tên máy, IP hoặc địa chỉ báo hiệu HTTPS/WSS, hỗ trợ đường dẫn và cổng rõ ràng | Nếu chưa bật tự lưu trữ thì lưu dự phòng, nếu đã bật thì kết nối lại |
| `server_port` | `1`–`65535` | Như trên |
| `self_hosted` | `on` / `off` | Cấu hình địa chỉ, cổng trước rồi bật; kết nối lại sau khi lưu |
| `privacy` | `on` / `off` | Hành vi tự bật màn hình riêng tư của kết nối từ xa sau đó |
| `file_path` | Thư mục tuyệt đối đã tồn tại và ghi được; `default` khôi phục mặc định | Tệp nhận sau đó; đường dẫn có thể có dấu cách, không cần thêm dấu ngoặc kép |
| `autostart` | `on` / `off` | Khởi động khi đăng nhập màn hình đồ họa lần sau; không đồng nghĩa dịch vụ hệ thống khởi động cùng máy |
| `daemon` | `on` / `off` | Có hiệu lực ở lần khởi động sau trong chế độ thường; bảng điều khiển không màn hình vẫn ở tiền cảnh |

Ví dụ, nhập trong bảng điều khiển đang chạy:

```text
settings
1
en-US
back
settings set codec av1
settings set file_path /home/user/Received Files
```

Khi có phiên từ xa (kể cả đang kết nối) hoặc đổi mật khẩu chưa hoàn tất, cài đặt liên quan kết nối bị từ chối để tránh gián đoạn phiên. Nên kết thúc phiên từ xa trước khi đổi địa chỉ và cổng máy chủ. Chuyển tự lưu trữ có thể chuyển sang ID và mật khẩu tương ứng máy chủ đó; căn cứ `status` sau kết nối lại. Chất lượng hình ảnh và tốc độ khung hình do máy điều khiển quản lý, chế độ Linux không màn hình dùng ghi hình X11, không có thêm mục sửa toàn cục cho các mục này.

Đổi mật khẩu cần kết nối máy chủ báo hiệu. Bảng điều khiển hiện kết quả gửi, xác nhận và kết nối lại; chỉ dùng mật khẩu mới sau khi máy chủ xác nhận, dùng cùng quy trình lưu thông tin xác thực và phục hồi hết thời gian như GUI. Khi ngoại tuyến, sai định dạng mật khẩu hoặc thay đổi chưa hoàn tất sẽ có thông báo rõ. Đổi mật khẩu kích hoạt kết nối lại.

Đường dẫn nhật ký hiện khi khởi động, tiếp tục dùng `crossdesk-YYYYMMDD-HHMMSS.log` và `minirtc-YYYYMMDD-HHMMSS.log`, giữ tên tệp và quy tắc luân chuyển cũ, không tạo riêng nhật ký chế độ không màn hình. Chẩn đoán khởi động không màn hình ghi vào nhật ký CrossDesk; bản sao nhật ký terminal và nội dung thành phần bên thứ ba ghi trực tiếp stdout/stderr được xử lý im lặng. Đầu ra trạng thái ID/mật khẩu gửi thẳng terminal, không qua hệ thống nhật ký. Chạy không tương tác (ví dụ systemd, chuyển hướng đầu ra) mặc định không hiện mật khẩu rõ; đóng đầu vào chuẩn không kết thúc dịch vụ nền.

Bảng điều khiển tương tác chạy ở tiền cảnh; cài đặt daemon tích hợp không tách tiến trình trong chế độ này. Khi đã có tiến trình bản cũ, cần thoát phiên bản đó rồi chạy bản mới mới dùng được bảng điều khiển.

## Tạo màn hình ảo độc lập

Bản thường `crossdesk` trên Debian/Ubuntu không phụ thuộc Xvfb hay Xfce, tái sử dụng màn hình hiện có không cần cài thêm chúng. Nếu cần màn hình độc lập đầy đủ, có thể chọn gói `crossdesk-virtual-desktop-xfce`: chứa cùng chương trình CrossDesk và bắt buộc cài Xvfb, Xfce, công cụ phiên D-Bus qua APT (`--no-install-recommends` cũng không bỏ qua). Thành phần này do kho phần mềm hệ thống cung cấp, gói cài không phải môi trường màn hình ngoại tuyến đầy đủ.

Chỉ được cài một trong hai gói. Thoát CrossDesk trước, dùng `sudo apt install ./goi-cai-dat-da-chon.deb` để nâng cấp hoặc chuyển phiên bản, rồi chạy chương trình, cấu hình người dùng cũ được giữ. Cài và chuyển không tự khởi động phiên ảo, sửa màn hình mặc định hay xóa Xfce đã cài trước. Tải cập nhật chọn theo loại gói đã cài; nếu chưa phát hành bản tải tương ứng hoặc siêu dữ liệu thiếu, mở danh sách Releases cho bạn chọn, không tự đổi loại gói khác.

Khi ban đầu chưa có phiên đồ họa hoặc muốn mở màn hình riêng, có thể khởi động phiên Xfce độc lập. Ví dụ Ubuntu / Debian (bản màn hình ảo đã gồm phụ thuộc này; người dùng bản thường và chạy từ nguồn cài theo nhu cầu):

```bash
sudo apt install xvfb xfce4 dbus
crossdesk --headless-size 1920x1080 --headless-session startxfce4
```

Độ phân giải mặc định 1920×1080, chiều rộng và cao phải là số chẵn trong phạm vi 320–8192. `--headless-session` nhận tên hoặc đường dẫn chương trình thực thi, khởi động qua `dbus-run-session` riêng. Nếu cần tham số, tạo script thực thi dùng `exec` để chạy môi trường màn hình hoặc trình quản lý cửa sổ.

Nếu chỉ gỡ lỗi ghi hình, bạn có thể chủ động chạy Xvfb trần:

```bash
sudo apt install xvfb
crossdesk --headless-session none
```

Xvfb trần chỉ cung cấp màn hình, **không tạo Dock, thanh tác vụ hay cửa sổ ứng dụng gốc**. Chỉ dùng `--headless-size` mà không chỉ định phiên màn hình cũng thuộc trường hợp này, nhật ký chẩn đoán khởi động sẽ ghi lời nhắc. Tình trạng trước đây “chỉ thấy CrossDesk, còn lại đen” chính là do vào màn hình trần độc lập như vậy.

## Tự chọn và giới hạn

- Khởi động đồ họa thông thường tiếp tục dùng `DISPLAY` hoặc phiên Wayland hiện có. Khi X11 không có đầu ra RandR hoạt động hoặc không có phần mở rộng RandR, ghi hình cửa sổ gốc khả dụng.
- `--headless`, hoặc khi không có `DISPLAY` kết nối được và chưa đặt môi trường Wayland, sẽ tự chọn màn hình X11 hiện có trước rồi mới chuyển sang Xvfb độc lập; khi không có môi trường màn hình, Xvfb trần dự phòng thông báo rõ không có Dock và ứng dụng gốc.
- Không thể dùng lẫn `--headless-display` với `--headless-size` / `--headless-session`.
- `--no-headless` tắt tự phát hiện và dự phòng; `--headless-help` xem giải thích lệnh.
- Không di chuyển ứng dụng giữa các phiên hiển thị hay tự chuyển phiên đang chạy sang màn hình khác. Khi phiên vật lý đã mất hiệu lực, tạo Xvfb khác không khôi phục được ứng dụng của nó.

## Chạy nền

Khi tạo Xvfb độc lập, giữ một tiến trình quản lý ở tiền cảnh để giám sát Xvfb, phiên màn hình và CrossDesk. Khi thoát chương trình, gửi SIGINT / SIGTERM / SIGHUP hoặc bất kỳ tiến trình được giám sát nào thoát, các tiến trình con của phiên đó và tệp xác thực tạm được dọn. Xvfb tự cấp số hiển thị để tránh chiếm màn hình hiện có; lắng nghe TCP tắt, kết nối X11 dùng cookie xác thực ngẫu nhiên, tệp xác thực chỉ người dùng hiện tại đọc được.

Nhật ký chẩn đoán khởi động ghi đường dẫn `DISPLAY` và `XAUTHORITY`; sau khi đặt hai biến này trong terminal cùng người dùng, bạn có thể chạy ứng dụng khác vào màn hình ảo. Tệp xác thực bị xóa khi phiên thoát.

Để giữ dịch vụ sau khởi động máy, có thể tạo unit systemd cấp người dùng `~/.config/systemd/user/crossdesk-headless.service`, đổi `ExecStart` thành đường dẫn chương trình thực tế:

```ini
[Unit]
Description=CrossDesk virtual desktop

[Service]
Type=simple
ExecStart=/usr/bin/crossdesk --headless-size 1920x1080 --headless-session startxfce4
Restart=on-failure
RestartSec=5
KillMode=mixed
TimeoutStopSec=15

[Install]
WantedBy=default.target
```

```bash
systemctl --user daemon-reload
systemctl --user enable --now crossdesk-headless.service
journalctl --user -u crossdesk-headless.service -f
```

Dịch vụ cần tái sử dụng màn hình gốc nên đổi `ExecStart` thành `/usr/bin/crossdesk` (tự phát hiện và chọn; bảo đảm đã đăng nhập phiên đồ họa). Để chạy dịch vụ người dùng khi chưa đăng nhập, quản trị viên cần bật linger cho người dùng đó: `sudo loginctl enable-linger tên-người-dùng`. Ở chế độ Xvfb riêng, tùy chọn daemon tích hợp không có hiệu lực; systemd chịu trách nhiệm khởi động lại toàn bộ màn hình ảo. Dừng chế độ này kết thúc ứng dụng trong màn hình ảo, hãy lưu công việc trước.

## Giới hạn khi chạy

GUI hiển thị ảo dùng kết xuất phần mềm, hình ảnh đi qua ghi hình X11 → NV12. Mã hóa video vẫn theo cài đặt CrossDesk và khả năng bộ mã hóa máy này. Âm thanh phụ thuộc dịch vụ âm thanh khả dụng của người dùng hiện tại; bản thân Xvfb không cung cấp card âm thanh ảo. Triển khai này dành cho Linux; xem hỗ trợ Windows không màn hình trong [câu hỏi thường gặp](FAQ.md).
