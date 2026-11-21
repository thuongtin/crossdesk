# Tương thích và giới hạn CLI

[Về hướng dẫn CLI](../README.md) · [Khắc phục cài đặt](npm-distribution.md) · [Thao tác terminal](terminal-client.md)

CLI hiện là bản phát triển. Dưới đây giúp xác định phù hợp môi trường của bạn không; gói phát hành cụ thể vẫn theo ghi chú phiên bản.

## Phạm vi nền tảng

- macOS arm64 đã có biên bản biên dịch đầy đủ, khởi chạy npm và kết nối từ xa.
- Linux x64 / arm64, Windows x64 là mục tiêu biên dịch/phân phối; chạy đầy đủ, điều khiển chuột/phím thực và khôi phục khi thoát terminal vẫn cần xác nhận trên thiết bị tương ứng.
- Chương trình tự biên dịch có thể cần hệ thống mới hơn gói phát hành, không thể xét tương thích chỉ bằng CPU. Hãy kiểm tra `build-info.json` trong gói.

## Phạm vi terminal

| Môi trường | Khuyến nghị và giới hạn |
| --- | --- |
| Terminal thường | Dùng ANSI trước; chi tiết chữ giới hạn bởi lưới ký tự, có thể phóng hoặc giảm font |
| Kitty / Sixel | Cần terminal qua kiểm tra khả năng; có đầu ra giao thức không nghĩa tương thích mọi terminal/phiên bản |
| SSH | Cấp PTY, làm mới ảnh tùy mạng và tốc độ đầu ra terminal |
| tmux / screen | Dùng ANSI |
| Tiện ích Pi | Có khung nhìn ANSI; làm mới TUI thực, phím nâng cao, chuyển chuột vẫn còn phạm vi cần xác nhận |

## Giới hạn tính năng

CLI điều khiển máy qua ID, mặc định chỉ đọc, tắt tiếng. Không có IP trực tiếp, phát âm thanh, truyền tệp, đồng bộ clipboard hay xác thực thiết bị đáng tin cậy. Máy được điều khiển vẫn cần phiên đồ họa ghi được.

Chỉ đọc là công tắc nhập cục bộ CLI, không phải quyền chỉ đọc máy chủ riêng. Terminal thường gửi phím kiểu nhấn rồi nhả, không đảm bảo giữ phím, mọi tổ hợp, bố cục đầy đủ giống ứng dụng máy tính.

`doctor --json` chỉ kiểm tra môi trường cục bộ, không kết nối từ xa, cũng không chứng minh hiển thị pixel/báo cáo chuột khả dụng. FPS trạng thái là số đầu ra thực; hình ANSI tĩnh có thể 0 FPS, không nhất thiết mất kết nối.

Nếu lỗi, ghi tên/phiên bản terminal, hệ thống, chế độ render và bước tái hiện, phản hồi kèm [khắc phục sự cố](terminal-client.md#khắc-phục-sự-cố-và-giới-hạn).

## Biên bản kiểm chứng

Biên bản dưới giữ ngày và phạm vi kiểm chứng cụ thể; qua một kiểm tra không nghĩa mọi nền tảng/terminal đã kiểm chứng chạy xong.

## Tích hợp kho cục bộ (2026-10-30)

Môi trường macOS arm64, Xcode SDK 27.0, Xmake 3.1.1, C++17, tắt CUDA. Mã CLI tích hợp vào `apps/cli` CrossDesk, dùng chung wire và MiniRTC hiện có; không thêm/sửa kiểm thử trong dự án.

- Cấu hình riêng CLI và biên dịch release đạt, đích `crossdesk-cli`; `doctor --json` trả JSON hợp lệ, liên kết động chỉ thư viện/framework hệ thống, không Slint / SDL.
- Bật cả máy tính và CLI thì `crossdesk`, `crossdesk-cli` đều biên dịch đạt cấu hình debug.
- Kiểm thử cấu trúc kho, wire, trạng thái bàn phím hiện có đạt, không sửa nguồn kiểm thử.
- Kiểm tra cầu nối ngoài dự án đạt: JSON, xác minh phiên bản, phân mảnh, EOF, trần thông điệp, bên đọc chậm, tín hiệu cục bộ lỗi, mô tả xác thực, từ chối không TTY.
- POSIX PTY ngoài dự án đạt: mật khẩu ẩn, không DISPLAY/WAYLAND_DISPLAY, khôi phục chế độ thô, quá hạn dò Kitty, từ chối Kitty rõ trong tmux.
- Tạo được gói npm native macOS arm64 và khởi chạy chung; cài ngoại tuyến, `doctor` trình khởi chạy, phân giải đường chương trình native đạt. Sản phẩm cục bộ này cần tối thiểu macOS 27.0.
- Giải nguồn từ gói native rồi cấu hình lại/biên dịch CLI, chạy `doctor` thành công trong thư mục không siêu dữ liệu Git, không nguồn ứng dụng máy tính.
- Sau kiểm chứng khôi phục cấu hình máy tính debug trước tích hợp, CLI mặc định tắt; nhị phân dòng lệnh mới giữ tại `build/macosx/arm64/release/crossdesk-cli`.

## Hồi quy tối ưu (2026-10-30)

- Biên dịch Release, hồi quy bridge/PTY nhị phân thực đạt. ASan/UBSan bao phủ phiên/render cũ, giữ khung hợp lệ khi khung sai, tái dùng đệm NV12, loại tham số xem trước trùng, trạng thái ACK, ngắt lặp, đặt lại thống kê mạng khi kết nối lại.
- Bridge sản xuất với truyền tải giả ngoài dự án kiểm chứng vẽ lại tĩnh, chi tiết, đổi chất lượng, cửa sổ đổi, tính nguyên tử cấu hình sai và áp lực ngược ACK khung: đạt.
- So ANSI trước/sau gồm 8/24/256 màu, UTF-8/ASCII, half/quadrant, toàn cảnh/chi tiết, checksum đầu ra khớp. Lưới 240×100 cục bộ, 200 lần sinh sai phân liên tục: lưới tĩnh khoảng 447.4 ms → 3.6 ms, đổi một dòng khoảng 422.8 ms → 9.5 ms. Chỉ đo sinh văn bản sai phân, không gồm giải mã video, co giãn ảnh, hiển thị terminal.
- Đóng gói npm thật, cài ngoại tuyến, biên dịch lại nguồn gói đạt; chèn lỗi ngoài dự án xác nhận giữ sản phẩm cũ, không xuất nửa bộ khi tạo gói thứ hai thất bại.

Mã hồi quy, benchmark, truyền tải giả và sản phẩm kiểm chứng mới đều ở thư mục tạm ngoài dự án, không thêm kiểm thử vào dự án/submodule. Biên dịch/chạy đầy đủ đợt này bao phủ macOS arm64, chưa kiểm chứng lại máy Linux/Windows thật.

## Thu phóng neo chuột và gợi ý phím tắt (2026-11-02)

- Bỏ phím tắt cục bộ `d`, giữ nhấn chuột vào chi tiết và `--zoom native`. `f` khôi phục toàn màn hình, đặt lại tỷ lệ/vị trí/chi tiết; `+/-`, `=` và lăn chỉ đọc neo chuột khi thu phóng.
- ASan/UBSan ngoài dự án đạt 900 kiểm tra hình học thu phóng: ASCII, ANSI half/quadrant, viewport Kitty/Sixel, toàn cảnh/chi tiết, nhập liên tục, giới hạn mép, báo cáo SGR/truyền thống, mất tiêu điểm/đổi chế độ; xác nhận `d` không đổi khung nhìn cục bộ nữa, điều khiển vẫn chuyển chữ thường.
- Bốn tổ hợp điều khiển/cục bộ, 1～240 cột, tổng 960 bố trí trạng thái đạt, bao phủ nhãn phím đầy đủ, gợi ý thu phóng/toàn cảnh, thao tác theo chế độ. Gợi ý Pi qua kiểm tra cú pháp Node.js; không kiểm chứng Pi TUI thật hay kết nối thiết bị từ xa.
- Mã và sản phẩm kiểm chứng mới đều lưu thư mục tạm ngoài dự án.

## Điều chỉnh phím điều khiển (2026-10-30, lịch sử)

- Cấu hình debug hiện bật cả máy tính và CLI: `xmake build crossdesk-cli`, `xmake run crossdesk-cli doctor --json`, kiểm tra trợ giúp đều đạt.
- ASan/UBSan ngoài dự án bao phủ tiền tố lệnh, phím chỉ đọc, Ctrl+T/Ctrl+C/Ctrl+] từ xa, nhả chuột, hủy lệnh, cách ly dán, phân mảnh UTF-8/CSI, mất tiêu điểm/thoát; kiểm tra phiên／render cũ đạt.
- Phím di chuyển dùng `i` lên, `j` trái, `k` xuống, `l` phải; ASan/UBSan ngoài dự án xác nhận bốn hướng khớp phím mũi tên chế độ thường／ứng dụng; chữ thường ở điều khiển vẫn gửi từ xa.
- Lúc đó chi tiết／toàn cảnh dùng `d`／`f` kề nhau (`d` đã bỏ 2026-11-02); ASan/UBSan ngoài dự án xác nhận chi tiết giữ tâm, toàn cảnh đặt lại thu phóng/tâm, tiền tố và nhấn chuột đúng; điều khiển vẫn gửi chữ thường từ xa.
- Bridge nhị phân thực kiểm chứng trạng thái lệnh, phục hồi lỗi, phản hồi thoát; view sản xuất với PTY truyền tải giả kiểm chứng phím, gợi ý chế độ, mật khẩu ẩn, khôi phục terminal. Hồi quy bridge và PTY nhị phân thực cũng đạt.
- Kiểm tra kiểu nghiêm ngặt SDK Pi 1.4.0 và thành phần mô phỏng đạt: chuyển phím, gợi ý lệnh, hủy, thu phóng, thoát, đóng sau khởi động bridge thất bại.

Đợt này không kết nối thiết bị từ xa thật, không kiểm chứng Linux／Windows hay Pi TUI thật. Mã kiểm thử và sản phẩm mới ở thư mục tạm hệ thống ngoài dự án.

## Điểm vào ID trực tiếp và gói nguồn npm (2026-10-30)

- Nhập trực tiếp ID 9 chữ số và bí danh `view` đều qua kiểm tra tham số, giữ số không đầu, cùng tùy chọn kết nối; ID sai từ chối trước hỏi mật khẩu. Lời nhắc mật khẩu và hủy Ctrl+C trong terminal macOS arm64 đạt.
- Cài ngoại tuyến gói npm cục bộ, trợ giúp khởi chạy, `doctor --json` đạt; gói nguồn có đủ `libs/controller/include`; bổ sung header chẩn đoán rồi cấu hình độc lập/biên dịch release/chạy `doctor` từ nguồn gói đạt.
- Kiểm thử cấu trúc kho, wire, trạng thái bàn phím cũ đạt; không thêm/sửa kiểm thử trong dự án, kiểm chứng thêm và sản phẩm đều ở thư mục tạm ngoài dự án.
- Lần chạy đầu Build and Release #899 và CLI native builds and npm packages #1 đều thất bại checkout vì chưa đẩy commit MiniRTC `b9ee1fa7`. Sau khi đẩy lên remote MiniRTC, clone nông đệ quy mới đạt.

## Phạm vi kiểm chứng giai đoạn phát triển độc lập

Dưới đây tóm tắt biên bản trước chuyển vào, không nghĩa đợt này kết nối lại thiết bị từ xa:

- Đã kiểm chứng debug/release macOS arm64, NDJSON CLI bridge, mật khẩu ẩn và khôi phục terminal POSIX PTY.
- ASan/UBSan ngoài dự án bao phủ trạng thái phiên, chỉ đọc, nhả nhập, sở hữu bộ nhớ khung, render, thu phóng/di chuyển, áp lực ngược đầu ra và ACK khung.
- Đã kiểm chứng kiểu SDK Pi 1.4.0 và vòng đời thành phần mô phỏng; chưa nghiệm thu hình ảnh Pi TUI thật.
- Giao thức danh tính ban đầu từng kết nối thiết bị thật để xem ANSI chỉ đọc và kiểm chứng dữ liệu Kitty; giải nén dữ liệu Kitty không bằng nghiệm thu ứng dụng Kitty/hiển thị SSH thật.
- Sau đổi sang danh tính tạm `cli-`, CLI chính thức và máy chủ WSS cục bộ kiểm chứng đăng ký tạm, Join, không tái dùng danh tính, dọn khi thoát; dịch vụ trực tuyến chưa triển khai đợt đó, kiểm chứng ảnh cũ không thay tích hợp trực tuyến giao thức danh tính mới.
- Đã kiểm chứng tạo/cài npm macOS arm64 cục bộ; Linux / Windows khi đó chỉ kiểm tra biên dịch chéo chương trình chính và I/O nền tảng, chưa có liên kết MiniRTC đầy đủ/kết nối máy thật.

## Sửa tương thích chuột terminal (2026-10-30)

- Truyền tải giả ngoài dự án tái hiện: bộ phân tích cũ gửi ba byte tải báo cáo `CSI M` truyền thống thành văn bản bàn phím, `mouse_reports` vẫn 0; nhấn SGR chỉ gửi nhấn／nhả, không định vị trước.
- ASan/UBSan sau sửa đạt, bao phủ mọi phân mảnh hai phần/đầu vào từng byte SGR／truyền thống, trái/phải/giữa, kéo, lăn, định vị trước nhấn, byte tọa độ cao, nhả viền đen, chỉ đọc／lệnh cục bộ, nhả mất tiêu điểm, gửi lỗi. Hồi quy lệnh cục bộ liên tục cũ cũng đạt.
- Biên dịch CLI debug và `doctor --json` macOS arm64 đạt. CLI sản xuất với POSIX PTY truyền tải giả ngoài dự án đạt ở cửa sổ 120／50 cột: nhập chuột từng byte, bộ đếm `Mouse RX`／`M:`, thống kê nhật ký, bật/tắt báo cáo, khôi phục terminal gốc.
- Mọi mã kiểm thử, script, sản phẩm thêm ở thư mục tạm ngoài dự án. Chưa bao phủ SSH thật, GNOME Terminal hoặc máy Linux được điều khiển thật, không thể dựa đó xác nhận vấn đề tại chỗ người dùng đã hết.

## Tự phát hiện tỷ lệ ký tự (2026-10-30)

- Biên dịch CLI debug macOS arm64 đạt. ASan/UBSan ngoài dự án bao phủ báo cáo kích thước ký tự／vùng văn bản, thứ tự/ưu tiên trả lời, cách ly trả lời một phần qua truy vấn, nhập từng byte, cách ly lệnh cục bộ／dán, kích thước sai, tỷ lệ ảnh/ánh xạ chuột ANSI half／quadrant／ASCII.
- Mô phỏng tỷ lệ khác 0.5 với ký tự 9×20 pixel, viewport 120×40 ký tự, nguồn 1920×1080: mặc định cũ sai tỷ lệ khoảng 8%～9.3%; dùng kích thước thật còn 1.25%, phần dư là làm tròn lấy mẫu nguyên của lưới.
- POSIX PTY CLI sản xuất với truyền tải giả ngoài dự án đạt: phản hồi pixel vùng văn bản／hàng cột kiểu GNOME, kích thước ký tự trực tiếp, đổi font, co giãn cửa sổ, không trả／trả một phần／sai, về TTY, ghi đè thủ công, auto, Kitty. Đổi font không cần khung mới cũng vẽ lại được, thoát khôi phục terminal.
- Kiểm thử/sản phẩm mới ở thư mục tạm ngoài dự án; chưa nghiệm thu hình ảnh GNOME Terminal Linux thật hoặc SSH.

## Bổ sung tỷ lệ ký tự Windows và Pi／bridge (2026-10-30)

- Biên dịch CLI debug macOS arm64 đạt; chương trình chính, I/O nền tảng, phân tích nhập/kích thước Windows x64 qua biên dịch chéo, I/O liên kết độc lập đạt (gồm user32／advapi32), không nghĩa MiniRTC Windows biên dịch đầy đủ hay chạy máy thật.
- Mô phỏng Win32 API ngoài dự án dùng trực tiếp hàm đọc kích thước sản xuất, bao phủ font console hiển thị, ConPTY ẩn quay về, font thất bại／sai, nhả handle; ASan/UBSan đạt. ASan/UBSan phân tích kích thước và hồi quy POSIX PTY cũ lại đạt.
- Bridge sản xuất với truyền tải giả ngoài dự án kiểm chứng giữ trường tỷ lệ tùy chọn, số hữu hạn/phạm vi, tính nguyên tử yêu cầu sai, tỷ lệ bố trí thực, vẽ lại tĩnh, siêu dữ liệu tỷ lệ khung.
- Kiểu nghiêm ngặt SDK Pi 1.4.0 và chủ mô phỏng đạt: dùng SDK thật phân mảnh/ghép gói, trả lời cặp vùng văn bản, ưu tiên ký tự, đệm chủ, font／cửa sổ đổi, ghi đè, bỏ/ACK khung tỷ lệ cũ, hồi quy phím, dọn bộ nghe/hẹn giờ sau khởi động lỗi／thoát bất thường.
- Mọi mã/sản phẩm kiểm chứng mới ngoài dự án trong thư mục tạm hệ thống; chưa nghiệm thu hình ảnh Windows, GNOME／SSH hay Pi TUI thật.

## Tối ưu đường nối bốn góc phần tư ANSI (2026-10-30)

- Biên dịch CLI debug và `doctor --json` macOS arm64 đạt. ASan/UBSan ngoài dự án bao phủ gộp tương phản thấp, nét ngang/dọc／chéo／hai màu, biên màu đồng độ sáng, trần méo NV12 ngẫu nhiên, phạm vi cập nhật cục bộ, tính xác định chuyển đổi lặp.
- Mẫu tương phản thấp tổng hợp gộp cả 3,072 ký tự không trống thành nền thuần. Sai số nối có trọng số của chuyển sắc tổng hợp giảm khoảng 207,373 xuống 180,187 (13.1%); chỉ tính biên cả hai đầu chưa gộp thuần màu thì khoảng 198,007 xuống 169,621. Chỉ số đo độ lệch chênh màu tái tạo giữa mẫu kề so với chênh màu nguồn, giảm trọng số biên tương phản cao thật; không phải phần trăm giảm sọc trên máy thật.
- Kiểm tra đầu ra 8／24／256 màu, half／ASCII khớp bản cũ, bố trí/chuột theo viewport／tỷ lệ／thu phóng khác, sai phân lưới tĩnh, Kitty đều đạt.
- Benchmark tổng hợp cục bộ `-O2`, nguồn 1920×1080, viewport 240×100 ký tự, mỗi mục 12 chuyển đổi: hình mượt khoảng 7.14 → 6.03 ms/khung, nhiễu sáng ngẫu nhiên khoảng 5.44 → 7.73 ms/khung. Chỉ đo chuyển đổi, không giải mã video, xuất ANSI, vẽ terminal.
- Mã kiểm tra, benchmark, dữ liệu thêm ở thư mục tạm hệ thống ngoài dự án; không thêm kiểm thử trong dự án, không kết nối từ xa thật hay nghiệm thu hình ảnh ký tự／giãn dòng terminal.

## Tăng tốc chuyển đổi và đầu ra ANSI (2026-10-30)

- Biên dịch CLI debug và `doctor --json` macOS arm64 đạt. ASan/UBSan và `-O2` ngoài dự án đều đạt: màu／ký tự từng ô bản tăng tốc khớp bản tối ưu nối trước, gồm NV12 ngẫu nhiên, tương phản thấp/mượt, half／quadrant／ASCII, thu phóng/chi tiết; dữ liệu Kitty giữ nguyên.
- Mô phỏng chuỗi thoát ANSI xác nhận màn hình cuối giống nhau, bao phủ mọi ký tự góc phần tư, 8／24／256 màu, nền thuần/tái dùng bảng màu, ô đầu/cuối, sửa cục bộ, viewport／vị trí đổi và dòng bridge vẽ độc lập. Không thêm tệp/ca kiểm thử vào dự án.
- Cục bộ `-O2`, nguồn 1920×1080, viewport 240×100 ký tự, mỗi mục 24 chuyển đổi: hình mượt khoảng 6.26 → 4.83 ms/khung, nhiễu sáng ngẫu nhiên khoảng 7.75 → 5.72 ms/khung. Giữ lọc diện tích, gộp tương phản thấp, ràng buộc liên tục biên.
- Lưới TrueColor tổng hợp 240×100, mỗi mục 100 lần sinh đầu ra: cả khung khoảng 2.03 → 1.43 ms; sửa cục bộ 120×50 ô khoảng 1.02 → 0.36 ms, đầu ra 461,743 → 231,026 byte; sửa con trỏ 2×3 ô khoảng 0.072 → 0.013 ms, đầu ra 27,763 → 262 byte. Cả khung nền thuần 875,352 → 449,826 byte.
- Kết quả đo chuyển đổi, mã hóa, lượng đầu ra, không gồm vẽ terminal/mạng thật. Mẫu cả khung biến màu cao vẫn cần khoảng 923 KB, vẫn giới hạn ngân sách 1 MiB/s; không lấy nghịch đảo thời gian chuyển đổi làm FPS hiển thị từ xa. Mục tiêu cục bộ lúc đó 8 FPS; chính sách sau xem “Mặc định 30 FPS và thích ứng cấp khung”.
- Mã kiểm tra, mô phỏng, benchmark, sản phẩm thêm ở thư mục tạm ngoài dự án; chưa nghiệm thu terminal đa nền tảng hoặc FPS từ xa thực.

## Sửa đen giữa khung và thông lượng Kitty cục bộ (2026-10-30)

- Biên dịch debug, release macOS arm64 đạt, release `doctor --json` đạt. Release dùng cấu hình riêng ngoài dự án, giữ debug dự án; script kiểm tra, fixture PTY, dữ liệu thêm đều thư mục tạm hệ thống.
- ASan/UBSan và mô phỏng Kitty xác nhận: ID 31/32 luân phiên tải, giữ ảnh cũ đến khi xong mọi khối; tạo placement ảnh mới trước giải phóng cũ; tối đa hai ảnh đầy đủ thường trú, sau đổi chỉ một. RGB giải nén/kích thước cũ/mới khớp từng byte, bao phủ đổi kích thước/vị trí/thu phóng; một khung dưới 2MiB, dọn ngắt ở mọi biên khối đạt.
- 12 tổ hợp half／quadrant／ASCII và 8／24／256 màu ANSI giữ nguyên ô chuyển đổi/byte đầu ra so trước sửa.
- So PTY cục bộ với mã sản xuất `View`, `Output`, Kitty, `-O2`, callback NV12 tổng hợp liên tục: mục tiêu 8 FPS, khung khoảng 1.25 MB, tốc độ nhận đủ ổn định khoảng 0.84 → 6.71 FPS, trung vị khoảng khung 1,192 → 148 ms. Chỉ đo nhận hết đầu ra, không mạng thật/giải mã/GPU Ghostty, không hứa FPS máy thật.
- Ngân sách nhật ký Kitty cục bộ 8MiB/s; đặt riêng `SSH_TTY`, `SSH_CONNECTION`, `SSH_CLIENT` đều giữ 1MiB/s, ANSI cũng 1MiB/s. ID khung liên tiếp chỉ đổi sau ghi hết, thoát dọn hai ID.
- Thoát khi PTY chậm đang tải khung thứ hai: giữ khung đầu, chưa gửi khung hai, thoát sau khoảng 2.06s; dọn hai ID, khôi phục màn hình thay thế／chuỗi con trỏ/termios gốc. Kiểm thử thấy ghi dọn một lần cũ mất khi đầy đệm, nay thêm thử ghi ngắn không chặn tối đa 500ms.
- PTY mô phỏng shell cha giữ phiên, terminal ngừng đọc hoàn toàn: CLI về sau khoảng 2.53s, khôi phục termios/cờ không chặn. POSIX đổi sang `TCSANOW` và dọn nhập để tránh `TCSAFLUSH` chờ xả đầu ra. Không đọc hẳn thì không đảm bảo byte dọn màn hình tới nơi.
- Máy này có Ghostty 1.4.0 nhưng công cụ tự động bị từ chối truy cập cửa sổ, chưa nghiệm thu trực quan cửa sổ thật. CLI cài npm không tự cập nhật theo biên dịch cục bộ, đo lại cần chạy trực tiếp release lần này.

## Bộ nhớ chung Kitty cục bộ và mục tiêu 30 FPS (2026-10-30)

- Kitty POSIX cục bộ thêm dò bộ nhớ chung, tên duy nhất, quyền 0600, xác nhận tải, quá hạn 500ms quay về. Một đệm chờ không quá 1920×1080×3 byte; giữ hình cũ trước xác nhận, thoát giải phóng chưa xác nhận. Windows tạm giữ nội tuyến, SSH không dò bộ nhớ chung.
- Khi đó có bộ nhớ chung và không chỉ định `--fps` thì mục tiêu mặc định 30; tự về nội tuyến trở lại 8, FPS tường minh giữ nguyên. Chính sách sau xem “Mặc định 30 FPS và thích ứng cấp khung”. Mục tiêu trên 15 dùng chế độ tương tác giao thức xem trước cũ `fps=60, frame_rate=30`; không thêm giá trị giao thức hay sửa máy được điều khiển.
- ASan/UBSan ngoài dự án đạt: 110 tổ hợp khung／viewport／tỷ lệ／thu phóng／chi tiết ngẫu nhiên, lọc diện tích nguyên sai mỗi kênh tối đa 1/255 so tham chiếu số thực; Kitty nội tuyến/ANSI giữ đầu ra cũ. Bao phủ quyền bộ nhớ chung, nội dung đọc, tên, trần dung lượng, unlink hai bên, xác nhận phân mảnh, phản hồi ID sai／tràn／trùng.
- PTY với object release CLI thật, callback NV12 nhiễu tổng hợp 1920×1080 liên tục, viewport báo 1568×1088: nhận hết nội tuyến khoảng 4.06 FPS, bộ nhớ chung 29.04 FPS; xử lý cục bộ trung bình 166.6 → 12.3 ms, byte terminal khoảng 1,703,174 → 188 byte/khung. Bộ nhớ chung vẫn truyền RGB đầy đủ, không phải tổng chỉ 188 byte; sao chép, vẽ terminal, mạng thật còn chi phí khác.
- Tường minh 8 FPS, dò không hỗ trợ／im lặng, lỗi tải khung ba／mất xác nhận, xác nhận trễ 80ms, tắt dò SSH đều đạt. Không hiện chưa xác nhận trước quay về, quay về lúc chạy chỉ một lần, khôi phục mục tiêu tự động/yêu cầu giao thức đúng; dọn mọi đối tượng chung. Thoát ngay sau tải khung hai vẫn giữ khung đầu đã gửi, dọn chưa xác nhận và hai ID ảnh.
- Cùng đầu vào tổng hợp, debug khoảng 5.91 FPS, xử lý cục bộ 163 ms, cho thấy `-O0` vẫn ảnh hưởng rõ chuyển pixel; nghiệm thu hiệu năng dùng release. Số chính xác trên là tốc độ nhận cục bộ, không FPS GPU Ghostty.
- Người dùng chạy release mới trong Ghostty cục bộ phản hồi “mượt hơn rõ rệt”. Nguồn kiểm thử, dữ liệu, fixture biên dịch, sản phẩm thêm đều thư mục tạm hệ thống, không thêm kiểm thử dự án/submodule.
- Nhật ký máy thật lần đó xác nhận `kitty_transport=shared_memory`, mục tiêu 30, yêu cầu xem trước được chấp nhận, quay về lúc chạy 0 lần. Phiên khoảng 13.61s gửi đủ 273 khung, trung bình 20.06 FPS (gồm chờ đầu khoảng 1.49s); xử lý cục bộ trung bình 10.87ms, xác nhận bộ nhớ chung 0.42ms. Nhật ký ghi tốc độ gửi, không đo tần số trình bày GPU terminal.

## Mặc định 30 FPS và thích ứng cấp khung (2026-10-30)

- CLI tương tác thống nhất mục tiêu cục bộ 30 FPS, quay về Kitty bộ nhớ chung không sửa mục tiêu nữa; yêu cầu từ xa thích ứng riêng giữa 8／15／30 FPS, 15 cần khả năng máy từ xa. `--fps` tường minh giữ yêu cầu cố định, bridge mặc định vẫn 8.
- Biên dịch debug／release macOS arm64, release `doctor --json`, kiểm tra trợ giúp đạt. ASan/UBSan ngoài dự án kiểm chứng ngưỡng tăng/giảm, trễ chuyển mức, phục hồi, thiếu 15 FPS, FPS tường minh, nguồn thưa, từ chối yêu cầu, đặt lại ước lượng.
- PTY với object release CLI thật, truyền tải giả hỗ trợ xác nhận xem trước/cấp khung động, NV12 tổng hợp 1920×1080, viewport pixel 1568×1088. Nguồn cố định 30 FPS qua bộ nhớ chung nhận đủ khoảng 30 FPS, không tự giảm mức; không còn dùng khe làm mới sớm khi chưa có khung mới.
- Nội tuyến entropy cao thực khoảng 4 FPS thì tự hạ yêu cầu 30 xuống 8, mục tiêu cục bộ giữ 30; `--fps 30` cùng điều kiện vẫn khoảng 4 FPS, yêu cầu từ xa không đổi. Lỗi tải／quá hạn/dò im lặng đều giữ mục tiêu cục bộ 30, rồi chỉnh cấp khung theo thời gian đầy đủ.
- Xác nhận tải trễ 90ms khiến nhận khoảng 9 FPS, yêu cầu tự chỉnh 15; thiếu khả năng 15 thì không gửi mức đó. Sau trễ 200ms trở lại xác nhận bình thường, xác minh yêu cầu phục hồi 30 → 8 → 30. Nguồn chỉ 2 FPS, máy từ xa từ chối xem trước đều không tự giảm sai.
- SSH giữ ngân sách 1MiB/s, không dò bộ nhớ chung; xuất chậm giảm yêu cầu từ xa nhưng giữ mục tiêu cục bộ. ANSI mặc định 30／tường minh 15, bridge mặc định 8／ACK đạt. Mọi cảnh Kitty tiếp tục xác minh không hiện chưa xác nhận, tối đa hai ảnh terminal, giải phóng đối tượng chung, khôi phục terminal sau thoát.
- Số trên là tốc độ nhận đầu ra đầy đủ từ nguồn tổng hợp/PTY, không giải mã thật/mạng/GPU Ghostty. Mọi mã kiểm thử, dữ liệu, script biên dịch, sản phẩm thêm ở thư mục tạm hệ thống ngoài dự án; không thêm kiểm thử dự án lần này, chưa nghiệm thu cửa sổ Ghostty thật bản mới.

## Sai phân nhiều đoạn ANSI và thông lượng cục bộ (2026-10-31)

- Ngân sách liên tục ANSI tương tác cục bộ tăng 1MiB/s lên 8MiB/s; SSH/bridge vẫn 1MiB/s. Giữ trần thông điệp 2MiB, đột biến 1MiB, ghi ngắn không chặn, thay khung mới nhất, chốt mốc sai phân sau xuất đầy đủ.
- Sửa phân tán cùng dòng xuất nhiều đoạn, gộp khoảng ngắn theo chi phí byte mã hóa thực, tính cả đặt màu ô tiếp. Sai phân bỏ màu giống sau lượng tử, màu chữ dấu cách, trường ký tự／nền ASCII không hiện; không sửa lấy mẫu, khớp góc phần tư, truyền Kitty.
- CLI debug／release macOS arm64 và `doctor --json` cả hai đạt, xong khôi phục debug bật máy tính+CLI. ASan/UBSan ngoài dự án đạt: mọi ký tự góc phần tư, 8／24／256 màu, ASCII, ô đầu/cuối, khoảng dài/ngắn, bảng màu tương đương, cỡ/vị trí khác, màn hình 600 khung cập nhật ngẫu nhiên liên tục khớp vẽ lại cả khung; 140 chuyển đổi khớp từng ô, dòng bridge/dữ liệu Kitty giữ nguyên.
- `-O2`, lưới tổng hợp 240×100: 10 dòng mỗi dòng hai sửa phân tán giảm 90,058 xuống 931 byte; sửa cách cột 919,260 xuống 562,601 byte; cả màn hình đổi nhưng màu sau lượng tử giống giảm 72,716 xuống 0 byte. Cả khung biến màu cao vẫn 922,929 byte, con trỏ 2×3 vẫn 262 byte. Chọn nhiều đoạn tăng tính mã hóa trong một số cảnh xen kẽ để giảm đầu ra terminal.
- So PTY cục bộ bằng CLI／renderer sản xuất `-O3`, NV12 tổng hợp 1920×1080 đổi liên tục, viewport 240×100 ký tự, mục tiêu 30 FPS cố định: sau bỏ khởi động nhận đủ khoảng 1.64 → 6.42 FPS, khoảng 3.9 lần; trung bình khung 640 KB. Đặt riêng `SSH_TTY`, `SSH_CONNECTION`, `SSH_CLIENT` ngân sách đều 1MiB/s; ANSI cục bộ có `TMUX` dùng 8MiB/s.
- Dừng đọc PTY 1.3s, gửi thoát trong lúc dừng; đọc lại thì bộ đếm khung nhận đủ/gửi khớp, thoát trong khoảng 1.17s, khôi phục màn hình thay thế, termios, cờ không chặn. Không kết nối máy từ xa thật, chưa kiểm chứng GPU terminal thật, SSH, tmux hiển thị thực; FPS là nhận đủ từ nguồn tổng hợp.
- Nguồn kiểm tra, script, dữ liệu, sản phẩm thêm ở thư mục tạm hệ thống ngoài dự án, không thêm/sửa tệp／ca kiểm thử trong dự án.

## Đầu ra đồ họa Sixel (2026-11-02)

- Thêm `--renderer sixel`; auto chọn Kitty → Sixel → ANSI, dò Sixel bằng tham số DA1 `4`. Pixel tường minh không phản hồi hợp lệ hoặc tmux/screen thì từ chối; `doctor --json` thêm `sixel=probe_on_view`, không chủ động dò.
- Biên dịch CLI debug macOS arm64, trợ giúp, `doctor --json` đạt. ASan/UBSan ngoài dự án bao phủ phân tích phản hồi khả năng, giới hạn viewport 20,000 cỡ ngẫu nhiên/tính lặp nhất quán, 7 ảnh NV12 tổng hợp (đen, trắng, chuyển sắc, màu, thiếu sáu dòng, cắt/thu phóng, viewport lớn).
- Bộ giải mã Sixel độc lập ngoài dự án xác minh phủ pixel đầy đủ, sai màu lượng tử, biên ảnh, lưu／khôi phục con trỏ; mẫu entropy cao 1920×1080 kích hoạt về 4 mức xám, đầu ra khoảng 1.30MB, giữ cỡ gốc và dưới trần thông điệp.
- 11 POSIX PTY dùng CLI／I/O sản xuất với phiên giả ngoài dự án đạt: ưu tiên Kitty, chọn Sixel tự/tường minh, DA1 phân mảnh, lỗi／không phản hồi quay về, tường minh thất bại, hạn chế tmux/screen, đổi cửa sổ/thu phóng chi tiết; SSH giữ 1MiB/s, thoát khôi phục termios/màn hình thay thế, Sixel gửi lệnh khôi phục DECSDM.
- Không kết nối từ xa thật, chưa nghiệm thu Xshell, terminal Sixel khác hay SSH thật, không đảm bảo phiên bản Xshell nào hỗ trợ. Nguồn/script/dữ liệu/sản phẩm kiểm thử thêm ở thư mục tạm hệ thống ngoài dự án, không thêm/sửa tệp／ca kiểm thử trong dự án.

## Giới hạn đo thực tế

Kiểm tra cục bộ không gồm biên dịch đầy đủ Linux / Windows, điều khiển đa nền tảng, hiển thị terminal Kitty/Sixel/SSH thật, Pi TUI thật hay tài nguyên dài hạn. Cấu hình CI và tên gói npm năm nền tảng không nghĩa đã phát hành/nghiệm thu.

CLI cần máy chủ hỗ trợ đăng ký tạm `cli`. `doctor --json` chỉ báo môi trường cục bộ, không đăng nhập mạng hay chứng minh máy từ xa kết nối được.
