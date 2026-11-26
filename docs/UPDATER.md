# Cập nhật CrossDesk

[Về trang chủ](../README.md) · [Cài đặt và cập nhật](USER_GUIDE.md#settings) · [Biên dịch và phát hành](#biên-dịch-và-phát-hành-trình-nâng-cấp)

Trên máy tính, bạn có thể xem phiên bản mới tại **Menu chính → Giới thiệu → Kiểm tra cập nhật**. Sau khi phát hiện bản mới, đọc ghi chú cập nhật trước, rồi tiếp tục qua “Cài đặt bản cập nhật” hoặc liên kết tải xuống trên giao diện.

## Cài đặt bản cập nhật

1. Lưu nội dung đang sửa trên máy từ xa, chờ truyền tệp hoàn tất. Cài đặt sẽ ngắt kết nối.
2. Nhấn **Cài đặt bản cập nhật** và xác nhận, chờ tải xuống và xác minh hoàn tất.
3. Cấp quyền cài đặt theo lời nhắc hệ điều hành.
4. Cài xong, chờ CrossDesk mở lại, xác nhận phiên bản trong “Giới thiệu”, rồi kết nối lại máy từ xa.

Trên macOS / Linux, hủy cấp quyền ở giai đoạn chuẩn bị giữ chương trình và kết nối cũ. Tải xuống hoặc xác minh thất bại không đóng chương trình chính, bạn có thể kiểm tra mạng rồi thử lại.

## Vì sao chỉ có nút tải xuống

Các trường hợp sau cần cài thủ công:

- Dùng bản portable hoặc bản tự biên dịch chưa cấu hình khóa công khai xác minh cập nhật.
- Vị trí cài không được cập nhật trong ứng dụng hỗ trợ.
- Gói phát hành hiện tại không có thông tin cập nhật đã ký khả dụng hoặc xác minh không thành công.

Tải gói cài đầy đủ khớp hệ thống và kiến trúc của bạn từ [trang Releases của CrossDesk Postcard](https://github.com/thuongtin/crossdesk/releases), đóng CrossDesk rồi cài. Giữ loại cài đặt cũ; chọn bản Linux thường hoặc màn hình ảo theo nhu cầu thực tế.

Không cài gói từ trang web hay trang Releases của dự án gốc `kunkundi/crossdesk`: gói đó thay bản
Postcard bằng bản upstream, mất cấu hình máy chủ báo hiệu nội bộ và nguồn cập nhật của bản fork.

## Lưu ý từng nền tảng

| Nền tảng | Cách cập nhật và yêu cầu |
| --- | --- |
| Bản cài đặt Windows | Dùng gói đầy đủ cập nhật chương trình chính, dịch vụ và trình trợ giúp; cần quyền quản trị viên |
| Bản Windows portable | Tải và giải nén gói portable đầy đủ; nếu đã đăng ký dịch vụ, còn cần cập nhật thư mục dịch vụ trỏ đến, xem [quản lý dịch vụ](USER_GUIDE.md#windows-service-development-update) |
| macOS | Cài trong ứng dụng hỗ trợ `/Applications/CrossDesk.app`; vị trí khác cài PKG thủ công |
| Linux | Bản cài DEB cần thành phần cấp quyền hệ thống và trình quản lý gói khả dụng; cập nhật chọn gói thường hoặc màn hình ảo theo loại đã cài |
| iOS / iPadOS | Cập nhật từ App Store; ứng dụng tự ký cài lại theo cách ký cũ |
| Android / HarmonyOS | Tải qua liên kết do ứng dụng hoặc trang phát hành cung cấp, cài bằng cách ký tương thích |

Nếu Linux tự cấu hình systemd hoặc trình giám sát tiến trình khác, dừng dịch vụ tương ứng trước rồi cài cập nhật, sau đó khởi động lại. Nếu không, tiến trình cũ có thể ngăn cài đặt.

## Xử lý khi cập nhật thất bại

| Hiện tượng | Cách xử lý |
| --- | --- |
| Không tải được hoặc xác minh thất bại | Kiểm tra mạng, giờ hệ thống và nguồn tải, thử lại hoặc tải thủ công gói chính thức đầy đủ |
| Yêu cầu cấp quyền nhưng không tiếp tục | Xác nhận cửa sổ cấp quyền hệ thống; trên Linux kiểm tra tác nhân cấp quyền màn hình có khả dụng không |
| Báo chương trình vẫn chạy | Thoát CrossDesk trong phiên người dùng khác và dừng dịch vụ giám sát tự cấu hình rồi thử lại |
| Không tự mở sau cài đặt | Chạy CrossDesk thủ công; nếu vẫn thất bại, sửa cài đặt bằng gói đầy đủ |
| Nhập liệu màn hình khóa không dùng được sau cập nhật | Xác nhận chương trình chính, dịch vụ và trình trợ giúp phiên Windows cùng phiên bản |
| Chỉ khác ngày biên dịch nhưng không nhắc cập nhật | Cập nhật so sánh phiên bản số và số bản vá, chỉ đổi ngày không kích hoạt nâng cấp |

Cài đặt gián đoạn không bảo đảm tự quay lui. Không chỉ thay một tệp thực thi để sửa cài đặt; nên dùng gói đầy đủ của nền tảng tương ứng. Nếu vẫn không khôi phục được, xuất log qua **Phản hồi và góp ý → Xuất nhật ký chẩn đoán** rồi gửi gói ZIP cho quản trị viên CrossDesk, kèm phiên bản trước/sau và nội dung lỗi.

## Bản tự biên dịch

Không đặt khóa công khai cập nhật vẫn chạy và kiểm tra phiên bản bình thường nhưng cần tải và cài thủ công. Khi bật cập nhật trong ứng dụng, cần cấu hình [tham số biên dịch](BUILD.md#update-public-key) và [ký phát hành](#khóa-phát-hành) bên dưới.

## Biên dịch và phát hành trình nâng cấp

Lõi nâng cấp Windows, macOS và Linux ở `apps/desktop/src/updater`, triển khai nền tảng ở `apps/desktop/src/platform/<nền-tảng>/updater`. Bản cài đặt biên dịch trình nâng cấp cùng chương trình chính; bản portable chỉ tải thủ công.

Bản CrossDesk Postcard không phát hành bằng GitHub Actions. Gói, chữ ký và feed được tạo bằng
`scripts/release/release.sh` theo [hướng dẫn phát hành](RELEASE.md); feed
`crossdesk-desktop-update.json` nằm trên nhánh `desktop-updates` của `thuongtin/crossdesk` và
được ký bằng khóa riêng của bản fork. Các mục về `Build and Release`, Actions variable và
`update_version_manifest.py` bên dưới mô tả quy trình của upstream, giữ lại để đối chiếu khi đồng bộ.

### Quy trình phối hợp cài đặt

1. Người dùng nhấn “Cài đặt bản cập nhật” và xác nhận kết nối và truyền tệp sẽ bị ngắt.
2. Chương trình chính tải bất đồng bộ, hiện tiến độ và xác minh chữ ký, kích thước, SHA-256.
3. Trình nâng cấp khởi động từ thư mục tạm riêng, lấy khóa bảo trì cho thư mục cài đặt tương ứng.
4. macOS/Linux yêu cầu quyền hệ thống trước, sau đó tiến trình cài đã nâng quyền xác minh lại chữ ký,
   sao chép gói vào thư mục riêng chỉ quản trị viên ghi được và xác minh bản sao. Chỉ sau các bước này mới
   thông báo chương trình chính thoát; trong khi chờ cấp quyền, kết nối từ xa vẫn chạy, hủy cấp quyền không đóng
   chương trình chính. Giai đoạn chuẩn bị chờ tối đa năm phút.
5. Khi nhận thông báo sẵn sàng, chương trình chính gửi xác nhận cài và thoát bình thường. Windows giữ quy trình cũ:
   chờ GUI và tiến trình nền cùng người dùng thoát trước, rồi chạy tiến trình cài nâng quyền để xác minh bản sao.
6. Sau khi xác nhận chương trình liên quan đã thoát, tiến trình cài gọi trình cài nền tảng. macOS/Linux không cài nếu chưa nhận
   xác nhận cài của chương trình chính, nhận thông báo hủy hoặc hết thời gian chờ xác nhận.
7. Trình nâng cấp khởi động lại chương trình bằng danh tính bên gọi ban đầu. Chương trình mới xác nhận khởi động sau khi vào vòng lặp sự kiện GUI;
   nếu không nhận xác nhận trong 90 giây thì ghi nhận thất bại. Xác nhận khởi động không phụ thuộc kết nối máy chủ từ xa.

Ký mã hệ thống không phải cơ chế xác minh nguồn cập nhật ở đây. Kiểm tra bảo mật Windows/macOS,
quyền quản trị viên và xử lý quyền trong gói cài macOS hiện có vẫn áp dụng.

## Khóa phát hành

Ứng dụng tích hợp một khóa công khai Ed25519, bên phát hành giữ khóa riêng tương ứng. Không commit khóa riêng vào
kho mã, cũng không cung cấp cho quy trình biên dịch thường hay PR không đáng tin cậy.

Dùng OpenSSL tạo khóa trong môi trường đáng tin cậy:

```sh
openssl genpkey -algorithm ED25519 -out crossdesk-update-private.pem
openssl pkey -in crossdesk-update-private.pem -pubout -outform DER -out crossdesk-update-public.der
python3 -c "from pathlib import Path; p=Path('crossdesk-update-public.der').read_bytes(); assert len(p)==44 and p[:12].hex()=='302a300506032b6570032100'; print(p[12:].hex())"
```

Cấu hình trong kho GitHub:

- Actions variable `CROSSDESK_UPDATE_PUBLIC_KEY`: Khóa công khai 64 ký tự thập lục phân viết thường ở trên.
- Actions secret `CROSSDESK_UPDATE_PRIVATE_KEY`: Toàn bộ nội dung tệp khóa riêng PEM.

Quy trình biên dịch máy tính truyền khóa công khai cho Xmake. Chỉ bước tạo manifest phát hành đọc khóa riêng.
Lần đầu cần cài thủ công phiên bản chứa trình nâng cấp, khóa công khai và siêu dữ liệu cài đặt mới.

Khi đóng gói thủ công từ nhánh bằng GitHub Actions, trong `Run workflow` của
`Build and Release`, nhập `desktop_version` (ví dụ `1.7.2`), giữ `patch` là
`0`. Ba nền tảng máy tính dùng cùng phiên bản số và thêm ngày biên dịch. Bản biên dịch nhánh
để trống dùng `0.0.0-ngày`; biên dịch tag phiên bản tiếp tục dùng số phiên bản trong tag.
`desktop_version` chỉ ảnh hưởng gói cài máy tính, không đổi phiên bản di động.

Tác vụ tiền đề `prepare-version` tạo thống nhất phiên bản máy tính đầy đủ, truyền qua job outputs cho
Linux, macOS, Windows và tác vụ phát hành. Tag không có ngày dùng ngày theo giờ Bắc Kinh vào ngày chạy tác vụ tiền đề,
tag có ngày rõ ràng dùng ngày trong tag; tác vụ sau không tính lại.
Khi biên dịch qua nửa đêm hoặc chạy lại riêng tác vụ phát hành, tiếp tục dùng phiên bản tác vụ tiền đề đã xác định.

Không dùng tên nhánh như `vauto-update-20261020` làm số phiên bản để kiểm chứng nâng cấp.
Phiên bản mới phải tăng phiên bản số hoặc patch; chỉ đổi ngày không được coi là nâng cấp. Ví dụ cài thủ công
`v1.7.2-20261020` trước, rồi kiểm chứng nâng cấp lên `v1.7.3-20261020`.
Biên dịch nhánh thủ công chỉ tải sản phẩm biên dịch Actions lên, không tự phát hành manifest phiên bản.

Script phát hành so sánh phiên bản số và patch, bỏ qua ngày biên dịch. Khi sửa hoặc phát hành lại Release máy tính
cũ, giữ manifest hiện có trên máy chủ và nêu lý do bỏ qua trong nhật ký Actions;
cùng phiên bản vẫn có thể cập nhật ghi chú phát hành, bản mới phát hành bình thường. Bản ghi tải xuống từng nền tảng cũng không bị phiên bản thấp hơn
ghi đè, kể cả phát hành di động riêng và gói di động kèm Release máy tính.

Phiên bản đầy đủ của mọi gói cài máy tính và gói portable trong Release máy tính phải khớp phiên bản tag sau khi bỏ
tiền tố `v`, gồm cả vị trí ngày và patch; không được chỉ so sánh phiên bản số.
Nếu bất kỳ gói nào không khớp, script báo tag, nền tảng, tên tệp và phiên bản hai bên, dừng tạo manifest,
không ghi tệp đầu ra. Khi phát hành thủ công, nên dùng tag phiên bản đầy đủ khớp gói cài, ví dụ
`v1.7.4-20261021`. Gói cài iOS, Android giữ phiên bản độc lập.

Biên dịch cục bộ có thể đặt biến môi trường cùng tên hoặc dùng:

```sh
xmake f --CROSSDESK_UPDATE_PUBLIC_KEY=<khóa-công-khai-64-ký-tự-thập-lục-phân-viết-thường>
xmake b crossdesk
```

Bản biên dịch không có khóa công khai vẫn chạy và kiểm tra phiên bản bình thường nhưng không tự cài.
Nếu khóa riêng và công khai không khớp, tạo manifest thất bại. Hiện chỉ hỗ trợ một khóa công khai; trước khi đổi khóa cần
thiết kế phiên bản chuyển tiếp, không được thay thẳng khóa riêng phát hành rồi để ứng dụng cũ tiếp tục tự nâng cấp.

## Định dạng manifest

`.github/scripts/update_version_manifest.py` ghi `size`, `sha256` cho gói cài.
Sau khi cấu hình khóa riêng, gói cài máy tính còn chứa:

```json
{
  "update": {
    "payload": "{\"filename\":\"...\",\"platform\":\"windows-x64\",\"schema\":1,\"sha256\":\"...\",\"size\":123,\"url\":\"...\",\"version\":\"...\"}",
    "signature": "chữ-ký-Ed25519-128-ký-tự-thập-lục-phân-viết-thường"
  }
}
```

Chữ ký bao phủ byte UTF-8 gốc của chuỗi `payload`; ứng dụng xác minh chữ ký trước rồi phân tích.
Chữ ký gồm phiên bản, nền tảng, tên gói, địa chỉ tải, kích thước và hàm băm để tránh tráo bản ghi tải xuống.
Bản fork chỉ chấp nhận địa chỉ gói `https://github.com/thuongtin/crossdesk/releases/download/v<phiên-bản>/<tên-gói>`,
với tên gói khớp phiên bản và nền tảng. Chuyển hướng tải xuống chỉ được đi tới CDN tài sản release
của GitHub (`release-assets.githubusercontent.com` hoặc `objects.githubusercontent.com`) qua HTTPS;
giới hạn kích thước gói 2 GiB, cấm cài cùng phiên bản và hạ cấp.
Bản portable không tự cài: ứng dụng chỉ đọc phiên bản trong mục `downloads` của đúng nền tảng
(mục portable, nếu chưa có thì mục gói cài của cùng nền tảng) rồi mở trang Releases của bản fork.
Giao diện máy tính chạy trên HarmonyOS cũng vậy: chỉ đọc mục `harmony-arm64`, nên bản phát hành
không có gói HarmonyOS không được báo là có cập nhật.
Gói portable và di động không có tùy chọn tự cài này.

## Ranh giới nền tảng

- Windows: Thư mục cài phải khớp `InstallLocation` trong bản ghi gỡ cài HKLM.
  Dừng dịch vụ CrossDesk và chờ thoát; sau khi xác nhận mọi chương trình chính cùng đường dẫn đã kết thúc,
  gọi NSIS bằng `/S /CROSSDESK_UPDATE /D=...`. Chế độ nâng cấp không buộc kết thúc
  chương trình theo tên tiến trình; trình nâng cấp chịu trách nhiệm khởi động lại. Dịch vụ chạy trước đó được thử khôi phục.
- macOS: Hỗ trợ `/Applications/CrossDesk.app`, gọi
  `/usr/sbin/installer` qua cấp quyền hệ thống để cài PKG. Vị trí cài khác tải thủ công.
  Trình nâng cấp chỉ được phụ thuộc thư viện động hệ thống; điều này được kiểm tra khi đóng gói.
- Linux: Hỗ trợ `/usr/lib/crossdesk/crossdesk-bin`, chọn
  gói thường hoặc màn hình ảo theo `package-name`. Dùng `pkexec` cấp quyền và `apt-get install`
  cài DEB cục bộ; cấm trình quản lý gói tự xóa phụ thuộc. Cần tác nhân cấp quyền khả dụng.
  Khi khởi động lại, khôi phục tham số gốc `--headless`, `--no-headless`, `--headless-display`,
  `--headless-size` và `--headless-session`, kể cả khởi động lại sau cài thất bại;
  không phát lại tham số chế độ chạy nội bộ.
  Quản trị viên phải phối hợp dừng systemd/headless supervisor tùy chỉnh trước;
  phát hiện chương trình vẫn chạy sẽ hủy nâng cấp, không đoán hay dừng dịch vụ khác.

Chờ thoát có giới hạn thời gian; nếu còn chương trình chạy thuộc người dùng khác sẽ báo thất bại. Hệ thống nhả khóa bảo trì khi tiến trình
thoát, tệp khóa POSIX còn lại không bị coi là đang nâng cấp.

## Xử lý thất bại và kiểm chứng

Tải xuống, xác minh chữ ký hoặc chuẩn bị thất bại không đóng chương trình chính. Cài thất bại sau khi chương trình chính thoát sẽ thử
khởi động lại; macOS/Linux giữ chương trình và kết nối cũ khi hủy cấp quyền trước lúc thoát.
`last-update` ở thư mục bộ nhớ đệm ứng dụng, trỏ đến thư mục tạm riêng; `request.json` và
`result.json` giữ phiên bản nâng cấp và kết quả, tác vụ chưa hoàn tất không tự phát lại. Sau khi bản mới xác nhận
khởi động, thông báo trình nâng cấp trước rồi xóa `last-update` để tránh báo nhầm nâng cấp gián đoạn khi thư mục tạm bị hệ thống
dọn; bản ghi đã được ứng dụng cũ xác nhận thành công cũng bị xóa ở lần khởi động sau.
Sau đó trình nâng cấp xóa gói cài lớn, giữ một ít tệp chẩn đoán. Tác vụ thất bại giữ bản ghi và gói đã tải
để thuận tiện điều tra.

Thao tác cài NSIS, PKG và DEB không phải giao dịch xuyên nền tảng. Hiện không cam kết tự quay lui:
nếu cài bị gián đoạn hoặc bản mới khởi động thất bại, giữ lỗi và cung cấp cách sửa cài thủ công. Không thể
chỉ khôi phục EXE để quay lui dịch vụ, thư viện phụ thuộc hoặc di chuyển cấu hình.

```sh
xmake b update_checker_test version_checker_test
xmake r update_checker_test
xmake r version_checker_test
python3 -m unittest discover -s .github/scripts -p test_update_version_manifest.py
xmake lua apps/desktop/tests/build_define_scope_test.lua windows true
```

Trước phát hành vẫn cần kiểm chứng trong môi trường cài đặt cách ly trên từng nền tảng: hủy cấp quyền, phiên khác đang dùng, dịch vụ
thoát, cài thất bại, tự cập nhật trình nâng cấp và xác nhận chương trình mới khởi động. Không tự chạy gói cài sản xuất trên máy phát triển
để thay thế các kiểm thử cài đặt này.
