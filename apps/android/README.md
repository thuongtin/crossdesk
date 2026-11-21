# CrossDesk Android

[Tải và cài đặt](../../README.md#android) · [Hướng dẫn sử dụng](../../docs/USER_GUIDE.md#android) · [Biên dịch nguồn](#biên-dịch) · [Ký và phát hành](#ký-release)

Đây là ứng dụng điều khiển Android gốc, sử dụng giao diện Java, JNI và MiniRTC, cùng giao thức tín hiệu, ICE, SRTP và nhập liệu với bản máy tính; không sử dụng WebView. Ứng dụng hỗ trợ điện thoại và máy tính bảng arm64 chạy Android 8.0 (API 26) trở lên. Máy tính, Android, iOS và HarmonyOS dùng chung phiên bản phát hành ứng dụng, nhưng số bản dựng di động độc lập theo từng nền tảng. Chạy `python3 scripts/version.py show client` tại gốc kho để xem hoặc `set client <phiên-bản>` để sửa đồng bộ; xem [quản lý phiên bản thành phần](../../docs/VERSIONING.md).

## Cài đặt và kết nối

Bạn có thể tải APK đã ký từ [trang chủ](https://www.crossdesk.cn/) hoặc [GitHub Releases](https://github.com/kunkundi/crossdesk/releases). Sau khi mở ứng dụng và cấp quyền truy cập mạng, nhập ID 9 chữ số và mật khẩu của máy tính. Nếu tự tạo gói cài đặt, hãy làm theo các bước chuẩn bị bộ công cụ, biên dịch và ký bên dưới.

## Xác thực thiết bị đáng tin cậy

“Cài đặt → Thông tin ghép đôi máy này → Xem dấu vân tay” cho phép xem dấu vân tay của máy này và sao chép thông tin ghép đôi. Dán thông tin đó vào “Quản lý thiết bị đáng tin cậy → Thêm thiết bị đáng tin cậy” trên máy được điều khiển, đối chiếu dấu vân tay rồi thêm thiết bị. Sau đó bạn có thể kết nối máy đã bật “Chỉ cho phép thiết bị đáng tin cậy điều khiển”; mật khẩu kết nối vẫn cần được xác minh.

Ứng dụng di động dùng giao thức thách thức–đáp ứng Ed25519 chung. Khóa riêng được mã hóa bằng Android Keystore / AES-GCM và lưu bền vững trong vùng riêng của ứng dụng, không ghi vào bộ nhớ đệm, nhật ký hay thông tin ghép đôi. Khởi động lại, xóa bộ nhớ đệm hoặc đổi máy chủ/ID đăng nhập không làm đổi khóa công khai. Nếu khóa hiện có bị hỏng, không đọc được hoặc bị mất, quá trình khởi tạo sẽ dừng và báo lỗi, không tự thay khóa. Xóa dữ liệu hoặc gỡ rồi cài lại ứng dụng thì cần ghép đôi lại.

## Tính năng hiện tại

- Hỗ trợ máy chủ tín hiệu WSS chính thức hoặc tự triển khai. Sau khi được cấp quyền, ứng dụng tự đăng ký/đăng nhập danh tính máy này khi vào tiền cảnh; trang chủ hiển thị trực tiếp trạng thái kết nối, kết nối lại hoặc lỗi. Khi kết nối máy tính, ứng dụng dùng lại danh tính đó; đổi máy chủ sẽ đăng nhập lại, rút quyền sẽ đóng kết nối. Phiên từ xa đã có ưu tiên duy trì qua PiP hệ thống; khi không có PiP, dịch vụ tiền cảnh kèm thông báo kết nối được dùng làm dự phòng. Kết nối tín hiệu rảnh của trang chủ sẽ đóng khi vào nền.
- Ưu tiên giải mã H.264 / AV1 bằng phần cứng MediaCodec và gửi đệm phần cứng thẳng tới Surface. Nếu thiết bị thiếu khả năng phần cứng, vượt độ phân giải hoặc gặp lỗi khi chạy, ứng dụng chuyển về OpenH264 / dav1d với cùng định dạng mã hóa. Luồng hiển thị riêng chuyển đầu ra NV12 phần mềm sang RGBA và chỉ giữ khung chờ mới nhất. Hàng đợi giải mã khung nén chung trước đây của MiniRTC không thay đổi.
- Phát PCM đơn kênh 48 kHz, hỗ trợ tắt tiếng và giới hạn dung lượng hàng đợi âm thanh.
- Hỗ trợ chuột tương đối (bàn di chuột), định vị tuyệt đối, nhấn một ngón, nhấn giữ để kéo ở chế độ tuyệt đối, nhấn giữ để nhấp phải và bàn phím/chuột ngoài. Cử chỉ giống iOS: chụm hai ngón trên toàn vùng chạm, kể cả viền đen ngoài ảnh, để thu phóng quanh điểm chạm (1–10 lần); sau khi phóng, kéo hai ngón để di chuyển ảnh; ở tỷ lệ gốc, vuốt dọc hai ngón điều khiển con lăn. Khi chuột tương đối chạm mép trên/dưới của phần ảnh đang thấy rồi tiếp tục di ra ngoài, ảnh di chuyển dọc theo hướng ngược lại, không di ngang. Cơ chế này cũng hoạt động khi chưa phóng và khi giữ nút chuột ảo để kéo; ngừng di ngón tay thì ảnh dừng, nhấc tay giữ nguyên vị trí. Ở giới hạn, phần ảnh còn thấy chiếm ít nhất 20% chiều cao vùng hiển thị hiện tại, độc lập với tỷ lệ phóng; nếu ảnh thấp hơn mức đó thì giữ toàn bộ ảnh. Phạm vi tính cả viền đen trên/dưới. Khi kết thúc phóng hoặc về 1 lần, vị trí được giữ nguyên, chỉ giới hạn nếu vượt biên hiển thị; chuyển màn hình sẽ căn giữa lại.
- Hiển thị giữ đúng tỷ lệ ngang/dọc và hỗ trợ chuyển giữa nhiều màn hình từ xa. Chất lượng (thấp/trung bình/cao), tốc độ ghi hình (30/60 fps) và tùy chọn hình ảnh giống iOS: có dò khả năng, xác nhận từ máy tính, quay lại khi lỗi hoặc quá hạn 5 giây và xử lý thứ tự phản hồi khi thao tác liên tiếp.
- “Cài đặt → Hướng màn hình → Khóa hướng ngang sau khi kết nối” mặc định tắt và được lưu trên máy. Khi bật, điện thoại chuyển sang và giữ hướng ngang sau khi kết nối; khi tắt, ứng dụng theo hướng thiết bị. Máy tính bảng và chế độ đa cửa sổ không chịu ảnh hưởng. Thoát phiên sẽ khôi phục hướng trang chủ; quay lại phiên nền sẽ dùng hướng của phiên đó. Logic phát hiện tư thế điện thoại rồi bỏ khóa ngang ban đầu đã được loại bỏ.
- Tham số ban đầu dựa trên khả năng giải mã và kích thước màn hình thường gặp của thiết bị di động: cạnh dài màn hình không quá 1280 pixel mặc định dùng chất lượng thấp (tối đa 720p)/30 fps; các trường hợp khác dùng chất lượng trung bình (tối đa 1080p)/30 fps. Tùy chọn hình ảnh người dùng đã lưu không thay đổi; bạn vẫn có thể chọn thủ công chất lượng cao/60 fps.
- Trạng thái mạng hiển thị tốc độ gửi/nhận và tỷ lệ mất gói riêng cho video, âm thanh, dữ liệu và tổng, cùng tốc độ khung hình thực được gửi để hiển thị, độ phân giải, độ trễ hình ảnh, RTT đã làm mượt, P2P/TURN và trạng thái SRTP thực tế. Dữ liệu làm mới mỗi giây; phép đo hết hạn hoặc không hợp lệ hiển thị “—”.
- Bóng điều khiển nổi có thể kéo được và menu điều khiển có ba cột. Mặc định ứng dụng dùng bộ gõ hệ thống, giữ gợi ý từ, tự sửa và đọc chính tả; văn bản đã xác nhận được nhập trực tiếp vào máy từ xa. Bàn phím máy tính cố định ở dưới, gồm hai trang “Chữ cái và chữ số / Ký hiệu và phím chức năng”, dùng chung bố cục hai dòng cuối và thích ứng ngang/dọc theo chiều cao bàn phím hệ thống. Nút chuyển, thu gọn và phản hồi xúc giác bàn phím Android được thống nhất.
- “Chuột ảo” trong menu nổi mở thanh ngang nút trái／con lăn／nút phải 140 × 48 dp. Thanh đi theo bóng điều khiển, có thể thu bất cứ lúc nào và không ép nhỏ ảnh từ xa. Giữ nút trái hoặc phải rồi kéo cùng ngón tay để kéo thả; ra ngoài phạm vi nút vẫn tiếp tục, nhả tay thì kết thúc. Bạn cũng có thể dùng ngón khác di chuyển con trỏ trên ảnh. Các nút tách rời, phẳng và không chữ; con lăn giữa có hình thức 18 × 30 dp, cả ba nút vẫn giữ vùng chạm 44 × 44 dp và được ngăn bằng khoảng trống. Không có bóng, hiệu ứng nổi/lõm hoặc dịch chuyển khi nhấn; chỉ màu sắc thay đổi. Vạch ngang đồng màu chuyển động lặp theo ngón tay lên/xuống và dừng khi nhả. Bạn có thể vuốt dọc vượt phạm vi nút và vẫn điều khiển cuộn từ xa sau khi phóng ảnh. Đổi chế độ, đổi màn hình, vào nền hoặc hủy chạm đều nhả nút. Menu trạng thái bung đủ chiều cao nội dung, chỉ cuộn khi vùng an toàn không đủ chỗ.
- Menu nổi có công tắc “Màn hình riêng tư” trực tiếp, không cần mở trang chi tiết. Nút hiển thị “Đã bật màn hình riêng tư” hoặc “Đã tắt màn hình riêng tư” theo trạng thái máy tính xác nhận, dùng cùng màu cho bật và tắt. Chuyển đổi bình thường không thêm thông báo chữ, chỉ báo khi có bất thường. Khi bật, chuột/phím cục bộ trên máy tính bị chặn như ở bản máy tính. Sau khi khai báo khả năng điều khiển, ứng dụng có thể dùng thiết lập tự bật khi kết nối của máy tính. Quá 5 giây không cập nhật thì không còn hiển thị thành công; lệnh chưa được xác nhận sau 15 giây sẽ báo quá hạn. Duy trì nền và phục hồi PiP dùng cùng trạng thái phiên.
- Thanh điều khiển nổi hỗ trợ chọn/gửi tệp, nhận tệp từ máy tính, xem tiến độ đã xác nhận và lưu tệp nhận được. Ctrl+Alt+Del dùng dịch vụ hệ thống Windows từ xa; nếu dịch vụ chưa bật, ứng dụng sẽ hiển thị gợi ý.
- Danh tính thiết bị và mật khẩu kết nối được chọn lưu đều mã hóa bằng Android Keystore / AES-GCM; lần đầu truy cập mạng cần có sự đồng ý. Khi đổi ứng dụng trong phiên từ xa, ứng dụng tự dùng PiP hệ thống; nếu không khả dụng thì tự duy trì kết nối bằng thông báo.
- Thẻ trang chủ, các mục trạng thái/thông báo/cài đặt ở trên, bảng mật khẩu và trang cài đặt theo nhóm giống iOS. Kết nối gần đây được tách theo máy chủ; nhấn giữ để xóa bản ghi và mật khẩu.
- Trang chủ máy tính bảng hỗ trợ ngang/dọc và chia màn hình, tự thích ứng chiều rộng khả dụng: từ 900 dp, phần kết nối và gần đây nằm cạnh nhau; cửa sổ hẹp hơn thì xếp dọc. Danh sách gần đây có 1–4 cột theo chiều rộng, còn điện thoại ở chiều rộng thông thường vẫn hai cột. Trang chủ rộng tối đa 1200 dp; cài đặt, thông báo và trang đọc tối đa 720 dp. Trong cửa sổ rộng, hộp mật khẩu và quyền riêng tư được căn giữa, giới hạn chiều rộng. Xoay hoặc chỉnh chia màn hình chỉ bố trí lại trang hiện có, giữ nội dung nhập, phần cài đặt đang sửa và phiên hiện tại; trang chủ điện thoại toàn màn hình vẫn giữ hướng dọc.
- Chuyển trang theo cách tương tác của iOS: cài đặt, quyền riêng tư, giới thiệu và thông báo trượt vào từ phải, quay lại trượt ngược và trang nền đi theo nhẹ. Vào/thoát phiên từ xa dùng hiệu ứng mờ dần 0.2 giây, tuân theo công tắc và tốc độ hoạt ảnh của Android.
- Điều khiển chuột và tùy chọn hình ảnh trong cài đặt, cùng chất lượng, tốc độ khung hình và tùy chọn hình ảnh trên thanh nổi, dùng nền lựa chọn trượt trong 0.2 giây. Nhấn liên tiếp sẽ đổi hướng từ vị trí hiện tại; máy tính từ chối hoặc quá hạn thì trượt về mượt mà; hệ thống tắt hoạt ảnh thì chuyển ngay.
- Thông báo dùng chung giao thức tín hiệu với iOS, hỗ trợ huy hiệu chưa đọc, phân trang, kéo làm mới, cập nhật đẩy, chi tiết và liên kết web. Trạng thái đã đọc/xóa lưu trên máy theo máy chủ, danh tính máy này và phiên bản thông báo.
- Sau khi kết nối và nhận thông tin đối tác, ứng dụng kiểm phiên bản máy tính theo cùng quy tắc phiên bản/bản vá của iOS. Bản cũ hiện “Khuyến nghị cập nhật”; nhấn “Đã hiểu” để tiếp tục điều khiển. Mỗi kết nối chỉ kiểm/nhắc tối đa một lần; phiên bản thiếu hoặc không hợp lệ được nhắc như bản cũ, lỗi kiểm tra không làm gián đoạn kết nối. Chỉ lấy siêu dữ liệu phiên bản HTTPS công khai, không tải lên danh tính hay phiên bản đối tác. Chưa cấp quyền thì không yêu cầu; ngắt hoặc vào nền thì hủy kiểm tra và bỏ phản hồi hết hạn.
- Có tùy chọn lưu ảnh xem trước từ xa, mặc định tắt. Sau khi xác nhận bật, từ kết nối tiếp theo ứng dụng lưu một ảnh 640×360 cho thẻ kết nối gần đây. Bạn có thể xóa ảnh xem trước; tắt tùy chọn hoặc rút quyền sẽ xóa ảnh đã có.
- “Giới thiệu CrossDesk” đồng nhất với iOS về trang phiên bản, giấy phép phần mềm, thành phần nguồn mở và nguồn/hướng dẫn biên dịch, gồm thông báo OpenFEC, chi tiết thành phần và toàn văn giấy phép có thể chọn/sao chép ngoại tuyến. Số phiên bản đọc từ APK hiện tại; quá trình biên dịch ghi phiên bản nền của ứng dụng và MiniRTC.

Hiện chưa có chức năng Android được điều khiển hoặc toàn bộ tính năng bổ sung của iOS. Khả năng phần cứng tùy thiết bị và định dạng mã hóa; khi chuyển về phần mềm, độ phân giải hoặc tốc độ khung hình cao có thể tăng điện năng. Bạn có thể giảm chất lượng hoặc tốc độ ghi hình trong menu nổi. Hiện chỉ biên dịch `arm64-v8a`, không hỗ trợ giả lập x86 hoặc thiết bị 32 bit.

## Gợi ý cập nhật máy này

Sau khi được cấp quyền mạng, ứng dụng kiểm tệp phiên bản công khai một lần mỗi khi khởi động hoặc từ nền về, và kiểm lại sau khi kết thúc kết nối từ xa ở tiền cảnh. Các lần kích hoạt đồng thời dùng chung yêu cầu đang chạy. Không kiểm định kỳ hoặc tự thử lại; khi lỗi, ứng dụng giữ thông báo bản mới đã phát hiện và chờ lần kích hoạt sau hoặc kiểm thủ công. Vào nền hoặc rút quyền sẽ hủy yêu cầu và bỏ phản hồi hết hạn.

Máy này chỉ đọc `downloads.android-arm64`, dùng lại quy tắc so sánh phiên bản/bản vá của PC. Chỉ đổi ngày biên dịch thì không nhắc nâng cấp; thiếu trường phiên bản nền tảng được coi là kiểm tra thất bại. Khi có bản mới, góc trên phải nút cài đặt ở trang chủ hiện huy hiệu chấm than trắng trên nền đỏ, cùng cỡ và vị trí với huy hiệu thông báo. “Giới thiệu” trong cài đặt hiện “Phiên bản mới” và chấm đỏ; trang “Giới thiệu” hiển thị số mới, liên kết cập nhật trên trang chủ và “Kiểm tra cập nhật”. Chức năng này độc lập với lời nhắc nâng cấp máy tính được điều khiển.

## Biên dịch

Cài trên macOS hoặc Linux:

- JDK 17 trở lên (có thể dùng JBR kèm Android Studio).
- Android SDK Platform 36, Build Tools 36.0.0, NDK `28.2.13676358`.
- Xmake **3.1.1 trở lên**, Python 3.9+, Git, CMake và công cụ biên dịch máy chủ.
- setuptools, wheel Python để biên dịch công cụ máy chủ của phụ thuộc.

Khởi tạo submodule tại gốc kho trước:

```sh
git submodule update --init --recursive
```

Ví dụ dòng lệnh macOS:

```sh
export ANDROID_HOME="$HOME/Library/Android/sdk"
export JAVA_HOME="/Applications/Android Studio.app/Contents/jbr/Contents/Home"
# Dùng Xmake trong PATH (tối thiểu 3.1.1), hoặc chỉ định tệp khác qua XMAKE_BIN.
cd apps/android
./gradlew :app:assembleDebug
```

Bạn cũng có thể mở `apps/android` bằng Android Studio và cài các phiên bản trên trong SDK Manager. IDE có thể ghi vị trí SDK vào `local.properties` đã bỏ qua. Nếu Xmake không nằm trong PATH của IDE, hãy đặt `XMAKE_BIN` trước khi mở IDE.

Đầu ra `app/build/outputs/apk/debug/app-debug.apk`, đã ký bằng khóa debug Android máy này:

```sh
"$ANDROID_HOME/platform-tools/adb" install -r app/build/outputs/apk/debug/app-debug.apk
```

Bản Release dùng `./gradlew :app:assembleRelease`, mặc định xuất APK chưa ký; bạn cần cấu hình chữ ký riêng trước khi phát hành. Debug APK cũng dùng thư viện gốc Release đã tối ưu để bảo đảm hiệu năng giải mã phần mềm.

Lần biên dịch đầu sẽ tải và biên dịch phụ thuộc từ nguồn, thời gian tùy mạng; các lần sau dùng lại bộ nhớ đệm. Android biên dịch trực tiếp nguồn trong `deps/submodules/minirtc`. Sản phẩm và đệm phụ thuộc ở `.native/`, cấu hình dự án Xmake ở `native/.xmake/`, lưu riêng với máy tính và iOS:

- Gradle 9.9.0, AGP 9.1.0, NDK r28c cố định trong dự án.
- Kho phụ thuộc Xmake theo chính sách PC: dùng kho mặc định hoặc kho do môi trường gọi chỉ định, không cố định hay kiểm commit kho. Sản phẩm Android và đệm gói cài đặt vẫn lưu riêng.
- `scripts/build_native.py` gọi `native/xmake.lua` độc lập, trực tiếp đưa vào định nghĩa biên dịch của submodule MiniRTC. Quá trình biên dịch không sao chép nguồn MiniRTC hay áp bản vá.
- Nhánh Android trong nguồn MiniRTC bật codec phần mềm OpenH264, SVT-AV1, dav1d và các công thức phụ thuộc, tương thích header Android của OpenFEC và tắt UPnP. SVT-AV1 dùng khả năng dò ARM CPU tích hợp. Ứng dụng hiện là bên điều khiển chỉ nhận video, nhưng thư viện đồng thời cung cấp mã hóa AV1. Xác minh TLS nằm trong MiniRTC; STUN, TURN UDP/TCP và kết nối trực tiếp vẫn do MiniRTC cung cấp.
- `thirdparty/openssl/xmake.lua` của MiniRTC quản lý chung công thức iOS và Android. Android dùng tường minh công cụ lập chỉ mục archive của NDK, tránh `ranlib` macOS làm hỏng thư viện tĩnh ELF.
- Mã hóa SVT-AV1 và giải mã dav1d đã được kiểm chứng trên giả lập Android 15 arm64 / 16 KB, gồm mã hóa phần mềm, quay về phần mềm khi yêu cầu tăng tốc phần cứng, cập nhật bitrate, ép khung khóa và chuyển độ phân giải 320×192 → 256×144 → 320×192. Mỗi nhóm trong hai nhóm thử đều mã hóa/giải mã thành công 24 khung. Đây là kiểm chứng tính đúng đắn của codec, không đại diện hiệu năng mã hóa thời gian thực trên máy thật.
- Phụ thuộc liên kết tĩnh vào `libcrossdesk_android.so`, giữ căn chỉnh ELF / APK 16 KB.

Sau khi sửa mã gốc, chỉ cần chạy lại Gradle. Khi đổi phiên bản hoặc công thức phụ thuộc, hãy kiểm đồng bộ nội dung giấy phép; nếu cần, xóa `.native/packages` rồi biên dịch lại. Thay đổi nguồn và công thức MiniRTC được lưu trực tiếp trong submodule; hãy commit MiniRTC trước rồi cập nhật tham chiếu submodule trong kho chính.

## GitHub Actions

Bạn có thể chạy riêng [`Build Android`](../../.github/workflows/build-android.yml) thủ công trên trang Actions. PR sửa Android, MiniRTC, giao thức chung hoặc tài liệu giấy phép cũng kích hoạt quy trình. [`Build and Release`](../../.github/workflows/build.yml) hiện có gọi cùng quy trình khi đẩy nhánh, đẩy nhãn hoặc chạy thủ công. Phát hành nhãn `v` chờ Android biên dịch và ký thành công, rồi thêm Release APK đã ký vào GitHub Release và tập sản phẩm trên máy chủ tải hiện có. Trước khi đẩy kho chính, hãy bảo đảm commit MiniRTC được tham chiếu đã được đẩy tới remote submodule; nếu không, CI không thể checkout xong.

Phát hành riêng dùng [`Release Mobile`](../../.github/workflows/release-mobile.yml). Trước hết chạy `python3 scripts/version.py set client <phiên-bản>` tại gốc kho để đồng bộ phiên bản mọi ứng dụng và số bản dựng di động; kiểm và commit thay đổi rồi tạo nhãn `android-v<versionName>-YYYYMMDD` cho commit đó, ví dụ:

```sh
git tag android-v1.7.6-20261112
git push origin android-v1.7.6-20261112
```

Nhãn này chỉ kích hoạt biên dịch Android. Quy trình ký bằng khóa phát hành cố định, xác minh Release APK rồi phát hành lên GitHub Release riêng và máy chủ tải hiện có. Nếu ký thất bại hoặc thiếu Secrets ký, quy trình dừng, không chuyển sang APK chưa ký. Phiên bản nhãn bắt buộc khớp ứng dụng và ngày phải hợp lệ. Để thử lại, chạy `Release Mobile` thủ công ở Actions và điền nhãn đã có vào `source_tag`; dù kích hoạt từ nhánh khác, quy trình vẫn checkout nguồn của nhãn đó. Tài liệu nguồn APK ghi nhãn Android cùng commit chính xác của ứng dụng và MiniRTC.

Phát hành di động riêng giữ nguyên nhãn `latest` máy tính và lựa chọn Release mới nhất trên GitHub. Khi tải APK lên xong, địa chỉ máy chủ được hợp vào `downloads.android-arm64` của `https://version.crossdesk.cn/version.json` chung, cùng phiên bản và ngày phát hành Android trong mục đó; thông tin phiên bản máy tính cấp cao nhất và mục tải nền tảng khác được giữ nguyên. Tải lên máy chủ giữ tệp nền tảng khác và dùng lại Secrets `SERVER_HOST`, `SERVER_USER`, `SERVER_KEY` hiện có. Nhãn phiên bản máy tính thông thường vẫn phát hành mọi nền tảng qua `Build and Release` và cập nhật từng mục tải; mọi tác vụ cập nhật tệp phiên bản dùng chung một hàng đợi. Chạy riêng `Build Android` vẫn chỉ tải lên sản phẩm biên dịch Actions.

Nếu Release đã có cần tải bổ sung lên máy chủ hoặc làm mới thông tin tải, bạn có thể chạy `Update version.json from Release` thủ công và điền `source_tag` đã có, không cần biên dịch lại.

Quy trình dùng Ubuntu 24.04, JDK 17, Python 3.13, SDK 36, Build Tools 36.0.0, NDK r28c và kiểm biên dịch bằng Xmake tối thiểu 3.1.1; cục bộ cho phép Xmake mới hơn. Gradle dùng Wrapper trong dự án; trình biên dịch gốc chọn kho Xmake như PC. SDK/NDK dùng đệm riêng theo hệ thống máy chủ, kiến trúc và phiên bản bộ công cụ. Trúng đệm thì bỏ cài SDK; không trúng thì lưu đệm ngay sau khi cài, để lần sau dùng lại dù bước biên dịch APK sau đó thất bại. Môi trường thực thi Xmake và tải pip cũng có đệm riêng; công cụ hệ thống Ubuntu chỉ cài những mục còn thiếu.

Đệm Gradle và đệm phụ thuộc gốc Android được quản lý riêng. Đệm gốc tách theo kiến trúc máy chủ, script biên dịch và công thức MiniRTC, không dùng lại cấu hình hay object của máy tính/iOS. Runner GitHub vẫn là môi trường mới mỗi lần khởi chạy, nên lần đầu hoặc khi đệm mất hiệu lực vẫn cần cài đặt. Nhánh và nhãn khác có thể đọc đệm nhánh mặc định; vì vậy nên chạy `Build Android` một lần trên nhánh mặc định để làm nóng đệm trước khi tạo nhãn phát hành.

CI chỉ biên dịch/đóng gói Debug và Release APK, kiểm tên gói, `versionName` / `versionCode`, arm64 ABI, căn chỉnh ELF / ZIP 16 KB, giấy phép đầy đủ và chữ ký Debug. Unit test, kiểm giao thức C++/hàng đợi khung cục bộ, Lint và kiểm thử thiết bị chuyển sang chạy cục bộ theo nhu cầu. Kiểm thử thiết bị chạy trên điện thoại hoặc giả lập arm64 theo quy trình bên dưới.

Phát hành chung `v*` trích phiên bản số từ nhãn và ghi vào `versionName` APK qua thuộc tính Gradle; phiên bản trong ứng dụng đọc cùng thông tin gói. Phiên bản sản phẩm, ngày và số hotfix tùy chọn khớp bản máy tính, ví dụ `v1.6.1-20261010` hoặc `v1.6.1-2-20261010`. Biên dịch nhánh và phát hành riêng `android-v*` dùng phiên bản trong `gradle.properties`; phát hành riêng kiểm nhãn khớp cấu hình. Ngày biên dịch theo múi giờ Thượng Hải; nhãn có ngày giữ ngày trên nhãn như iOS. Actions cung cấp:

- `crossdesk-android-arm64-<phiên-bản>.apk`: Release APK đã ký, phát hành Android riêng/nhãn toàn nền tảng lấy APK này, cài thẳng được.
- `crossdesk-android-arm64-unsigned-<phiên-bản>.apk`: sản phẩm trung gian quy trình, dùng tự ký/kiểm bản sửa được.
- `crossdesk-android-arm64-debug-<phiên-bản>.apk`: ứng dụng debug cài được, giữ 14 ngày.

Debug APK dùng chứng chỉ debug tạm cho mỗi lần chạy, nên có thể không ghi đè được bản đã ký khác trên điện thoại. Gỡ ứng dụng cũ sẽ xóa kết nối và mật khẩu cục bộ. Để nâng cấp ghi đè liên tục, bạn nên ký Release APK bằng cùng khóa riêng và tăng `crossdeskVersionCode` trong `gradle.properties` mỗi lần phát hành mới, kể cả phát hành chung và hotfix. Số thứ tự nâng cấp cài đặt này độc lập với phiên bản hiển thị, không suy từ ngày hay số lần chạy CI. APK chưa ký đã được căn chỉnh; bạn có thể ký bằng SDK `apksigner sign --ks <kho-khoa-cua-ban> --out <da-ky.apk> <chua-ky.apk>`, rồi kiểm bằng `apksigner verify`.

### Ký Release

[`Sign Android Release`](../../.github/workflows/sign-android.yml) ký trong job riêng, không checkout hay thực thi nguồn ứng dụng. Job biên dịch và biên dịch PR không nhận khóa phát hành. Job ký dùng Build Tools 36.0.0, chạy `zipalign -P 16` trước, rồi truyền mật khẩu cho `apksigner` qua biến môi trường. Sau khi xác minh chữ ký và căn chỉnh 16 KB, job tải lên APK đã ký. Kho khóa tạm được dọn cuối bước; sản phẩm không chứa khóa hay mật khẩu.

Trước phát hành đầu, cấu hình trong Settings → Secrets and variables → Actions của kho:

| Secret | Nội dung |
| --- | --- |
| `ANDROID_KEYSTORE_BASE64` | Nội dung Base64 tệp kho khóa phát hành |
| `ANDROID_KEYSTORE_PASSWORD` | Mật khẩu kho khóa |
| `ANDROID_KEY_ALIAS` | alias khóa riêng phát hành |
| `ANDROID_KEY_PASSWORD` | Mật khẩu khóa riêng phát hành |

Ký APK không cần tài khoản nhà phát triển Google Play. Nếu chưa có khóa phát hành, bạn có thể tạo một lần bằng `keytool` của JDK, lưu kho khóa ngoài dự án và sao lưu khóa cùng mật khẩu:

```sh
keytool -genkeypair -storetype PKCS12 -keystore /path/outside/repo/crossdesk-release.p12 \
  -alias crossdesk-release -keyalg RSA -keysize 4096 -validity 10000
```

PKCS12 dùng cùng mật khẩu cho kho khóa và khóa riêng; hãy cấu hình nó vào cả hai Secrets mật khẩu. Bạn có thể dùng `gh secret set` ghi Secrets từ stdin để tránh đặt mật khẩu trong tham số dòng lệnh. Ví dụ tải kho khóa trên macOS:

```sh
base64 -i /path/outside/repo/crossdesk-release.p12 | gh secret set ANDROID_KEYSTORE_BASE64 --repo kunkundi/crossdesk
gh secret set ANDROID_KEYSTORE_PASSWORD --repo kunkundi/crossdesk
gh secret set ANDROID_KEY_ALIAS --repo kunkundi/crossdesk
gh secret set ANDROID_KEY_PASSWORD --repo kunkundi/crossdesk
```

Các phiên bản sau tiếp tục dùng Secrets này; không được tạo khóa mới mỗi lần biên dịch hay commit khóa/mật khẩu vào Git. Với Android Release chưa ký đã có, bạn có thể chạy riêng `Sign Android Release` và điền `source_tag`, không cần biên dịch lại. Tác vụ thủ công này chỉ tải lên sản phẩm Actions đã ký, không sửa Release đã phát hành. Quy trình phát hành thông thường tự ký, tải lên máy chủ và trỏ `downloads.android-arm64` tới APK đã ký. Khi Release cũ giữ cả APK đã ký và chưa ký, tệp phiên bản ưu tiên tệp đã ký cùng phiên bản.

## Sử dụng và an toàn kết nối

Sau khi được cấp quyền, trang chủ tự kết nối máy chủ. “Đã kết nối máy chủ” màu xanh nghĩa là đăng nhập tín hiệu hoàn tất; nếu lỗi, bạn có thể nhấn trạng thái để thử lại. Trang cài đặt hiển thị ID máy này. Giữ máy tính trực tuyến, nhập ID thiết bị 9 chữ số trên trang chủ Android rồi nhấn “Kết nối”; lần đầu, nhập mật khẩu trong bảng bật lên ở dưới và có thể chọn lưu mã hóa. Sau khi lưu thành công, nhấn kết nối gần đây hoặc nhập lại cùng ID sẽ dùng trực tiếp mật khẩu đã lưu theo máy chủ, kể cả sau khi khởi động lại ứng dụng. Nếu không đọc được mật khẩu hoặc máy từ xa từ chối, bảng nhập sẽ hiện lại. Chỉ thay mật khẩu mới sau khi kết nối thành công; hủy hoặc kết nối lỗi không ghi đè mật khẩu cũ. Với máy chủ tự triển khai, mở “Cài đặt → Máy chủ” để sửa tên máy chủ và cổng, không nhập `https://`, đường dẫn hoặc URL đầy đủ. Mặc định là `api.crossdesk.cn:9099`.

Như iOS, thẻ kết nối gần đây hiển thị biểu tượng khóa cho mật khẩu đã lưu và trạng thái trực tuyến／ngoại tuyến cạnh ID. Thiết bị trực tuyến được ưu tiên, cùng trạng thái thì giữ thứ tự kết nối gần đây trước đó. Sau khi cấp quyền và đăng nhập máy chủ, ứng dụng đăng ký `recent_connections_presence` qua tín hiệu danh tính, nhận snapshot đầy đủ và `presence_update`. Dữ liệu làm mới mỗi 30 giây; quá 60 giây không cập nhật thì hiển thị ngoại tuyến. Mất kết nối, đổi máy chủ hoặc vào nền sẽ xóa trạng thái trực tuyến; kết nối lại sẽ đợi snapshot mới. Dịch vụ tự triển khai cần hỗ trợ cùng giao thức trạng thái trực tuyến thiết bị.

Tin cậy chứng chỉ lấy từ các chứng chỉ gốc do TrustManager mặc định Android xuất ra; Network Security Configuration của ứng dụng tin tường minh CA hệ thống và CA do người dùng cài. Sau khi JNI đặt đường dẫn tệp chứng chỉ, nhánh Android MiniRTC tải tường minh bằng `SSL_CTX_load_verify_locations`, xác minh chuỗi chứng chỉ và tên máy DNS / IP, không bỏ qua TLS. Tiến trình ứng dụng Android mang cờ `AT_SECURE`, nên không thể dựa vào trình tải đường dẫn mặc định OpenSSL để đọc `SSL_CERT_FILE`; nếu làm vậy, mọi máy chủ đều không dựng được chuỗi tin cậy. Máy chủ tự triển khai có thể dùng chứng chỉ được hệ thống tin hoặc cài CA ký chứng chỉ máy chủ qua cài đặt Android. Sau khi cài/gỡ CA, hãy khởi động lại CrossDesk rồi kết nối; chứng chỉ máy chủ cần còn hạn và chứa tên miền hoặc IP khớp địa chỉ kết nối.

Danh tính lưu riêng theo tên máy chủ và cổng; sao lưu và chuyển thiết bị không chứa dữ liệu ứng dụng. Mật khẩu không ghi vào bản ghi kết nối, chỉ ghi vào kho mã hóa riêng khi bật rõ “Lưu mật khẩu”; nếu không bật thì chỉ dùng cho phiên hiện tại. Thông báo kết nối thành công chỉ xử lý một lần, callback truyền tải sẵn sàng lặp không lưu lại mật khẩu hay dựng lại trang phiên; kho mật khẩu từ chối giá trị rỗng ghi đè. Mật khẩu bản cũ đã bị giá trị rỗng ghi đè không thể khôi phục. Gặp bản ghi đó, ứng dụng nhắc nhập lại một lần và giữ “Lưu mật khẩu” bật. Vì nhật ký chẩn đoán MiniRTC chứa danh tính đăng nhập, lớp cầu nối Android tắt nhật ký phiên của nó. Kho danh tính hỏng sẽ báo lỗi, không âm thầm đăng ký lại.

“Bảng nhớ tạm” trong phiên gửi văn bản người dùng nhập. Văn bản từ xa nhận được trước hết giữ trong phiên; chỉ khi nhấn “Sao chép văn bản từ xa” mới ghi vào bảng nhớ tạm điện thoại. Thoát phiên sẽ xóa nội dung phiên; mỗi lần gửi tối đa 128 KB. Phím bổ trợ của bàn phím máy tính được chọn cục bộ, chỉ áp dụng cho phím tiếp theo và tự đặt lại sau khi gửi; tên hiển thị theo máy từ xa là Win / Super / Cmd và Alt / Option. Bàn phím hệ thống dò khả năng qua `supports_text_input`; máy Windows/macOS hỗ trợ UTF-8 có thể nhận trực tiếp tiếng Trung, emoji và văn bản đọc chính tả. Máy khác hoặc bản cũ chuyển về phím ASCII; văn bản không hỗ trợ sẽ gợi dùng bảng nhớ tạm, không xóa nội dung từ xa trước. Chữ đang ghép chưa xác nhận không được gửi; tự sửa chỉ thay hậu tố đã thay đổi của văn bản đã gửi. Nhấn vào máy từ xa, thu gọn, đổi bàn phím hoặc vào nền đều xóa ngữ cảnh nhập cục bộ.

Nhập văn bản Windows dùng sự kiện bàn phím Unicode, không cần đổi bộ gõ trên máy tính hay ghi đè bảng nhớ tạm. Bản Windows hỗ trợ chuyển tiếp văn bản sẽ gửi đầy đủ UTF-8 qua dịch vụ Windows tới màn hình khóa, đăng nhập và desktop an toàn UAC, giữ nguyên số, chữ hoa/thường, ký hiệu, dấu cách và chữ Trung, không đổi mật khẩu thành phím số hoặc ASCII. Kênh này cần cập nhật đồng thời ứng dụng Windows, CrossDesk Service và chương trình trợ giúp phiên. Chỉ cập nhật Android hoặc chỉ xác nhận dịch vụ cũ đang chạy đều không bật bản sửa. Bản cũ có thể đã khai `supports_text_input` nhưng chỉ hỗ trợ nhập ở desktop thông thường, nên vẫn có thể gặp “bàn phím máy tính nhập được, bộ gõ hệ thống như Sogou không nhập được”. Hiệu quả nhập trên màn hình khóa cần kiểm chứng trong môi trường Windows đã cập nhật.

“Gửi tệp” gọi bộ chọn tệp hệ thống, đọc tệp đã chọn ở nền rồi truyền bằng cùng giao thức `file` / `file_feedback` của máy tính, hỗ trợ tệp rỗng và tên tiếng Trung. Mỗi lần gửi một tệp, chia khối 64 KiB và giới hạn tối đa 1 MiB dữ liệu chưa xác nhận để khống chế hàng đợi gửi. Tiến độ theo xác nhận của bên nhận; 30 giây không có tiến triển hợp lệ sẽ báo thất bại. Khi nhận, ứng dụng kiểm kích thước và theo dõi phạm vi byte đầy đủ, cho phép callback khác luồng đến đảo thứ tự; tệp chưa xong không có mục lưu. Tệp trùng tên lưu riêng, không ghi đè tệp có sẵn. “Lưu tệp đã nhận” xuất tệp nhận gần nhất; nếu nhận liên tiếp nhiều tệp, nên lưu từng tệp.

Trong khi chọn hoặc lưu tệp, phiên được giữ và PiP tự động tạm tắt; hủy chọn không ngắt kết nối. Nếu ở đó quá 5 phút mà ứng dụng vẫn không ở tiền cảnh, phiên sẽ kết thúc. Bản sao gửi và tệp nhận tạm lưu trong đệm riêng của ứng dụng, xóa khi kết thúc phiên. Bạn nên xuất tệp nhận cần giữ trước khi ngắt; ứng dụng không xin quyền truy cập toàn bộ bộ nhớ lưu trữ.

## Kết nối nền và hình trong hình

Như iOS, ứng dụng dùng PiP hệ thống để hiển thị video từ xa thực và duy trì phiên. Sau khi kết nối thành công và đã có hình từ xa, Android 12 trở lên bật PiP tự động hệ thống; Android 8–11 thử vào PiP khi rời ứng dụng. Thiết bị cần hỗ trợ và hệ thống cần cho CrossDesk dùng tính năng này. Chưa kết nối, chưa có hình hoặc đang mở bộ chọn tệp thì không tự vào.

Đủ phiên bản Android không có nghĩa firmware cung cấp PiP; một số thiết bị ở chế độ ít bộ nhớ sẽ tắt tính năng. Kết quả dò `FEATURE_PICTURE_IN_PICTURE` của hệ thống là căn cứ; chỉ cấp quyền PiP không bổ sung được khả năng còn thiếu. Nếu không có PiP hệ thống hoặc bị cấm quyền, ứng dụng tự khởi động dịch vụ tiền cảnh dự phòng sau khi kết nối; vào PiP thất bại cũng thử dùng dự phòng này. Menu điều khiển không có nút “Hình trong hình” hay “Duy trì nền”, cả hai tự xử lý khi đổi ứng dụng.

PiP chỉ hiển thị toàn bộ ảnh từ xa, ẩn con trỏ cục bộ, bóng nổi, thanh chuột và bàn phím, đồng thời nhả phím từ xa. Ứng dụng dùng Surface video và kết nối RTC cũ, không đóng âm thanh/video khi vào cửa sổ nhỏ. Bạn có thể quay lại toàn màn hình từ cửa sổ hệ thống để tiếp tục thao tác. Tỷ lệ ảnh cập nhật theo kích thước từ xa; ảnh quá rộng hoặc dài hiển thị đầy đủ kèm viền đen trong phạm vi tỷ lệ hệ thống cho phép.

PiP bình thường không khởi động dịch vụ dự phòng. Cửa sổ nhỏ đang hiển thị giữ Activity ở trạng thái started; vào PiP chỉ tạm dừng tương tác. Đóng cửa sổ nhỏ sẽ kết thúc phiên, không tự chuyển thành kết nối nền ẩn.

Chế độ dự phòng giữ kết nối RTC/tín hiệu gốc qua dịch vụ tiền cảnh và hiển thị thông báo thường trực với “Quay lại”/“Ngắt kết nối”. Ở nền, ứng dụng dừng kết xuất cục bộ và phát âm thanh, không tạo cửa sổ nổi; truyền tải và giải mã video không tạm dừng ở lớp giao thức. Quay lại ứng dụng sẽ gắn Surface lại, yêu cầu khung khóa và phục hồi trạng thái tắt tiếng, màn hình, cài đặt hình ảnh cũ, không đăng nhập hay kết nối từ xa lại. Trạng thái phiên độc lập với Activity, nên trang bị hệ thống thu hồi vẫn khôi phục được; nếu cả tiến trình bị chấm dứt thì không khôi phục kết nối cũ.

Dịch vụ chỉ dùng cho phiên từ xa do người dùng chủ động tạo và đã kết nối thành công, không dùng để chờ ở trang chủ. Android 14 trở lên dùng loại dịch vụ tiền cảnh `specialUse` và khai mục đích kết nối điều khiển từ xa. Android 13 trở lên xin quyền thông báo khi lần đầu bật dự phòng; nếu từ chối, hệ thống chỉ hiển thị dịch vụ trong trình quản lý ứng dụng đang hoạt động. Khóa đánh thức CPU một phần chỉ giữ khi phiên dự phòng ở nền, nhả khi về tiền cảnh, ngắt, xóa tác vụ hoặc hủy dịch vụ. Ngắt từ thông báo, ngắt từ xa, xóa tác vụ, rút quyền và buộc dừng đều kết thúc phiên/dịch vụ, không tự kết nối lại. Bộ chọn tệp vẫn có giới hạn năm phút nói trên; hệ thống buộc kết thúc hoặc mất mạng vẫn có thể làm hết kết nối.

“Tùy chọn hình ảnh” trong cài đặt trang chủ lưu cục bộ làm mặc định cho kết nối sau; thiết lập trong phiên chỉ ảnh hưởng kết nối hiện tại như iOS. Chất lượng ban đầu là thấp nếu cạnh dài không quá 1280 pixel, hoặc trung bình nếu lớn hơn, với tốc độ ghi 30 fps. Máy tính báo hỗ trợ rồi ứng dụng tự gửi thiết lập; bản cũ chưa báo thì các điều khiển bị vô hiệu. FPS trên trang mạng đếm khung thực gửi tới Android Surface; độ trễ hình ảnh ước từ thời điểm ghi đã hiệu chỉnh của MiniRTC tới lúc gửi hiển thị, chưa hiệu chỉnh thì hiện “—”. RTT làm mượt với trọng số 0.25; dữ liệu 3 giây không cập nhật sẽ hết hạn. Chuyển màn hình xóa FPS, độ phân giải và trễ hình cũ, giữ thống kê mạng.

Tính hợp lệ của thời điểm ghi được kiểm lúc nhận; thời gian giải mã và chờ hiển thị cục bộ được tính vào trễ hình ảnh. Trễ thực quá 5 giây vẫn tiếp tục hiển thị, không bị lọc như timestamp không hợp lệ. Luồng hiển thị tái dùng đệm CPU NV12 của bộ giải mã, chỉ giữ một khung chờ; đổi màn hình hoặc Surface sẽ xóa khung cũ. Khi kết thúc phiên, luồng hiển thị dừng trước khi hủy kết nối gốc.

## Cấp quyền riêng tư

Nếu chưa cấp quyền riêng tư, ứng dụng tự mở chính sách ngoại tuyến khi khởi động lần đầu hoặc trở về từ nền. Bạn cần chủ động chọn đã đọc và đồng ý mới truy cập mạng, giống iOS. Chọn chưa đồng ý sẽ không lặp hộp trong lần ở lại này, nhưng vẫn xem được cài đặt và bản ghi kết nối cục bộ. Cài đặt → Quyền riêng tư → Quyền riêng tư và cấp quyền cho phép đọc ngoại tuyến hoặc rút quyền; sau khi rút, lần vào tiếp theo sẽ hỏi lại.

## Xem trước màn hình từ xa

“Lưu ảnh xem trước màn hình từ xa” trong Cài đặt → Quyền riêng tư mặc định tắt, chỉ có thể bật sau khi cấp quyền mạng. Như iOS, trước khi bật ứng dụng giải thích ảnh có thể chứa thông tin cá nhân/công việc. Mỗi kết nối chỉ lưu một ảnh sau khi hình thực được hiển thị, cắt giữa thành 640×360 và dùng JPEG chất lượng 78. Ảnh lấy từ Surface video từ xa, không có menu nổi, bàn phím, con trỏ cục bộ hay viền đen ngoài ảnh. Nén, đọc/ghi và giải mã chạy tuần tự ở luồng nền, không sao chép bitmap 4K đầy đủ hoặc chụp liên tục.

Ảnh ở thư mục riêng `RecentConnectionThumbnails`, không được sao lưu, tải lên hoặc ghi vào album hệ thống. Tệp tách theo máy chủ và ID từ xa; thẻ gần đây tải bất đồng bộ. Khi chưa bật, không có ảnh hoặc ảnh hỏng, thẻ vẫn dùng bìa nền tảng. Xóa hoặc loại bản ghi kết nối sẽ xóa ảnh tương ứng.

“Xóa ảnh xem trước” giữ bản ghi kết nối, mật khẩu và công tắc, nên kết nối sau vẫn lưu ảnh mới. Tắt lưu hoặc rút quyền mạng sẽ tắt chức năng lưu và xóa toàn bộ ảnh. Thao tác xóa vô hiệu ngay yêu cầu chụp/đọc chưa xong; xóa và lưu dùng cùng hàng đợi tuần tự để tác vụ chụp cũ không ghi ảnh trở lại. Lỗi xóa có thông báo và có thể thử lại.

## Thông báo chung

Chuông trang chủ đếm mọi thông báo chưa đọc, trên 99 hiển thị 99+. Sau khi cấp quyền và kết nối máy chủ, ứng dụng yêu cầu `announcements_list` qua kết nối tín hiệu danh tính có sẵn: tóm tắt 200/trang, nội dung 20/trang, không phụ thuộc phiên với máy tính từ xa. Danh sách hỗ trợ làm mới, cuộn tải thêm, nhấn giữ hoặc vuốt trái để xóa; xóa cần xác nhận và chỉ ẩn phiên bản hiện tại trên máy này. Mở chi tiết đánh dấu đã đọc; máy chủ sửa phiên bản thì thông báo hiện lại và tính chưa đọc. Nội dung giữ văn bản thuần/xuống dòng, hỗ trợ HTTP(S) và liên kết `[văn bản](https://...)`, không thực thi HTML hay giao thức khác.

Dấu đã đọc/xóa lưu nguyên tử trong tệp ở `AnnouncementReads`, không sao lưu, tách theo tên máy chủ, cổng và ID máy này. Ghi thất bại sẽ báo lỗi và giữ trạng thái cũ; nội dung thông báo chỉ đệm trong RAM. Nhận `announcements_changed` sẽ làm mới; phiên bản danh mục đổi trong lúc phân trang thì lấy lại. Yêu cầu quá hạn hoặc phản hồi từ trước khi mất kết nối không ghi đè trạng thái mới. Lỗi yêu cầu hoặc quá hạn 12 giây có thể thử lại. Chưa cấp quyền thì không truy cập mạng, vào nền thì dừng yêu cầu. Dịch vụ tự triển khai cần hỗ trợ cùng giao thức thông báo của máy tính/iOS.

## Kiểm chứng

Kết nối thiết bị arm64 hoặc mở giả lập arm64, chạy tại `apps/android`:

```sh
./gradlew :app:assembleDebug :app:assembleDebugAndroidTest :app:lintDebug
"$ANDROID_HOME/platform-tools/adb" install -r app/build/outputs/apk/debug/app-debug.apk
"$ANDROID_HOME/platform-tools/adb" install -r app/build/outputs/apk/androidTest/debug/app-debug-androidTest.apk
"$ANDROID_HOME/platform-tools/adb" shell am instrument -w -r cn.crossdesk.mobile.test/androidx.test.runner.AndroidJUnitRunner
"$ANDROID_HOME/platform-tools/adb" uninstall cn.crossdesk.mobile.test
python3 test/verify_apk.py app/build/outputs/apk/debug/app-debug.apk
python3 test/run_native.py
```

Quy trình trên cài ứng dụng và chỉ gỡ gói kiểm thử, giữ dữ liệu ứng dụng hiện có. Nếu có nhiều thiết bị, thêm `-s <so-se-ri-thiet-bi>` vào mỗi lệnh `adb`. `connectedDebugAndroidTest` có thể gỡ ứng dụng được kiểm thử và dữ liệu của nó; không dùng trên thiết bị có kết nối hoặc mật khẩu đã lưu.

Nguồn, tài nguyên, điểm kiểm thử tập trung `test/`:

- `test/native/`: hồi quy giao thức C++, hàng khung/trễ hình cục bộ, `test/run_native.py` biên dịch/chạy.
- `test/android/java/`: hồi quy chức năng Android không UI, logic cần Android API/JNI, gồm làm mới/hết hạn trực tuyến, kết nối lại tín hiệu. Source set `androidTest` Gradle trỏ tường minh vào đây.
- `test/android/assets/tls/`: chứng chỉ TLS loopback cục bộ, chỉ vào APK kiểm thử.
- `test/verify_apk.py`: kiểm APK phát hành arm64 ABI, căn ELF / ZIP 16 KB, giấy phép đầy đủ, tài liệu nguồn.

Hồi quy thiết bị chỉ giữ các kiểm tra chứng chỉ TLS, mã hóa xác thực/cách ly máy chủ, vòng đời kết nối gốc, ánh xạ phím, trạng thái hình ảnh, hình học thu phóng/tọa độ, mẫu thống kê mạng, lưu thông báo/xem trước và tương thích phiên bản. Kiểm kho ảnh dùng bitmap màu thuần được sinh ra, không ghi màn hình hay video.

Cử chỉ cần kiểm thủ công trong phiên từ xa: ở cả hai chế độ chuột, kiểm chụm phóng tới 10 lần, thu về 1 lần, di hai ngón sau khi phóng và giới hạn biên. Ở tỷ lệ gốc, vuốt hai ngón vẫn điều khiển con lăn; chụm và di ảnh không nên tạo nhấp hoặc kéo từ xa. Sau khi phóng, kiểm vị trí nhấp khớp con trỏ, tọa độ đúng sau khi đổi ngang/dọc và chuyển màn hình/kết nối lại đặt về 1 lần. Hồi quy tự động chỉ bao phủ hình học, không chèn cử chỉ UI.

Kiểm chứng UI đều thực hiện thủ công, không duy trì ca tự động cho điều hướng trang, bố trí, hoạt ảnh, cửa sổ nổi hay chụp ảnh màn hình. `test/verify_apk.py` kiểm tính đầy đủ của giấy phép và tài liệu biên dịch.

Tích hợp thanh điều khiển cần kiểm trong phiên thật: chọn/hủy chọn tệp, nội dung tệp gửi/nhận khớp, xuất sau khi nhận và mất kết nối giữa lúc truyền. Với Windows từ xa, kiểm riêng Ctrl+Alt+Del khi dịch vụ hệ thống bật và tắt.

PiP cần kiểm trong phiên thật: tự vào khi đổi ứng dụng, hình tiếp tục cập nhật khi ứng dụng khác ở tiền cảnh, trở lại toàn màn hình, đóng cửa sổ nhỏ, tắt tiếng, ảnh quá rộng hoặc dọc, hủy bộ chọn tệp, hệ thống tắt PiP và khóa màn hình.

Dịch vụ tiền cảnh dự phòng cần kiểm trên máy thật không có PiP gốc: vào/ra nền, tái dùng phiên sau khi dựng lại Activity, ngắt từ thông báo, cách ly thông báo cũ, nhả khóa đánh thức khi từ xa ngắt và phục hồi mạng/hình ảnh thực tế. Trước và sau kiểm thử thiết bị, nên sao lưu, đối chiếu và khôi phục cài đặt máy chủ cùng các thiết lập ứng dụng, tránh để lại địa chỉ thử cục bộ khi kiểm thử bị ngắt.

## Mã hóa và giải mã MediaCodec

Triển khai theo phân lớp NVCodec của MiniRTC: `src/media/mediacodec/` phụ trách backend phần cứng, hỏi khả năng và tài nguyên JNI/Surface; `src/media/video/encode/mediacodec/` và `src/media/video/decode/mediacodec/` lần lượt nối giao diện mã hóa/giải mã, đồng thời xử lý quay về phần mềm với cùng định dạng. Dự án Android biên dịch trực tiếp các nguồn này. API chung chỉ cần bật `hardware_acceleration` và `native_video_output`. Lớp thích ứng JNI Android đặt JavaVM vào triển khai riêng của backend trong `JNI_OnLoad`; cầu nối không được cài làm header SDK hay xuất symbol thư viện động. Hiện hỏi khả năng và tạo SurfaceTexture phụ thuộc JNI; thiếu JavaVM vẫn dùng codec phần mềm được. Luồng xuất riêng vẫn lấy khung cuối sau khi hình ngừng cập nhật và ngủ khi rảnh.

- H.264 / AV1 chọn thành phần phần cứng qua truy vấn khả năng hệ thống, hỗ trợ chỉnh bitrate mã hóa, yêu cầu khung khóa và đổi độ phân giải. Đầu vào mã hóa là NV12 8 bit; phần cứng cần nhận NV12 / I420 ByteBuffer, không đáp ứng thì quay về OpenH264 / SVT-AV1. Ứng dụng hiện vẫn là bên điều khiển; khả năng mã hóa của thư viện không có nghĩa đã triển khai Android được điều khiển.
- Đầu ra giải mã gốc là đệm MediaCodec có thể giữ/nhả, được luồng hiển thị gửi trực tiếp tới Surface. Đổi màn hình, hủy Surface hoặc quay về đầu ra phần mềm đều bỏ gắn Surface cũ. Đệm không được hiển thị lại sau khi dựng lại hoặc hủy bộ giải mã; đệm chưa hiển thị tự được trả về.
- Khi tắt đầu ra gốc, ứng dụng hỗ trợ chuyển đầu ra NV12 / I420 phần cứng thành NV12 gọn theo stride, chiều cao đệm và vùng cắt; bố trí không rõ sẽ quay về phần mềm.
- `OMX.sprd.av1.decoder` trên Retroid Pocket 2S thực tế tải dav1d phần mềm. Dù hệ thống đánh dấu là thành phần phần cứng, nó vẫn bị loại tường minh và AV1 quay về phần mềm. H.264 có thể dùng `OMX.sprd.h264.encoder` / `OMX.sprd.h264.decoder`.

Đường AV1 phần cứng chưa được kiểm chứng trên máy thật tương ứng; Retroid dùng dav1d phần mềm.

## Tài liệu nguồn mở

Ứng dụng dùng GPL-3.0-only, MiniRTC dùng LGPL-3.0-only, thành phần khác giữ giấy phép riêng. `scripts/generate_notices.py` tái dùng thông báo phụ thuộc hiện có của kho, chọn các thành phần Android thực dùng; `licenses/additional-notices.json` bổ sung OpenH264, GNU libiconv và runtime NDK / LLVM. Trang thông tin nguồn mở trong APK cung cấp toàn văn. Bạn có thể sửa nguồn thư viện, liên kết lại rồi ký/cài bằng khóa Android riêng.

Cài đặt → Giới thiệu có cùng nhóm và mục với iOS: phiên bản, giấy phép phần mềm, thành phần nguồn mở, nguồn/hướng dẫn biên dịch. Trang giấy phép phần mềm cho đọc riêng GNU GPL phiên bản 3 và thông báo quyền phần mềm nguồn mở. Trang thành phần trình bày riêng thông báo OpenFEC, còn lại liệt kê tên và giấy phép; chi tiết có phiên bản, nguồn, công thức biên dịch áp dụng và từng tài liệu giấy phép. Nội dung được sinh theo bản biên dịch Android, không hiển thị phụ thuộc iOS không dùng, và hướng dẫn ký Android thay hướng dẫn Apple.

APK giữ đầy đủ `THIRD_PARTY_NOTICES.txt`, đồng thời sinh chỉ mục nhẹ `ThirdPartyLicenses.json`, 72 tài liệu giấy phép riêng và `SourceMetadata.json`. Chỉ đọc thân tài liệu khi mở; văn bản dài chia thành đoạn có giới hạn, hỗ trợ chọn, sao chép và liên kết web. Quá trình biên dịch vẫn sinh/đóng gói tài liệu giấy phép, nhưng CI và kiểm APK không còn kiểm nội dung, hash hay danh sách thành phần. Trang nguồn hiển thị nhãn/commit và gợi ý thay đổi chưa phát hành; liên kết nguồn trỏ về commit nền. Hướng dẫn biên dịch Android, script và công thức MiniRTC, OpenSSL, SVT-AV1 hiện tại kèm APK ngoại tuyến, tránh liên kết bản phát triển trỏ về tệp Android chưa có trong commit nền.

## Phím tắt và tùy chỉnh

Thanh công cụ 44 dp trên bàn phím hệ thống có dải phím tắt thường dùng cuộn ngang, mặc định Esc, Tab, sao chép, dán, cùng mục “Thêm”. Bảng Thêm chia nhóm “Thường dùng / Phím / Hệ thống / Của tôi”, thay vùng bàn phím và giữ chiều cao bộ gõ. Preset thích ứng Windows, macOS và Linux từ xa; preset Linux cần chọn GNOME hoặc KDE Plasma thủ công, nền tảng không rõ chỉ có phím chung. Tùy chọn bảo mật Windows tiếp tục dùng kênh dịch vụ Ctrl+Alt+Del.

“Quản lý và tùy chỉnh” dùng cửa sổ cục bộ riêng, nội dung sửa không gửi từ xa. Có thể lưu tối đa 16 tổ hợp tùy chỉnh, ghim tối đa 8 cho mỗi nền tảng từ xa, hỗ trợ sắp xếp, bỏ ghim, sửa và xóa. Tổ hợp tùy chỉnh giữ phím bổ trợ thực người dùng đã chọn. Toàn bộ tổ hợp được xếp hàng, các phím bổ trợ nhả theo thứ tự ngược. Trước khi thực thi phím tắt, ứng dụng gửi văn bản đã xác nhận và đặt lại ngữ cảnh tự sửa; nếu còn ghép từ thì nhắc chọn từ hoặc hủy trước.
