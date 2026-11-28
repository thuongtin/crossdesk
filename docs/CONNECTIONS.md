# Phương thức kết nối, thiết bị đáng tin cậy và di chuyển danh tính

[Về trang chủ](../README.md) · [Hướng dẫn sử dụng](USER_GUIDE.md) · [Khắc phục sự cố kết nối](FAQ.md) · [Kiến trúc và giao thức kết nối](connection-architecture.md)

Tài liệu này giúp bạn chọn phương thức kết nối, quản lý thiết bị được phép truy cập và giữ danh tính kết nối trước khi cài đặt lại hoặc di chuyển.

## Chọn phương thức kết nối

| Phương thức | Tình huống phù hợp | Cần chuẩn bị trước kết nối |
| --- | --- | --- |
| ID thiết bị | Truy cập qua các mạng khác nhau, hoặc kết nối từ trình duyệt, điện thoại, terminal | Máy tính được điều khiển trực tuyến, hai bên kết nối cùng máy chủ, chuẩn bị ID gồm 9 chữ số và mật khẩu |
| IP trực tiếp | Máy tính, điện thoại hoặc máy tính bảng truy cập được máy tính được điều khiển qua mạng LAN hoặc VPN | Máy tính được điều khiển bật IP trực tiếp, chuẩn bị IP hoặc tên miền, cổng và mật khẩu của chế độ này trên máy đó |

Kết nối ID tự thử truyền trực tiếp giữa các thiết bị; nếu không thể kết nối trực tiếp thì có thể dùng relay. IP trực tiếp không phụ thuộc signaling server, cũng không tự xuyên qua bộ định tuyến Internet. IP trực tiếp hiện dành cho ứng dụng máy tính thông thường Windows, macOS, Linux; ứng dụng di động Android, iOS và HarmonyOS cũng có thể làm máy điều khiển. Web và CLI terminal dùng kết nối ID.

<a id="41-id-模式"></a>

## Kết nối bằng ID thiết bị

1. Chạy CrossDesk trên máy tính được điều khiển, chờ hiển thị “Đã kết nối máy chủ”.
2. Nhập ID gồm 9 chữ số của máy tính được điều khiển trên máy điều khiển, nhấn “Kết nối”.
3. Nhập mật khẩu kết nối hiện tại của máy tính được điều khiển. Nếu cần kết nối trực tiếp lần sau, chọn “Nhớ mật khẩu”.

Khi đổi hoặc làm mới mật khẩu chế độ ID, máy được điều khiển cần kết nối máy chủ. Chờ đổi thành công và ứng dụng kết nối lại rồi mới dùng mật khẩu mới. Mật khẩu gồm 6 chữ số hoặc chữ cái tiếng Anh, phân biệt hoa thường.

**Nâng cấp self-host:** Ứng dụng máy tính mới cần máy chủ hỗ trợ xác thực danh tính thiết bị; hãy nâng cấp máy chủ trước rồi nâng cấp ứng dụng. Lần đầu nâng cấp danh tính cũ cần mật khẩu hiện có; sau khi nâng cấp, thiết bị còn dựa vào khóa danh tính lưu trên máy này để đăng nhập. Mật khẩu kết nối vẫn dùng để cho phép truy cập từ xa, không thể dùng riêng để khôi phục danh tính thiết bị đã mất.

Xem [cấu hình self-host](SELF_HOSTING.md) và [sao lưu danh tính](#backup).

<a id="42-ip-模式"></a>

## Kết nối IP trực tiếp

1. Trên ứng dụng máy tính, bật **IP trực tiếp** trong **Cài đặt → Chung → Kết nối mạng** rồi lưu.
2. Chuyển sang chế độ IP trực tiếp ở cửa sổ chính, sao chép `IP:cổng` hiển thị trên máy được điều khiển.
3. Dán địa chỉ trên máy điều khiển và kết nối. Điện thoại hoặc máy tính bảng dùng ô **ID thiết bị, IP hoặc tên miền** ở màn hình chính. Nhập mật khẩu sáu ký tự của máy được điều khiển ở chế độ IP.

Cổng mặc định là `52888`; thực tế hãy căn cứ hiển thị trên máy được điều khiển. Có thể nhập IPv4, IPv6 hoặc tên miền. `home.example.net` dùng cổng mặc định, `home.example.net:55001` dùng cổng chỉ định. Trong cùng mạng LAN có thể nhập tên mà máy được điều khiển công bố, ví dụ `mini.local`. Khi hệ thống phân giải thất bại hoặc địa chỉ trả về không kết nối được, máy điều khiển Windows và Linux sẽ truy vấn lại một lần qua multicast trong mạng LAN. Khi chỉ định cổng IPv6 dùng dấu ngoặc vuông, ví dụ `[2001:db8::20]:52888`. Mật khẩu ID và IP độc lập với nhau.

Mặc định lắng nghe mọi giao diện mạng, có thể truy cập qua địa chỉ mạng dây, Wi-Fi hoặc VPN. Tường lửa cần cho phép TCP và UDP liên lạc. Chỉ chuyển lại chế độ ID không dừng lắng nghe IP; để dừng, tắt IP trực tiếp trong cài đặt rồi lưu, việc này cũng ngắt các phiên IP hiện có. Xem đầy đủ cách dùng địa chỉ, cổng và IPv6 trong [hướng dẫn IP trực tiếp](USER_GUIDE.md#lan-direct).

## Xem trạng thái kết nối và mã hóa

Sau khi kết nối, mở **Trạng thái mạng** trên thanh công cụ phiên để xem trực tiếp / relay và trạng thái mã hóa. Trên máy tính, di chuột lên nhãn **Trực tiếp** để xem IP và cổng mà máy này và máy từ xa đang dùng. Trên điện thoại hoặc máy tính bảng, chạm **Kết nối trực tiếp P2P** để mở danh sách địa chỉ, chạm lần nữa để thu gọn. Khi đi qua NAT, địa chỉ có thể là địa chỉ công cộng sau khi ánh xạ. Phía dưới DeskHub cũng có mục tương ứng.

IP trực tiếp yêu cầu truyền mã hóa. Để tương thích ứng dụng cũ, kết nối ID native có thể dùng truyền không mã hóa hoặc chỉ mã hóa một phần; không chỉ dựa vào “Đã kết nối máy chủ” để xác định trạng thái mã hóa phiên điều khiển từ xa. Nên dùng các phiên bản gần đây tương thích trên cả hai bên; nếu chưa xác nhận trạng thái, kiểm tra phiên bản và thông tin mạng trước.

Khi lưu mật khẩu cho một tên miền, máy điều khiển đồng thời ghi lại fingerprint SHA-256 của khóa công khai TLS trên máy được điều khiển. Sau khi DDNS đổi địa chỉ, bạn vẫn kết nối lại được từ cùng một mục lịch sử. Nếu khóa công khai của thiết bị thay đổi, máy điều khiển hiển thị fingerprint cũ và mới trước khi gửi mật khẩu. Sau khi xác nhận đã thay thiết bị, chọn **Kết nối thiết bị mới** và nhập lại mật khẩu. Ở lần kết nối đầu tiên và khi nhập trực tiếp địa chỉ IP dạng số, bạn cần tự đối chiếu đúng máy đích.

<a id="43-可选可信设备认证"></a>
<a id="trusted-devices"></a>

## Chỉ cho phép thiết bị đáng tin cậy điều khiển

Tùy chọn này mặc định tắt. Sau khi bật, máy điều khiển bắt buộc nằm trong danh sách tin cậy của máy này và vẫn cần nhập đúng mật khẩu kết nối.

1. **Sao chép thông tin ghép đôi trên máy điều khiển.** Trên máy tính, mở **Cài đặt → Chung → Bảo vệ quyền riêng tư → Quản lý thiết bị đáng tin cậy**; trên điện thoại và máy tính bảng, mở **Cài đặt → Thông tin ghép đôi máy này**.
2. **Thêm trên máy được điều khiển.** Dán thông tin ghép đôi và thêm tại **Quản lý thiết bị đáng tin cậy → Thêm thiết bị đáng tin cậy**, đối chiếu fingerprint với máy điều khiển qua kênh đáng tin cậy.
3. **Bật giới hạn.** Máy được điều khiển chọn **Chỉ cho phép thiết bị đáng tin cậy điều khiển** rồi lưu, sau đó kết nối theo quy trình bình thường.

Thông tin ghép đôi không chứa mật khẩu kết nối hay khóa riêng. Tin cậy là một chiều; nếu muốn điều khiển lẫn nhau, cần thêm bên kia trên cả hai máy. Thêm và xóa được lưu ngay; khi bật giới hạn, xóa một thiết bị sẽ ngắt kết nối của nó; danh sách rỗng sẽ từ chối tất cả máy điều khiển.

Ứng dụng máy tính mới, iOS, Android và HarmonyOS di động cung cấp mục ghép đôi. Ứng dụng cũ và CLI terminal hiện tại không hỗ trợ xác thực này, không thể kết nối đến máy tính được điều khiển đang bật giới hạn. Khi dùng gói đã phát hành, hãy xác nhận cài đặt đã có mục ghép đôi.

<a id="backup"></a>

## Cài đặt lại, xóa dữ liệu và di chuyển

### Máy tính

Trong chế độ máy tính thông thường, để di chuyển danh tính gốc cần giữ cùng nhau **bộ nhớ đệm ID / mật khẩu và `identity.json`**. Nếu còn muốn giữ danh sách được phép điều khiển máy này, đồng thời sao lưu `trusted-devices/devices.json` trong thư mục cấu hình.

Vị trí dữ liệu mặc định như sau. Khi di chuyển, giữ cả thư mục bộ nhớ đệm và cấu hình để tránh bỏ sót danh tính, danh sách và cài đặt.

| Môi trường | Thư mục bộ nhớ đệm | Thư mục cấu hình |
| --- | --- | --- |
| macOS / Linux | `~/.cache/CrossDesk/` | `~/.config/CrossDesk/` |
| Bản cài đặt Windows | `%LOCALAPPDATA%\CrossDesk\cache\` | `%APPDATA%\CrossDesk\` |
| Bản Windows di động không cần cài đặt | `data/` bên cạnh tệp thực thi | Cùng thư mục `data/` đó |

Khi macOS / Linux đặt `XDG_CACHE_HOME` hoặc `XDG_CONFIG_HOME`, lần lượt dùng `CrossDesk/` trong thư mục tương ứng. Bản Windows Debug dùng bộ nhớ đệm `cache/` trong thư mục làm việc.

Trước khi sao lưu hoặc khôi phục, thoát CrossDesk và giữ cấu trúc thư mục cùng quyền truy cập gốc. Bản sao lưu danh tính chứa khóa có thể dùng đăng nhập, nên bảo quản cẩn thận, không gửi làm tệp chẩn đoán đính kèm, cũng không để hai máy tính cùng dùng danh tính sao chép.

Chế độ không giám sát Windows dùng dữ liệu chia sẻ được bảo vệ trong `%ProgramData%\CrossDeskUnattended`; cần quản trị viên dừng ứng dụng và dịch vụ rồi mới sao lưu, không thể chỉ sao chép bộ nhớ đệm người dùng thông thường. Xem quản lý dịch vụ trong [Windows không giám sát](USER_GUIDE.md#windows-service).

Chỉ giữ mật khẩu không khôi phục được ID đã liên kết; chỉ giữ khóa cũng không tìm lại được ID đã cấp. Sau khi mất, nên khôi phục bản sao lưu đầy đủ hoặc đăng ký lại danh tính rồi để các thiết bị khác ghép đôi lại. Nếu tệp danh tính hỏng, không ghi đè trực tiếp bản sao lưu vẫn có thể khôi phục.

### Điện thoại và máy tính bảng

Khởi động lại ứng dụng, dọn bộ nhớ đệm hoặc đổi máy chủ không thay đổi danh tính ghép đôi. Đổi điện thoại cần ghép đôi lại; Android, HarmonyOS cũng cần ghép đôi lại sau khi xóa dữ liệu ứng dụng hoặc cài đặt lại. Khóa công khai trong thông tin ghép đôi không phải bản sao lưu có thể khôi phục khóa riêng.

## Cần kiểm tra gì trước khi kết nối thất bại

- **Thiết bị ngoại tuyến:** Xác nhận máy được điều khiển đang chạy, không ngủ, hai bên dùng cùng máy chủ.
- **Sai mật khẩu:** Đối chiếu mật khẩu của chế độ hiện tại, nhập lại thay vì tiếp tục dùng mật khẩu cũ đã lưu.
- **Xác minh thiết bị đáng tin cậy thất bại:** Đối chiếu lại fingerprint và danh sách, xác nhận ứng dụng hỗ trợ ghép đôi.
- **Thử quá thường xuyên:** Dừng thử liên tục, chờ theo thông báo rồi dùng mật khẩu đúng. Thời gian chờ sau khi IP trực tiếp kích hoạt giới hạn là 15 phút.
- **Kết nối được máy chủ nhưng không kết nối được máy tính:** Chế độ ID có thể thử buộc dùng relay; chế độ IP kiểm tra địa chỉ, cổng thực tế và tường lửa hai bên.

Xem thêm cách khắc phục trong [câu hỏi thường gặp](FAQ.md).
