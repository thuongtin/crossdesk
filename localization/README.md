# Bản địa hóa giao diện CrossDesk

Ứng dụng máy tính và ứng dụng di động Android, iOS, HarmonyOS hỗ trợ 11 ngôn ngữ giao diện sau:

| Ngôn ngữ | Mã định danh |
| --- | --- |
| Tiếng Trung giản thể | `zh-Hans` |
| English | `en` |
| Русский | `ru` |
| Tiếng Trung phồn thể | `zh-Hant` |
| Tiếng Nhật | `ja` |
| 한국어 | `ko` |
| Deutsch | `de` |
| Français | `fr` |
| Español | `es` |
| Português (Brasil) | `pt-BR` |
| Tiếng Việt | `vi` |

## Chọn ngôn ngữ

- Bản cài đặt mới của ứng dụng máy tính phiên bản tiếng Việt mặc định chọn rõ tiếng Việt; bạn cũng có thể chỉ định ngôn ngữ hoặc “Theo hệ thống” trong cài đặt. Lựa chọn ngôn ngữ trong cấu hình hiện có được giữ: ý nghĩa 0–9 không đổi, 10 vẫn là “Theo hệ thống”, 11 là tiếng Việt (cột 10 trong bảng chuỗi, đếm từ 0).
- Thiết bị di động dùng ngôn ngữ hệ thống; Android và iOS cũng hỗ trợ cài đặt ngôn ngữ ứng dụng do hệ điều hành cung cấp.
- Biến thể vùng khớp với ngôn ngữ tương ứng; tiếng Trung ưu tiên nhận diện `Hans` / `Hant`, Đài Loan, Hồng Kông, Ma Cao dùng phồn thể. Các biến thể vùng tiếng Bồ Đào Nha dùng chung bản dịch tiếng Bồ Đào Nha Brazil.
- Nền tảng hỗ trợ danh sách ngôn ngữ ưu tiên của hệ thống dùng ngôn ngữ khả dụng trong danh sách; nếu không khớp thì dùng tiếng Việt. Tài nguyên mặc định Android là tiếng Việt, tiếng Anh dùng `values-en`; vùng phát triển iOS là `vi`; HarmonyOS cũng dùng tiếng Việt khi không cung cấp ngôn ngữ hoặc đọc thất bại. Mở lại ứng dụng sau khi đổi ngôn ngữ hệ thống để bảo đảm mọi trang và chuỗi trạng thái đều cập nhật.
- Chỉ dịch giao diện ứng dụng. Không dịch tên thiết bị từ xa, tên tệp, nội dung người dùng nhập, thông báo máy chủ và nội dung giao thức. Chính sách quyền riêng tư của phiên bản này có bản dịch tiếng Việt và giữ bản tiếng Anh; giấy phép bên thứ ba giữ nguyên văn.

## Khi ngôn ngữ hiển thị không đúng

Trước hết kiểm tra ngôn ngữ ưu tiên của hệ thống hoặc cài đặt ngôn ngữ ứng dụng, rồi mở lại CrossDesk. Trên máy tính, bạn cũng có thể chọn ngôn ngữ thủ công trong cài đặt và lưu. Tên thiết bị, nội dung tệp và thông báo không được dịch mà giữ nguyên là hành vi bình thường.

Nếu phát hiện bản dịch giao diện sai hoặc chữ bị cắt, bạn có thể [báo cáo sự cố](https://github.com/kunkundi/crossdesk/issues), cung cấp ngôn ngữ sử dụng, phiên bản ứng dụng, trang đang xem và bản dịch đề xuất. Hãy che mật khẩu kết nối và nội dung riêng tư trong ảnh chụp.

## Bảo trì tài nguyên


`catalog.json` là điểm chỉnh sửa duy nhất; kết quả sinh được đưa vào thư mục từng nền tảng và commit, ứng dụng không phụ thuộc dịch vụ dịch khi chạy.

```sh
python3 localization/generate.py
python3 localization/generate.py --check
```

Trình sinh dùng thư viện chuẩn Python, xuất bảng chuỗi C++ máy tính, tài nguyên gốc và khai báo ngôn ngữ Android, tài nguyên `.lproj` iOS và bảng chuỗi HarmonyOS. `--check` kiểm tra độ bao phủ ngôn ngữ, chỗ giữ chỗ định dạng và các tệp sinh có đồng bộ hay không.

Khi thêm chuỗi, cung cấp bản dịch mọi ngôn ngữ trong `strings` và thêm vào danh sách nền tảng tương ứng trong `platforms`. Khóa đặt tên cho máy tính cũng thêm vào `desktop`; khóa dùng bởi Slint được nối vào cuối danh sách `slint`, không được sắp xếp lại chỉ số hiện có. 3 mục đầu trong thứ tự ngôn ngữ máy tính tương thích cấu hình cũ; đổi thứ tự cần di chuyển cấu hình.

Cần giữ đầy đủ chỗ giữ chỗ: Android/HarmonyOS dùng dạng vị trí như `{0}`; iOS dùng định dạng vị trí như `%1$@`; chuỗi máy tính hiện có dùng định dạng vốn có. Các ngôn ngữ có thể đổi vị trí chỗ giữ chỗ nhưng không được đổi số lượng hoặc kiểu. Bản dịch ban đầu của ngôn ngữ mới có phần được máy hỗ trợ; trước khi phát hành, người dùng ngôn ngữ tương ứng nên rà soát thuật ngữ, giọng văn và văn bản dài.
