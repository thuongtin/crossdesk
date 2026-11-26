# Câu hỏi thường gặp / FAQ

[Về README](../README.md) · [English](#english)

## Chưa kết nối máy chủ hoặc báo lỗi chứng chỉ TLS

Trước hết xác nhận mạng hoạt động và hai bên dùng đúng cấu hình máy chủ dự kiến. Với môi trường self-host, kiểm tra cổng signaling, thời hạn chứng chỉ, tên máy có khớp hay không và tin cậy hệ thống; xem [hướng dẫn self-host](SELF_HOSTING.md). Sau khi giải quyết bước này, mới kiểm tra thiết bị từ xa và kết nối P2P.

## Đã kết nối máy chủ nhưng thiết bị từ xa ngoại tuyến

Xác nhận CrossDesk trên máy được điều khiển vẫn chạy, hai bên kết nối cùng dịch vụ signaling, rồi sao chép lại ID máy này hiện tại của máy được điều khiển. Sau khi chuyển máy chủ, ID và lịch sử kết nối cũ có thể không còn phù hợp.

## Kết nối P2P thất bại

Chiến lược kết nối mặc định đã cho phép chuyển sang TURN relay khi cần. Bạn có thể bật **Buộc kết nối qua relay** trong **☰ → Cài đặt**, lưu rồi kết nối lại để kiểm tra sự cố kết nối trực tiếp. Người dùng self-host còn cần kiểm tra Coturn chạy bình thường và đã mở cổng relay TCP/UDP cùng phạm vi cổng đa phương tiện UDP.

Thay đổi tương thích của dịch vụ công cộng cũng ảnh hưởng ứng dụng cũ; hãy đối chiếu [ghi chú Release](https://github.com/kunkundi/crossdesk/releases), cố gắng dùng các phiên bản gần đây tương thích trên cả hai bên.

## macOS đã kết nối nhưng màn hình đen hoặc không thao tác được bàn phím và chuột

Kiểm tra quyền ghi màn hình và trợ năng của CrossDesk trong “Cài đặt hệ thống → Quyền riêng tư & Bảo mật”. Sau khi thay vị trí ứng dụng hoặc chữ ký, hay cập nhật bản phát triển, có thể cần cấp quyền lại và khởi động lại ứng dụng.

Các phiên bản được cùng nhóm ký chính thức và tương thích về mã định danh ứng dụng cùng yêu cầu chữ ký thường có thể giữ quyền khi nâng cấp bình thường. Sau khi nâng cấp từ bản cũ chưa ký hoặc ký ad-hoc, thoát bản cũ và chạy bản mới; bản mới xử lý chuyển đổi quyền trợ năng và ghi màn hình cho người dùng hiện tại trong nền, rồi hướng dẫn cấp quyền lại. Trạng thái hoàn tất được lưu riêng cho từng quyền.

Khi chuyển đổi thất bại hoặc bị gián đoạn, mở “Cài đặt → Quyền hệ thống”, nhấn “Thử chuyển đổi quyền lại”. Bạn cũng có thể chọn “Để sau” để tiếp tục dùng ứng dụng; các tính năng máy được điều khiển bị ảnh hưởng được bật sau khi cấp quyền. Nếu cài đặt hệ thống vẫn hiển thị đã cấp quyền nhưng tính năng bất thường, thoát hẳn CrossDesk, xóa mục cũ trong danh sách quyền tương ứng rồi thêm lại `/Applications/CrossDesk.app`, sau đó chạy ứng dụng.

Nếu sự cố vẫn còn, thoát CrossDesk lần nữa, chỉ thực hiện một lệnh tương ứng với lỗi trong terminal của người dùng hiện đăng nhập để đặt lại quyền đó của CrossDesk.

```bash
# Lỗi quyền ghi màn hình
tccutil reset ScreenCapture cn.crossdesk.app
# Lỗi quyền trợ năng
tccutil reset Accessibility cn.crossdesk.app
```

Sau khi thực hiện, chạy lại CrossDesk và cấp quyền trong cài đặt hệ thống theo lời nhắc. Nếu còn mục đường dẫn hoặc phiên bản cũ, bạn có thể xóa mục cũ trong danh sách quyền tương ứng rồi thêm lại ứng dụng hiện tại.

## Linux Wayland không ghi hình được màn hình

Ghi hình Wayland phụ thuộc vào bản biên dịch đã bật hỗ trợ Wayland, PipeWire 0.3 trên hệ thống chủ và quyền cổng thông tin màn hình; nếu không ghi hình được, bạn cũng có thể đối chiếu trong phiên X11. Xem tùy chọn biên dịch trong [hướng dẫn biên dịch Linux](BUILD.md#linux).

## Máy tính Linux không kết nối màn hình hoặc không có phiên đồ họa

Khởi động CrossDesk bình thường để tự phát hiện và chọn màn hình X11 hiện có của người dùng hiện tại, giữ Dock và ứng dụng cũ, không cần biết số màn hình; nếu có nhiều phiên, tự chọn theo trạng thái đăng nhập và hoạt động. Chỉ thấy CrossDesk trên nền đen thường do đã tạo Xvfb trần không chạy môi trường desktop; hãy chuyển sang tái sử dụng màn hình gốc. Nếu cần màn hình ảo độc lập, cài `xvfb xfce4 dbus`, dùng `--headless-size 1920x1080 --headless-session startxfce4`. Xem triển khai và giới hạn trong [chạy Linux headless](HEADLESS.md).

## Không thao tác được sau khi Windows khóa màn hình

Trên máy tính được điều khiển, kiểm tra CrossDesk Service đã cài và đang chạy hay chưa, đồng thời giữ ứng dụng CrossDesk chạy. Bản portable có thể cài qua “Cài đặt → Dịch vụ điều khiển khi khóa màn hình”. Xem lệnh kiểm tra trạng thái dịch vụ trong [tài liệu dịch vụ Windows](../README.md#windows-service).

Dùng được “Tự động khóa màn hình sau khi ngắt kết nối” không có nghĩa là dịch vụ điều khiển khi khóa màn hình khả dụng: ứng dụng gọi chức năng hệ thống để khóa màn hình, còn nhập mật khẩu đăng nhập cần dịch vụ. Nếu điện thoại Android thấy màn hình đăng nhập nhưng bộ gõ không nhập được, bạn có thể chuyển sang **Bàn phím máy tính** trong bàn phím từ xa, đối chiếu xem chữ số, chữ cái và Enter có hoạt động không; khi phản hồi, nêu phiên bản hai bên, bản cài đặt hay bản portable và biểu hiện của hai kiểu bàn phím.

Nếu bàn phím máy tính dùng được nhưng bộ gõ hệ thống điện thoại không dùng được, hãy dùng gói cài đặt Windows đầy đủ cập nhật máy tính được điều khiển để ứng dụng, dịch vụ và trình trợ giúp phiên cùng một phiên bản. Chỉ cập nhật điện thoại hoặc thay `CrossDesk.exe` có thể vẫn dùng dịch vụ cũ.

Nếu nhật ký có `Secure desktop text injection failed: error=unknown_command`, thường là phiên bản dịch vụ và chương trình chính không khớp. Với bản cài đặt, dùng gói đầy đủ để sửa chữa; nếu tự triển khai bản biên dịch, cập nhật dịch vụ theo [các bước cập nhật dịch vụ Windows](USER_GUIDE.md#windows-service-development-update) rồi kết nối lại.

## Kết nối từ thiết bị di động nhưng chế độ riêng tư không bật

Ứng dụng di động native Android, iOS và HarmonyOS có thể bật/tắt “Chế độ riêng tư” ngay trong menu nổi; nút hiển thị trạng thái được máy tính xác nhận. Khi bật, hình ảnh máy tính bị che và bàn phím, chuột cục bộ bị chặn. Để tự bật, máy tính còn cần chọn “Bật chế độ riêng tư khi có kết nối đến” và ở màn hình hỗ trợ lớp che; màn hình khóa, giao diện đăng nhập và chế độ không giám sát chưa gắn giao diện người dùng không cung cấp lớp che.

Ứng dụng cũ có thể không cung cấp điều khiển chế độ riêng tư; hãy xác nhận cả hai bên đã cập nhật và ứng dụng di động có mục này. Nếu báo thất bại, xem nguyên nhân máy tính trả về; ứng dụng điều khiển cũ hoặc Web cùng trực tuyến có thể ngăn bật. “Tự động khóa màn hình sau khi ngắt kết nối” là tính năng riêng. Trên máy Windows được điều khiển, bạn có thể dùng Ctrl+Alt+Shift+F12 để khôi phục khẩn cấp hiển thị và nhập liệu cục bộ.

## Máy Windows được điều khiển không gắn màn hình

CrossDesk tự chọn phương thức ghi hình khả dụng và thử dùng màn hình ảo đi kèm gói cài đặt Windows hoặc gói di động không cần cài đặt. Hãy xác nhận CrossDesk Service đã cài và chạy được; nếu chưa cài dịch vụ, quyền thông thường có thể không chuẩn bị được màn hình ảo.

Chế độ không giám sát ưu tiên khôi phục độ phân giải đã ghi nhận, nếu chưa có thì dùng 1920×1080. Khi trình điều khiển không khả dụng, vẫn có thể vào hình ảnh tương thích do hệ thống cung cấp, độ phân giải và tốc độ khung hình sẽ giảm. Kết nối lại sau khi gắn màn hình thật cũng giúp kiểm tra hình ảnh bình thường đã khôi phục hay chưa.

Trình điều khiển đi kèm gói cài đặt ở `usbmmidd_v2`, tệp giấy phép được cung cấp cùng gói. Gỡ bản cài đặt sẽ xóa trình điều khiển do CrossDesk cài, giữ nguyên trình điều khiển đã cài trước cho phần mềm khác. Nếu bản portable cần xóa thủ công, bạn có thể gỡ “USB Mobile Monitor Virtual Display” trong Trình quản lý thiết bị.

## Kết nối thất bại sau khi đổi mật khẩu

Mật khẩu gồm 6 chữ số hoặc chữ cái tiếng Anh, phân biệt hoa thường. Khi đổi ở chế độ ID, cần kết nối máy chủ, chờ thành công và kết nối lại rồi dùng mật khẩu mới; IP trực tiếp dùng mật khẩu riêng, có hiệu lực ngay sau khi đổi thành công. Máy điều khiển đã lưu mật khẩu cũ cần nhập lại.

## Mật khẩu đúng nhưng vẫn báo không phải thiết bị đáng tin cậy

Máy tính được điều khiển có thể đã bật “Chỉ cho phép thiết bị đáng tin cậy điều khiển”. Sao chép thông tin ghép đôi trên máy điều khiển trước, rồi thêm vào danh sách tin cậy của máy được điều khiển và đối chiếu fingerprint; trên điện thoại, mở **Cài đặt → Thông tin ghép đôi máy này**. Ứng dụng cũ và CLI terminal hiện tại không hỗ trợ xác thực này. Sau khi đổi thiết bị hoặc xóa dữ liệu ứng dụng, có thể cần ghép đôi lại; xem [tài liệu thiết bị đáng tin cậy](CONNECTIONS.md#trusted-devices).

## Tệp nhận được nằm ở đâu

Trên máy tính, xem **☰ → Cài đặt → Đường dẫn lưu tệp**. Tệp nhận trên iOS nằm trong `Documents/Received` của ứng dụng, có thể xuất qua mục chia sẻ. Android và HarmonyOS giữ tệp qua mục lưu / xuất trong phiên; Android xóa tệp nhận tạm sau khi kết thúc phiên, hãy xuất trước khi ngắt kết nối.

## Vì sao vẫn kết nối được sau khi đóng cửa sổ chính

Đóng cửa sổ chính sẽ ẩn cửa sổ, ứng dụng vẫn chạy nền. Khi cần kết thúc chương trình, chọn thoát từ khay hệ thống hoặc thanh menu macOS.

## Biên dịch Windows khi chưa cài CUDA như thế nào

Bạn có thể biên dịch bằng `--USE_CUDA=false` trước. Khi cần hỗ trợ mã hóa và giải mã CUDA, cài phụ thuộc tương ứng theo [hướng dẫn biên dịch](BUILD.md); trong PowerShell, nên dùng `$env:CUDA_PATH = "<thư mục cài đặt thực tế>"` để đặt biến môi trường.

## Vì sao sản phẩm biên dịch iOS không thể cài trực tiếp

Bạn có thể cài trực tiếp từ [App Store](https://apps.apple.com/app/id6828859211). Nếu sản phẩm biên dịch GitHub được đánh dấu chưa ký, cần chữ ký của bạn mới cài được lên thiết bị; bạn cũng có thể ký và chạy bằng Xcode. Xem [yêu cầu biên dịch iOS](../apps/ios/README.md).

---

<a id="english"></a>

## English

[Back to README](../README_EN.md)

| Symptom | What to check |
| --- | --- |
| Server disconnected / TLS error | Network access, signaling endpoint, certificate dates, hostname, and system trust. See [self-hosting](SELF_HOSTING_EN.md). |
| Remote device offline | Keep the host running, connect both sides to the same signaling server, and copy its current ID again. |
| P2P connection failed | The default connection policy already allows TURN relay fallback. Enable Force Relay in Settings, save, and reconnect to troubleshoot direct connections. For self-hosting, check Coturn and relay/media firewall ports. Review [release compatibility notes](https://github.com/kunkundi/crossdesk/releases). |
| Black screen / no input on macOS | Grant Screen Recording and Accessibility, then reopen the app. Release updates with compatible signing identities normally retain permissions. For migration or stale grants, see [macOS permission repair](#repair-macos-permissions). |
| No capture on Linux Wayland | Check the Wayland build option, host PipeWire 0.3, and desktop portal permission. Compare with an X11 session. See [Linux build instructions](BUILD_EN.md#linux). |
| Linux host has no monitor or graphical session | Start CrossDesk normally to automatically discover and select your existing desktop, preserving its Dock and apps; no display number is needed. A bare Xvfb shows only CrossDesk on a black background; use an existing session or install Xfce and specify `--headless-session startxfce4`. See [Linux headless operation](HEADLESS_EN.md). |
| Cannot control a Windows lock screen | Install/start CrossDesk Service and keep the host client running. See [service commands](../README_EN.md#windows-service). |
| Android can connect but cannot type a Windows login password | Locking on disconnect does not verify service availability. Check CrossDesk Service, then compare the phone IME with **Computer keyboard** in the remote keyboard panel. Include both client versions, installer/portable edition, and both keyboard results in the report. |
| Privacy screen does not activate for a mobile connection | Use **Privacy screen** in the native mobile floating menu. Update both clients if the entry is missing. Automatic activation also requires the host setting and a supported user desktop; older or Web controllers connected at the same time may prevent it. Locking on disconnect is separate. |
| Windows host has no monitor attached | CrossDesk automatically selects a capture method and tries the bundled virtual display. Check that CrossDesk Service is installed and running. Unattended mode prefers a saved resolution, otherwise 1920×1080. If the driver is unavailable, resolution and frame rate may be limited. Reconnect after attaching a physical monitor to check recovery. |
| Removing the bundled virtual display driver | The installer removes a driver installed by CrossDesk. For a portable build, uninstall “USB Mobile Monitor Virtual Display” in Device Manager when it is no longer needed. |
| Saved password no longer works | Use six case-sensitive ASCII letters or digits. ID password changes require a server connection; wait for confirmation. Direct IP has a separate password that applies immediately after a successful change. Enter the new password on the controller. |
| Correct password but device is not trusted | Copy the controller’s pairing information, add it to the host’s trusted list, and compare fingerprints. Mobile: Settings → This device’s pairing info. Older clients and the current terminal CLI do not support this restriction. |
| Cannot find received files | Desktop: Settings → File Save Path. iOS: share from `Documents/Received`. Android / HarmonyOS: use the save/export action. Export Android files before disconnecting, as its temporary received files are cleared when the session ends. |
| Main window closed but app still running | Closing hides the window. Use the tray/menu-bar exit command to quit. |
| Windows build without CUDA | Build with `--USE_CUDA=false`, or follow the [CUDA build instructions](BUILD_EN.md). Use `$env:CUDA_PATH` in PowerShell. |
| iOS artifact cannot be installed directly | Install from the [App Store](https://apps.apple.com/app/id6828859211), or sign the unsigned build yourself. See [iOS installation](../apps/ios/README.md). |

### Repair macOS permissions

Release upgrades with compatible signing identities normally retain permissions. After upgrading an unsigned or ad-hoc build, quit the old app and launch the new version. It migrates Accessibility and Screen Recording in the background for the current user, then guides you through authorization. Completion is saved separately for each permission.

For failed or interrupted migrations, open **Settings → System permissions** and choose **Retry migration**. Choose **Later** to continue using the client. Affected host features become available after authorization. If access appears enabled while capture or input still fails, quit CrossDesk, remove the old entry from the affected privacy list, add `/Applications/CrossDesk.app` again, and reopen the app.

If the issue persists, quit CrossDesk and run only the command for the affected permission in the logged-in user's terminal.

```bash
# Screen Recording permission issue
tccutil reset ScreenCapture cn.crossdesk.app
# Accessibility permission issue
tccutil reset Accessibility cn.crossdesk.app
```

Reopen CrossDesk and grant the requested permission in System Settings. If entries for older paths or builds remain, remove the old entry from the affected permission list and add the current app again.

If the issue persists, export diagnostic logs from **Feedback & suggestions** and send the ZIP to your CrossDesk administrator with both OS versions, client versions, connection mode, and reproduction steps. Remove passwords and private device information from logs or screenshots before sharing them.
