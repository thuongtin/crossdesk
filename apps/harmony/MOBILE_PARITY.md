# Đối chiếu tính năng ứng dụng điều khiển di động HarmonyOS và Android

[Biên dịch và cài HarmonyOS](README.md) · [Thao tác di động](../../docs/USER_GUIDE.md#harmonyos)

Điện thoại, máy tính bảng dùng `pages/Mobile`, vai trò cố định điều khiển. Chuẩn đối chiếu là `apps/android` trong kho này, không gồm khả năng PC được điều khiển. HAP riêng di động chỉ khai báo `INTERNET`, `KEEP_BACKGROUND_RUNNING`; không xin quyền ghi màn hình, chèn nhập, camera, micro hay toàn bộ thư viện tệp.

## Biên dịch và cài đặt

```bash
python3 apps/harmony/scripts/build.py --mobile
python3 apps/harmony/tests/run.py --mobile
```

Cần Python 3.10+, Xmake 3.1.1+, DevEco SDK. Tương thích API 20, đích API 26, ARM64. Sản phẩm:

```
apps/harmony/.native/mobile-project/entry/build/default/outputs/default/entry-default-unsigned.hap
```

Gỡ lỗi điện thoại thật: mở `.native/mobile-project` đã sinh trong DevEco, cấu hình chữ ký máy này rồi cài. Không commit tài liệu ký vào kho. Chạy lại script sẽ tạo lại dự án cách ly, bỏ cấu hình ký. ACL/chữ ký PC không dùng cho gói di động.

Biên dịch di động chỉ sao chép `ets/mobile/`, `pages/Mobile.ets`, cầu nối N-API và tài nguyên dùng thực; chỉ một trang vào `pages/Mobile`. `crossdesk_harmony_mobile` biên dịch phiên điều khiển và phát âm thanh/video chung thành thư viện gốc riêng, xuất `.native/mobile-libs/arm64-v8a/`; không biên dịch phiên PC, ghi màn hình, chèn nhập, không xuất API cầu nối PC. Chỉ biên dịch lại thư viện dùng `build.py --mobile --native-only`, sau đó đóng gói `--mobile --hap-only`. PC liên kết trực tiếp MiniRTC/dịch vụ nền tảng, không sinh lõi ArkUI PC cũ nữa; hai bên không ghi đè sản phẩm nhau.

Dự án sinh dựng lại thư mục nguồn/thư viện mỗi lần để bỏ sót bản cũ; hãy sửa nguồn dưới `entry/src/main/` rồi chạy script. Font máy tính không vào gói di động; giấy phép nguồn mở chỉ giữ tài liệu đầy đủ có thể đọc từng mục và kiểm SHA-256 trong UI, bỏ bản tổng hợp trùng.

## Đối chiếu tính năng

| Tính năng Android | Triển khai HarmonyOS |
| --- | --- |
| Tự đăng nhập tiền cảnh, dừng nền khi rảnh, cách ly máy chủ | identity peer riêng; chỉ tạo controller peer khi chủ động điều khiển; từ chối mọi kết nối được điều khiển đến |
| Màn hình riêng tư máy tính | Bật/tắt trực tiếp menu nổi, không trang chi tiết; nút theo trạng thái máy tính xác nhận hiện “Đã bật màn hình riêng tư” hoặc “Đã tắt màn hình riêng tư”, cùng màu khi bật/tắt, đổi thường không thêm chữ, chỉ gợi ý khi lỗi; bật thì chặn bàn phím/chuột cục bộ như máy tính; trạng thái hết hạn 5 giây, lệnh quá hạn 15 giây, cách ly phiên |
| Xác nhận quyền riêng tư lần đầu, rút lại, đọc ngoại tuyến | Trước đồng ý không yêu cầu mạng danh tính/thông báo/trực tuyến/phiên bản; rút lại kết thúc phiên và xóa xem trước |
| Danh tính mã hóa, tùy chọn nhớ mật khẩu truy cập | Asset Store hệ thống, đọc sau mở khóa thiết bị đầu tiên, cấm đồng bộ thiết bị; Preferences thường không lưu mật khẩu |
| Kết nối gần đây, xóa, trực tuyến | 20 máy gần nhất, ưu tiên trực tuyến, đăng ký tín hiệu thật, làm mới 30 giây/hết hạn 60 giây; xóa đồng thời mật khẩu/xem trước |
| H.264 / AV1 | Giải mã phần cứng hệ thống; thiếu khả năng, lỗi cấu hình/chạy thì giữ định dạng thương lượng và về OpenH264 / dav1d; hiển thị NV12 GLES |
| Âm thanh từ xa, tắt tiếng | OHAudio đơn kênh 48 kHz, tắt tiếng đồng bộ menu/PiP |
| Chuột tương đối/tuyệt đối, chạm | Nhấn, kéo tuyệt đối, giữ nhấn phải, thu phóng tiêu điểm 1–10 lần, hai ngón di chuyển khi phóng, lăn hai ngón khi 1 lần |
| Bàn phím/chuột ngoài | Ánh xạ phím vật lý, chuột di chuyển/nút/lăn; mất tiêu điểm, nền, ngắt, chuyển màn hình nhả nhập |
| Điều khiển nổi | Bóng thương hiệu kéo được, menu ba cột (bốn khi ngang thấp), bàn phím ASCII/máy tính kéo được, tổ hợp phím đầy đủ, chuột ảo 140×48 |
| Thiết lập video | Thấp/trung bình/cao, 30/60 fps, FPS/chất lượng/cân bằng; thương lượng khả năng, số yêu cầu, bỏ ACK cũ, từ chối quay lại, quá hạn 5 giây |
| Chẩn đoán mạng | Lưu lượng video/âm thanh/dữ liệu/tổng, tốc độ hai chiều, mất gói, FPS/kích thước gửi thực, trễ hình, RTT mượt, P2P/TURN, SRTP; hết hạn 3 giây |
| Nhiều màn hình, Ctrl+Alt+Del | Chuyển theo danh sách từ xa; chỉ gửi SAS khi dịch vụ Windows từ xa báo khả dụng |
| Bảng nhớ tạm | Chủ động gửi (tối đa 128 KB), dán từ xa, sao chép thủ công văn bản nhận; thoát xóa văn bản bộ nhớ |
| Truyền tệp hai chiều | Chọn/xuất tệp hệ thống, tiến độ, chia khối/ACK đáng tin, phủ thứ tự đảo, dọn lỗi/quá hạn; dùng thẳng engine tệp Android chung |
| PiP hệ thống, duy trì nền | VIDEO_LIVE PiP với tắt tiếng, tác vụ truyền dữ liệu nền/thông báo dự phòng; chặn PiP trong chọn tệp, hệ thống từ chối nền thì kết thúc phiên |
| Thông báo | Tóm tắt 200/trang, nội dung 20/trang, đã đọc/xóa/sửa, phân trang, làm mới, quá hạn 12 giây, cách ly danh tính/máy chủ, lọc trả lời trễ |
| Gợi ý phiên bản | Mỗi phiên kiểm phiên bản từ xa; cập nhật máy này dùng `harmony-arm64`, thiếu mục phát hành thì báo rõ không có thông tin bản HarmonyOS |
| Xem trước từ xa | Mặc định tắt, xác nhận riêng, từ kết nối sau mỗi phiên một ảnh 640×360, đệm riêng, không tải lên; tắt/rút lại xóa, tác vụ bất đồng bộ không khôi phục nội dung đã rút |
| Giới thiệu và tuân thủ nguồn mở | Phiên bản gói thực, giấy phép/danh sách thành phần ngoại tuyến, xác minh SHA-256 tài liệu, phiên bản nguồn/hướng dẫn biên dịch |
| Bố trí điện thoại/máy tính bảng | Ngưỡng hai cột 900 vp, gần đây 1–4 cột, thân tối đa 1200/720 vp, trang chủ điện thoại dọc, phiên/máy tính bảng tự xoay, chuyển trang/tùy chọn 200 ms |

API khác nhau: HarmonyOS dùng Asset Store, OHAudio, ArkUI PiP và tác vụ nền, Android dùng Keystore, AAudio, Android PiP và dịch vụ tiền cảnh. Phím hệ thống dành riêng, chính sách nền, khả năng giải mã phần cứng tùy thiết bị.

## Giao diện di động

Vùng cuộn di động tường minh ẩn thanh cuộn phủ, gồm lớp trong/ngoài trang chủ, thiết lập/thông báo, tài liệu dài, hộp thoại, menu điều khiển, bàn phím ảo, ô bảng nhớ tạm; giữ chạm/lăn cũ, tránh thanh trượt che chữ/nút/phím. Vùng cuộn mới nên theo thiết lập này.

Trang chủ, thiết lập, hộp thoại, menu điều khiển lấy Android `MobileUi` làm chuẩn: nền xám trắng, hành động chính xanh, phân cấp chữ, biểu tượng nét, bo góc/bóng thống nhất. Trang chủ có thẻ kết nối, thiết bị gần đây, rỗng căn giữa; rộng dùng cột kết nối rộng cố định/danh sách thích ứng; thiết lập dùng thẻ nhóm, chọn phân đoạn, máy chủ nội tuyến; quyền riêng tư, giới thiệu, thông báo, giấy phép cùng điều hướng/danh sách.

Mật khẩu truy cập hiện bảng dưới ở điện thoại, giữa ở máy tính bảng; bàn phím nhập bật thì thu vùng dùng, giữ mật khẩu/công tắc lưu/nút kết nối, cửa sổ nhỏ cuộn được. Bảng chính sách giữ tiêu đề/hành động cấp quyền thấy, nội dung cuộn riêng. Xóa, rút lại, dọn, ngắt dùng hộp xác nhận thống nhất. Menu điều khiển có lưới biểu tượng, thiết lập hình, thống kê mạng căn chỉnh, bóng thương hiệu, bàn phím/chuột kéo; giới hạn vị trí menu/bàn phím theo cửa sổ.

Kiểm tra UI đợt này dùng bản xem trước trạng thái ngoại tuyến tên gói riêng, không quyền mạng trên giả lập điện thoại/máy tính bảng API 26: trang chủ, thiết lập, xác nhận/hủy, hiện/ẩn mật khẩu/tránh bàn phím nhập, hộp thoại, bảng điều khiển. Thiết bị/truyền tải/kích thước trong xem trước là dữ liệu thử, không cơ sở nghiệm thu mạng. Biểu tượng/con trỏ vẽ theo mật độ, tránh tọa độ pixel SVG thu nhỏ trên màn hình mật độ cao.

## Kiểm chứng tự động

`tests/run.py --mobile` chạy kiểm thử mô hình ArkTS, bộ điều khiển, vòng đời nền tảng sản xuất: quyền riêng tư trước, gọi danh tính/mật khẩu, callback lặp/xuyên phiên, tuần tự tác vụ nền kết nối lại, PiP loại trùng/tắt tiếng, bảo vệ chọn tệp, nút độc lập hai ngón/chuột ảo, trả lời chất lượng đảo/số vòng lại, thống kê hết hạn, trực tuyến/thông báo phân trang đầy đủ.

Cũng chạy hồi quy giao thức gốc/ống video Android, truyền tệp chung, đầu vào/ghi màn hình HarmonyOS và wire chung. Mã chung tại `libs/controller/include`; header Android cũ giữ xuất tương thích để tránh lệch giao thức hai nền tảng.

`tests/verify_hap.py --mobile <hap>` kiểm loại phone/tablet, quyền tối thiểu, chế độ nền, ArkTS biên dịch, phụ thuộc thư viện ARM64, tài liệu giấy phép. Script biên dịch tự chạy.

## Nghiệm thu thiết bị

### Xác nhận bộ giải mã thực

Chẩn đoán giải mã xuất qua thẻ HiLog `CrossDeskDecoder`, không bị tắt nhật ký chung MiniRTC ảnh hưởng. Lọc thẻ trong HiLog DevEco hoặc chạy `hdc shell hilog -x -T CrossDeskDecoder`. `Decode request` ghi khả năng phần cứng định dạng hiện tại, `Decoder selected` ghi triển khai chọn/lý do về phần mềm; nên xác nhận bộ thực chạy thành công bằng `mode`, `name` của `First decoded frame`, không chỉ cấu hình tăng tốc/kết quả khởi tạo. `Decode summary` cuối phiên đếm riêng khung thành công phần cứng/phần mềm, nhận biết quay về giữa chừng. Chỉ ghi định dạng, bộ giải mã, kích thước, giai đoạn lỗi, số khung, không xác thực hay nội dung video.

Đo giả lập Mate 90 Pro API 26 ARM64 ngày 2026-10-15: phiên H.264 3840×2160 dùng OpenH264, phần mềm 2722 khung thành công, phần cứng 0; AV1 tổng hợp 640×360 dùng Dav1dAv1, phần mềm 1203, phần cứng 0. Cả hai `hardware_available=0`, lý do `no hardware capability`. Chỉ chứng minh đường giải mã giả lập hiện tại, không khả năng phần cứng máy thật.

### Tính năng đã bao phủ

Đã cài/chạy giả lập điện thoại (Mate 90 Pro), máy tính bảng (MatePad Pro 13) API 26 ARM64, kiểm chứng đồng ý lần đầu, đăng nhập, đồng bộ thông báo, thiết lập/hai cột. Trên điện thoại dùng bên gửi tổng hợp MiniRTC cùng phiên bản kiểm chứng hình H.264 / AV1 640×360, nhận PCM 48 kHz, trả chuột/bàn phím ảo, yêu cầu/trả chất lượng, chuyển hai màn hình, bảng nhớ tạm, nhận tệp và xuất/chọn hệ thống rồi gửi ngược khớp byte, xoay/khôi phục PiP nền. Đã phát hiện/sửa ID công khai ghi đè xác thực đăng nhập đủ, mất thông tin máy trước callback thành công, PiP che bộ chọn tệp sau về tiền cảnh; bổ sung hồi quy. Kiểm thử phía máy chủ không thay chạm/đa ngón, phím chuột ngoài, tuyến âm thanh, hiệu năng codec phần cứng, nền/PiP dài hạn trên máy thật; vẫn nên nghiệm thu máy thật API 20 và hệ thống hiện tại trước phát hành.

Có thể dùng `tests/mobile_synthetic_host.cpp` làm bên gửi thử tương tác: chỉ sinh sọc màu, PCM im lặng, màn hình ảo, tệp nhỏ, không ghi màn hình/nhập máy này. Tham số `<server> <port> [av1]`, danh tính ghi `fixture-login` trong thư mục làm việc, trạng thái ghi `fixture-status`, tự kết thúc sau 15 phút. Dùng thư mục thử riêng tư độc lập, xóa danh tính/nhật ký/tệp nhận sau thử. Fixture HarmonyOS biên dịch bằng `tests/build_mobile_fixture.py`; shell giả lập hạn chế có thể không tải được thư viện hệ thống, có thể liên kết MiniRTC trên máy phát triển chạy cùng nguồn.
