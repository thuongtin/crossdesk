# Chuyển ứng dụng máy tính Slint sang HarmonyOS

[Hướng dẫn máy tính](../../../docs/USER_GUIDE.md) · [Điểm vào biên dịch HarmonyOS](../README.md) · [Quyền và chữ ký](#quyền-bền-vững-cho-máy-được-điều-khiển-api-26)

Mục tiêu là dùng cùng giao diện và hành vi sản phẩm với Windows, macOS, Linux. Thư mục này biên dịch trực tiếp `apps/desktop/src/gui/application/gui_application.cpp`, `gui/runtime`, `gui/features` và `crossdesk_ui.slint` gốc. HarmonyOS PC chỉ duy trì điểm vào này; phiên ArkUI PC và chương trình kiểm chứng Slint cũ đã được loại bỏ.

**Hiện đây là bản phát triển chuyển nền tảng chạy được, chưa đạt tiêu chuẩn nghiệm thu mọi hành vi PC đồng nhất.** PC và di động dùng chung thông tin ứng dụng, tên gói thống nhất `com.chunkdash.crossdesk`, tên trên desktop là **CrossDesk**. Di động dùng dự án ArkUI riêng. Dữ liệu gói kiểm chứng PC cũ `cn.crossdesk.slintdesktop` không tự chuyển sang ứng dụng mới với tên gói chung.

## Ràng buộc đồng nhất

- Giao diện, bố trí, menu, xác nhận mật khẩu, lưu/hủy cài đặt, lịch sử, nhóm, Bức tường màn hình, thẻ phiên, máy trạng thái truyền tệp, đồng bộ bảng nhớ tạm và kết nối mạng đều từ mã máy tính chung. Thay đổi nghiệp vụ nên sửa trong mã chung, không được viết lại bộ hành vi riêng trong ArkTS.
- ArkTS chỉ phụ trách cửa sổ hệ thống, XComponent, sự kiện phím/chuột, bộ gõ, bảng nhớ tạm, bộ chọn tệp, cấp quyền thư mục, khay hệ thống và mở liên kết.
- Lớp nền tảng C++ triển khai cửa sổ/kết xuất, âm thanh, ghi màn hình và chèn nhập. `include/SDL3` riêng chỉ thích ứng dịch vụ SDL mà mã chung thực dùng, không phải bản chuyển SDL đầy đủ và không được dùng cho đích khác.
- Tọa độ màn hình, cửa sổ và bộ gõ dùng màn hình thực, vùng làm việc và tỷ lệ từng cửa sổ của hệ thống. Thuộc tính không viền Slint ánh xạ trực tiếp sang cửa sổ gốc; thuộc tính luôn trên hiện chỉ bảo đảm thứ bậc cửa sổ con trong ứng dụng.
- Mọi đối tượng, sự kiện, timer và kết xuất Slint chạy trên cùng luồng GUI. Dịch vụ hệ thống xử lý qua luồng ArkTS. Thoát sẽ hủy chờ, chuyển cửa sổ giữ phiên hiện có.
- Harmony cũng định nghĩa `__linux__`. Mã chung chỉ loại `__OHOS__` ở nơi dùng X11/Wayland, cửa sổ Linux gốc và các phần tương tự; mã POSIX và TLS tiếp tục dùng lại.

## Biên dịch và chạy cục bộ

Thích ứng desktop hiện dùng hành vi [cửa sổ con độc lập](https://developer.huawei.com/consumer/cn/doc/HarmonyOS-Guides/subwindow-guide) API 26, yêu cầu HarmonyOS API 26; di động vẫn tương thích API 20. Sản phẩm hiện là arm64 và đã được kiểm chứng trên giả lập 2in1 cục bộ.

```bash
# Biên dịch MiniRTC, Slint runtime, ứng dụng máy tính chung, HAP; cài vào giả lập đang chạy
python3 apps/harmony/scripts/build_slint_desktop.py --desktop --install

# Biên dịch tăng dần sau đó; đổi bản vá runtime vẫn tự biên dịch lại
python3 apps/harmony/scripts/build_slint_desktop.py --desktop --reuse-runtime --install

# Hồi quy ghi hình/nhập gốc, wire, thích ứng nền tảng Slint
python3 apps/harmony/tests/run.py --desktop
```

`--device` chỉ định thiết bị HDC. `--version` đặt cùng số phiên bản với bản máy tính khác, mặc định `0.0.0`, nên gói phát triển sẽ có lời nhắc bản mới hiện có. Khi không có tệp phát hành HarmonyOS, mục cập nhật mở trang chủ, không được tải gói Linux.

Cần Slint compiler 1.17.1 trên máy chủ, thư viện chuẩn Rust OHOS và DevEco SDK. Định vị bộ công cụ và yêu cầu tải lần đầu xem [biên dịch HarmonyOS](../README.md), tùy chọn ký xem dưới. Nguồn Slint/httplib đều cố định phiên bản và SHA-256. Dự án sinh ở `.native/slint-poc/desktop-project`, mở được bằng DevEco Studio. HAP tại `entry/build/default/outputs/default/entry-default-unsigned.hap`; cài máy thật vẫn cần chữ ký debug riêng.

## Biên bản kiểm chứng (2026-10-15)

Sau khi tinh gọn dự án, `tests/run.py --desktop --mobile`, HAP PC mặc định, HAP PC khai mọi quyền tùy chọn và biên dịch di động đầy đủ đều được chạy lại thành công. Kiểm gói xác nhận PC chỉ hỗ trợ `2in1`, di động chỉ `phone/tablet`, mỗi bên chỉ chứa ba thư viện gốc cần thiết; PC không có thư viện kiểm chứng cũ hay renderer video di động. Các tương tác thiết bị dưới đây từ lần kiểm chứng trước; đợt tinh gọn này không lặp phiên đầu cuối trên máy thật hoặc giả lập.

| Phạm vi | Bằng chứng / trạng thái |
| --- | --- |
| Ứng dụng máy tính chung | Biên dịch chéo toàn bộ C++, không có symbol chưa phân giải; HAP qua kiểm ELF arm64, SONAME và phụ thuộc động |
| Khởi động và đăng nhập | Giả lập 2in1 cục bộ khởi động, kết nối tín hiệu với xác minh CA hệ thống, khởi động lại khôi phục danh tính và kết nối gần đây |
| Phiên từ xa thực | Kết nối máy MiniRTC thử được điều khiển; hộp mật khẩu chung, cửa sổ phiên, thanh công cụ và ảnh thử NV12 động hoạt động đúng. Máy từ xa thực nhận luồng control / keyboard / mouse và sự kiện ngắt sau khi đóng phiên |
| Nhập tiếng Trung | Chọn hết/thay ô bí danh kết nối; chọn “你好” (xin chào) từ gợi ý Pinyin hệ thống `nihao`, gửi vào ô Slint gốc |
| Bàn phím | Bổ sung scan code, bit mở rộng, phím bổ trợ trái/phải và F13–F24. Dùng đối soát trạng thái phím chung: nhấn bổ trợ trước, nhả phím thường trước. Kiểm host đạt cho Ctrl trái/phải, hai Shift, snapshot đảo thứ tự, phím lặp, thử lại khi nhả lỗi và dọn khi mất tiêu điểm |
| Đo nhập ở máy được điều khiển | Máy điều khiển MiniRTC Mac gửi nhập thật qua mạng: kéo cửa sổ HarmonyOS, Ctrl+A thay văn bản, Shift phải+mũi tên chọn rồi xóa, Ctrl phải và hai Ctrl nhả trái trước vẫn giữ tổ hợp đều đạt. Ngắt khi đang giữ Ctrl phải rồi nhập cục bộ bình thường, không kẹt bổ trợ |
| Phím tắt bảng nhớ tạm | Trong HAP cuối, Ctrl+A/C, End, Ctrl+V, Ctrl+A/X, Ctrl+V từ xa sao chép đầy đủ `987` thành `987 987`, cắt rồi dán lại phục hồi. Bổ sung hồi quy để đồng bộ nền không chiếm quyền dán của người dùng |
| Cửa sổ | Bức tường màn hình dùng cửa sổ riêng, vào/thoát toàn màn hình khôi phục đúng; đóng chính rồi mở lại từ điểm vào giữ tiến trình và đăng nhập |
| Dựng lại cửa sổ | Di chuyển Bức tường màn hình trong giả lập, đóng/mở lại giữ vị trí và kích thước. Sửa khối đen do đệm vẽ cục bộ bị xóa sau khi dựng lại XComponent |
| Thích ứng màn hình | Đã nối sự kiện thêm/bớt màn hình, đổi độ phân giải và vùng làm việc. Kiểm host đạt cho tọa độ âm, ID không liên tục, ID chính 0, vùng khả dụng và chọn màn gần nhất. Nhiều màn ngoài và tỷ lệ trộn vẫn cần máy thật nghiệm thu |
| Thử thích ứng được điều khiển | Biên dịch trực tiếp `devices.cpp` sản xuất, mô phỏng API hệ thống để kiểm Display1/Display2, quay về khi lỗi, giữ/khôi phục số khi rút màn, tốc độ ghi, tạm dừng cảnh riêng tư và từ chối nhập khi quyền hết hiệu lực |
| Hồi quy ghi gốc | Biên dịch trực tiếp `capture.cpp` sản xuất, mô phỏng cấp quyền bất đồng bộ. Kiểm không gửi trước khi bắt đầu, ưu tiên GPU/quay về khi lỗi, xả hàng Surface CPU sau gộp thông báo, chờ/quá hạn/nhả fence, cảnh riêng tư, dừng/phục hồi, phạm vi âm thanh, dừng lặp và dọn khởi tạo lỗi. Tắt micro không ảnh hưởng video/nhập |
| Giả lập được điều khiển thực | Sau khi đổi đầu ra Surface hệ thống và đọc texture GLES, máy điều khiển Mac thực giải mã màn hình 3120×2080. Đã kiểm nền, cửa sổ Slint, văn bản, màu và hướng; di cửa sổ vẫn cập nhật liên tục. Cùng tiến trình HarmonyOS ngắt, cấp lại quyền rồi nối lại vẫn nhận ảnh đúng; sau ngắt, thanh được điều khiển mất, Bức tường vẫn tạo/vẽ được. Chọn “Quay lại” trong hộp quyền trả hủy và số khung giải mã 0; ngắt khi chờ quyền cũng dọn đúng |
| Trạng thái chuột | Kiểm nhả đạt khi nhiều nút cùng nhấn, kéo ngoài biên, hủy và mất tiêu điểm lặp. Chuyển màn nhả phím/chuột màn cũ rồi ánh xạ sang phạm vi pixel đích |
| Bức tường màn hình | Trạng thái rỗng chung, nhập thiết bị từ gần đây, xác nhận mật khẩu, kết nối một thiết bị và xem trước video động đều đạt; nhiều phiên hàng loạt chưa nghiệm thu đầy đủ |
| Khay hệ thống | Đã nối khay PC gốc; đóng chính giữ tiến trình, chuột trái khôi phục, menu mở cài đặt chung và thoát đều đạt. Thoát dọn kết nối chung và bỏ biểu tượng |
| Âm thanh | Âm thử đơn kênh 48 kHz tổng hợp được MiniRTC giải mã rồi đưa vào OH_AudioRenderer; sau nối tự phát, tắt tiếng dừng, bỏ tắt thì phục hồi. Chưa nghiệm thu nghe loa máy thật |
| Tệp | Bộ chọn hệ thống gửi tệp nhị phân 1 MiB, máy từ xa so từng byte khớp. Nhận tệp UTF-8 vào thư mục mặc định của ứng dụng và thư mục công khai người dùng chọn; khởi động lại tiến trình vẫn nhận được mà không chọn lại thư mục |
| Hộp thoại và tiêu điểm | Sửa sập do thư mục ban đầu rỗng; bộ chọn gắn cửa sổ khởi thao tác, hủy về phiên cũ. Thoát từ khay khi bộ chọn chưa đóng vẫn hủy chờ và dọn phiên xong |
| Hồi quy macOS | Chương trình smoke Slint chung biên dịch, liên kết và chạy thành công. Thêm hồi quy nhập cách quãng rồi Ctrl+A thay, trước sửa lỗi, sau sửa đạt. Chưa hoàn tất hồi quy liên kết toàn ứng dụng macOS |

Ô mật khẩu giữ điều khiển văn bản rõ của máy tính chung. Khi nhập mật khẩu tiếng Anh, bạn hãy chuyển bộ gõ hệ thống sang tiếng Anh hoặc gửi xong văn bản đang ghép trước. Hồi quy tự động cũng bắt buộc kiểm soát bộ gõ, không được coi gợi ý Pinyin chưa gửi là mật khẩu đã nhập.

Thử tệp dùng máy MiniRTC tổng hợp độc lập và tệp thử, không ghi desktop máy này. Nhận vào thư mục mặc định và công khai đều được kiểm nội dung UTF-8. Tệp gửi có 1,048,576 byte, SHA-256 là `fbbab289f7f94b25736c58be46a994c441fd02552cc6022352e3d86d2fab7c83`. Biên bản và tệp tạm đã dọn, kết nối cũ được giữ.

Tích hợp được điều khiển chỉ chia desktop giả lập HarmonyOS, tạm lưu khung giải mã ở máy điều khiển để kiểm pixel thực rồi dọn khi xong; không ghi desktop trạm làm việc. Callback dữ liệu thô trên giả lập cục bộ chỉ sinh âm thanh, đệm video Surface ánh xạ CPU cũng gần toàn đen. Sau khi dùng `StartScreenCaptureWithSurface` với texture ngoài NativeImage, ma trận biến đổi V2, đọc GLES và chuyển NV12, máy điều khiển hiển thị được desktop thật. Giữ Surface CPU và callback thô làm dự phòng khi khởi tạo thất bại. Đặt tốc độ ghi tuân [ràng buộc AVScreenCapture](https://github.com/openharmony/docs/blob/master/en/application-dev/reference/apis-media-kit/capi-native-avscreen-capture-h.md), thực hiện sau thông báo khởi động bất đồng bộ thành công.

Texture ghi và EGL context được tạo, dùng, nhả trên luồng riêng; NativeImage giữ cửa sổ producer để tránh nhả lặp. Dừng ghi chỉ hủy tài nguyên của mình, không kết thúc EGL display chung Slint/ArkUI. Giao diện, phiên, mã hóa/gửi và dọn kết nối vẫn dùng logic máy tính chung. Kéo từ xa trước bị quản lý cửa sổ từ chối vì MOVE mang `BUTTON_NONE`; sau khi mang nút thực đang nhấn, cửa sổ giả lập di chuyển và trở lại được. Hỏi vị trí con trỏ toàn hệ thống vẫn trả lỗi khả năng, được xử lý riêng với chèn nhập đã kiểm chứng.

Bộ gõ HarmonyOS có thể chiếm tổ hợp Ctrl từ bàn phím vật lý. Trong `onKeyPreIme`, `KeyboardHost` chuyển Ctrl/Meta và tổ hợp sang phân phối phím Slint cũ; văn bản thường và Ctrl+Space giữ đường bộ gõ hệ thống. Callback chọn hết, cắt, sao, dán, di chuyển và vùng chọn của bộ gõ cũng chuyển tới Slint TextInput. Timer định dạng ID chung đổi sang bật/tắt bằng thuộc tính `running`, tránh timer gốc lặp liên tục xóa vùng chọn. Bản sửa cũng áp dụng cho nền tảng máy tính khác.

Quyền đọc bảng nhớ tạm do Ctrl+V hệ thống cung cấp chỉ dùng một lần. Dán tường minh dùng yêu cầu `clipboardPaste` riêng; đồng bộ nền kiểm `READ_PASTEBOARD` trước để polling bảng nhớ chung không chiếm quyền dán. Dịch vụ bảng nhớ chạy tuần tự, đọc thất bại không giả thành chuỗi rỗng thành công. Đã nối thông báo `SystemPasteboard.update`; cửa sổ ẩn/mất tiêu điểm vẫn gửi `SDL_EVENT_CLIPBOARD_UPDATE` tới máy tính chung, còn loại trùng, giới hạn dung lượng và lọc phiên tiếp tục dùng `ClipboardController`. Đọc nền dùng đệm vô hiệu theo phiên bản thay đổi; đổi trong lúc đọc thì đọc lại, rút quyền xóa đệm và từ chối kết quả. Dán tường minh không vào đệm nền. Gói debug thường hiện chưa có ACL ký `READ_PASTEBOARD`, nên chưa nghiệm thu quyền đồng bộ nền xuyên ứng dụng trên máy thật. Giới hạn xem [bảng nhớ hệ thống](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides-v5/use_pasteboard_to_copy_and_paste-V5).

## Quyền nhập toàn cục và bảng nhớ tạm nền

`KeyboardHook` dùng `OH_Input_AddKeyEventHook` API 21, không dùng nghe cửa sổ cục bộ hay đăng ký phím nóng. `SessionDeviceManager` chung quyết vòng đời bắt phím; callback gốc chỉ đưa vào hàng phím chung. Mất tiêu điểm, rút quyền hoặc hủy sẽ nhả trạng thái bắt; phím không rõ và sự kiện nhả không được bắt trả lại hệ thống. Gỡ hook thất bại thì callback chỉ cho qua, không truy cập phiên đã hủy. Chưa cấp quyền vẫn dùng sự kiện cửa sổ Slint cũ; nhận thay đổi quyền sẽ tự thử lại, không thử lại lỗi định kỳ làm ngắt tổ hợp dự phòng người dùng đang giữ.

`PermissionDefinitions.json` của SDK API 26 máy này định nghĩa `HOOK_KEY_EVENT` là `system_basic / manual_settings` cho tablet, 2in1; `READ_PASTEBOARD` là `system_basic / user_grant`. Cả hai cần ACL đã duyệt trong profile ký; khai báo mã không thay được cấp quyền chữ ký. `DesktopAccess` xử lý tuần tự giao diện cấp quyền, mỗi lần chạy chỉ hướng dẫn một lần, người dùng hủy không lặp hộp, đồng thời nghe thay đổi quyền của mình. Bàn phím dùng `openPermissionOnSetting`, bảng nhớ dùng `requestPermissionsFromUser`. Gói thường không khai hai quyền, cũng không mở trang cấp quyền không dùng được.

Sau khi có profile được duyệt, xuất một cấu hình `signingConfigs` DevEco ra JSON riêng ngoài kho; `material.certpath`, `material.storeFile` và `material.profile` dùng đường tuyệt đối:

```sh
python3 apps/harmony/scripts/build_slint_desktop.py --desktop --reuse-runtime \
  --desktop-permissions --signing-config /absolute/private/signing.json --install
```

`--desktop-permissions` sinh khai báo quyền, giải thích mục đích và công tắc luồng cấp quyền. `--signing-config` sinh cấu hình ký và chọn cài `entry-default-signed.hap`. Cấu hình sinh chỉ ở đệm `.native` đã bỏ qua, quyền tệp `0600`. Không tham số ký vẫn xuất `entry-default-unsigned.hap` thường. Bạn có thể dùng riêng `--desktop-permissions` để kiểm biên dịch có khai quyền, nhưng không được vì vậy coi là đã có quyền hoặc cài máy thật được.

Lệnh từ xa `lock_workstation` đã nối đường Meta+L có kiểm quyền ghi/nhập: nhả phím/chuột dư trước, gửi rồi nhả theo cặp, thất bại giữ trạng thái chờ dọn. Windows `send_sas` không có API HarmonyOS công khai tương đương và trả rõ không hỗ trợ. PC/di động chỉ hiển thị nút tổ hợp hệ thống khi máy được điều khiển là Windows, ẩn nếu HarmonyOS hoặc hệ khác. Tổ hợp Ctrl/Alt/Meta thường được hook gốc chuyển đúng thứ tự; hệ thống cuối cùng cho chặn phím dành riêng nào vẫn cần kiểm máy thật sau cấp quyền.

Biên bản: HAP thường và HAP khai quyền biên dịch thành công. Cài gói thường trên giả lập 2in1 cục bộ, hook toàn cục thực trả `201`, không lặp hộp quyền, vẫn nhận luồng `keyboard` từ đường dự phòng cửa sổ chung. Ctrl+A/C, End, Ctrl+V thật sao `246` thành `246 246`. Hồi quy SDK mô phỏng C++/ArkTS sản xuất gồm Alt+Tab, Meta+L, phím lặp, Enter bàn số, hủy, mất tiêu điểm, rút quyền, đăng ký lặp, gỡ lỗi, thông báo nền, đệm hết hiệu lực, đổi/rút quyền khi đọc và không lặp hướng dẫn sau từ chối. Smoke Slint chung macOS thêm nhấn thật menu phím tắt, kiểm sau bỏ SAS mục đầu là khóa màn hình, vùng trúng co theo menu và đổi lại Windows vẫn đúng. Không tuyên bố hook đã cấp quyền hoặc đọc nền xuyên ứng dụng được kiểm thành công trong gói không ACL.

<a id="permissions"></a>

## Quyền bền vững cho máy được điều khiển (API 26)

Đã nối luồng cấp quyền một lần trong cài đặt hệ thống rồi tái dùng cho các kết nối sau. Luồng giải quyết xác nhận ghi màn hình và chuột/phím lặp khi nối lại trong lúc ứng dụng đang chạy; tự khởi sau reboot, điều khiển trước đăng nhập, khóa và phục hồi sau ngủ vẫn cần nghiệm thu riêng. Đã ký/cài giả lập bằng Profile debug chứa hai ACL này, nhưng chưa hoàn tất lần cấp quyền đầu trong cài đặt máy này hay kiểm máy thật nối lại không xác nhận.

| Quyền | Mục đích | SDK yêu cầu |
| --- | --- | --- |
| `ohos.permission.CUSTOM_SCREEN_RECORDING` | Quyền ghi bền, ghi màn hình chỉ định không cần xác nhận lặp | Từ API 22, `system_basic / manual_settings` |
| `ohos.permission.CONTROL_DEVICE` | Chèn chuột/phím trực tiếp, không xin nhập tạm mỗi lần | Từ API 26, `system_basic / manual_settings`, PC/2in1 |

Cả hai quyền cần ACL của Profile ký và người dùng cấp trong cài đặt hệ thống. `CUSTOM_SCREEN_CAPTURE` là quyền chụp ảnh, không thay quyền ghi màn hình trên. `HOOK_KEY_EVENT` là quyền bắt phím tắt bên điều khiển, cũng không thay `CONTROL_DEVICE` ở máy được điều khiển.

Dùng tùy chọn riêng `--host-permissions` để sinh khai báo, giải thích mục đích và hướng dẫn cài đặt, không yêu cầu xin đồng thời hook bàn phím toàn cục hoặc quyền bảng nhớ:

```sh
# Sinh dự án DevEco và biên dịch gói chưa ký để xin quyền/kiểm biên dịch.
python3 apps/harmony/scripts/build_slint_desktop.py --desktop --reuse-runtime --host-permissions
```

### Xin quyền và ký

1. Mở `apps/harmony/.native/slint-poc/desktop-project` trong DevEco Studio. PC và di động đều có bundle name `com.chunkdash.crossdesk`; ứng dụng AGC, Profile ký và tên gói bắt buộc khớp. Biên dịch đọc tên gói từ `AppScope/app.json5` chung, không hỗ trợ ghi đè riêng cho PC.
2. Trong **File > Project Structure > Project > Signing Configs**, đăng nhập tài khoản Huawei developer của bạn và liên kết ứng dụng đã đăng ký AGC. Ở DevEco Studio hỗ trợ quy trình này, dùng **Enable ACL Permissions** xin hai quyền trên cho `entry / EntryAbility`. Chọn thời điểm gọi `always` vì thu nhỏ vào nền vẫn cần nhận kết nối đã xác thực. Nếu không có `CONTROL_DEVICE`, kiểm hỗ trợ API 26 của IDE/SDK và xác nhận phạm vi mở quyền trên thẻ “Quyền ACL” của dự án AGC. Phía web cần lấy ACL trước rồi thêm vào Profile mới sinh.
3. Điền mục đích và lý do riêng cho từng quyền, gửi tài liệu trình diễn chức năng thực. Bạn có thể dùng phần dưới làm nháp rồi sửa theo tình huống phát hành thực tế:

   - `CUSTOM_SCREEN_RECORDING`: CrossDesk cung cấp chức năng desktop từ xa được điều khiển cho HarmonyOS PC. Sau khi chủ thiết bị chủ động cấp quyền trong cài đặt máy này, ứng dụng chỉ ghi màn hình chỉ định và truyền hình khi kết nối từ xa qua xác minh mật khẩu, phiên còn hợp lệ; ngắt sẽ dừng ghi. Xin quyền bền vững để chủ truy cập khi rời thiết bị, tránh yêu cầu xác nhận tại chỗ mỗi kết nối.
   - `CONTROL_DEVICE`: Trong phiên từ xa đã xác thực và ghi hình hợp lệ, chuyển thao tác từ xa thành nhập chuột/phím cục bộ. Khi dừng chia sẻ, hệ thống vào cảnh riêng tư hoặc quyền hết hiệu lực, ứng dụng không nhận nhập từ xa. Người dùng có thể rút quyền trong cài đặt hệ thống.

4. Kiểm ACL của Profile sinh thực sự chứa hai quyền. Giai đoạn debug, quy trình ký tự động chính thức có thể cấp Profile tạm; hạn và quyền theo kết quả cấp thực tế. Giai đoạn phát hành cần hoàn tất xin quyền chính thức và Profile phát hành. Chủ tài khoản/thiết bị cần thực hiện đăng nhập, nộp đơn và cấp quyền trong cài đặt.
5. Lưu một đối tượng `app.signingConfigs` từ `build-profile.json5` dự án ra JSON riêng ngoài kho, không kèm mảng bọc. Giữ trường ký gốc; `material.certpath`, `material.storeFile` và `material.profile` đều dùng đường tuyệt đối. Script biên dịch sẽ sinh lại cấu hình dự án, nên xuất đối tượng trước khi chạy script lại.

```sh
python3 apps/harmony/scripts/build_slint_desktop.py --desktop --reuse-runtime \
  --host-permissions --signing-config /absolute/private/signing.json \
  --device YOUR_DEVICE_ID --install
```

Profile bắt buộc ứng với `com.chunkdash.crossdesk`, không cần thêm `--bundle-name`. Tùy chọn này chỉ giữ tương thích lệnh cũ và chỉ nhận tên gói chung. Nếu đích đã có ứng dụng cùng tên gói, script thử cập nhật, cần thỏa điều kiện chữ ký/phiên bản. Script chỉ dùng cấu hình ký truyền tường minh, không tự kế thừa chữ ký nguồn. ACL DevEco ghi vào nguồn cũng sinh lại theo công tắc lần biên dịch này, tránh khai lặp hoặc vô tình bật quyền trong bản thường.

Nếu cần cả phím tắt bên điều khiển và đồng bộ bảng nhớ nền, có thể thêm `--desktop-permissions`; Profile khi đó còn cần hai ACL tương ứng. Biên dịch gói chưa ký thành công không có nghĩa có quyền hay cài máy thật được.

Căn cứ điểm xin và quy trình: [xin quyền hạn chế Huawei](https://developer.huawei.com/consumer/cn/doc/doccenter-capabilities/declare-permissions-in-acl), [ký tự động và ACL DevEco](https://developer.huawei.com/consumer/cn/doc/doccenter-deveco-studio/ide-signing-auto).

### Hành vi chạy và nghiệm thu

Khi cửa sổ chính hiện lần đầu, bản bật `--host-permissions` gộp lời nhắc các công tắc “Chia sẻ màn hình” và “Trợ năng” còn thiếu. Nhấn “Mở cài đặt” sẽ mở trang chi tiết hệ thống của ứng dụng này. Có đủ hai quyền thì không hiện hộp; thiếu một thì chỉ giải thích quyền đó. Chọn “Chưa bật” hoặc đóng hướng dẫn sẽ không mở lại trong lần chạy này; bạn có thể tự vào hệ thống bật hoặc khởi động lại để hướng dẫn lại. Bản thường không khai hai quyền và không mở cài đặt tương ứng.

Đường “Quyền riêng tư và bảo mật > Trợ năng” trong hộp `openPermissionOnSetting` của PC không khớp trang cài ứng dụng thực, mà ứng dụng không có API sửa lời hệ thống. Vì vậy, quyền được điều khiển chuyển sang hướng dẫn một lần qua `UIContext` của cửa sổ, rồi mở `application_info_entry` theo [ví dụ mở cài đặt ứng dụng Huawei](https://developer.huawei.com/consumer/cn/doc/doccenter-dev-faq/faqs-access-control-16). `pushParams` lấy tên gói thực của ứng dụng đang chạy, tương thích gói kiểm chứng mặc định và gói đã ký. Nếu chuyển trang thất bại, ứng dụng chỉ cách tự tìm ứng dụng/công tắc, không về hộp hệ thống có đường sai. Chỉ mở cài đặt không được coi là có quyền; ứng dụng vẫn đọc trạng thái cấp quyền hệ thống.

Trước khi khởi động và sau khi cài đặt đổi, ArkTS đều đọc quyền thực hệ thống rồi đồng bộ sang C++. Chỉ cho nhập khi “ghi hình hợp lệ và có `CONTROL_DEVICE` hoặc đã có quyền nhập tạm”. Không được chỉ hỏi `OH_Input_QueryAuthorizedStatus`: API 26 nói rõ API này chỉ phản ánh quyền qua hộp thoại, không phản ánh `CONTROL_DEVICE`. Xem [Input Kit](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md#oh_input_requestinjection).

Khi có `CUSTOM_SCREEN_RECORDING`, thiết lập ghi màn hình chỉ định không mở bộ chọn nội dung, hệ thống kiểm quyền có cho khởi động không xác nhận hay không; nếu chưa có, luồng cấp quyền cũ được giữ. Hành vi theo SDK máy này và [`IsSkipPrivacyWindow` của dịch vụ ghi OpenHarmony](https://github.com/openharmony/multimedia_player_framework/blob/master/services/services/screen_capture/server/screen_capture_server.cpp), cần kiểm firmware HarmonyOS đích. Mọi đường đều đợi `STARTED` rồi mới gửi hình. Ngắt vẫn dừng ghi, nhả phím/chuột và rút quyền nhập tạm, không duy trì quyền bằng ghi liên tục.

Hồi quy host bao phủ nhập khi quyền bền có nhưng trạng thái quyền tạm vẫn `UNAUTHORIZED`, ngắt/nối lại, rút quyền, cảnh riêng tư, tuần tự cài đặt, hủy không lặp hướng dẫn, thoát không mở thiết lập chờ và quay về khi chính sách ghi thất bại. Nghiệm thu máy thật cần gói ký có ACL tương ứng, lần lượt kiểm cấp đầu, hai lần nối lại, thoát/mở, rút riêng từng quyền rồi cấp lại. Khóa/đánh thức và sau reboot phải ghi riêng, không thay bằng thử SDK mô phỏng.

Kiểm chứng 2026-10-15: `python3 apps/harmony/tests/run.py --desktop` đạt; bản thường và bản `--host-permissions` đều qua ArkTS, liên kết gốc và kiểm HAP. Script kiểm khai báo quyền trong gói khớp công tắc chọn.

Kiểm chữ ký 2026-10-15: dùng Profile debug `com.chunkdash.crossdesk` DevEco cục bộ sinh, xác nhận ACL chứa hai quyền và đã đăng ký giả lập hiện tại. HAP ký qua `hap-sign-tool verify-app` chính thức, gồm kiểm chữ ký mã và quyền, rồi cài/chạy thành công trên giả lập 2in1 API 26. Hệ thống đọc được hai quyền nhưng chưa bật cấp đầu trong cài đặt; không vì vậy tuyên bố nối lại không xác nhận đã đạt. Gói giao ở `.native/artifacts/acl-host/crossdesk-acl-signed.hap`. Đồng thời kiểm bản thường không kế thừa chữ ký nguồn hay ACL tùy chọn.

Sửa hướng dẫn 2026-10-15: hồi quy gồm gộp hai công tắc, chỉ thiếu một, đủ quyền không nhắc, hủy không chuyển/không lặp, định tuyến tên gói thực, callback quá hạn sau thoát, chuyển trang lỗi và mở cài đặt không đồng nghĩa có quyền. Hồi quy host `--desktop` đầy đủ và biên dịch ký ACL đạt; bật quyền thực và nối lại từ xa vẫn cần thiết bị nghiệm thu.

## Cửa sổ và nhập liệu không ACL (2026-10-15)

Tiếp tục dùng cùng giao diện Slint, Bức tường màn hình, phiên và logic chỉnh sửa chung. Đợt sửa này tập trung lớp thích ứng cửa sổ/sự kiện nhập HarmonyOS, không thêm tương tác sản phẩm riêng cho nền tảng.

- Trạng thái phóng/khôi phục của thanh tiêu đề gốc được gửi về Slint thành xác nhận đầy đủ. Setter Slint gọi lại nền tảng đồng bộ; `WindowMode` ngăn trạng thái trung gian trong xác nhận điều khiển lại cửa sổ hệ thống, sửa vòng vừa khôi phục đã phóng lại. Sự kiện thuộc tính chỉ mang chế độ thực sự đổi; phiên bản trạng thái lọc lệnh trễ, kể cả đổi trạng thái gốc trong lúc đợi dịch vụ tiêu đề/kích thước.
- Thu nhỏ giữ chế độ toàn màn hình/phóng cũ. Cửa sổ chính phục hồi bằng `restore()`, cửa sổ con bằng `showWindow()`; hiển thị lặp không đặt lại kích thước khôi phục chính. Từ toàn màn hình về phóng thường dùng `EXIT_IMMERSIVE` tường minh. API xem [tham chiếu cửa sổ chính thức](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-window).
- Ẩn/dựng lại cửa sổ con sẽ gỡ listener, lọc callback đóng, mất tiêu điểm, kích thước và trạng thái cũ theo instance cửa sổ gốc. Sự kiện XComponent mang ID surface; hủy trễ và nhập trên surface cũ không tác động surface mới. Tạo thất bại dọn phần dở, hàng lệnh cửa sổ hoàn tất rồi giải phóng; lỗi dịch vụ tiêu đề/thứ bậc không còn chặn xử lý kích thước/chế độ.
- Bộ gõ thêm số phiên riêng. Khi đổi ô nhập trong cùng cửa sổ, callback/cập nhật cũ và attach đang chờ không được ghi editor mới; phân phối gốc kiểm lại số phiên. Hủy cửa sổ vô hiệu callback bộ gõ ngay và nhả kết nối sau khi tác vụ bất đồng bộ xong.
- Sự kiện bàn phím vật lý mang snapshot CapsLock/NumLock hệ thống và Unicode ASCII in được do SDK cấp. Bàn số ánh xạ phím điều hướng khi NumLock tắt, Alt phải ánh xạ riêng thành Slint AltGr. Lặp và nhả tiếp tục dùng ký hiệu lần nhấn đầu. Điều này không khẳng định bao phủ bố cục phi ASCII mà SDK chưa cung cấp hoặc toàn bộ hành vi bộ gõ.

Kiểm chứng: thêm hồi quy thích ứng cửa sổ ArkTS sản xuất cho khôi phục/toàn màn hình, lệnh hết hạn, dựng lại, callback trễ, lỗi dịch vụ tùy chọn và dọn tạo thất bại. C++ kiểm ghi lại trạng thái Slint đồng bộ, bộ gõ kiểm đổi ô cùng cửa sổ và hủy trong attach; mở rộng kiểm phím khóa, ký hiệu bố cục, Alt phải và bàn số. `python3 apps/harmony/tests/run.py --desktop` và HAP thường không ACL đạt. Giả lập 2in1 thực kiểm cửa sổ chính phóng rồi về kích thước cũ, thu nhỏ rồi kích hoạt, Bức tường vào/thoát toàn màn hình và đóng/dựng lại. Surface mới tiếp tục vẽ, không còn trạng thái qua lại. Gói cuối kiểm nhập thường cùng Ctrl+A/C, End, Ctrl+V sao `246` thành `246 246`, sau đó xóa nội dung thử và khôi phục vị trí cửa sổ cũ.

## Hồi quy bung bảng trạng thái mạng (2026-10-15)

Đã tái hiện lỗi nhấn biểu tượng mạng rồi giữ chuột trên nút thì bảng không bung, rời chuột mới bung một phần. `GuiApplication` chung gọi `set_vector` cho mô hình thống kê mỗi 16 ms; dù số dòng không đổi, phụ thuộc số dòng vẫn bị đặt lại và hoạt ảnh chiều cao thanh điều khiển khởi lại liên tục. Sửa thành chỉ reset mô hình khi số dòng đổi, thời gian còn lại chỉ cập nhật dòng đổi, giữ bố trí Slint và hoạt ảnh 60 ms chung các PC.

Thêm `slint_stream_render_test` dùng timer Slint thật và software render với đệm giữ, liên tục làm mới thống kê, nhấn bung/thu nhiều lần ở bố trí neo trái/phải mà không di chuột, đồng thời kiểm pixel và vùng nhập đầy đủ. Trước sửa tái hiện lỗi, sau sửa cả nó và `slint_ui_smoke_test` đều đạt. HAP không ACL thường đã biên dịch/cài; giả lập 2in1 nối nguồn video tổng hợp xác nhận bung/thu không còn phải đợi chuột rời.

```bash
xmake build slint_stream_render_test
xmake run slint_stream_render_test
```

## Hồi quy nút và hộp thoại khác (2026-10-15)

Tiếp tục kiểm đường cập nhật mô hình và nhập hộp thoại của máy tính chung, sửa các lỗi sau; mọi PC vẫn dùng cùng thành phần/callback Slint:

- Mô hình màn hình và truyền tệp cũng chỉ reset toàn bộ khi số dòng đổi; còn lại chỉ cập nhật dòng thay đổi, tránh dựng lại menu/danh sách mỗi khung.
- Tiêu đề, biên và vùng trống bảng nổi truyền tệp trước chuyển nhấp/chuột lăn tới máy từ xa. Nay bảng chiếm toàn bộ vùng nhập cục bộ; kéo ra ngoài hoặc đúng lúc đóng đều không rò sự kiện nhả chuột từ xa.
- Sau khi mở dropdown mã hóa, trạng thái sang trong phiên sẽ vô hiệu cài đặt nhưng menu cũ vẫn đổi mã hóa được. `CompactComboBox` chung nay đóng menu và hủy timer bung khi bị vô hiệu; callback tùy chọn cũng kiểm tính hợp lệ hiện tại.

Thêm `slint_popup_render_test` chạy trực tiếp cửa sổ Slint chung, dùng đệm pixel giữ, sự kiện chuột/phím thật và timer cập nhật mô hình 16 ms. Kiểm gồm màn hình, phím tắt, video, truyền tệp, lọc nhóm, dropdown/thẻ cài đặt, tin nhắn, giới thiệu, mạng Bức tường, menu/hộp thêm, chọn bên điều khiển và xác nhận ngắt trên máy được điều khiển. Kiểm vẽ khi chuột còn ở vị trí kích hoạt, làm mới dữ liệu khi giữ nhấn, Esc/hủy đóng và nhập cục bộ không rò từ xa. Ca bảng tệp và dropdown vô hiệu đều lỗi trước sửa, đạt sau sửa.

Kiểm chứng: `slint_popup_render_test`, `slint_stream_render_test` và `slint_ui_smoke_test` đạt trên macOS cục bộ; HAP thường không ACL biên dịch/cài thành công. Giả lập 2in1 thực kiểm menu chính, dropdown và thẻ cài đặt, mạng Bức tường, menu/hộp thêm: nhấn giữ chuột tại chỗ vẫn hiện ngay, hủy/đóng thì phục hồi. Đợt này giả lập không nối thiết bị thật; làm mới liên tục trong phiên và chuyển trạng thái bảng được điều khiển do hồi quy Slint chung bao phủ, không có nghĩa đã nghiệm thu đầu cuối đầy đủ mọi nền tảng.

```bash
xmake build slint_popup_render_test
xmake run slint_popup_render_test
```

## Đồng bộ chuột máy được điều khiển (2026-10-15)

HarmonyOS được điều khiển trước bỏ qua `Start(show_cursor)` chung, luôn yêu cầu ghi chuột trong video và chỉ gửi `embedded`. Nay phiên PC gốc tắt chuột trong ghi màn hình, dùng `OH_Input_GetCursorInfo` đồng bộ hình ngữ nghĩa mũi tên, văn bản, liên kết, kéo, bận, đổi kích thước và trạng thái hiện/ẩn, dùng lại giao thức `cursor_state` và logic hiển thị có sẵn ở bên điều khiển macOS/Windows. Phiên cần chuột trong video vẫn dùng `show_cursor=true`. Chỉ báo vẽ riêng sau khi xác nhận bỏ chuột; bỏ thất bại thử chuột trong video, cả hai lỗi thì báo lấy mẫu không khả dụng. Chuyển màn hình, dừng/phục hồi và phục hồi cảnh riêng tư giữ chính sách này.

Hỏi vị trí ưu tiên `OH_Input_GetPointerLocation`; nếu hỏi nền không khả dụng, dùng sự kiện chuột thật nhận từ `OH_Input_AddMouseEventMonitor` khi ghi. Cả hai đổi tọa độ pixel trong màn và ID màn vật lý thành số màn đã thương lượng/vị trí chuẩn hóa, tương thích bố trí âm và màn chồng. Chỉ đăng ký nghe trong phiên ghi chuột riêng hợp lệ; tạm dừng, cảnh riêng tư, chuyển màn hoặc dừng hẳn sẽ xóa vị trí và gỡ nghe. Callback gỡ thất bại giữ vô hiệu. Không có truy vấn hay sự kiện hợp lệ thì không giả vị trí, nhưng giữ hình/trạng thái hiện đã lấy được.

Đồng bộ vị trí nền dùng **`ohos.permission.INPUT_MONITORING`**, với `system_basic / system_grant`, cần ACL tương ứng trong Profile ký. Quyền nằm trong [danh sách ACL ký tự động Huawei](https://developer.huawei.com/consumer/cn/doc/doccenter-deveco-studio/ide-signing-auto), nhưng vẫn phải thỏa tình huống ứng dụng và yêu cầu phê duyệt. Có ACL rồi, thêm `--cursor-position-permission` khi biên dịch để sinh/kiểm khai báo. Quyền độc lập với ghi màn hình và chèn nhập; bản thường hoặc bản đã có hai quyền được điều khiển không tự thêm ACL này. API theo [nghe chuột OpenHarmony Input Kit](https://github.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-input-kit/capi-oh-input-manager-h.md#oh_input_addmouseeventmonitor).

Đính chính: SDK API 26 máy này thực có `INPUT_DEVICE_CONFIGURATOR` và nói quyền có thể cho hỏi trực tiếp vị trí nền. Tuy nhiên, định nghĩa SDK và dấu quyền chứng chỉ không chứng minh quyền đã mở trên trang xin ACL HarmonyOS. Khuyến nghị xin trực tiếp trước đây dựa vào đó là chưa đủ; bản hiện tại không khai hoặc phụ thuộc quyền này nữa. `INPUT_MONITORING` dùng nghe sự kiện, không cấp quyền hỏi trực tiếp `OH_Input_GetPointerLocation`. Chưa có quyền nghe vẫn không bảo đảm vị trí nền; đồng bộ hình và trạng thái hiện chuột không phụ thuộc quyền này.

Hồi quy gọi mã thích ứng ghi và lấy mẫu chuột sản xuất, bao phủ ánh xạ hình, ẩn/không thiết bị, vị trí từ listener sau khi hỏi nền bị từ chối, từ chối quyền nghe, dọn khi dừng/chuyển, callback trễ, gỡ lỗi/thử lại, hệ thống đã gỡ listener, lấy mẫu lỗi, ánh xạ ID, tọa độ sai, giao thức khứ hồi, chính sách hiển thị chung và quay về chế độ ghi. `python3 apps/harmony/tests/run.py --desktop` đạt; HAP máy tính API 26 AArch64 có khai quyền nghe biên dịch đạt, kiểm gói chỉ khai `INPUT_MONITORING`, không `INPUT_DEVICE_CONFIGURATOR`. Vẫn cần HarmonyOS PC thật với quyền ký đã duyệt và máy điều khiển macOS/Windows nghiệm thu đầu cuối; SDK mô phỏng không thay việc kiểm dịch vụ chuột và quyền ký thật.

## Thiết lập tự khởi động khi bật máy (2026-10-15)

“Khởi động và chạy nền” trong cài đặt chung giữ kiểu công tắc tự khởi cũ. Trên HarmonyOS, công tắc hiện theo truy vấn hệ thống; nhấn sẽ mở hướng dẫn, không đổi trạng thái trực tiếp. Hướng dẫn có trạng thái chi tiết và “Làm mới trạng thái”. Mở cài đặt, trở về tiền cảnh hoặc kích hoạt lại cửa sổ gốc sẽ hỏi lại. Lỗi truy vấn, thiết bị không hỗ trợ và chưa bật hiển thị riêng. Quá 5 giây có thể thử lại; truy vấn cũ, phản hồi trễ sau quá hạn và callback sau thoát không ghi đè trạng thái mới. PC khác giữ hành vi công tắc cũ.

Người dùng HarmonyOS PC cần vào **Cài đặt hệ thống → Ứng dụng và dịch vụ nguyên tử → Quản lý khởi động ứng dụng → Thêm ứng dụng**, chọn **CrossDesk**, nhấn “Hoàn tất”; muốn tắt thì xóa khỏi danh sách đó. Ứng dụng cung cấp hướng dẫn đầy đủ, không dùng deep link cài đặt chưa kiểm chứng hay coi lưu cấu hình cục bộ là hệ thống đã bật. Các bước theo [hướng dẫn tự khởi máy tính Huawei](https://consumer.huawei.com/cn/support/content/zh-cn16048401/). API 26 công khai trong SDK máy này chỉ có `isAutoStartupSupported()` và `getAutoStartupStatusForSelf()`, không cần thêm ACL tự khởi. API hệ thống `setApplicationAutoStartup()` không áp dụng cho ứng dụng thường.

Sau khi khởi động, ứng dụng vẫn khôi phục danh tính/kết nối máy chủ qua `GuiApplication::InitializeModules()` và `GuiRuntime::CreateConnectionPeer()` chung; khay cũng dùng luồng khởi động cũ. Thiết lập tự khởi chỉ quyết hệ thống có mở ứng dụng hay không, không đồng nghĩa có quyền ghi, chèn nhập hoặc điều khiển trước đăng nhập.

Kiểm chứng: `python3 apps/harmony/tests/run.py --desktop` đạt. Hồi quy ArkTS sản xuất mới bao phủ đổi trạng thái, lỗi truy vấn, quá hạn, kết quả cũ, thoát tiến trình, các sự kiện khởi động/tiền cảnh/kích hoạt cửa sổ và lệnh refresh gốc. `slint_popup_render_test` và `slint_ui_smoke_test` biên dịch/chạy đạt trên macOS: nhấn khi công tắc bật/tắt đều mở hướng dẫn mà không đổi trạng thái, kiểm nút refresh, đổi trạng thái bất đồng bộ, bố trí cửa sổ nhỏ nhất, Esc/xác nhận đóng và nền tảng khác vẫn đổi công tắc trực tiếp. HAP thường không ACL và kiểm gói đạt, đã cài lên giả lập 2in1 API 26; khởi động tự nối máy chủ, công tắc hiện tắt, nhấn mở hướng dẫn hiện “Đã tắt”, refresh thủ công và quay về từ cài đặt hệ thống đều đúng. Không sửa danh sách tự khởi của giả lập; đổi bật/tắt được kiểm bằng SDK mô phỏng. Tự khởi hệ thống trên máy thật và tự trực tuyến sau reboot/đổi người dùng vẫn cần thiết bị nghiệm thu.

## Hạng mục chưa nghiệm thu

Không được đánh dấu các mục sau là hỗ trợ đầy đủ chỉ vì logic chung đã biên dịch:

- Máy thật được điều khiển: cấp, từ chối, rút quyền ghi/chèn nhập và phục hồi cảnh riêng tư; ghi âm thanh hệ thống. Đã có liệt kê nhiều màn gắn lúc khởi động, chuyển màn và ánh xạ nhập, nhưng giả lập chỉ một màn. Chỉ chèn nhập khi ghi hợp lệ và hệ thống cấp quyền.
- Nghe âm thanh trên máy điều khiển thật, kết nối dài, nhiều phiên và thao tác Bức tường hàng loạt. Hình từ xa hiện kiểm bằng nguồn thử tổng hợp.
- Hủy giữa truyền, tệp lớn/đầy đĩa, người dùng rút quyền thư mục và truyền đa tác vụ nền. Gửi, nhận cơ bản và quyền sau khởi động lại đã đạt.
- Máy thật reboot/đăng nhập sau tự khởi, dịch vụ hệ thống và màn riêng tư. Trạng thái tự khởi, hướng dẫn và làm mới khi quay lại đã nối; người dùng quản lý bật/tắt trong cài đặt hệ thống. Quyền ghi/nhập bền không giám sát vẫn cần máy thật có ACL nghiệm thu, xem trên. Khay gốc đã nối, chỉ bật khi hệ thống báo hỗ trợ.
- Nhiều màn ở máy được điều khiển: MiniRTC thương lượng DisplayN lúc khởi tạo. Rút màn đã thương lượng thì giữ số và về màn còn dùng được, cắm lại có thể khôi phục. Thêm màn khi đang chạy cần khởi tạo lại bên kết nối, hiện là khởi động lại ứng dụng; không gửi trực tiếp luồng chưa thương lượng vào phiên có sẵn. Chuyển màn bằng khởi ghi hệ thống lại, có thể bị hỏi quyền lại, chưa đồng nhất hoàn toàn với chuyển liền mạch trên PC khác.
- Cấp hook bàn phím toàn cục trên máy thật, định tuyến phím dành riêng ở các firmware PC, khóa Meta+L thực và đồng bộ bảng nhớ nền xuyên ứng dụng. Gói không ACL chỉ kiểm được từ chối/quay về; SDK mô phỏng thành công không thay nghiệm thu đầu cuối sau cấp quyền hệ thống.
- Cửa sổ/nhập: nhiều màn ngoài, tỷ lệ trộn, phục hồi vị trí/kích thước dài hạn, mọi phạm vi ghép của bộ gõ, bố cục phím phi US, phím tắt dành riêng hệ thống và lớp nổi luôn trên xuyên ứng dụng. Dịch vụ chuột không có ở giả lập sẽ báo thiếu khả năng.
- Nhập video GPU trực tiếp, hiệu năng và điện năng. Video hiện qua đường ảnh CPU Slint chung dự phòng rồi trình bày bằng GLES3; điện thoại/máy tính bảng dùng ứng dụng di động riêng.

## Vị trí mã

- `CMakeLists.txt`: tập nguồn máy tính chung và phụ thuộc gốc, chỉ biên dịch cầu nối `slint_desktop`.
- `font-discovery.patch`: tắt tự dò Fontconfig không phù hợp runtime OHOS; backend vẫn tải font tường minh.
- `../native/audio.*`: phát OHAudio chung cho PC/di động; PC không biên dịch renderer video di động.
- `DisplayHost.ets` / `displays.h`: màn hình/vùng làm việc hệ thống, đổi tọa độ toàn cục/trong màn, màn của cửa sổ và sự kiện cắm rút nóng.
- `window-properties.patch`: chuyển thuộc tính không viền/luôn trên của cửa sổ Slint gốc sang lớp HarmonyOS; bản vá chỉ tác động OHOS.
- `pointer.h`: theo dõi trạng thái nhấn chuột từng cửa sổ và nhả đầy đủ khi mất tiêu điểm/hủy.
- `desktop.cpp` / `surface.inc`: luồng Slint GUI, thích ứng cửa sổ và trình bày EGL/GLES3.
- `DesktopHost.ets` / `Index.ets`: cửa sổ, surface và phân phối sự kiện HarmonyOS gốc.
- `TextInputHost.ets` / `os-services.patch`: cầu nối bộ gõ/hình chuột còn thiếu trong backend C++ Slint 1.17.1; bản vá chỉ đổi đích OHOS.
- `keyboard_hook.cpp` / `DesktopAccess.ets`: hook phím hệ thống, thay đổi quyền và đường bắt phím chung dự phòng.
- `KeyboardHost.ets`: định tuyến phím tắt vật lý trước bộ gõ và nhả theo cặp, gọi xử lý phím Slint gốc.
- `services.cpp` / `devices.cpp`: dịch vụ nền tảng cho tệp, bảng nhớ tạm, âm thanh, ghi hình và nhập liệu.
- `../native/capture.cpp`: vòng đời ghi gốc, quyền bất đồng bộ và xử lý đệm âm thanh/video.
- `../native/capture_surface.cpp`: đọc NativeImage/GLES từ Surface ghi hệ thống, biến đổi hướng và quản lý tài nguyên luồng.
- `../native/input.cpp` / `key_mapping.h`: chèn phím/chuột được điều khiển, ánh xạ scan code mở rộng, đối soát snapshot và nhả sau rút quyền/ngắt.
- `FileHost.ets`: bộ chọn tệp hệ thống, snapshot đọc URI, quyền thư mục bền và kích hoạt khi tiến trình khởi động. Chưa chọn thư mục công khai thì nhận vào `files/desktop/Received Files` của ứng dụng; có thể chọn thư mục công khai trong cài đặt chung.
- `TrayHost.ets` / `tray.cpp`: nối sự kiện khay PC về callback hiện, cài đặt và thoát của máy tính chung.
- `AutostartHost.ets` / `platform/harmony/harmony_autostart.h` trong thư mục nguồn máy tính: hỏi trạng thái tự khởi hệ thống bất đồng bộ, cung cấp trạng thái chỉ đọc và cầu refresh cho cài đặt chung.
- `platform/harmony/gui/window_state.h` trong thư mục nguồn máy tính: trạng thái kích hoạt cửa sổ gốc cho logic âm thanh, bảng nhớ và nhả nhập chung.
- `keyboard.h`: ánh xạ có trạng thái từ phím HarmonyOS sang phím Slint.

Nhật ký: `hdc shell hilog -T CrossDeskDesktop` cho ứng dụng máy tính chung, `hdc shell hilog -T SlintDesktop` cho surface/ghi. Không nên sao xác thực đăng nhập vào báo lỗi; mật khẩu máy này của bản máy tính chung mặc định thấy được, hãy ẩn trước khi ghi/chụp màn hình.
