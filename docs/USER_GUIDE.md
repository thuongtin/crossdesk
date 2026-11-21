# Hướng dẫn sử dụng CrossDesk

[Về trang chủ dự án](../README.md) · [English](USER_GUIDE_EN.md)

Lần đầu sử dụng, hãy xem [tải xuống và cài đặt](../README.md#download) trước. Tài liệu này giới thiệu ứng dụng theo trình tự cài đặt, kết nối và thao tác hằng ngày; một số tính năng mới có thể chưa nằm trong gói đã phát hành, hãy xem thêm ghi chú phiên bản tương ứng.

[Cài đặt và quyền](#installation) · [Kết nối nhanh](#quick-start) · [Kết nối IP trực tiếp](#lan-direct) · [Thiết bị đáng tin cậy](#trusted-devices) · [Bức tường màn hình (DeskHub)](#deskhub) · [Thao tác phiên](#session) · [Cài đặt và cập nhật](#settings) · [iOS](#ios) · [Android](#android) · [HarmonyOS](#harmonyos) · [Windows không giám sát](#windows-service) · [Tự lưu trữ (self-host)](#self-hosting)

<a id="installation"></a>

## Thông tin cài đặt bổ sung và quyền hệ thống

Linux cung cấp hai gói có thể cài độc lập cho mỗi kiến trúc; **chọn một trong hai**:

| Gói cài đặt | Tình huống phù hợp |
| --- | --- |
| `crossdesk-linux-<arch>-<version>.deb` (khuyên dùng) | Máy tính đã có môi trường màn hình; không cài thêm Xfce hoặc Xvfb |
| `crossdesk-virtual-desktop-xfce-linux-<arch>-<version>.deb` | Máy chủ cần tạo màn hình độc lập; APT cài thêm **Xfce, Xvfb và công cụ phiên D-Bus** |

Ví dụ cài bản thường (thay tên tệp bằng tên thực tế đã tải, với arm64 chọn kiến trúc tương ứng):

```bash
sudo apt install "./crossdesk-linux-amd64-<version>.deb"
```

Nếu cần bản màn hình ảo, thay bằng cách cài:

```bash
sudo apt install "./crossdesk-virtual-desktop-xfce-linux-amd64-<version>.deb"
```

Hai gói chứa cùng một chương trình đầy đủ, thay thế lẫn nhau và giữ cấu hình người dùng; bản thường cũng tạo được màn hình ảo sau khi bạn tự bổ sung phụ thuộc. Cài bản màn hình ảo không tự khởi động phiên hay đổi màn hình mặc định; xem cách khởi động trong [chạy Linux không màn hình](HEADLESS.md). Nút tải xuống khi cập nhật ứng dụng chọn đúng loại gói đã cài. Nâng cấp phiên bản hiện có không tự xóa Xfce đã cài trước đó.

**Lần chạy đầu trên macOS:** Theo lời nhắc của ứng dụng, cấp cho CrossDesk quyền **Ghi màn hình** (hệ thống mới có thể hiển thị là “Ghi màn hình và âm thanh hệ thống”) và **Trợ năng** trong “Cài đặt hệ thống → Quyền riêng tư & Bảo mật”, rồi mở lại ứng dụng theo lời nhắc hệ thống. Quyền thứ nhất dùng để ghi hình màn hình, quyền thứ hai dùng cho nhập liệu bàn phím và chuột từ xa.

<a id="quick-start"></a>

## Kết nối nhanh

### Kết nối từ máy tính khác bằng ID thiết bị

1. **Chuẩn bị máy tính được điều khiển.** Cài đặt và chạy CrossDesk, chuyển sang chế độ kết nối ID, chờ phía dưới hiển thị “Đã kết nối máy chủ”. Xem **ID máy này** và **Mật khẩu** trong “Màn hình máy này” bên trái rồi cung cấp cho máy điều khiển.
2. **Nhập ID máy từ xa.** Tại “Máy tính từ xa → ID máy từ xa” bên phải máy điều khiển, nhập **ID gồm 9 chữ số** của máy được điều khiển, nhấn **Kết nối**.
3. **Hoàn tất xác minh mật khẩu.** Nhập mật khẩu kết nối hiện tại của máy được điều khiển theo hộp thoại, xác nhận rồi chờ hình ảnh từ xa xuất hiện. Khi cần lưu mật khẩu, chọn “Nhớ mật khẩu”.
4. **Kết nối lại.** Thiết bị đã kết nối thành công xuất hiện trong “Kết nối gần đây”, bạn có thể kết nối lại qua thẻ. Nhấn menu “…” ở góc trên bên phải thẻ để sửa tên gợi nhớ, đổi nhóm hoặc xóa lịch sử; nút góc dưới bên phải dùng để kết nối / ngắt kết nối; bên phải tiêu đề có thể lọc thiết bị theo nhóm.

Nút mắt bên cạnh mật khẩu máy này dùng để hiện / ẩn mật khẩu, nút làm mới tạo mật khẩu ngẫu nhiên mới, nút bút chì cho phép đổi thủ công. Mật khẩu thống nhất là **6 chữ số hoặc chữ cái tiếng Anh**. Khi đổi hoặc làm mới, cần giữ kết nối máy chủ; chờ đổi thành công và ứng dụng kết nối lại rồi mới sao chép mật khẩu hiện tại. Máy điều khiển đã lưu mật khẩu cũ cần nhập lại.

<a id="lan-direct"></a>

### Kết nối IP trực tiếp qua mạng LAN

Lối vào IP trực tiếp hiện dành cho ứng dụng máy tính thông thường trên Windows, macOS, Linux. Địa chỉ IPv4 hoặc IPv6 của hai máy tính cần truy cập được nhau; có hay không có Internet đều được, không cần chờ kết nối máy chủ báo hiệu. Trình duyệt, thiết bị di động, dịch vụ Windows trước đăng nhập và bảng điều khiển Linux không màn hình hiện vẫn dùng quy trình kết nối bằng ID thiết bị.

1. **Bật và chuyển chế độ.** Trên cả hai bên, bật **kết nối IP trực tiếp** trong **Cài đặt → Chung → Kết nối mạng** rồi lưu (mặc định bật), sau đó dùng nút góc trên bên trái cửa sổ chính để chuyển sang **Chế độ kết nối IP**.
2. **Lấy địa chỉ bên kia.** Máy được điều khiển xem địa chỉ trong “IP / cổng máy này”; nút sao chép sẽ sao chép đầy đủ `IP:cổng`. Khi có nhiều địa chỉ, chọn địa chỉ mà máy điều khiển truy cập được.
3. **Kết nối và xác minh.** Máy điều khiển nhập ví dụ `192.168.1.20:52888` hoặc `[2001:db8::20]:52888` trong “IP / cổng từ xa”, nhấn “Kết nối”, rồi nhập mật khẩu sáu ký tự của máy được điều khiển trong chế độ này. Nếu bỏ cổng, dùng **52888**; hỗ trợ địa chỉ số IPv4 và IPv6, không hỗ trợ tên máy.
4. **Lưu và kết nối lại.** Bạn có thể chọn “Nhớ mật khẩu” trong hộp thoại mật khẩu. Kết nối gần đây hiển thị riêng theo chế độ ID / IP; không chọn vẫn lưu lịch sử thiết bị, nhưng lần sau cần nhập lại mật khẩu. Bạn có thể kết nối lại, sửa tên gợi nhớ hoặc xóa lịch sử từ thẻ.

<p align="center">
  <img src="images/desktop-lan-password-zh.png" width="720" alt="Hộp thoại mật khẩu IP trực tiếp và tùy chọn nhớ mật khẩu, nền là thiết bị LAN minh họa" />
</p>

**Địa chỉ và cổng:** Nhấn “Đổi IP” để chọn địa chỉ hiển thị và chia sẻ, nhấn “Đổi cổng” để sửa cổng của IP đó. Mặc định mọi giao diện dùng chung một cổng; sau khi tắt “Lắng nghe trên tất cả giao diện IP”, chương trình đồng thời lắng nghe IPv4 và IPv6 trên các card mạng vật lý khả dụng, lưu cổng riêng cho từng IP. Đổi địa chỉ hiển thị không vô hiệu hóa địa chỉ khác. Khi cấp cổng lần đầu cho một IP, ưu tiên dùng **52888**; nếu đã bị chiếm, tự cấp và lưu cổng khả dụng. Nếu cổng đã cấu hình bị xung đột, chương trình thông báo và bạn cần đổi thủ công. Phạm vi cổng là **1–65535**; đổi thất bại giữ cài đặt cũ; bên kết nối cần dùng cổng thực tế hiển thị trên giao diện. Tường lửa LAN cần cho phép kết nối thương lượng TCP và lưu lượng điều khiển từ xa UDP của CrossDesk.

**Lắng nghe tất cả giao diện:** Mặc định bật, có thể truy cập qua địa chỉ mạng dây, Wi-Fi, VPN hoặc card mạng ảo, dùng chung một cổng. Đổi cổng của bất kỳ địa chỉ nào cũng đổi cổng dùng chung; tắt tùy chọn sẽ khôi phục cổng riêng cho IP từng card mạng vật lý. Sau khi thay đổi cần lưu. Hãy chia sẻ địa chỉ thực tế trong giao diện, không nhập `0.0.0.0` hay `::`. Hai đầu VPN vẫn cần cho phép TCP và UDP liên lạc; không hỗ trợ kết nối địa chỉ loopback của máy này và không tự xuyên qua bộ định tuyến Internet.

**Địa chỉ IPv6:** Khi không kèm cổng, có thể nhập `2001:db8::20`; khi chỉ định cổng, dùng `[2001:db8::20]:52888`. Địa chỉ link-local còn cần phạm vi của card mạng máy điều khiển, ví dụ `[fe80::1234%14]:52888` hoặc `%en0` nếu hệ thống hỗ trợ. Chỉ số phạm vi sao chép từ máy được điều khiển có thể khác; khi kết nối cần thay bằng số hoặc tên card mạng mà máy điều khiển dùng để truy cập mạng đó.

**Mật khẩu:** IP trực tiếp dùng mật khẩu riêng gồm **6 chữ cái tiếng Anh hoặc chữ số, phân biệt hoa thường**. Nếu chưa đặt sẽ tự tạo; nút mắt, làm mới và bút chì lần lượt dùng để hiện / ẩn, tạo ngẫu nhiên và tùy chỉnh. Đổi thành công có hiệu lực ngay, không cần khởi động lại hay kết nối máy chủ. Máy điều khiển đã lưu mật khẩu cũ cần nhập lại.

**Chế độ và công tắc:** Chuyển chế độ được lưu và dùng tiếp ở lần khởi động sau. Chuyển lại chế độ ID chỉ đổi lối vào kết nối và lịch sử gần đây; lắng nghe IP vẫn chạy. Khi cần dừng IP trực tiếp, bỏ chọn “Bật kết nối IP” trong cài đặt rồi lưu: chương trình ngắt các phiên IP trực tiếp hiện có, giải phóng cổng lắng nghe, về chế độ ID và ẩn nút chế độ; các phiên ID hiện có tiếp tục chạy, mật khẩu IP, cổng và lịch sử gần đây được giữ. Bật lại sẽ dùng tiếp các cài đặt này.

### Kết nối từ trình duyệt

1. Giữ CrossDesk chạy trên máy tính được điều khiển và xác nhận đã kết nối máy chủ.
2. Mở [ứng dụng Web](https://web.crossdesk.cn/), nhập **ID thiết bị từ xa** và **Mật khẩu**, nhấn “Kết nối”.
3. Sau khi kết nối, dùng các điều khiển màn hình, chế độ chuột và bàn phím trên trang để thao tác máy từ xa; điện thoại và máy tính bảng cũng có thể truy cập qua trình duyệt.

Nếu dùng máy chủ tự triển khai, cần cho máy điều khiển và máy được điều khiển kết nối cùng dịch vụ; xem cấu hình triển khai Web trong [CrossDesk Web Client](https://github.com/kunkundi/crossdesk-web-client).

<p align="center">
  <img src="images/web-client.png" width="800" alt="Trang kết nối chính của ứng dụng Web" />
</p>

<a id="deskhub"></a>

## Bức tường màn hình (DeskHub): xem và điều khiển tập trung nhiều thiết bị

Nhấn **Bức tường màn hình (DeskHub)** ở góc trên bên trái cửa sổ chính để mở cửa sổ riêng; cửa sổ kết nối vẫn có thể dùng cho kết nối từ xa thông thường và cài đặt toàn cục. Bức tường màn hình lưu thành viên thiết bị, nhóm, thứ tự và bố cục cửa sổ; sau khi khởi động lại, cần kết nối thiết bị thủ công.

1. **Thêm thiết bị.** Nhấn **Thêm → Thêm bằng ID thiết bị**, nhập ID, mật khẩu và nhóm tùy chọn, nhấn “Thêm và kết nối”; chỉ lưu thông tin xác thực khi chọn “Nhớ mật khẩu”. Bạn cũng có thể chọn **Thêm từ kết nối gần đây**, chọn nhiều bản ghi hiện có rồi nhập vào; sau đó dùng “Chọn → Kết nối” để kết nối.
2. **Xem và sắp xếp.** Chuyển từ “Tất cả thiết bị” sang nhóm chỉ định; kéo đường phân cách lưới để điều chỉnh đồng loạt kích thước hình ảnh, kéo thiết bị lên thiết bị khác để đổi vị trí. Chế độ thu gọn ẩn tên và thanh công cụ quản lý; toàn màn hình mở rộng vùng xem; lật trang vẫn giữ kết nối.
3. **Phóng to và điều khiển.** Nhấn hình ảnh thiết bị để vào chế độ xem phóng to chỉ xem; hình thu nhỏ phía trên cho phép chuyển thiết bị, kéo thanh phân cách giữa để điều chỉnh vùng trên và dưới. Chỉ gửi bàn phím và chuột sau khi nhấn **Tiếp quản điều khiển**; nhấn “Nhả điều khiển” hoặc **Ctrl / ⌘ + Shift + Esc** để nhả. Chuyển thiết bị hoặc cửa sổ mất tiêu điểm cũng nhả điều khiển.
4. **Thao tác hàng loạt.** Nhấn **Chọn**, chọn thiết bị hoặc “Chọn tất cả” để kết nối, ngắt kết nối, hoặc vào “Thêm” để tắt / bật tiếng, phân phối tệp. “Chọn tất cả” bao phủ mọi trang của nhóm hiện tại; đổi nhóm xóa lựa chọn.

<p align="center">
  <img src="images/desktop-deskhub-focus-zh.png" width="960" alt="Chế độ phóng to Bức tường màn hình: hình thu nhỏ thiết bị phía trên, hình ảnh chính phía dưới, lối vào chỉ xem, bật điều khiển nhóm và tiếp quản điều khiển; hình ảnh minh họa" />
</p>

**Điều khiển nhóm:** Trong chế độ phóng to, nhấn “Bật điều khiển nhóm” để đồng bộ thao tác bàn phím và chuột của thiết bị hiện tại đến **tất cả thiết bị đã kết nối**, kể cả nhóm khác, trang khác và kết nối không hiển thị trong bức tường; phạm vi không bị giới hạn bởi lựa chọn hàng loạt. Các thiết bị nhận cùng tọa độ và phím; không điều chỉnh tỷ lệ riêng khi kích thước màn hình khác nhau. Nhả điều khiển đồng thời kết thúc điều khiển nhóm.

Kết nối Bức tường màn hình mặc định tắt tiếng; khi cần âm thanh, bật qua “Chọn → Thêm → Bật tiếng”. **Trạng thái mạng** phía dưới cho phép xem kết nối trực tiếp / chuyển tiếp, trạng thái mã hóa, tốc độ truyền nhận, độ trễ video, tỷ lệ mất gói, độ phân giải và tốc độ khung hình của tất cả thiết bị đã kết nối. Khi cần bộ công cụ đầy đủ cho một phiên như chuyển màn hình, màn hình riêng tư, bạn có thể nhấn thiết bị đã kết nối trong “Kết nối gần đây” của cửa sổ chính để mở cửa sổ phiên riêng.

**Đóng Bức tường màn hình sẽ ngắt toàn bộ kết nối thiết bị từ xa và đóng các cửa sổ phiên riêng dùng chung kết nối; thu nhỏ không ngắt kết nối.** Nhóm, trang và phạm vi lựa chọn không giới hạn phạm vi ngắt khi đóng. Xem thêm thao tác, chiến lược xem trước và tương thích phiên bản trong [tài liệu Bức tường màn hình (DeskHub)](deskhub.md).

<a id="session"></a>

## Thao tác thường dùng trong phiên

Cửa sổ từ xa chuyển thiết bị qua thẻ phiên; thanh điều khiển cung cấp các thao tác sau; khi thanh đang thu gọn, nhấn nút mở rộng trước. Di chuột lên biểu tượng để xem gợi ý.

| Lối vào | Công dụng |
| --- | --- |
| Màn hình | Chuyển màn hình từ xa |
| Bàn phím | Gửi tổ hợp phím; `Ctrl+Alt+Del` của Windows phụ thuộc dịch vụ từ xa |
| Chuột | Điều khiển / nhả chuột từ xa |
| Loa | Phát âm thanh / tắt tiếng |
| Thư mục | Mở cửa sổ duyệt tệp hai khung, duyệt thư mục máy này và máy từ xa, truyền tệp và thư mục hai chiều |
| Cài đặt hình ảnh | Điều chỉnh chất lượng hình ảnh, tốc độ khung hình ghi hình, độ phân giải từ xa được hỗ trợ và tùy chọn hình ảnh trong phiên hiện tại |
| Trạng thái mạng | Xem lưu lượng, tỷ lệ mất gói, tốc độ khung hình, độ phân giải, trực tiếp / chuyển tiếp, trạng thái mã hóa, bộ mã hóa máy được điều khiển và bộ giải mã máy điều khiển |
| Màn hình riêng tư | Bật hoặc tắt màn hình riêng tư của máy tính được điều khiển; di chuột để xem trạng thái hiện tại và thao tác khả dụng |
| Toàn màn hình | Vào / thoát toàn màn hình |
| Ngắt kết nối | Kết thúc phiên từ xa hiện tại |

Ứng dụng máy tính có thể đồng bộ bảng nhớ tạm văn bản. Bạn có thể đổi nơi nhận tệp trong “Cài đặt → Chung → Đường dẫn lưu tệp”. Đóng cửa sổ chính sẽ ẩn cửa sổ và giữ chương trình chạy; để thoát hoàn toàn, dùng mục thoát trong khay hệ thống hoặc thanh menu.

Ứng dụng máy tính Windows, macOS và Linux hỗ trợ sao chép dán tệp hai chiều; cả hai bên đều cần phiên bản hỗ trợ dán theo yêu cầu. Sao chép tệp trong trình quản lý tệp hệ thống rồi chuyển về cửa sổ điều khiển từ xa hiện tại chỉ đồng bộ danh sách tệp; chỉ bắt đầu truyền khi dán ở đầu kia (menu chuột phải hoặc phím tắt: Windows/Linux dùng `Ctrl+V`, macOS Finder dùng `⌘V`). Góc dưới bên phải bên nhận hiển thị tiến độ nhận lô tệp này; khi hoàn tất, thao tác dán tự tiếp tục, không cần dán lại. Bạn cũng có thể sao chép tệp ở máy từ xa rồi về máy này để dán, không cần mở cửa sổ truyền tệp.

Mỗi lần sao chép tối đa 128 tệp thông thường; với thư mục, hãy dùng truyền tệp hai khung. Truyền qua bảng nhớ tạm được xử lý như sao chép, không xóa tệp nguồn; bên nhận có thể hủy cả lô. Thông báo tiến độ tự thu gọn sau khi hoàn tất hoặc hủy; bên gửi không mở hộp tiến độ gửi tệp. Hãy giữ kết nối và CrossDesk trên cả hai bên chạy cho đến khi dán xong. Bộ nhớ đệm tải xuống được dọn khi thay bảng nhớ tạm, ngắt kết nối hoặc thoát; dán lại cùng lô tệp có thể dùng bộ nhớ đệm. Sao chép lại văn bản hoặc tệp làm danh sách cũ mất hiệu lực, lô cũ không ghi đè bảng nhớ tạm mới. Phiên nền và xem trước Bức tường màn hình không nhận các tệp được sao chép trên máy này.

Bên nhận Linux tương thích FUSE 2 và FUSE 3, dùng thư viện thời gian chạy và công cụ gắn kết hiện có của hệ thống để cung cấp tệp đọc theo yêu cầu. Khi chạy, thử FUSE 3 trước (`libfuse3.so.3` hoặc `libfuse3.so.4`), nếu không khả dụng thì thử FUSE 2 (`libfuse.so.2`). Người dùng hiện tại vẫn cần truy cập được `/dev/fuse` và gắn kết hệ thống tệp FUSE.

Gói Debian khai báo phụ thuộc chọn một trong hai: `fuse3 (>= 3.2) | fuse (>= 2.9)`. Khi cài qua APT hoặc nâng cấp trong ứng dụng, có sẵn bất kỳ phiên bản nào đều đáp ứng phụ thuộc, không bắt buộc thay bằng phiên bản kia; nếu chưa có cả hai, trình quản lý gói cài FUSE 3 và thư viện thời gian chạy của nó. Nâng cấp trong ứng dụng giữ biện pháp bảo vệ không cho tự xóa gói. Dùng trực tiếp `dpkg -i` không tải phụ thuộc còn thiếu; nên dùng `sudo apt install ./安装包.deb`.

Nếu môi trường triển khai thủ công thiếu FUSE, hoặc container hay quyền hệ thống ngăn gắn kết, bên nhận nhắc kiểm tra cài đặt FUSE và quyền gắn kết, bạn vẫn có thể dùng cửa sổ truyền tệp; điều khiển từ xa, bảng nhớ tạm văn bản và sao chép tệp đến thiết bị khác không chịu giới hạn này.

Trên macOS, dùng Finder để dán vào thư mục cục bộ; tệp trùng tên được giữ lại và tệp mới được lưu bằng tên có số thứ tự.

Thanh tác vụ Windows hiển thị cửa sổ chính và cửa sổ điều khiển từ xa thành hai biểu tượng riêng, cho phép chuyển và thu nhỏ riêng. Sau khi đóng cửa sổ chính, cửa sổ điều khiển từ xa và kết nối vẫn được giữ; khi cần cửa sổ chính, mở lại từ khay hệ thống. Biểu tượng thanh tác vụ của cửa sổ điều khiển từ xa chỉ xuất hiện khi có cửa sổ phiên; để ghim ứng dụng, hãy dùng biểu tượng cửa sổ chính.

Trên Linux, cửa sổ chính và cửa sổ điều khiển từ xa dùng mã nhóm ứng dụng khác nhau, hỗ trợ X11, XWayland và Wayland gốc; cách hiển thị thực tế trên thanh tác vụ do môi trường màn hình và cài đặt nhóm quyết định. Đóng cửa sổ chính không ảnh hưởng kết nối từ xa; bạn có thể mở lại từ khay hệ thống. Để ghim ứng dụng, hãy dùng trình khởi chạy cửa sổ chính.

macOS giữ một biểu tượng ứng dụng CrossDesk trên Dock; cửa sổ chính và cửa sổ điều khiển từ xa có thể chuyển và thu nhỏ độc lập, không tự gộp thành thẻ cửa sổ hệ thống. Sau khi đóng cửa sổ chính, miễn còn cửa sổ hiển thị hoặc đã thu nhỏ, biểu tượng Dock vẫn được giữ và kết nối từ xa tiếp tục chạy; khi mọi cửa sổ đóng, ứng dụng ở lại thanh menu. Bạn có thể mở lại cửa sổ chính từ thanh menu.

“Bộ mã hóa máy được điều khiển” và “Bộ giải mã máy điều khiển” trong trạng thái mạng hiển thị cách xử lý video thực tế đang dùng, giúp xác định có dùng tăng tốc phần cứng hay không. Thiết bị cũ không hỗ trợ hoặc chưa nhận dữ liệu sẽ hiển thị “—”. Khi báo cáo hình ảnh giật, bạn có thể đính kèm thông tin này.

### Duyệt thư mục và truyền tệp

Nhấn nút thư mục trên thanh công cụ phiên để xem đồng thời danh sách tệp máy này và máy từ xa. Bạn có thể nhập đường dẫn, về thư mục cha, lọc theo tên, chọn nhiều tệp hoặc thư mục rồi nhấn **Gửi đến máy từ xa** / **Gửi đến máy này**, hoặc kéo thả giữa hai khung. Thư mục đích cần tồn tại; tệp trùng tên được lưu riêng theo quy tắc đặt tên khi xung đột; duyệt thư mục từ xa cần máy được điều khiển tương thích.

Danh sách tác vụ phía dưới hiển thị trạng thái và tiến độ truyền, hỗ trợ hủy tác vụ chưa hoàn tất và xóa lịch sử đã hoàn tất. Đường dẫn, tên tệp và trạng thái hoàn tất trong hình đều là dữ liệu minh họa.

<p align="center">
  <img src="images/desktop-files-zh.png" width="960" alt="Cửa sổ truyền tệp: danh sách tệp máy này và máy từ xa, hướng gửi và trạng thái tác vụ; dữ liệu minh họa" />
</p>

### Điều chỉnh hình ảnh trong phiên

Sau khi kết nối thành công, nhấn **Cài đặt hình ảnh** (biểu tượng thanh trượt) trên thanh công cụ để chọn trực tiếp chất lượng thấp / trung bình / cao, **30 / 60 fps**, cùng ưu tiên độ mượt / ưu tiên chất lượng / cân bằng. Tùy chọn được gửi đến máy được điều khiển hiện tại, không cần ngắt rồi kết nối lại; hình ảnh thực tế phụ thuộc mạng, thiết bị và khả năng bộ mã hóa. Nếu máy được điều khiển cũ không hỗ trợ, menu thông báo và vô hiệu hóa tùy chọn.

Khi máy được điều khiển dùng Windows, macOS hoặc Linux X11, bạn còn có thể chọn độ phân giải màn hình nền được màn hình hiện tại hỗ trợ trong **Cài đặt hình ảnh → Độ phân giải**. Điều chỉnh không cần kết nối lại; khi máy điều khiển cuối cùng ngắt kết nối, chế độ hiển thị trước lần điều chỉnh đầu tiên trong thời gian kết nối được khôi phục. Linux cần ghi hình X11 và máy chủ hiển thị hỗ trợ RandR; Wayland, XWayland và cấu hình hiển thị dùng thu phóng hoặc di chuyển khung nhìn tùy chỉnh hiện chưa được hỗ trợ. Độ phân giải khả dụng do màn hình hoặc máy chủ hiển thị ảo cung cấp; màn hình ảo chỉ có một chế độ sẽ không được tạo thêm chế độ.

<p align="center">
  <img src="images/desktop-stream-video-settings-zh.png" width="800" alt="Menu cài đặt hình ảnh của phiên từ xa hiện tại: độ phân giải, chất lượng hình ảnh, tốc độ khung hình ghi hình và tùy chọn hình ảnh; nền là ảnh minh họa trang công khai" />
</p>

Ứng dụng máy tính và di động gốc ghi nhớ độ phân giải đã được xác nhận thành công theo thiết bị từ xa và màn hình. Khi kết nối lại, nếu màn hình đó vẫn hỗ trợ chế độ đã lưu, ứng dụng yêu cầu áp dụng lại; hành vi khôi phục chế độ hiển thị ban đầu của máy được điều khiển khi máy điều khiển cuối cùng ngắt kết nối không thay đổi.

<a id="settings"></a>

## Cài đặt và cập nhật ứng dụng

Nhấn **☰ → Cài đặt** ở góc trên bên phải cửa sổ chính, chuyển **Chung / Bức tường màn hình (DeskHub)** ở phía trên, sau khi sửa nhấn **Lưu**. Chuyển thẻ vẫn giữ thay đổi chưa lưu; nhấn “Hủy” để bỏ thay đổi. “Chung” sắp xếp tùy chọn theo các mục giao diện, video, kết nối mạng; bạn có thể cuộn xuống để xem cài đặt còn lại. Nên điều chỉnh tùy chọn mã hóa và kết nối trước khi thiết lập kết nối; một số tùy chọn không thể sửa khi phiên đang chạy. Chất lượng hình ảnh, tốc độ khung hình và tùy chọn hình ảnh của phiên thông thường được điều chỉnh trên thanh công cụ phiên từ xa.

<p align="center">
  <img src="images/desktop-settings-zh.png" width="720" alt="Cài đặt chung mới: thẻ Chung và Bức tường màn hình, ngôn ngữ, mã hóa video và cài đặt kết nối mạng" />
</p>

| Cài đặt | Giải thích |
| --- | --- |
| Ngôn ngữ | Mặc định theo hệ thống, cũng có thể chọn thủ công tiếng Trung giản thể, tiếng Trung phồn thể, tiếng Anh, tiếng Nga, tiếng Nhật, tiếng Hàn, tiếng Đức, tiếng Pháp, tiếng Tây Ban Nha hoặc tiếng Bồ Đào Nha Brazil; xem [tài liệu bản địa hóa](../localization/README.md) |
| Định dạng mã hóa video | H.264 / AV1; H.264 tự ưu tiên mã hóa và giải mã phần cứng, chuyển về phần mềm nếu không khả dụng hoặc khởi tạo thất bại; AV1 trên máy tính dùng mã hóa và giải mã phần mềm |
| Bật kết nối IP | Mặc định bật; lưu sẽ bật/tắt IP trực tiếp ngay. Tắt sẽ ngắt phiên IP, dừng lắng nghe và ẩn nút chế độ cửa sổ chính, giữ trạng thái sau khi khởi động lại |
| Buộc kết nối chuyển tiếp | Khi tắt, tự chọn phương thức kết nối, cho phép P2P và chuyển sang chuyển tiếp TURN khi cần; khi bật, chỉ kết nối qua chuyển tiếp |
| Lưu ảnh xem trước màn hình từ xa | Mặc định bật, lưu ảnh xem trước màn hình từ xa; khi tắt, vùng hình thu nhỏ hiển thị nền tảng máy được điều khiển và xóa ảnh xem trước hiện có, giữ lịch sử kết nối và mật khẩu |
| Bật màn hình riêng tư khi có kết nối đến | Thiết lập có tự bật màn hình riêng tư khi máy này được kết nối từ xa hay không; trong phiên có thể chuyển qua thanh công cụ máy điều khiển |
| Cấu hình máy chủ tự lưu trữ (self-host) | Nhấn “Chỉnh sửa” để nhập địa chỉ máy chủ và cổng dịch vụ báo hiệu, bật bằng công tắc bên phải |
| Tự khởi động cùng hệ thống | Bản Release đã cài đặt trên Windows: cung cấp điều khiển từ xa trước đăng nhập bằng ID, mật khẩu hiện có; sau khi người dùng đăng nhập, GUI tự khởi động và kết nối dịch vụ nền để khôi phục đầy đủ tính năng. Bản Windows di động không cần cài đặt và Debug không hỗ trợ công tắc này. Trên nền tảng máy tính khác, lưu sẽ thiết lập tự khởi động khi đăng nhập màn hình |
| Bật tiến trình nền | Ở chế độ máy tính Windows / macOS / Linux, lưu sẽ bật hoặc tắt ngay tự khởi động lại sau khi thoát bất thường, không cần khởi động lại chương trình; thoát bình thường hoặc đăng xuất sẽ không khởi chạy lại, chế độ bảng điều khiển không màn hình không bật tiến trình giám sát tích hợp |
| Đường dẫn lưu tệp | Chọn thư mục nhận tệp |

Công tắc IP trực tiếp trong nhóm kết nối mạng độc lập với cài đặt chuyển tiếp dùng cho kết nối ID:

<p align="center">
  <img src="images/desktop-settings-network-zh.png" width="720" alt="Nhóm kết nối mạng trong cài đặt chung: công tắc bật IP trực tiếp và giải thích hành vi sau khi tắt" />
</p>

<details>
<summary>Nâng cao: điều chỉnh thủ công băng thông khởi đầu kết nối</summary>

Thông thường không cần sửa các giá trị này. Khi hình ảnh giật, hãy thử cài đặt chất lượng hình ảnh và tốc độ khung hình trong phiên trước.

Sau khi thoát ứng dụng, sửa phần `[Settings]` của `config.ini` trong thư mục bộ nhớ đệm; có hiệu lực sau khi khởi động lại:

```ini
[Settings]
direct_start_bitrate_bps = 2500000
relay_start_bitrate_bps = 1500000
disable_receiver_padding_probing = true
```

Cả hai giá trị đều có đơn vị bps, lần lượt điều khiển ước tính khởi đầu của cơ chế kiểm soát tắc nghẽn ở bên gửi nội dung đa phương tiện gốc trên máy này cho đường P2P / IP trực tiếp và chuyển tiếp TURN. Giá trị mặc định trên máy tính là 2.5 Mbps và 1.5 Mbps, phạm vi cho phép là 300000–20000000; giá trị thiếu hoặc không hợp lệ dùng mặc định tương ứng và được ghi bổ sung vào tệp cấu hình khi khởi động. Sau kết nối vẫn điều chỉnh động theo phản hồi mạng, ngân sách băng thông Bức tường màn hình tiếp tục có hiệu lực. Khi xem hình ảnh từ xa, nên sửa cấu hình máy được điều khiển đang gửi hình ảnh. Kết nối trình duyệt dùng cơ chế điều khiển băng thông riêng, không chịu ảnh hưởng của hai tùy chọn này.

Trên macOS / Linux, tệp này mặc định ở `~/.cache/CrossDesk/` (khi đặt `XDG_CACHE_HOME`, dùng `CrossDesk/` trong thư mục đó); bản Release đã cài đặt trên Windows ở `%LOCALAPPDATA%\CrossDesk\cache\`, Windows Debug dùng `cache/` trong thư mục làm việc, bản di động không cần cài đặt dùng `data/` bên cạnh tệp thực thi.

Khi Windows đã cấu hình máy chủ nền, cấu hình INI lưu trong `host_config` của `%ProgramData%\CrossDeskUnattended\shared-identity.json` được ưu tiên. Quản trị viên cần sửa các mục này khi cả GUI và dịch vụ nền đều dừng, rồi khởi động lại dịch vụ và GUI; chỉ sửa bộ nhớ đệm người dùng hoặc `host/config.ini` sẽ bị cấu hình dùng chung ghi đè khi khởi động.

`disable_receiver_padding_probing` trên máy tính mặc định là `true`, cấm kết nối chưa từng gửi video cục bộ gửi gói thăm dò đệm; đặt `false` có thể khôi phục thăm dò đệm phía nhận. Sau khi bắt đầu gửi video cục bộ, vẫn giữ thăm dò bình thường và khôi phục băng thông khi hình ảnh tĩnh. Giá trị thiếu hoặc không hợp lệ dùng mặc định; cấu hình tường minh hiện có giữ nguyên. Tùy chọn áp dụng cho kết nối trực tiếp và chuyển tiếp gốc, vị trí cấu hình và yêu cầu khởi động lại như trên; cấu hình dùng chung máy chủ nền Windows cũng gồm mục này.

</details>

### Truyền hình ảnh Bức tường màn hình (DeskHub)

Cấu hình thống nhất thiết bị trong bức tường tại **Cài đặt → Bức tường màn hình (DeskHub)**; lưu sẽ áp dụng ngay cho phiên Bức tường màn hình hiện tại và dùng cho các kết nối sau; kết nối thông thường không bị ảnh hưởng.

| Cài đặt | Giải thích |
| --- | --- |
| Giới hạn độ phân giải | Tự động, 320 × 180, 640 × 360, 1280 × 720, 1920 × 1080, 2560 × 1440, 3840 × 2160 hoặc độ phân giải gốc; chế độ tự động điều chỉnh theo lưới / chế độ phóng to, giới hạn cố định giữ tỷ lệ và không phóng lớn hình ảnh nguồn |
| Tốc độ khung hình | Theo cài đặt thiết bị, hoặc chỉ định 15 / 30 / 60 FPS |
| Tổng băng thông video | Mặc định 20 Mbps, có thể chọn 2 / 5 / 10 / 20 / 50 / 100 Mbps; các phiên Bức tường màn hình hỗ trợ khả năng này dùng chung, ưu tiên cấp cho hình ảnh phóng to, không gồm âm thanh, truyền tệp và kết nối thông thường, cũng không phải giới hạn cứng của lưu lượng tức thời |

<p align="center">
  <img src="images/desktop-settings-deskhub-zh.png" width="720" alt="Cài đặt truyền hình ảnh Bức tường màn hình: độ phân giải tự động, tốc độ khung hình theo thiết bị và tổng băng thông video 20 Mbps" />
</p>

Thiết bị hỗ trợ đăng ký xem trước có thể tạm dừng truyền video khi không hiển thị, khôi phục khi hiển thị lại; máy được điều khiển cũ quay về cách xử lý theo khả năng, không bảo đảm mọi tùy chọn độ phân giải, tốc độ khung hình hay băng thông đều có hiệu lực.

### Kiểm tra cập nhật

Ứng dụng kiểm tra phiên bản sau khi khởi động, hiển thị thông báo khi có phiên bản mới. Bạn cũng có thể mở **☰ → Giới thiệu → Kiểm tra cập nhật** để kiểm tra thủ công, nhấn **Nhật ký cập nhật** để xem ghi chú. Chọn cài bản cập nhật hoặc tải xuống theo hướng dẫn giao diện; lưu công việc và chờ truyền tệp hoàn tất trước khi cài. Xem các bước cụ thể và xử lý lỗi trong [hướng dẫn cập nhật](UPDATER.md).

<p align="center">
  <img src="images/desktop-about-zh.png" width="720" alt="Cửa sổ giới thiệu và nút kiểm tra cập nhật, development là phiên bản minh họa, chưa thực hiện kiểm tra phiên bản trực tuyến" />
</p>

<a id="feedback"></a>

### Phản hồi và nhật ký chẩn đoán

Mở **☰ → Phản hồi và góp ý** trong cửa sổ chính, nhấn **Gửi phản hồi trên GitHub** để gửi sự cố hoặc góp ý; nhấn **Xuất nhật ký chẩn đoán** để lưu nhật ký CrossDesk, MiniRTC và thông tin phiên bản cơ bản thành tệp ZIP. Sau khi xuất thành công, giao diện hiển thị vị trí lưu; bạn có thể đính kèm gói nhật ký vào báo cáo, nêu hệ điều hành hai bên, phiên bản, phương thức kết nối và các bước tái hiện.

<p align="center">
  <img src="images/desktop-feedback-zh.png" width="720" alt="Phản hồi và góp ý: lối vào phản hồi GitHub và nút xuất nhật ký chẩn đoán" />
</p>

<a id="trusted-devices"></a>

## Thiết bị đáng tin cậy và ghép đôi

Nếu máy tính bật “Chỉ cho phép thiết bị đáng tin cậy điều khiển”, hãy sao chép thông tin ghép đôi trên máy điều khiển trước, rồi thêm vào danh sách tin cậy của máy tính và đối chiếu dấu vân tay. Lối vào trên máy tính là **Cài đặt → Chung → Bảo vệ quyền riêng tư → Quản lý thiết bị đáng tin cậy**; trên iOS, Android, HarmonyOS di động là **Cài đặt → Thông tin ghép đôi máy này**. Sau khi ghép đôi vẫn cần xác minh mật khẩu kết nối.

Trước khi bật giới hạn, xác nhận máy điều khiển định dùng hỗ trợ ghép đôi; CLI terminal hiện chưa hỗ trợ. Xem đầy đủ các bước và thông tin cài lại, di chuyển trong [hướng dẫn thiết bị đáng tin cậy](CONNECTIONS.md#trusted-devices).

<a id="ios"></a>

## Ứng dụng iOS / iPadOS gốc

Ứng dụng iPhone và iPad dùng để điều khiển máy tính từ xa, hiện không thể chia sẻ điện thoại hay máy tính bảng làm màn hình được điều khiển.

1. Cài từ [App Store](https://apps.apple.com/app/id6828859211), yêu cầu hệ thống cụ thể theo trang cửa hàng; bạn cũng có thể dùng Xcode biên dịch và ký theo [hướng dẫn sử dụng và biên dịch iOS](../apps/ios/README.md), mã nguồn hiện nhắm tới iOS 16+.
2. Nhập ID máy từ xa ở trang chủ, nhấn kết nối và nhập mật khẩu; sau khi kết nối thành công có thể kết nối lại từ “Kết nối gần đây”.
3. Chọn chế độ chuột trong cài đặt: **Vị trí tương đối** di chuyển con trỏ như bàn di chuột, **Vị trí tuyệt đối** ánh xạ vị trí chạm trực tiếp lên hình ảnh từ xa.
4. Nhấn biểu tượng nổi để mở menu; có thể chuyển màn hình, hiện bàn phím, điều khiển âm thanh hoặc gửi tệp. Tệp nhận nằm trong `Documents/Received` của ứng dụng, có thể xuất bằng nút chia sẻ cạnh trạng thái truyền.

Hỗ trợ nhận văn bản từ xa vào bảng nhớ tạm iOS; nhấn **Bảng nhớ tạm** trong menu nổi để gửi văn bản máy này đến máy từ xa. Giới hạn văn bản mỗi lần là **128 KiB**; khi văn bản rỗng hoặc quá giới hạn sẽ hiển thị thông báo.

Bạn có thể chụm ngón tay để thu phóng hình ảnh, dùng hai ngón tay di chuyển khung nhìn sau khi phóng to. Cử chỉ của ứng dụng gốc và trình duyệt có thể khác nhau. Ứng dụng theo ngôn ngữ hệ thống hoặc ngôn ngữ ứng dụng đặt riêng; xem phạm vi hỗ trợ trong [tài liệu ngôn ngữ](../localization/README.md).

<a id="android"></a>

## Ứng dụng điều khiển Android gốc

Ứng dụng Android hỗ trợ điện thoại và máy tính bảng arm64 dùng Android 8.0+; có thể tải APK đã ký từ [trang web chính thức](https://www.crossdesk.cn/) hoặc [GitHub Releases](https://github.com/kunkundi/crossdesk/releases) để cài. Để tự biên dịch, xem [hướng dẫn sử dụng và biên dịch Android](../apps/android/README.md).

1. Chạy CrossDesk trên máy tính, nhập ID thiết bị của máy tính ở trang chủ điện thoại, nhập mật khẩu sau khi kết nối.
2. Chọn chế độ chuột tương đối (bàn di chuột) hoặc vị trí tuyệt đối trong cài đặt. Hỗ trợ nhấn giữ để nhấp chuột phải, cuộn hai ngón tay, chụm để thu phóng, bàn phím và chuột ngoài.
3. Mở menu nổi để chuyển màn hình, điều chỉnh chất lượng hình ảnh và tốc độ khung hình, dùng chuột ảo, gửi nhận tệp, xem trạng thái mạng và gửi văn bản bảng nhớ tạm. Tệp nhận có thể xuất qua mục lưu.
4. Khi chuyển ứng dụng, phiên hiện có ưu tiên duy trì qua hình trong hình của hệ thống; nếu không khả dụng, thử dùng dịch vụ tiền cảnh có thông báo. Đóng cửa sổ hình trong hình hoặc không khởi động được cơ chế duy trì nền sẽ kết thúc phiên.

Android hỗ trợ H.264 / AV1; chất lượng hình ảnh và độ mượt thực tế phụ thuộc khả năng thiết bị. Điện thoại hiện không thể làm máy được điều khiển. Bạn có thể bật/tắt màn hình riêng tư của máy tính ngay trong menu nổi; nút hiển thị trạng thái được máy tính xác nhận; khi bật sẽ chặn bàn phím và chuột cục bộ của máy tính.

<a id="harmonyos"></a>

## Hỗ trợ HarmonyOS

HarmonyOS PC, điện thoại và máy tính bảng đều có bản phát triển, cần biên dịch và ký rồi mới cài. PC và thiết bị di động dùng gói khác nhau, hãy chọn đúng loại thiết bị.

| Ứng dụng | Hệ thống và kiến trúc | Tính năng |
| --- | --- | --- |
| HarmonyOS PC (`2in1`) | API 26, arm64 | Có thể làm máy điều khiển và máy được điều khiển, cung cấp giao diện máy tính, quản lý phiên, Bức tường màn hình (DeskHub) và truyền tệp |
| Điện thoại / máy tính bảng HarmonyOS | API 20+, arm64 | Ứng dụng điều khiển di động gốc, hỗ trợ hình ảnh và âm thanh từ xa, cảm ứng và bàn phím, chuột ngoài, chuyển nhiều màn hình, bảng nhớ tạm văn bản và truyền tệp hai chiều |

Điện thoại và máy tính bảng dùng trình chọn tệp hệ thống để gửi và xuất tệp; PC có thể chọn thư mục nhận trong cài đặt.

Hiện là bản phát triển, cần dùng DevEco biên dịch và ký theo [hướng dẫn biên dịch HarmonyOS](../apps/harmony/README.md) rồi mới cài. Ghi màn hình và điều khiển nhập liệu PC cần quyền hệ thống tương ứng; việc nghiệm thu trên thiết bị thật và phiên dài hạn vẫn đang tiến hành; xem quyền cụ thể, khác biệt nền tảng và phạm vi kiểm chứng trong [tài liệu PC](../apps/harmony/desktop-port/README.md) và [tài liệu tính năng di động](../apps/harmony/MOBILE_PARITY.md).

<a id="windows-service"></a>

## Màn hình khóa và đăng nhập Windows

Windows tự chọn cách ghi hình màn hình khả dụng, không cần đặt thủ công. Khi không có màn hình, ứng dụng thử dùng màn hình ảo; nếu không khả dụng, độ phân giải hoặc tốc độ khung hình có thể bị giới hạn.

**CrossDesk Service** (tên dịch vụ `CrossDeskService`) cung cấp báo cáo trạng thái màn hình khóa, đăng nhập, nhập thông tin xác thực và màn hình bảo mật, gửi `Ctrl+Alt+Del` và chuyển tiếp đầu vào bàn phím, chuột.

Bản cài đặt mặc định đăng ký dịch vụ khởi động theo yêu cầu; ứng dụng thử khởi động dịch vụ khi chạy; nếu chưa bật chế độ không giám sát, dịch vụ tự thoát khi máy này không còn tiến trình ứng dụng CrossDesk. Bản di động không cần cài đặt có thể cài dịch vụ trong hộp thoại nhắc hoặc “Cài đặt → Dịch vụ điều khiển khi khóa màn hình”, cần quyền quản trị viên.

Sau khi bật **Tự khởi động cùng hệ thống** trong bản cài đặt Windows, bạn có thể kết nối trước đăng nhập bằng cùng ID và mật khẩu; sau đăng nhập giao diện tự mở, kết nối hiện có được giữ. Tắt công tắc này sẽ tắt khởi động không giám sát. **Bật tiến trình nền** dùng để khôi phục sau khi giao diện thoát bất thường, là một tùy chọn khác. Bản di động không cần cài đặt và bản Debug không cung cấp công tắc tự khởi động không giám sát.

Trước lần bật đầu tiên, chạy ứng dụng và kết nối mạng để lấy ID máy này. Bật sẽ giữ ID, mật khẩu và lịch sử ghép đôi hiện tại; sau đó có thể dùng thông tin kết nối cũ truy cập màn hình đăng nhập.

<a id="windows-service-development-update"></a>

### Cập nhật dịch vụ Windows khi tự triển khai

Chạy trực tiếp ứng dụng vừa biên dịch chỉ thử khởi động dịch vụ đã đăng ký, không sửa đường dẫn chương trình của dịch vụ và không khởi động lại dịch vụ cũ đang chạy. Nếu bàn phím máy tính dùng được nhưng bộ gõ hệ thống không dùng được cả trong Notepad và màn hình khóa, đồng thời nhật ký hiển thị `Secure desktop text injection failed: error=unknown_command`, hãy cập nhật chương trình dịch vụ và đường dẫn đăng ký trước.

Mở **PowerShell với quyền quản trị viên** trong thư mục biên dịch đầy đủ chứa bản mới của `CrossDesk.exe`, `crossdesk_service.exe` và `crossdesk_session_helper.exe`, thực hiện:

```powershell
$crossdeskExe = (Resolve-Path .\CrossDesk.exe).Path;
foreach ($serviceAction in '--service-stop', '--service-install', '--service-start') {
    $serviceProcess = Start-Process -FilePath $crossdeskExe -ArgumentList $serviceAction -Wait -PassThru;
    if ($serviceProcess.ExitCode -ne 0) {
        throw "$serviceAction failed: $($serviceProcess.ExitCode)"
    }
}
```

Trong thời gian khởi động lại dịch vụ, điều khiển từ xa có thể gián đoạn ngắn. Sau khi xong, kết nối lại và kiểm tra riêng nhập liệu trên màn hình thông thường và màn hình khóa. Dịch vụ sẽ dùng chương trình trong thư mục biên dịch này, hãy giữ thư mục đó; khi quay lại bản cài đặt, cài lại gói đầy đủ để khôi phục đăng ký dịch vụ tương ứng.

Nếu đã bật “Tự khởi động cùng hệ thống”, sau khi đổi thư mục biên dịch còn cần dùng ứng dụng mới lưu lại cài đặt này một lần, hoặc chạy `--unattended-enable` của bản mới sau khi ứng dụng thông thường thoát để cập nhật tác vụ khởi động GUI sau đăng nhập. Chỉ sửa dịch vụ không cập nhật đường dẫn chương trình của tác vụ này.

### Điều khiển từ xa sau khi khởi động nhưng chưa đăng nhập

Bật trong “Cài đặt → Tự khởi động cùng hệ thống” rồi lưu; ID máy này và mật khẩu trên giao diện chính dùng chung trước và sau đăng nhập, không cần nhớ bộ thông tin kết nối khác. Bạn cũng có thể mở **PowerShell với quyền quản trị viên** trong thư mục cài đặt đầy đủ để bật cùng tính năng; trước khi bật bằng CLI lần đầu, đóng ứng dụng thông thường, hoặc dùng ngay công tắc cài đặt trong ứng dụng đang chạy:

```powershell
.\CrossDesk.exe --unattended-enable
# Xem ID thiết bị, mật khẩu dùng chung, trạng thái online và bên đang quản lý mode
.\CrossDesk.exe --unattended-status
```

Dùng `device_id` và `password` trả về để kết nối. Bật lại giữ danh tính dùng chung; chứng chỉ gốc của máy chủ tự triển khai cần được cài vào kho tin cậy của máy tính cục bộ.

Sau khi bật, dịch vụ khởi động cùng Windows, không cần đặt Windows tự đăng nhập hay lưu mật khẩu đăng nhập hệ thống. Thoát giao diện CrossDesk không dừng điều khiển từ xa nền; đăng xuất, chuyển người dùng, khởi động lại dịch vụ hoặc gián đoạn mạng vẫn có thể gây ngắt kết nối.

Trước đăng nhập có thể dùng hình ảnh, bàn phím, chuột, nhiều màn hình và điều khiển màn hình bảo mật. Âm thanh, tệp, bảng nhớ tạm và màn hình riêng tư cần người dùng đăng nhập và chạy giao diện CrossDesk; sau khi thoát giao diện, các tính năng này tạm không khả dụng. Dữ liệu dùng chung nằm ở `%ProgramData%\CrossDeskUnattended`, chỉ quản trị viên và SYSTEM truy cập được; nhật ký nằm trong thư mục con `logs` tại đó.

Khi không có màn hình vật lý, ứng dụng thử tạo màn hình ảo, ưu tiên độ phân giải đã ghi nhận, nếu chưa có thì dùng 1920×1080. Kết quả thực tế phụ thuộc trình điều khiển có khả dụng hay không; sau khi kết nối màn hình vật lý ổn định, ứng dụng thử chuyển lại.

```powershell
# Dừng máy được điều khiển không giám sát và khôi phục khởi động dịch vụ theo yêu cầu
.\CrossDesk.exe --unattended-disable
```

Tắt “Tự khởi động cùng hệ thống” trong cài đặt tương đương tắt chế độ không giám sát; kết quả bật/tắt bằng CLI cũng phản ánh trên công tắc này. Tắt hoặc gỡ dịch vụ hủy khởi động không giám sát nhưng giữ cấu hình máy được bảo vệ; nếu cần xóa hoàn toàn thông tin xác thực, quản trị viên xóa thư mục đó sau khi dừng dịch vụ.

<details>
<summary>Triển khai thủ công và lệnh quản lý dịch vụ</summary>

Giữ thư mục cài đặt đầy đủ; khi tự triển khai sản phẩm biên dịch, đặt `CrossDesk.exe`, `crossdesk_service.exe`, `crossdesk_session_helper.exe` và toàn bộ `.dll` của cùng lần biên dịch vào cùng thư mục. Mở PowerShell với quyền quản trị viên tại đó, thực hiện theo nhu cầu:

```powershell
.\CrossDesk.exe --service-install
.\CrossDesk.exe --service-start
.\CrossDesk.exe --service-status
.\CrossDesk.exe --service-ping
# Dừng hoặc gỡ dịch vụ
.\CrossDesk.exe --service-stop
.\CrossDesk.exe --service-uninstall
```

</details>

Khi gặp “Dịch vụ Windows từ xa không khả dụng”, kiểm tra cài đặt và trạng thái dịch vụ trên máy tính được điều khiển; kết nối màn hình thông thường vẫn dùng được, khả năng điều khiển giao diện được bảo vệ sẽ bị giới hạn.

<a id="self-hosting"></a>

## Máy chủ tự lưu trữ (self-host)

Bạn có thể tự triển khai dịch vụ báo hiệu và chuyển tiếp TURN, rồi nhập thông tin kết nối trong **☰ → Cài đặt → Chung → Cấu hình máy chủ tự lưu trữ (self-host)** của ứng dụng máy tính. Máy điều khiển và máy được điều khiển đều cần bật cùng bộ cấu hình.

- [Hướng dẫn tự lưu trữ (self-host)](SELF_HOSTING.md): lối vào triển khai Compose hiện tại, cài đặt ứng dụng, tin cậy chứng chỉ và khắc phục sự cố.
- [CrossDesk Server](https://github.com/kunkundi/crossdesk-server): mã nguồn máy chủ và cấu hình phát hành.
- [CrossDesk Web Client](https://github.com/kunkundi/crossdesk-web-client): mã nguồn trình duyệt và tài liệu tự lưu trữ (self-host).

<p align="center">
  <img src="images/desktop-self-hosted-zh.png" width="720" alt="Cài đặt máy chủ tự lưu trữ (self-host)" />
</p>
