# Ứng dụng terminal

[Điểm vào CLI](../README.md) · [Cài npm](npm-distribution.md) · [Giao diện cầu nối](bridge-protocol.md) · [Biên bản kiểm chứng](validation.md)

## Biên dịch và cài đặt

Cần Git, Xmake >= 3.1.1, trình biên dịch C++17, header/thư viện liên kết zlib hệ thống và công cụ hệ thống dùng khi biên dịch gói MiniRTC. macOS SDK đã có zlib; Linux cần gói phát triển tương ứng (như zlib1g-dev); Windows dùng Visual Studio C++ Build Tools, Xmake lấy zlib và liên kết tĩnh, runtime dùng MT. Phụ thuộc cần truy cập được kho gói Xmake. Linux dùng GCC/Clang; macOS dùng Xcode Command Line Tools. Không cần cài Slint, SDL, X11, PulseAudio hay SDK ghi màn hình.

Clone mã nguồn trước; nếu đã có kho, chạy chuẩn bị phụ thuộc và biên dịch từ gốc:

```sh
git clone --recurse-submodules https://github.com/kunkundi/crossdesk.git
cd crossdesk
./apps/cli/scripts/prepare-deps.sh
xmake f -y --CROSSDESK_BUILD_DESKTOP=n --CROSSDESK_BUILD_CLI=y --USE_CUDA=n -m release
xmake build crossdesk-cli
xmake run crossdesk-cli doctor --json
```

`prepare-deps.sh` khởi tạo submodule MiniRTC theo commit ghi trong kho chính, có thể chạy lại. Che thông tin xác thực trong nhật ký đã hợp nhất vào MiniRTC, không cần bản vá thêm; nâng MiniRTC chỉ cần đổi commit submodule. Kho mặc định bật biên dịch máy tính, tắt CLI. Tham số trên chỉ biên dịch CLI; nếu cần cả máy tính và CLI, đặt `CROSSDESK_BUILD_DESKTOP` thành `y`. Khôi phục cấu hình máy tính mặc định bằng `xmake f -y --CROSSDESK_BUILD_DESKTOP=y --CROSSDESK_BUILD_CLI=n`. Mọi lệnh chạy từ gốc kho.

Sản phẩm ở `build/<platform>/<arch>/<mode>/crossdesk-cli`. Có thể thêm thư mục tương ứng vào PATH hoặc sao chép tệp thực thi vào bin của bạn. Pi cần khởi động trực tiếp tệp này, không dùng `xmake run` có xuất nhật ký biên dịch làm chương trình cầu nối.

Phụ thuộc MiniRTC macOS vẫn liên kết VideoToolbox/CoreMedia và framework hệ thống như AppKit do GLib khai báo; CLI không tạo GUI, bộ ghi hình hay thiết bị âm thanh. Tham số giải mã phần cứng tắt tường minh. Linux không cần DISPLAY/WAYLAND_DISPLAY; đợt này chưa biên dịch kiểm chứng thực tế trên máy Linux.

Windows chỉ hỗ trợ x64, sản phẩm `build/windows/x64/release/crossdesk-cli.exe`; chạy `sh apps/cli/scripts/prepare-deps.sh` trong Git Bash trước rồi các lệnh Xmake trên trong Developer PowerShell, thêm `-p windows -a x64` khi cấu hình. Biên dịch trong Git Bash cũng bắt buộc chỉ định `-p windows` để tránh nhận thành MSYS. Windows dùng sự kiện bàn phím/chuột native Console / ConPTY và đầu ra VT, lưu/khôi phục chế độ console và bảng mã; không cần WSL. Windows Terminal, PowerShell / CMD hiện đại và terminal ConPTY VS Code là môi trường mục tiêu, vẫn cần nghiệm thu máy thật. Nếu cửa sổ mintty Git Bash riêng không có console native, nên khởi động qua cấu hình Git Bash của Windows Terminal.

Người dùng gói npm biên dịch sẵn không cần công cụ biên dịch; xem [phân phối npm](npm-distribution.md).

## Kết nối và danh tính

```sh
crossdesk-cli 123456789
crossdesk-cli 123456789 --renderer ansi --fps 8 --quality high --zoom native
crossdesk-cli 123456789 --renderer kitty --quality ultra --zoom native --display 0
crossdesk-cli 123456789 --control
crossdesk-cli 123456789 --server your-server.example --port 9099
```

Đặt ID thiết bị làm tham số đầu để kết nối; `crossdesk-cli view <ID-thiết-bị> [tùy-chọn]` giữ làm bí danh tương thích, hỗ trợ cùng tùy chọn. ID bắt buộc có 9 chữ số, giữ số không đầu, các số trên chỉ là ví dụ. Địa chỉ tín hiệu mặc định theo CrossDesk hiện tại: `api.crossdesk.cn:9099`. Nhập mật khẩu ẩn từ terminal cục bộ (POSIX `/dev/tty`, Windows `CONIN$` / `CONOUT$`), không có tham số mật khẩu dòng lệnh. CLI không đọc/sửa cấu hình máy tính. Mỗi lần kết nối dùng trực tiếp `cli` xin máy chủ cấp ID tạm `cli-xxxxxxxxx` (hậu tố 9 chữ số), đăng ký, Join và đa phương tiện dùng cùng một Peer. CLI chỉ làm máy điều khiển, không xin danh tính số thường hay `C-`, không lưu mật khẩu máy này hoặc thông tin kết nối lại. Mỗi khởi động/kết nối lại xin ID mới; sau ngắt, máy chủ dọn đăng ký theo quy tắc danh tính Web tạm.

Cần triển khai crossdesk-server mới hỗ trợ đăng ký `cli` trước; máy chủ cũ sẽ đăng nhập thất bại, ứng dụng không chuyển về danh tính thiết bị thường. `status.client_id` cầu nối trả ID máy chủ cấp.

Quy trình: tạo Peer điều khiển, đăng ký luồng → đăng ký bằng `cli` → SignalConnected trả `cli-xxxxxxxxx` → Join → Connected → bắt tay HostInformation hiện có → chọn màn hình, thương lượng khả năng → video. MiniRtcInit trả 0 không có nghĩa kết nối từ xa thành công. Tổng hạn kết nối mặc định 30 giây; mất tín hiệu kết thúc phiên này, người dùng kết nối lại, không tự lặp thử mật khẩu.

Giao diện tương tác phân biệt `MiniRtcConnectionConnecting to server` (kết nối signaling), `MiniRtcConnectionConnecting to device` (kết nối thiết bị), `MiniRtcConnectionConnected, waiting for first frame` (đã kết nối, chờ khung đầu) và `Displaying` (đang hiển thị). Trước khung đầu có lời chờ giữa màn hình và số giây; giai đoạn kết nối đếm từ lúc yêu cầu, giai đoạn chờ khung đầu đếm lại từ lúc kết nối thành công. Chỉ chuyển `Displaying` và xóa lời chờ sau khi ghi đầy đủ khung đầu vào terminal. Đã kết nối mà chưa có hình tạm thời không vì vậy thành kết nối thất bại, cũng không có nghĩa chưa bật relay. Bên phải thanh trạng thái là bitrate video nhận; có thống kê mạng thì hiện `MiniRtcTraversalP2P` hoặc `RELAY`, bitrate theo bps, kbps, Mbps; ban đầu bằng không hoặc rất thấp vẫn tiếp tục chờ video.

Giữ xác minh chuỗi chứng chỉ và tên máy MiniRTC WSS, cấp thông tin TURN tạm và thương lượng SRTP. Không có tham số bỏ kiểm tra chứng chỉ. Thống kê trạng thái thể hiện SRTP có thương lượng thành công thực không; giữ tương thích MiniRTC với bản cũ, không khẳng định mọi bản cũ chắc chắn bật SRTP. Lỗi chứng chỉ tự triển khai nên sửa chứng chỉ/chuỗi tin cậy hệ thống.

## ANSI, Kitty, Sixel và SSH

ANSI mặc định dùng ký tự bốn góc phần tư: mỗi ký tự chọn hai màu và hình góc phần tư có sai số nhỏ nhất từ mẫu 2×2, gấp đôi điểm lấy mẫu ngang so với khối nửa trên/dưới trước đây; mỗi ký tự vẫn chỉ hai màu nền/chữ. `--ansi-detail half` quay về khối nửa trên/dưới để tương thích font. Thu nhỏ dùng lọc trọng số diện tích để tránh lấy mẫu lân cận gần bỏ sót nét mảnh. Hỗ trợ TrueColor, 256 màu và 8 màu cơ bản; 256 màu chọn màu gần hơn trong khối màu thực hoặc thang xám. COLORTERM/TERM môi trường quyết định chế độ màu; locale không UTF-8 dùng dấu cách màu nền. TERM=dumb, đầu ra không TTY từ chối kết nối tương tác rõ ràng, không xuất chuỗi thoát toàn màn hình.

Bốn góc phần tư gộp ô tương phản thấp mà mọi kênh RGB đổi không quá 6/255 thành nền thuần, giảm đường nối ký tự ở vùng phẳng. Ô còn lại khớp riêng rồi tham khảo sai số lượng tử láng giềng trên/dưới/trái/phải để chỉnh màu, giữ chuyển sắc nguồn thực, giảm ràng buộc ở biên tương phản cao; điều chỉnh có trần méo, nét chữ mảnh và biên hai màu vượt ngưỡng tương phản thấp mà biểu diễn chính xác được giữ nguyên. Tự áp dụng cho CLI riêng và hình góc phần tư Pi／bridge, kết quả vẫn tùy font và giãn dòng terminal.

Hình co giãn giữ tỷ lệ theo ô ký tự, chừa viền đen và thanh trạng thái. Kết nối tương tác truy vấn kích thước ký tự qua luồng byte terminal, hỗ trợ phản hồi qua SSH: ưu tiên pixel ký tự `CSI 16 t`, nếu không kết hợp pixel toàn vùng văn bản `CSI 14 t` và tổng hàng/cột `CSI 18 t` để tính, tương thích báo cáo vùng văn bản GNOME/VTE. Tính xong mới trừ thanh trạng thái và giới hạn hàng/cột ảnh, không tính dòng dành riêng vào chiều cao ảnh. Truy vấn lại khi cửa sổ đổi, làm mới mỗi hai giây để bao phủ đổi font／giãn dòng; nhận phản hồi thì vẽ lại hình tĩnh, cập nhật ánh xạ chuột. Truy vấn không chặn kết nối; không phản hồi hợp lệ thì dùng pixel hệ thống, thiếu nữa thì tỷ lệ 0.5. Nếu từng nhận phản hồi hợp lệ, giữ kích thước hợp lệ gần nhất đến phản hồi hợp lệ tiếp. Làm tròn nguyên lưới ký tự vẫn có thể gây sai tỷ lệ nhỏ. Định dạng theo [báo cáo kích thước XTerm](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html) và [triển khai GNOME VTE](https://github.com/GNOME/vte/blob/master/src/vteseq.cc).

Console truyền thống hiển thị của Windows ưu tiên kích thước font hiện tại; Windows Terminal／ConPTY dùng phản hồi kích thước từ terminal hiển thị, không dùng font máy chủ console ẩn. Phiên bản terminal có thể không hỗ trợ truy vấn hoặc trả kích thước đồ họa ảo thay vì font thực, khi đó cần hiệu chỉnh thủ công.

Mặc định `--cell-aspect auto` tự xác định tỷ lệ. Nếu terminal chặn truy vấn hoặc cần hiệu chỉnh thủ công, đặt `--cell-aspect 0.45`, giá trị là **rộng／cao** một ký tự, cho phép 0.2～2; ví dụ ký tự 9×20 pixel dùng 0.45. Hình bị hẹp thì giảm, bị kéo rộng thì tăng. Tham số ưu tiên hơn tự dò, dùng cho toàn cảnh, chi tiết và ánh xạ chuột.

Mỗi lần CLI riêng chỉ xuất khoảng giữa ô đầu/cuối thay đổi trong dòng đổi; con trỏ/video cục bộ không ghi lại cả dòng nữa. Tái dùng trạng thái màu trong khung, gộp thiết lập màu theo chỉ số bảng màu thực, bỏ lệnh màu chữ cho dấu cách nền thuần. Pi／bridge vẫn gửi dòng đầy đủ vẽ độc lập, đồng thời dùng tối ưu mã hóa màu trên. Khớp ban đầu bốn góc phần tư dùng tổng tập con số nguyên tính màu/sai số cùng độ chính xác; khớp liên tục biên tái dùng mẫu có trọng số, giảm tính lặp.

Thu phóng dọn hình cũ tại ranh giới đầu ra đầy đủ. Tối đa 240 cột, 100 dòng ảnh; mặc định chừa một dòng trạng thái trên, hai dòng phím tắt dưới và một cột tránh tự xuống dòng. Thanh trạng thái đảo màu: trái chế độ điều khiển/tiến độ, phải tham số hình/mạng; hai dòng dưới lần lượt phím tắt phiên/khung nhìn. Video và lời chờ bắt đầu dưới thanh trên, tọa độ chuột theo vùng video, nhấn thanh trạng thái không gửi từ xa. Nhóm phím dùng chiều rộng cột và lề thống nhất; cửa sổ hẹp tự giảm cột, gộp nhóm, bỏ tham số phụ, tổng khung/byte đầu ra giữ trong nhật ký. Chế độ điều khiển chỉ gợi ý điểm vào lệnh cục bộ và thao tác sau; chỉ khi vào lệnh cục bộ mới hiện phím di chuyển/thu phóng; thiếu chiều cao terminal thì ưu tiên thu dòng gợi ý dưới.

Dòng phím tắt dùng tiêu đề rộng cố định như `KEYS |`, `VIEW |`, ngăn tiêu đề/nhóm phím bằng gạch đứng; tên phím gắn nhãn `<c>`, `<+/->`... `Zoom at mouse` là thu phóng neo chuột, `Full view` / `Full` khôi phục toàn cảnh, `Pan` di chuyển. Chỉ đọc gợi ý lăn để thu phóng, rộng còn gợi ý nhấn vào chi tiết; lệnh cục bộ gợi ý quay lại, đổi điều khiển, thao tác khung nhìn bằng phím, rộng hiện chọn neo chuột và `=` phóng to. Phân cách nhóm rộng căn cột; hẹp giữ nhãn phím đầy đủ, ưu tiên thu phóng/toàn cảnh, gộp/giảm nhóm theo chiều rộng.

Thanh trên hiện FPS thực theo số nguyên (như `FPS 30`), khoảng mỗi giây tính từ số khung xuất hoàn chỉnh ra terminal trong khoảng thống kê gần nhất; không có hình mới thì `FPS 0`. Đây là tốc độ gửi thực, không phải mục tiêu `--fps` hay tần số trình bày GPU terminal; cửa sổ hẹp ưu tiên bỏ tham số hình và mạng, giữ FPS.

ANSI vẫn không giữ được chữ cỡ thường trong hình thu nhỏ toàn màn hình. Tăng cột terminal trước (giảm font hoặc mở rộng cửa sổ), đọc vùng cục bộ dùng `--zoom native`. Chỉ đọc có thể dùng `+/-` hoặc con lăn thu phóng tại chuột, hoặc nhấn đích vào chi tiết vùng đó. Chi tiết chọn vùng nguồn theo viewport và tỷ lệ ký tự để tránh thu nhỏ chữ cho vừa cả màn hình; tầm nhìn ANSI nhỏ, dùng mũi tên để đọc vùng khác. `--zoom 1..32` là tỷ lệ so cả màn hình; `+/-` trong chi tiết là tỷ lệ so kích thước chi tiết nguyên bản. Thu phóng hoàn toàn cục bộ, tọa độ chuột điều khiển ánh xạ về vùng đúng trên màn hình gốc.

auto gửi truy vấn Kitty và thuộc tính thiết bị chính (DA1), chờ tối đa 250ms; ưu tiên Kitty, nếu không chọn Sixel khi thấy `4` trong tham số khả năng DA1, rồi về ANSI. Dò thất bại khi chỉ định `kitty`/`sixel` thì báo lỗi, không ép xuất ảnh vào terminal không rõ. auto trong tmux/screen dùng ANSI, chế độ pixel tường minh báo không hỗ trợ; không sửa cấu hình terminal người dùng.

Kitty cục bộ macOS/Linux còn dò `t=s` bằng đối tượng bộ nhớ chung 1×1 pixel, chờ tối đa 250ms; thành công thì ghi RGB chưa nén vào POSIX shared memory quyền 0600, chỉ gửi tên và lệnh hiển thị qua luồng byte terminal. Mỗi lần tên duy nhất, tối đa một bộ đệm chung chờ xác nhận (1920×1080×3 byte). Chỉ hiện ảnh mới và giải phóng đệm sau xác nhận tải thành công; lỗi tải, xác nhận quá 500ms hoặc cấp phát cục bộ thất bại thì phiên này về truyền nội tuyến, giữ hình cũ. Terminal và CLI đều dọn đối tượng chung, thoát cũng giải phóng đối tượng chưa xác nhận. SSH không thử bộ nhớ chung; Windows tạm dùng nội tuyến.

Kitty nội tuyến gửi RGB/Base64 nén zlib qua luồng byte terminal (mỗi khối tối đa 4096 ký tự), không dùng tệp ảnh tạm từ xa. Cả hai đường luân phiên ID ảnh=31/32: tải vào ID chưa hiện bằng `a=t`, xong tạo placement=1 bằng `a=p`, rồi xóa ảnh cũ. Chỉ luân phiên ID sau khi xuất hết lệnh hiển thị khung; giữ ảnh cũ khi tải/ghi ngắn, thoát dọn cả hai ID. Tránh đen giữa khung do terminal bỏ ảnh cũ trước khi truyền lại cùng ID.

Sinh hình theo pixel terminal báo (không rõ thì ước ký tự 10×20 pixel), tối đa 1920×1080. Giữ lọc diện tích, viền đen, tỷ lệ, tọa độ cắt; chi tiết nguyên bản cắt vùng nguồn theo viewport pixel. Đường bộ nhớ chung dùng trọng số diện tích nguyên tái dùng tính hàng ngang, tránh lặp số thực từng pixel, không giảm mẫu thêm vì ngân sách Base64 nữa. Không có pixel đáng tin thì dùng ước lượng, không hứa khớp từng pixel vật lý màn hình. Nén nội tuyến vượt ngân sách một lần xuất vẫn giảm kích thước từng mức; video entropy cao có thể thấp hơn FPS mục tiêu. Truyền/nén RGB không mất pixel không có nghĩa lấy mẫu diện tích hay mã hóa video thượng nguồn không mất dữ liệu.

Sixel truyền ảnh qua byte terminal, không cần thư viện ảnh thêm hay tệp tạm: mã hóa mỗi sáu dòng pixel theo mặt phẳng màu, nén lặp. Mỗi khung tối đa 248 màu (khối RGB cộng thang xám); ảnh phức tạp quá khoảng 1.9MB giảm dần 43 màu, 16 màu, cần thì 4 mức xám để giữ trần 2MiB một thông điệp cũ. Sixel mất màu do lượng tử; không có placement co giãn ảnh như Kitty, nên giới hạn hàng/cột theo pixel ký tự, giữ trong 1920×1080, thu phóng/chi tiết/chuột cùng viewport. Mỗi khung phủ cả vùng ảnh (gồm viền đen), phần cuối thiếu sáu dòng không vẽ vào thanh trạng thái; lưu／khôi phục DECSDM để ảnh bắt đầu từ con trỏ vùng video, thoát khôi phục. Không nhận pixel thì ước ký tự 10×20, căn thực vẫn tùy báo cáo terminal. Hình entropy cao hoặc SSH chậm nên thử `--fps 8` trước.

```sh
crossdesk-cli 123456789 --renderer sixel --fps 8
```

CLI hỗ trợ Sixel không đồng nghĩa Xshell chắc chắn hỗ trợ. [Danh sách tính năng Xshell chính thức](https://www.netsarang.com/en/xshell-all-features/) và [lịch sử cập nhật](https://www.netsarang.com/en/xshell-update-history/) đã xem chưa ghi hỗ trợ Sixel; dựa phản hồi khả năng và thử hiển thị thực, không thể chỉ đổi `TERM` để có ảnh pixel. Không phản hồi khả năng có thể dùng tiếp `--renderer ansi --zoom native`. Sixel và bit DA1 theo [chuỗi điều khiển XTerm](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Sixel-Graphics).

```sh
ssh -t your-server
crossdesk-cli 123456789 --renderer ansi --fps 8
```

Terminal cục bộ người dùng hiển thị byte SSH. Chưa kiểm chứng tương tác máy thật ANSI, Kitty / Sixel qua SSH và SSH+tmux. Không GUI nói về máy điều khiển; máy được điều khiển vẫn cần phiên đồ họa ghi được.

## Xem trước và áp lực ngược

`--fps 1..30` là mục tiêu đầu ra cục bộ. ANSI, Sixel, Kitty nội tuyến và bộ nhớ chung của CLI tương tác mặc định đều 30 FPS; chuyển về nội tuyến khi bộ nhớ chung lỗi vẫn giữ mục tiêu. bridge vẫn mặc định 8 FPS theo giao thức ứng dụng tích hợp.

Xem trước từ xa của CLI riêng và bridge đều yêu cầu ưu tiên độ rõ nét (giữ độ phân giải), gồm mặc định 30 FPS, `--fps` tường minh và tự đổi tốc độ xem trước. Tăng FPS không đổi chiến lược mã hóa về ưu tiên FPS; độ phân giải thực vẫn bị giới hạn bởi trần chất lượng, kích thước nguồn, hỗ trợ máy được điều khiển và điều kiện mã hóa／truyền tải.

Nếu không chỉ định `--fps`, xem trước từ xa bắt đầu 30 FPS, tự chọn yêu cầu 8／15／30 FPS theo thời gian từ bắt đầu chuyển đổi đến xuất xong cả khung; gồm chờ xác nhận tải bộ nhớ chung, không gồm thời gian rảnh chờ khung nguồn. Dùng ước lượng làm mượt, ngưỡng tăng/giảm khác nhau và ít nhất 8 mẫu khung mới, chậm liên tục 2 giây mới giảm, phục hồi liên tục 4 giây mới tăng, giữ dư địa cấp khung. 15 FPS chỉ dùng khi máy từ xa tuyên bố hỗ trợ; xem trước chưa được chấp nhận thì không tự điều chỉnh thêm. Đổi viewport, chuyển màn hình, quay về từ bộ nhớ chung đặt lại ước lượng. `--fps` tường minh cố định mục tiêu cục bộ và yêu cầu từ xa, tắt tự chỉnh; ví dụ `--fps 30` để đối chiếu chiến lược tự động.

Lập lịch cục bộ chỉ dùng khe làm mới khi có khung mới hoặc cần vẽ lại; trễ ngắn giữ nhịp, chặn lâu thì bỏ khe cũ, không vẽ bù liên tục khung lỗi thời. FPS mục tiêu không phải FPS terminal hiển thị thực.

Nếu máy từ xa khai báo PreviewSubscription, yêu cầu không quá 15 ánh xạ FPS hợp lệ 1/8/15 (15 cần khả năng tương ứng); trên 15 yêu cầu chế độ tương tác sẵn có: giao thức `fps=60` là dấu chế độ, `frame_rate=30` trong mã hóa mới là FPS. Trần độ phân giải dùng v2 thương lượng được, chỉ gửi ngân sách video khi có khả năng bandwidth. `--quality low|balanced|high|ultra` tương ứng:

| Mức | Trần độ phân giải | Enum chất lượng | Ngân sách video |
|---|---|---|---|
| low | 640×360 | 0 | 1Mbps |
| balanced | 1280×720 | 1 | 2.5Mbps |
| high (mặc định) | 1920×1080 | 2 | 6Mbps |
| ultra | 3840×2160 | 2 | 12Mbps |

Đây là trần yêu cầu, máy được điều khiển quyết định kích thước và bitrate thực; ultra không biến nguồn độ phân giải thấp thành 4K thật. Đường băng thông thấp có thể chọn balanced/low tường minh. Thu cửa sổ không hạ xem trước từ xa về cỡ lưới ký tự nữa; độ rõ đã chọn dùng khi phóng cục bộ sau. Bản cũ vẫn xem cơ bản, giới hạn FPS/co giãn cục bộ, không gửi mở rộng chưa khai báo. Kiểm tra request_id/ack/accepted xem trước qua trạng thái cầu nối; chưa xác nhận hay bị từ chối không giả thành công, cũng không chặn video cơ bản.

Callback đa phương tiện chỉ sao chép đồng bộ NV12 mượn, kiểm tra kích thước, dung lượng, mô tả native và stride; không giữ con trỏ mượn. Trần 16,777,216 pixel/khung, mỗi cạnh 8192, kích thước chẵn. Tối đa một khung chờ chuyển đổi; xác minh/sao chép trước thay thế, khung sai không đẩy hình có sẵn đi; trình phát giữ thêm khung chuyển đổi gần nhất để phóng/vẽ lại màn hình tĩnh, khi nhận mới thu hồi đệm trước cho sao chép sau. Chỉ bỏ ảnh sau giải mã. Chuyển màn hình/đóng tăng generation, vô hiệu khung cũ.

Chuyển đổi/xuất ở vòng sự kiện sở hữu phiên, độc lập callback đa phương tiện. Đầu ra tối đa một thông điệp đầy đủ (2MiB), ghi không chặn từng khối theo ngân sách token, trần đột biến vẫn 1MiB. ANSI, Kitty, Sixel tương tác cục bộ dùng 8MiB/s; có `SSH_CONNECTION`, `SSH_CLIENT` hoặc `SSH_TTY` thì cả ba giữ 1MiB/s, cầu nối Pi luôn 1MiB/s. Không bắt đầu hình mới được thì giữ khung giải mã mới nhất. Sai phân ANSI so màu/ký tự terminal dùng thật, bỏ màu lượng tử giống nhau và màu chữ không thấy của dấu cách; một dòng có thể xuất nhiều đoạn đổi, gộp khoảng ngắn theo chi phí byte thực của định vị con trỏ, đổi màu, ký tự. Chỉ chốt mốc sai phân sau xuất hết khung. Pi còn yêu cầu ACK dùng khung, tối đa một khung chờ; ánh xạ nhập chỉ sang lưới mới sau ACK tương ứng. Thanh trạng thái ưu tiên cập nhật giữa đầu ra hình đầy đủ, tránh bị bỏ đói khi ảnh lớn làm mới liên tục; sự kiện ghi được đánh thức thử ghi ngắn, vẫn theo ngân sách. Sự kiện điều khiển tối đa 64, đầu vào giao thức 64KiB/lồng 32 tầng, đệm dán 32KiB, văn bản mỗi lần tối đa 16KiB.

Đo FPS render pixel bằng release; lấy mẫu diện tích `-O0` debug tốn hơn rõ. Pixel bộ nhớ chung không qua ngân sách byte terminal, nhưng vẫn giới hạn bởi tốc độ chuyển đổi/tiêu thụ; 8MiB/s là trần đầu ra nội tuyến, không đảm bảo FPS. Giải mã, chuyển pixel, nén, tốc độ terminal và cỡ khung đều ảnh hưởng. Thoát thì hình chưa xong ghi tiếp tối đa 2s; ghi ngắn hủy truyền ảnh/khôi phục màn hình thử thêm tối đa 500ms. POSIX khôi phục termios và dọn đầu vào ngay, không chờ xả đầu ra; nếu terminal mãi không đọc, chỉ cố gửi dọn màn hình hết sức.

Thống kê gồm khung nhận, khung bị thay, khung sai, khung chờ, lần gửi đầy đủ thực, byte đầu ra, giây chạy, thời gian xếp hàng cục bộ gần nhất, bitrate/RTT/SRTP mạng. Chênh bộ đếm/chênh thời gian cho FPS khoảng; nhật ký cuối tương tác còn có FPS gửi trung bình, `renderer` thực và ngân sách `output_bytes_per_second`. `kitty_transport=shared_memory/inline/none` chỉ đường có hiệu lực, `target_fps` mục tiêu cục bộ, `preview_fps` mục tiêu xem trước từ xa yêu cầu gần nhất (không phải đã hiệu lực hay FPS đo thực), `adaptive_preview` chỉ tự chỉnh có bật, `preview_adjustments` ghi số đổi mức; mỗi lần còn ghi sự kiện `preview_fps`. `frame_service_ms` là ước lượng làm mượt thời gian xử lý/xuất cả khung gần nhất, `frame_processing_ms` là trung bình bố trí／chuyển đổi／chuẩn bị truyền cục bộ, `shared_ack_ms` là trung bình từ ghi xong lệnh tải đến phân tích xác nhận thành công, `shared_fallbacks` ghi số lần quay về lúc chạy. `startup_ms` trong tóm tắt `client.log` ghi mili giây từ bắt đầu kết nối đến thành công (`connected`), khung video đầu lấy để hiển thị vào hàng (`first_frame`), lệnh hiển thị khung đầu ghi hết terminal (`first_output`); giai đoạn chưa tới là `null`. Ghi xong TTY không bằng hiển thị xong, thời gian xếp hàng cục bộ không phải trễ đầu cuối.

## Nhập liệu và thoát

Mặc định chỉ đọc, tắt tiếng. `--control` hoặc nhấn `c` khi chỉ đọc bật nhập tường minh. `Ctrl+]` vào chế độ lệnh cục bộ liên tục: nhấn rồi nhả, sau đó nhấn phím lệnh trong bảng dưới, có thể thực hiện liên tiếp nhiều lệnh. Thanh trạng thái hiện ngay chế độ và lệnh khả dụng.

Terminal tích hợp macOS còn cần chọn “View → Allow Mouse Reporting” trong cửa sổ hiện tại. CLI bật báo cáo chuột nhưng không thể ghi đè tùy chọn Terminal đã tắt; không chọn thì nhấn, kéo, lăn không tới CLI. Xem [hướng dẫn Apple](https://support.apple.com/zh-cn/guide/terminal/trmlc69728a5/mac).

| Thao tác | Chỉ đọc | Điều khiển |
|---|---|---|
| Bật／tắt điều khiển từ xa | `c` | `Ctrl+]`, rồi `c` |
| Thoát CLI | `q` hoặc `Ctrl+C` | `Ctrl+]`, rồi `q` |
| Vào lệnh cục bộ | `?` hoặc `Ctrl+]` | `Ctrl+]` |
| Phóng to／thu nhỏ | `+` / `=` / `-` | `Ctrl+]`, rồi phím tương ứng |
| Khôi phục toàn hình | `f`, đặt lại tỷ lệ, vị trí, chi tiết | `Ctrl+]`, rồi `f` |
| Xem bằng chuột | Lăn thu phóng tại chuột; nhấn vào chi tiết nguyên bản | Vào lệnh cục bộ chỉ cập nhật neo thu phóng, dùng phím chỉnh |
| Di chuyển khung nhìn | Mũi tên hoặc `i` lên, `j` trái, `k` xuống, `l` phải | `Ctrl+]`, rồi phím tương ứng |

Vào lệnh cục bộ thì tạm dừng nhập từ xa, nhả phím／chuột đã ghi. Thu phóng, di chuyển, `c` hay lệnh không rõ đều giữ chế độ; `c` đổi có bật điều khiển sau quay lại thủ công, thanh hiện `CONTROL PAUSED` hoặc `READ ONLY`. `Esc` hoặc `Ctrl+C` quay lại chế độ đã chọn; `q` thoát CLI. Nhấn lại `Ctrl+]` cũng quay lại và gửi một lần `Ctrl+]` từ xa nếu chọn điều khiển. Chuột trong lệnh chỉ cập nhật neo cục bộ, không nhấn/kéo/lăn; bỏ nội dung bracketed paste, không thoát hay kích hoạt phím tắt. Mất tiêu điểm hoặc tràn đệm vẫn giữ lệnh cục bộ nhưng đặt chế độ quay lại thành chỉ đọc. Trong điều khiển, `c/q/?` thường, `Ctrl+T`, `Ctrl+C`, `Esc` riêng đều gửi từ xa. Chưa hỗ trợ gán lại phím.

Chỉ đọc: nhấn ảnh vào chi tiết nguyên bản tại đó; `+` / `=` phóng, `-` thu (1～32 lần). Phím và con lăn neo chuột, cố giữ nội dung dưới chuột tại chỗ; giới hạn cắt khi tới mép màn hình. Chưa nhận vị trí chuột hoặc chuột ở viền đen/thanh trạng thái thì dùng tâm hiện tại; mất tiêu điểm xóa vị trí. Mũi tên hoặc `i/j/k/l` lên/trái/xuống/phải, `f` khôi phục toàn hình và đặt lại tỷ lệ/vị trí/chi tiết. Bỏ nhấn viền đen. Các phím và chuột này vẫn gửi từ xa trong điều khiển; vào lệnh cục bộ có thể di chuột chọn neo, nhấn thu phóng liên tục đến quay lại thủ công. Phóng, di chuyển và định vị chuột chỉ đọc không gửi thao tác từ xa, cũng dùng cho hình tĩnh.

Hỗ trợ báo cáo chuột SGR và `CSI M` truyền thống: di chuyển, trái/phải/giữa, kéo, lăn; đệm đầy đủ báo cáo truyền thống bị chia qua lần đọc, không thực thi byte tọa độ như phím. Khởi động lần lượt bật báo cáo thường, kéo, mọi di chuyển, tương thích terminal chỉ có một phần chế độ; thoát tắt. Tọa độ truyền thống tối đa 223, cửa sổ lớn nên dùng terminal SGR. Định dạng theo [giao thức chuột XTerm](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Mouse-Tracking).

Nhấn/lăn gửi sự kiện di chuyển đúng vị trí trước, không phụ thuộc terminal báo trước; không gửi nhấn mới tại viền đen, nhả tại đó vẫn nhả nút cũ. Báo cáo truyền thống không phân biệt nút nhả nên nhả mọi nút đã nhấn ứng dụng ghi. Tọa độ ánh xạ vào màn hình chọn; `--display` là chỉ số bắt đầu từ không trong HostInformation, luồng video Display1, Display2...

Sau hiện ảnh trong điều khiển, `Mouse RX` (`M:` ở cửa sổ hẹp) đếm báo cáo chuột hợp lệ nhận, giúp kiểm tra dù không thấy con trỏ từ xa. Nếu nhấn/di chuột mà vẫn 0, kiểm tra cài báo cáo terminal và PTY SSH (`ssh -t`); nếu tăng nhưng máy từ xa không phản hồi, thoát xem thống kê nhật ký dưới. Mất tiêu điểm về chỉ đọc, có lại cần nhấn `c` để điều khiển.

Hỗ trợ văn bản UTF-8 và bracketed paste (máy từ xa phải khai báo text_input), Enter, Tab, Backspace, Esc, mũi tên, Home/End, Delete, PageUp/Down, F1–F4, Ctrl/Alt+chữ Latin. Không hỗ trợ khả năng văn bản từ xa thì báo, không thực thi dán như shell cục bộ. Terminal thường dùng tap nhấn/nhả ngay; không hứa giữ phím, mọi tổ hợp, IME hay bố cục đầy đủ. Pi giải mã phím nâng cao qua SDK chủ rồi đổi thành nhập cơ bản này.

Mất tiêu điểm, tắt điều khiển, chuyển màn hình, thoát và ngắt đều cố nhả nhập đã ghi; mạng mất không đảm bảo máy từ xa nhận. Thoát thường khôi phục chế độ terminal, báo cáo chuột, con trỏ, tự xuống dòng, màn hình thay thế theo đường thực thi thường. POSIX xử lý SIGINT/SIGTERM/SIGHUP; Windows xử lý Ctrl+C / Ctrl+Break console, khôi phục chế độ/bảng mã. Hủy chờ tối đa hai giây xả hình hiện tại rồi gửi chuỗi hủy/kết thúc và khôi phục; đầu ra hỏng hẳn hay SIGKILL không đảm bảo khôi phục, có thể chạy `reset` trong shell.

Chỉ đọc là giới hạn hành vi của ứng dụng này, không phải cấp quyền chỉ đọc riêng do máy chủ cưỡng chế. Không có bộ nhận tệp/clipboard/lệnh từ xa.

## Tiện ích Pi

Triển khai/kiểm tra kiểu theo SDK **1.4.0** của `@earendil-works/pi-coding-agent` và `pi-tui` đã tải. Chưa kiểm chứng gói `@mariozechner` cũ; ứng dụng tích hợp thiếu ctx.mode sẽ hạ cấp.

```sh
export CROSSDESK_CLI="$PWD/build/macosx/arm64/release/crossdesk-cli"
pi -e ./apps/cli/plugins/pi/index.ts
```

Nhập `/crossdesk` hoặc `/crossdesk 123456789` trong Pi. Nhập ID trước rồi mật khẩu trong thành phần che tùy chỉnh. Tiện ích ghi mật khẩu qua fd 3 riêng rồi đóng; không ghi cấu hình, trò chuyện, kết quả công cụ, ngữ cảnh mô hình. Thành phần xem và CLI riêng dùng cùng tiền tố lệnh `Ctrl+]` và phím chế độ trên, bridge phân tích chung. Plugin mặc định yêu cầu high, hiện dòng góc phần tư ANSI, cùng phím thu phóng/toàn cảnh/di chuyển, hạ màu theo môi trường, Pi tự bố trí/làm mới; không chiếm TTY trực tiếp, không gửi đồ họa Kitty hay tự mở báo cáo chuột.

Tiện ích Pi đọc kích thước ký tự chủ và truy vấn ký tự/toàn vùng văn bản qua giao diện ghi terminal／nghe nhập của chủ, làm mới mỗi hai giây/khi đổi cửa sổ; không chuyển phản hồi kích thước từ xa. Tỷ lệ truyền qua `viewport.cell_aspect` bridge; đổi thì vẽ lại tĩnh, bỏ khung tỷ lệ cũ đang gửi. Nếu tự dò không có/không đúng, đặt `CROSSDESK_CELL_ASPECT=0.45` trước khởi động Pi (rộng／cao, cho 0.2～2; `auto` khôi phục tự dò). Đóng khung nhìn/bridge dừng thì bỏ hẹn giờ truy vấn và nghe nhập.

Cần phát hiện toàn cục thì sao chép cả `apps/cli/plugins/pi` vào `~/.pi/agent/extensions/crossdesk/`, khởi động lại Pi hoặc `/reload`. Gỡ thì đóng khung nhìn trước, xóa thư mục, `/reload`; dùng `-e` thì chỉ bỏ tham số khởi động. Không cần sửa mật khẩu/cấu hình thiết bị trong gói. Mặc định tín hiệu chính thức; tự triển khai hiện dùng lệnh kết nối CLI riêng, plugin chưa có UI máy chủ.

Chỉ mở thành phần khi `ctx.mode === "tui"`, hasUI và tiến trình TTY hai chiều. RPC/print/json trả gợi ý trình phát riêng, không nhận nhúng video. Đóng UI/thoát phiên đóng stdin bridge trước, quá hạn SIGTERM rồi SIGKILL. Bridge chưa sẵn sàng/đã dừng thì đóng thành phần bằng `q`, `Esc`, `Ctrl+C`. Đã kiểm chứng kiểu SDK và vòng đời UI mô phỏng, chưa nghiệm thu làm mới TUI Pi thật, phím nâng cao, chuột. Xem kiểm chứng kết nối bridge và CLI riêng với thiết bị thật trong [biên bản](validation.md).

Cơ sở chính thức: [tài liệu tiện ích](https://github.com/badlogic/pi-mono/blob/main/packages/coding-agent/docs/extensions.md), [tài liệu TUI](https://github.com/badlogic/pi-mono/blob/main/packages/coding-agent/docs/tui.md), [giao thức đồ họa Kitty](https://sw.kovidgoyal.net/kitty/graphics-protocol/).

## Khắc phục sự cố và giới hạn

- `incorrect_password`: khởi động lại, nhập đúng mật khẩu, không tự thử lặp.
- `device_not_found` / `device_offline_or_unavailable`: kiểm tra ID, máy chủ, trực tuyến máy được điều khiển.
- `signaling_login_failed` / `signaling_disconnected`: kiểm tra máy chủ, mạng, proxy, tường lửa; xác nhận máy chủ hỗ trợ đăng ký `cli`.
- `cli_identity_not_supported`: máy chủ chưa trả danh tính `cli-` cộng 9 chữ số hợp lệ, cần nâng cấp/kiểm tra máy chủ.
- `tls_certificate_error`: kiểm tra chuỗi chứng chỉ, tên máy, đồng hồ, CA hệ thống, không tắt xác minh.
- `connection_timeout` / `connection_failed`: kiểm tra NAT/TURN và máy được điều khiển. Chỉ lấy xác thực TURN do máy chủ cấp.
- `display_not_available`: đổi sang chỉ số máy từ xa khai báo; một phiên chỉ hiện một màn hình một lúc.
- `media_initialization_failed`: kiểm tra phụ thuộc biên dịch và môi trường đa phương tiện cục bộ.
- Phím ở chế độ chỉ đọc không gửi từ xa; không khai báo giao thức văn bản thì không nhập văn bản được.
- Nhật ký ở thư mục tạm riêng tiến trình tạo (POSIX `/tmp/crossdesk-cli-*`, Windows `%TEMP%\crossdesk-cli-*`). Tóm tắt tương tác trong client.log: số báo cáo chuột terminal `mouse_reports`, số truyền thống `mouse_legacy_reports`, sự kiện chuột giao truyền tải thành công `mouse_forwarded`, gửi thất bại `mouse_send_failures`. Một báo cáo nhấn có thể sinh định vị và nút nên số gửi có thể lớn hơn số báo cáo; gửi xong không nghĩa máy từ xa đã thực thi. transport.log chỉ ghi trạng thái ICE/DTLS trong danh sách cho phép và công tắc điều khiển cục bộ, không mật khẩu, SDP, tín hiệu thô. Có thể xóa sau dùng. Lỗi thoát hiện sau khôi phục terminal, kèm thư mục nhật ký lần này.
- `connection_failed` là thiết lập kết nối thất bại; `connection_lost` là truyền tải hỏng sau kết nối thành công. `execv ... failed(1)` của xmake sau đó nghĩa chương trình thoát mã 1, tự nó không nghĩa sập tiến trình. Khi điều khiển lỗi, so công tắc và trạng thái trong transport.log; `mouse_reports` 0 là terminal chưa cấp sự kiện chuột nhận diện được. Báo cáo tăng mà `mouse_forwarded` 0 thì kiểm tra còn chỉ đọc／lệnh cục bộ không, đã có ảnh chưa, nhấn trong vùng video không; `mouse_send_failures` tăng thì kiểm tra kết nối.
- Nếu cả cửa sổ macOS Terminal thoát và CLI ghi `output closed`, nên xem báo cáo Terminal trong DiagnosticReports hệ thống. Đã thấy Terminal 2.15 vượt bộ đệm dòng `TTLogicalScreen expandUnicharOffsetCacheToLine:` khi tạo thẻ/đổi cỡ; khác `connection_failed` CLI. Tạm chạy cửa sổ riêng, tránh thao tác thẻ đó; không vì vậy coi lỗi Terminal đã sửa.
- `doctor --json` là báo cáo môi trường, không đăng nhập mạng, chủ động dò Kitty / Sixel hay chứng minh máy từ xa kết nối được. `input_shortcuts=local_commands_v2` chỉ hỗ trợ lệnh cục bộ liên tục ở đây; nhật ký khởi động còn ghi đường thực thi thật để phân biệt sản phẩm các dự án.
- Tỷ lệ ảnh lỗi thì xem `terminal_geometry` trong tóm tắt `client.log`: `cell_aspect` là tỷ lệ ký tự thực dùng, `reported` chỉ có phản hồi kích thước hợp lệ, `manual_aspect` chỉ ghi đè thủ công. Tỷ lệ/pixel trong `doctor` chỉ phản ánh TTY hệ thống, không truy vấn terminal, có thể khác giá trị tự dò khi tương tác.

Chế độ terminal không có phát âm thanh hay API đăng ký âm thanh; tiếng máy chủ SSH không truyền về cục bộ. Không daemon nền, Broker đa phương tiện chung hay IP trực tiếp. Ứng dụng tích hợp terminal dùng [cầu nối riêng](bridge-protocol.md), không có UI tùy chỉnh đáng tin thì mở terminal riêng. PNG chụp màn hình và thao tác có cấu trúc cho Agent do [chế độ tự động hóa](automation-protocol.md) và [dịch vụ MCP](../../mcp/README.md) riêng cung cấp.

Mỗi tiến trình trình phát/cầu nối chỉ sở hữu một phiên đa phương tiện của mình, không gửi video vào thông điệp mô hình.
