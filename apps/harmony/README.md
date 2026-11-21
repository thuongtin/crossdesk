# CrossDesk HarmonyOS

[Hướng dẫn sử dụng](../../docs/USER_GUIDE.md#harmonyos) · [Biên dịch và cài đặt](#biên-dịch) · [Thích ứng nền tảng PC](desktop-port/README.md) · [Tính năng di động](MOBILE_PARITY.md)

HarmonyOS PC chỉ giữ ứng dụng máy tính Slint chung; điện thoại và máy tính bảng dùng ứng dụng điều khiển ArkUI riêng. Đã bỏ giao diện ArkUI PC cũ, máy trạng thái phiên PC riêng và chương trình kiểm chứng Slint ban đầu.

| Thiết bị | Triển khai | Yêu cầu và ghi chú |
| --- | --- | --- |
| PC (`2in1`) | UI máy tính chung, logic nghiệp vụ và thích ứng HarmonyOS | API 26, arm64; [biên dịch, quyền và nghiệm thu máy tính](desktop-port/README.md) |
| Điện thoại / máy tính bảng | Ứng dụng điều khiển di động ArkUI | Tương thích API 20, đích API 26, arm64; [đối chiếu tính năng và nghiệm thu](MOBILE_PARITY.md) |

Cả hai vẫn là bản phát triển. Xem kiểm chứng giả lập đã hoàn tất và nghiệm thu thiết bị thật chưa hoàn tất trong tài liệu từng bản.

“Cài đặt → Hướng màn hình → Khóa hướng ngang sau khi kết nối” trên ứng dụng điều khiển điện thoại mặc định tắt, lưu trên máy. Khi bật, điện thoại vào và giữ hướng ngang khi kết nối; khi tắt, theo hướng thiết bị. Máy tính bảng và đa cửa sổ không chịu tùy chọn này. Thoát phiên khôi phục hướng trang chủ; quay lại phiên nền giữ cài đặt hướng của phiên đó. Đã bỏ logic bỏ khóa ngang ban đầu sau khi phát hiện tư thế điện thoại.

## Phím tắt di động

Điện thoại và máy tính bảng cung cấp thanh phím tắt thường dùng và mục “Thêm” trên bàn phím nổi hiện có; bảng thêm thay vùng phím, gồm “Thường dùng / Phím / Hệ thống / Của tôi”. Cấu hình sẵn thích ứng Windows, macOS, Linux từ xa; cấu hình hệ thống Linux cần chọn GNOME hoặc KDE Plasma. Điểm nhập hiện vẫn là bàn phím nổi ArkUI, chưa đổi thành bộ gõ hệ thống.

Trang quản lý xử lý riêng nhập liệu cục bộ, hỗ trợ 16 tổ hợp tùy chỉnh, 8 mục ghim cho mỗi nền tảng từ xa, cùng sắp xếp, sửa và xóa. Cấu hình lưu trên máy; tổ hợp gửi bằng cùng một tác vụ hàng đợi native, nhả phím bổ trợ ngược thứ tự và kiểm tra thế hệ kết nối.

## Xác thực thiết bị đáng tin cậy trên di động

“Cài đặt → Thông tin ghép đôi máy này” trên điện thoại và máy tính bảng cho xem dấu vân tay và sao chép thông tin ghép nối. Dán vào “Quản lý thiết bị đáng tin cậy → Thêm thiết bị đáng tin cậy” trên máy được điều khiển và đối chiếu dấu vân tay, rồi có thể kết nối máy bật “Chỉ cho phép thiết bị đáng tin cậy điều khiển”; vẫn cần xác minh mật khẩu kết nối.

Chế độ di động dùng giao thức thách thức–đáp ứng Ed25519 chung; khóa riêng lưu bền qua AssetStore, cấm đồng bộ, không đưa vào bộ nhớ đệm, nhật ký hay thông tin ghép nối. Khởi động lại, xóa bộ nhớ đệm, đổi máy chủ và ID đăng nhập máy này không đổi khóa công khai; khóa đã khởi tạo hỏng, không khả dụng hoặc mất sẽ báo lỗi, không tự đổi khóa. Sau khi xóa dữ liệu ứng dụng hoặc cài lại, cần ghép nối lại.

## Biên dịch

Kho phụ thuộc theo chính sách Xmake Windows/macOS/Linux: dùng kho mặc định hoặc `XMAKE_GLOBALDIR` / `XMAKE_MAIN_REPO` do môi trường gọi chỉ định, không cố định hay kiểm tra commit kho. HarmonyOS PC và di động dùng chung quy tắc này; SDK, sản phẩm biên dịch và bộ nhớ đệm cài phụ thuộc vẫn tách theo nền tảng.

Chạy ở gốc kho mã; script biên dịch hiện dành cho macOS:

```bash
# Mặc định biên dịch PC: MiniRTC, Slint runtime, ứng dụng máy tính chung và HAP
python3 apps/harmony/scripts/build.py

# Biên dịch tăng dần PC: tái dùng runtime đã tạo; thay đổi bản vá vẫn kích hoạt biên dịch lại
python3 apps/harmony/scripts/build.py --reuse-runtime

# Điện thoại / máy tính bảng: ứng dụng điều khiển riêng, không gồm mã ghi hình và chèn nhập liệu PC
python3 apps/harmony/scripts/build.py --mobile

# Chỉ chuẩn bị phụ thuộc MiniRTC cho PC; không biên dịch lõi ArkUI PC cũ nữa
python3 apps/harmony/scripts/build.py --native-only

# Đóng gói lại bằng phụ thuộc native hiện có
python3 apps/harmony/scripts/build.py --hap-only --reuse-runtime
python3 apps/harmony/scripts/build.py --mobile --hap-only
```

`--release` chọn HAP release; phụ thuộc native mặc định dùng release. Xem tùy chọn cài PC, chọn thiết bị, số phiên bản, quyền và ký trong `python3 apps/harmony/scripts/build_desktop.py --help`, hoặc truyền trực tiếp cho `build.py` mặc định. Chỉ biên dịch lõi điều khiển di động thì dùng `--mobile --native-only`.

Cần DevEco Studio (SDK, Native SDK, Node, ohpm, hvigor, Java), Xmake 3.1.1+ và submodule MiniRTC đầy đủ. PC còn cần Python 3.12+, Rust 1.92+ và Slint compiler 1.17.1 bản máy chủ; di động cần Python 3.10+. Nếu Python hệ thống quá cũ, có thể dùng `/opt/homebrew/bin/python3`.

| Biến môi trường | Công dụng / mặc định |
| --- | --- |
| `DEVECO_STUDIO_HOME` | `/Applications/DevEco-Studio.app/Contents` |
| `DEVECO_SDK_HOME` | `$DEVECO_STUDIO_HOME/sdk`, chứa `default/openharmony/native` |
| `XMAKE_BIN` | `xmake` trong PATH; máy này cũng có thể chỉ định `$HOME/.local/bin/xmake` |
| `SLINT_COMPILER` | Slint compiler bản máy chủ; mặc định tìm bộ nhớ đệm 1.17.1 của Xmake |

Script dùng cấu hình Xmake và bộ nhớ đệm gói riêng, không đổi cấu hình máy tính ở gốc kho. Phiên bản Slint, httplib và công cụ biên dịch Python do cấu hình biên dịch chỉ định; kho gói Xmake theo chính sách chọn ở trên. Biên dịch PC và di động dùng chung khóa để tránh ghi đè đồng thời phụ thuộc native.

## Dự án và sản phẩm

| Ứng dụng | Dự án DevEco được tạo |
| --- | --- |
| PC | `apps/harmony/.native/slint-poc/desktop-project` |
| Điện thoại / máy tính bảng | `apps/harmony/.native/mobile-project` |

HAP đều nằm trong `entry/build/default/outputs/default/entry-default-unsigned.hap` của dự án tương ứng. Tên bộ nhớ đệm PC `slint-poc` được giữ để tái dùng bộ nhớ đệm Rust hiện có; mã nguồn không còn chương trình kiểm chứng. PC và điện thoại / máy tính bảng dùng chung `AppScope/app.json5` và tài nguyên tên ứng dụng, thống nhất tên gói `com.chunkdash.crossdesk`, tên **CrossDesk**. Gói kiểm chứng PC cũ `cn.crossdesk.slintdesktop` là ứng dụng khác gói mới; dữ liệu cũ không tự chuyển.

Dùng DevEco mở dự án được tạo để gỡ lỗi và ký. Hãy sửa mã nguồn thư mục này rồi chạy lại script; dự án được tạo không phải điểm vào mã nguồn. Biên dịch di động tạo lại thư mục nguồn và thư viện; PC dọn sản phẩm đích cầu nối cũ khi chuyển từ bản biên dịch cũ lần đầu. Đóng gói cả hai đều kiểm tra loại thiết bị, quyền, ArkTS, thư viện ARM64, SONAME và phụ thuộc trong HAP thực, từ chối thư viện có điểm vào đã bỏ.

Đặt tài liệu ký và cấu hình cục bộ ngoài kho hoặc trong `.native` đã bỏ qua; PC dùng `--signing-config` tường minh, không kế thừa chữ ký dự án di động. MiniRTC là submodule Git riêng; khi bàn giao cần giữ các thay đổi thích ứng HarmonyOS trong đó.

## Mã nguồn và kiểm chứng

- `desktop-port/`: dịch vụ nền tảng PC, cửa sổ, bộ gõ, thích ứng Slint và bản vá runtime; nghiệp vụ và UI biên dịch trực tiếp từ `apps/desktop`.
- `entry/src/main/`: trang ArkUI di động và cầu nối N-API di động.
- `native/mobile_session.*`, `media.*`: phiên điều khiển di động và hiển thị video NV12.
- `native/audio.*`: dịch vụ phát OHAudio dùng chung hai bản.
- `native/capture*`, `input.*`: ghi màn hình PC và nhập liệu được điều khiển; `key_mapping.h` cũng được di động dùng.
- `libs/wire`: giao thức chung của kho mã.
- `libs/controller/include/file_transfer_core.h`: lõi truyền tệp chung cho HarmonyOS PC, nền tảng máy tính khác và di động; `mobile_file_transfer_session.h` chỉ giữ thích ứng vòng đời tệp di động.

Truyền tệp dùng chung logic chia khối, cửa sổ xác nhận, ghép lại sai thứ tự, hết hạn và hủy, giữ định dạng wire hiện có. PC tiếp tục dùng hàng đợi nhiều tệp, danh sách tiến độ, cài đặt thư mục và quy tắc thêm dấu thời gian khi trùng tên của máy tính chung, không xóa tệp gốc gửi; di động tiếp tục dùng bản sao tạm từ bộ chọn tệp và dọn sau thành công hoặc thất bại, tệp nhận vẫn lưu trong thư mục con riêng. Trạng thái nhận máy tính cách ly theo kết nối và đối tác, phản hồi chỉ nhận ACK đúng đối tác, ngắt kết nối dọn tệp nhận chưa xong.

```bash
python3 apps/harmony/tests/run.py --desktop --mobile
python3 apps/harmony/tests/verify_hap.py
python3 apps/harmony/tests/verify_hap.py --mobile \
  apps/harmony/.native/mobile-project/entry/build/default/outputs/default/entry-default-unsigned.hap
```

Kiểm thử phía máy chủ bao phủ giao thức, nhập liệu, ghi hình, màn hình, móc bàn phím, cửa sổ, bộ gõ, trạng thái tự khởi động và vòng đời di động. Chúng dùng khai báo SDK và giao diện hệ thống mô phỏng; ghi màn hình, cấp quyền nhập liệu, đa màn hình, âm thanh, nền và phiên dài hạn trên thiết bị thật vẫn cần nghiệm thu thiết bị.
