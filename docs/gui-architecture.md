# Cấu trúc thư mục và phân chia trách nhiệm GUI

Tài liệu này giải thích cấu trúc thư mục, kiểu cốt lõi, hướng phụ thuộc và nơi đặt mã trong `apps/desktop/src/gui`. Xem thao tác giao diện ở [hướng dẫn sử dụng máy tính](USER_GUIDE.md), giao thức kết nối ở [kiến trúc kết nối](connection-architecture.md).

Xem tô-pô mạng, xác thực và quy trình truyền tải của kết nối ID và IP trực tiếp trong [kiến trúc chế độ kết nối](connection-architecture.md).

## Cấu trúc thư mục

```text
apps/desktop/src/gui/
├── render.h                         # Điểm vào ổn định công khai Render
├── render.cpp                       # Tạo GuiApplication và chuyển tiếp Run
├── application/                     # Lớp vỏ ứng dụng Slint
│   ├── gui_application.h            # Khai báo GuiApplication
│   ├── gui_application.cpp          # Khởi tạo, callback UI, đồng bộ trạng thái và dọn dẹp
│   ├── application_state.h          # Trạng thái cửa sổ, tương tác và UI
│   ├── server_window_state.h        # Logic chọn máy điều khiển của cửa sổ máy được điều khiển
│   └── window_geometry.h           # Tính vị trí cửa sổ
├── rendering/                       # Kết nối Slint với trình kết xuất video gốc
│   └── slint_video_presenter.h/.cpp # Trình bày kết cấu video và thông báo kết xuất
├── runtime/                         # Runtime GUI không có giao diện
│   ├── gui_runtime.h/.cpp           # Giao diện phối hợp runtime và triển khai chung
│   ├── connection_runtime.cpp       # Kết nối, dò trực tuyến, hết thời gian và dọn phiên
│   ├── gui_state.h                  # Điểm kết hợp ApplicationState và RuntimeState
│   ├── runtime_state.h              # Trạng thái cấu hình, kết nối, nền tảng và liên lạc
│   ├── remote_session.h             # Một phiên từ xa RemoteSession
│   ├── device_presence_cache.h      # Bộ nhớ đệm trạng thái thiết bị trực tuyến
│   ├── peer_event_handler.h/.cpp    # Callback báo hiệu và trạng thái kết nối
│   ├── peer_media_callbacks.cpp     # Callback video và âm thanh
│   └── peer_data_callbacks.cpp      # Callback dữ liệu điều khiển, bảng nhớ tạm và tệp
├── features/                        # Mô-đun tính năng có thể phát triển độc lập
│   ├── clipboard/                   # Đồng bộ bảng nhớ tạm cục bộ và từ xa
│   ├── devices/                     # Vòng đời thiết bị đa phương tiện và đầu vào
│   ├── file_transfer/               # Hàng đợi tệp, gửi, xác nhận và tiến độ
│   ├── input/                       # Trạng thái bàn phím, lệnh và xử lý hết thời gian
│   └── settings/                    # Bộ nhớ đệm cấu hình và bí danh kết nối gần đây
├── ui/                              # Giao diện khai báo Slint
│   ├── crossdesk_ui.slint           # Điểm xuất thành phần giao diện
│   ├── common.slint                 # Điều khiển chung, cỡ chữ, màu và hình dạng biểu tượng
│   ├── main_window.slint            # Cửa sổ chính, bảng, cài đặt và hộp thoại
│   ├── stream_window.slint          # Cửa sổ luồng, thẻ và thanh điều khiển
│   ├── server_window.slint          # Cửa sổ máy được điều khiển
│   ├── ui_localization.h           # Ánh xạ dữ liệu bản địa hóa sang thuộc tính Slint
│   └── icons/                      # Tài nguyên biểu tượng SVG
└── assets/                          # Phông nhúng và dữ liệu bản địa hóa
```

Triển khai GUI riêng nền tảng tập trung tại `apps/desktop/src/platform/<os>/gui/`, gồm khay hệ thống, kết xuất Metal và quyền macOS, runtime dịch vụ Windows; trình kết xuất OpenGL dùng chung Windows và Linux ở `apps/desktop/src/platform/common/gui/`.

## Quan hệ phân lớp

```text
Render
  └── GuiApplication
        ├── Cửa sổ Slint, vòng lặp sự kiện và bộ định thời
        ├── Thành phần khai báo và callback ui/
        ├── SlintVideoPresenter → VideoRenderer nền tảng
        └── GuiRuntime
              ├── SessionDeviceManager
              ├── KeyboardController
              ├── ClipboardController
              ├── FileTransferManager
              ├── SettingsManager
              └── PeerEventHandler
```

Hướng phụ thuộc nên giữ từ trên xuống:

```text
Điểm vào công khai → Lớp ứng dụng → Lớp phối hợp runtime → Mô-đun tính năng → Thư viện tầng dưới
                               ↘ Lớp hiển thị
```

Mô-đun tính năng tầng dưới không nên phụ thuộc ngược bảng, thanh công cụ hay cửa sổ cụ thể.

Máy tính và iOS Bridge cùng phụ thuộc thư viện tĩnh `crossdesk_wire`. Header hợp đồng truyền trên mạng ở `libs/wire/include/`, gồm thông điệp điều khiển, kiểu con trỏ chuột, ID luồng hiển thị, tên luồng dữ liệu và định dạng truyền tệp trên mạng. Chèn đầu vào hệ điều hành, bảng nhớ tạm, đọc ghi tệp và trạng thái giao diện vẫn do từng ứng dụng triển khai; `libs/wire/` không được phụ thuộc header riêng máy tính hay hệ điều hành.

## Kiểu cốt lõi

### Render

`Render` là điểm vào ổn định mà phần khác của ứng dụng thấy được, chỉ chịu trách nhiệm:

- Quản lý vòng đời `GuiApplication`;
- Chuyển tiếp `Run()` cho `GuiApplication`;
- Cách ly kiểu nội bộ GUI, tránh chi tiết triển khai lan sang mô-đun khác.

Không thêm trạng thái cửa sổ, kết nối hoặc phương thức nghiệp vụ vào `Render`.

### GuiApplication

`GuiApplication` là lớp vỏ ứng dụng Slint, chịu trách nhiệm:

- Khởi tạo nhật ký, cấu hình, SDL và mô-đun tính năng;
- Tạo, hủy cửa sổ chính, cửa sổ luồng và cửa sổ máy được điều khiển Slint;
- Chạy vòng lặp sự kiện Slint, thực hiện `Tick()` và điều phối khung hình video qua bộ định thời;
- Chuyển callback Slint thành thao tác kết nối, bàn phím và chuột, xử lý thoát SDL và sự kiện bảng nhớ tạm;
- Đồng bộ trạng thái runtime sang thuộc tính và mô hình Slint, quản lý trình bày video gốc;
- Dọn tài nguyên theo thứ tự khi chương trình thoát.

Lớp này không nên triển khai chi tiết giao thức kết nối, truyền tệp, bảng nhớ tạm hoặc điều khiển thiết bị.

SDL vẫn dùng cho khả năng nền tảng như âm thanh, thông tin màn hình, bảng nhớ tạm. Lớp hiển thị ImGui cũ, xử lý sự kiện cửa sổ và tài nguyên riêng đã bỏ; `ImGuiLineStyle`, `ImGuiFontStyle` và `ImGuiCheckBox` trong `ui/common.slint` là thành phần Slint mà giao diện hiện dùng, tên chỉ phong cách thị giác kế thừa, không phụ thuộc thư viện ImGui. `assets/fonts/fa_solid_900.h` vẫn cung cấp phông biểu tượng Font Awesome cho cửa sổ Slint.

### GuiRuntime

`GuiRuntime` là lớp phối hợp không giao diện của tiến trình GUI, chịu trách nhiệm:

- Khởi tạo MiniRTC Peer và tham số callback;
- Phối hợp kết nối, phiên từ xa và trạng thái trực tuyến;
- Giữ Manager/Controller của từng tính năng;
- Xử lý thứ tự gọi xuyên mô-đun tính năng;
- Cung cấp ngữ cảnh runtime cho tích hợp nền tảng.

`GuiRuntime` nên giữ vai trò điều phối. Tính năng độc lập mới nên tạo Manager hoặc Controller trước, không nên tiếp tục dồn trực tiếp vào `GuiRuntime`.

### RemoteSession

`RemoteSession` biểu diễn một đầu cuối từ xa đang kết nối hoặc đã kết nối, vòng đời bao gồm:

- MiniRTC Peer và trạng thái kết nối;
- Khung hình video, kết cấu và vùng kết xuất;
- Trạng thái màn hình từ xa và thanh điều khiển;
- Trạng thái điều khiển âm thanh, chuột và bàn phím;
- Trạng thái truyền tệp;
- Trạng thái dịch vụ Windows và màn hình bảo mật.

`remote_sessions_` là bảng phiên theo chỉ mục ID từ xa. Runtime thống nhất xử lý tìm kiếm, kết nối và dọn phiên.

## Phân chia trạng thái

Định nghĩa trạng thái chia theo vòng đời và phạm vi sử dụng:

| Tệp | Phạm vi trạng thái |
| --- | --- |
| `application/application_state.h` | Thoát ứng dụng, tương tác, cập nhật, yêu cầu vòng đời cửa sổ và trạng thái UI chung |
| `GuiApplication::SlintUi` | Handle cửa sổ Slint, bộ định thời, bộ trình bày video, khay hệ thống và bộ nhớ đệm đồng bộ giao diện |
| `runtime/runtime_state.h` | Trạng thái cấu hình, Peer, bảng kết nối, dò trực tuyến và tích hợp nền tảng |
| `runtime/remote_session.h` | Trạng thái kết nối, đa phương tiện, điều khiển và tệp riêng của một phiên từ xa |
| `runtime/gui_state.h` | Chỉ kết hợp `ApplicationState` và `RuntimeState` |

Trước khi thêm trạng thái, nên xác định vòng đời:

- Chỉ thuộc một kết nối từ xa: đặt vào `RemoteSession`;
- Chỉ thuộc cửa sổ hoặc UI: đặt vào trạng thái con tương ứng của `ApplicationState`;
- Thuộc toàn bộ thời gian chạy GUI: đặt vào trạng thái con tương ứng của `RuntimeState`;
- Chỉ thuộc một mô-đun tính năng: ưu tiên làm thành viên riêng của Manager/Controller.

## Quy trình chạy chính

### Khởi động

```text
Render::Run
  → GuiApplication::Run
  → Khởi tạo đường dẫn, nhật ký và cấu hình
  → Khởi tạo SDL và mô-đun tính năng
  → GuiRuntime::CreateConnectionPeer
  → GuiApplication::InitializeUi tạo cửa sổ chính và gắn callback
  → Khởi động bộ định thời Tick và điều phối khung hình video
  → slint::run_event_loop
       → Tick phối hợp kết nối, xử lý sự kiện nền và đồng bộ trạng thái cửa sổ
```

### Thiết lập kết nối từ xa qua ID

```text
MainWindow::connect_requested
  → GuiApplication::ConnectFromUi
  → GuiRuntime::ConnectTo
  → Dò trạng thái trực tuyến
  → Tạo hoặc tái sử dụng RemoteSession
  → MiniRtcJoinConnection
  → PeerEventHandler nhận trạng thái kết nối
  → SyncStreamWindow tạo cửa sổ luồng và bắt đầu trình bày video
```

### Thiết lập kết nối IP trực tiếp

```text
Xác minh địa chỉ MainWindow và hộp thoại mật khẩu
  → GuiRuntime::ConnectLan
  → lan::SessionManager::Connect
  → WSS kết nối IP:cổng máy được điều khiển, máy đó xác minh mật khẩu
  → MiniRTC InitDirectPeer / StartDirectSession
  → WSS trao đổi SDP và ứng viên, HostOnly ICE thiết lập kênh UDP
  → DTLS/SRTP sẵn sàng, SessionManager đánh dấu đã cấp quyền
  → PeerEventHandler và xử lý cửa sổ luồng, đầu vào, tệp và bảng nhớ tạm hiện có
```

Khởi tạo và vòng đời LAN tập trung trong `runtime/lan_runtime.cpp`, lắng nghe, mật khẩu và giao thức thương lượng ở `src/network/lan`. Chuyển chế độ cửa sổ chính độc lập với bật/tắt tính năng IP; thao tác sau chịu trách nhiệm ngừng lắng nghe và rút hết phiên LAN.

### Nhận dữ liệu từ xa

```text
MiniRTC callback
  → PeerEventHandler
  ├── peer_media_callbacks: video, âm thanh
  ├── peer_data_callbacks: điều khiển, tệp, bảng nhớ tạm
  └── peer_event_handler: báo hiệu, trạng thái kết nối, thống kê mạng
       → Manager/Controller tương ứng
       → Cập nhật RemoteSession hoặc trạng thái runtime
```

### Thoát và dọn dẹp

```text
Slint quit / SDL quit / tray exit
  → GuiApplication::Cleanup
  → Dừng thiết bị và tác vụ nền
  → CloseAllRemoteSessions và chờ dọn phiên
  → Tách bộ trình bày video và giải phóng cửa sổ Slint, khay hệ thống
  → SDL_Quit
```

## Quy tắc nơi đặt mã mới

Khi thêm mã, chọn thư mục theo các quy tắc sau:

| Tính năng | Thư mục |
| --- | --- |
| Khởi tạo Slint/SDL, callback UI, đồng bộ trạng thái và vòng đời cửa sổ | `application/` |
| Kết nối Slint với trình kết xuất video gốc | `rendering/` |
| Kết nối, phiên, callback Peer và runtime nền tảng | `runtime/` |
| Vòng đời thiết bị video, âm thanh, chuột, bàn phím | `features/devices/` |
| Trạng thái bàn phím và xử lý đầu vào máy tính | `features/input/` |
| Thông điệp điều khiển trên mạng và định dạng truyền | `libs/wire/` |
| Đồng bộ bảng nhớ tạm | `features/clipboard/` |
| Truyền tệp | `features/file_transfer/` |
| Lưu bền cấu hình | `features/settings/` |
| Vùng nhúng cửa sổ chính, cài đặt và hộp thoại | `ui/main_window.slint` |
| Thẻ luồng, thanh điều khiển và bảng thống kê | `ui/stream_window.slint` |
| Cửa sổ máy được điều khiển | `ui/server_window.slint` |
| Điều khiển chung, cỡ chữ, màu và hình dạng biểu tượng | `ui/common.slint` |
| Khay hệ thống | `apps/desktop/src/platform/<os>/gui/tray/` |
| Phông nhúng và dữ liệu bản địa hóa | `assets/` |
| Tài nguyên biểu tượng SVG | `ui/icons/` |
