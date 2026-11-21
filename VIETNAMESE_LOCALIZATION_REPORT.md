# Báo cáo Việt hóa CrossDesk

## 1. Phạm vi, mốc đối chiếu và số liệu

Đối chiếu toàn bộ thay đổi với commit gốc `ff369cd`; mã ứng dụng tại `308596f8`. Báo cáo này là tệp bổ sung cuối đợt. Không đổi tên tệp/thư mục. Phạm vi dịch nội dung là Han → tiếng Việt theo các bước đã duyệt; tài liệu tiếng Anh có sẵn được giữ, trừ nhãn liên kết hoặc nội dung lỗi thời cần sửa. Không tuyên bố toàn bộ văn bản tiếng Anh đã được dịch.

- Cây gốc: **1097** mục Git; trước báo cáo: **1112**; sau khi thêm báo cáo: **1113**.
- Diff so với gốc: **111 tệp** = **110 tệp** của các bước 1–7 + **1 báo cáo**; không có đổi tên/xóa tệp.
- **Tệp cần Việt hóa trực tiếp: 96; đã dịch/biên soạn tiếng Việt: 83; còn chặn: 13 ảnh.** “Đã dịch” bao gồm tài liệu, chuỗi/resource sinh, thông điệp/giá trị kiểm thử tiếng Việt mới và báo cáo, không chỉ bản dịch văn xuôi.
- **28 tệp hỗ trợ** trong diff chỉ đổi tích hợp catalog, mặc định, chọn ngôn ngữ hoặc kiểm thử; được ghi “đã rà soát”, không cộng trùng vào số tệp dịch trực tiếp.
- **Đã rà soát tĩnh 1049 tệp văn bản** (bao gồm tệp đã dịch). Trạng thái độc quyền trong phụ lục: **83 đã dịch + 966 đã rà soát + 64 ngoại lệ = 1113**.
- **63 tệp nhị phân** chỉ kiểm tên/định dạng/tham chiếu, không OCR; **1 gitlink** MiniRTC chưa khởi tạo, không kiểm nội dung kho con. Hai nhóm này tạo 64 ngoại lệ theo tệp; 13 ảnh chặn là tập con, không cộng thêm.

Quy tắc tái lập trạng thái: trong diff, tệp có nội dung tiếng Việt được thêm là “đã dịch” (bao gồm báo cáo mới); tệp văn bản còn lại là “đã rà soát”. “Rà soát” đối với phần không thay đổi nghĩa là quét tự động toàn văn, không khẳng định đã đọc thủ công từng dòng. Phụ lục chỉ rõ tệp nào thuộc diff và tệp nào giữ nguyên. Số “cần Việt hóa” là phần công việc trực tiếp của đợt này cộng ảnh tiếng Trung còn lại, không phải số tệp có ký tự Han: phần lớn Han là khóa hay dữ liệu locale phải giữ.

## 2. Phương pháp và kết quả quét Han cuối

Nguồn danh sách là `git ls-files -z`, gồm thư mục ẩn như `.github`, không phụ thuộc ignore; đối chiếu bằng `git diff --name-status ff369cd HEAD`. Quét cả tên/đường dẫn, văn bản UTF-8/BOM và UTF-16 BOM. Dải nhận diện gồm CJK Extension A, Unified Ideographs, Compatibility Ideographs và các extension ngoài BMP đến U+323AF. Quét thêm escape dạng backslash-u bốn chữ số, backslash-u có ngoặc nhọn của Swift, backslash-U tám chữ số và cặp surrogate JSON/Java. Không coi byte của PNG/font là văn bản; không suy ra hình ảnh không có chữ từ kết quả quét văn bản.

- Đầu vào ứng dụng/tài liệu trước báo cáo: **14559 dòng Han, 102 tệp**; **10458 (a), 102 (d), 3999 (e)**; **0 dòng (b)/(c) chưa xử lý**, **0 đường dẫn/tên tệp/thư mục có Han**.
- Trong đó 3 dòng chỉ chứa Han qua escape: bộ nhận diện CJK của kiểm thử Android và hai đường dẫn Unicode trong kiểm thử Windows autostart; đều là (d), được liệt kê riêng dưới đây.
- Toàn bộ tệp Git sau khi thêm báo cáo: **14739 dòng Han / 103 tệp**, gồm **180 dòng bằng chứng trong chính báo cáo**; số dòng có escape: **6**. Phần tự trích dẫn thuộc (d), không phải chuỗi UI mới. Dòng trong báo cáo: 85–162, 319–420.

Phân loại: (a) khóa kỹ thuật và bảng lookup, giữ nguyên; (b) bình luận Han cần dịch; (c) văn bản người dùng cần đi qua catalog hoặc dịch trực tiếp; (d) fixture, nhận diện ngôn ngữ, neo tương thích và bằng chứng kiểm kê; (e) dữ liệu locale/tên ngôn ngữ gốc. Với dòng hỗn hợp khóa và bản dịch trong bảng sinh, (a) là loại chính; các cột zh/ja được giữ như dữ liệu locale. Không có bình luận Han chưa dịch hay văn bản UI Han bỏ qua catalog được phát hiện ngoài các ngoại lệ đã ghi.

## 3. Quy ước thuật ngữ

| Khái niệm | Thuật ngữ tiếng Việt |
|---|---|
| Remote control / remote desktop | Điều khiển từ xa / màn hình từ xa |
| Controller / host | Bên điều khiển / bên được điều khiển |
| Local / remote | Máy này / máy từ xa |
| Session / signaling | Phiên / báo hiệu |
| Connect / disconnect / reconnect | Kết nối / ngắt kết nối / kết nối lại |
| File transfer / file browser | Truyền tệp / trình quản lý tệp |
| Upload / download | Tải lên / tải xuống |
| Clipboard / shortcut | Bảng nhớ tạm / phím tắt |
| Privacy policy / privacy screen | Chính sách quyền riêng tư / màn hình riêng tư |
| Follow system / fallback | Theo hệ thống / ngôn ngữ dự phòng |
| Permission / accessibility | Quyền / trợ năng |
| Source / license / notice | Mã nguồn / giấy phép / thông báo |

Giữ nguyên CrossDesk, DeskHub, MiniRTC, MCP, CLI, P2P, TURN, ID, API, tên giấy phép và văn bản pháp lý. Một số nhãn cần biến thể ngữ cảnh; không đổi ID lệnh, placeholder, biến môi trường, đường dẫn thật, URL, phiên bản hay đơn vị. Placeholder dành cho người đọc trong ví dụ lệnh đã được dịch.

## 4. Ánh xạ ngôn ngữ và tương thích desktop

Không có ánh xạ đổi tên tệp/thư mục: **không đổi tên**. Không sửa nghĩa cấu hình desktop đã lưu:

| Giá trị lưu | Nghĩa |
|---|---|
| 0–9 | Giữ các ngôn ngữ lịch sử theo thứ tự zh-Hans, en, ru, zh-Hant, ja, ko, de, fr, es, pt-BR |
| 10 | Theo ngôn ngữ hệ thống, không phải tiếng Việt |
| 11 | Chọn tiếng Việt tường minh; mặc định cho cài mới |

Cột catalog tiếng Việt là **10** (catalog có 11 cột); hàm ánh xạ chuyển ID lưu 11 sang cột 10. ID 10 được giải quyết theo hệ thống trước; locale không hỗ trợ quay về tiếng Việt. Các cấu hình cũ 0–10 vẫn có nghĩa cũ. Android `values` dùng tiếng Việt, có `values-en`/`values-vi`; iOS có development region `vi` và `vi.lproj`; Harmony chọn cột 10 khi mặc định/locale không hỗ trợ. Khóa thiếu không đồng nghĩa với dịch tùy ý: một số API giữ khóa để tương thích; riêng hậu tố phân loại tiếng Trung được phép có giá trị Việt rỗng.

## 5. Thay đổi nội dung có chủ ý

| Vị trí | Thay đổi và lý do |
|---|---|
| <code>docs/USER_GUIDE.md:134; docs/FAQ.md:88</code> | Dịch placeholder tên gói/đường dẫn trong lệnh vì là hướng dẫn cho người đọc, không phải literal kỹ thuật. |
| <code>docs/USER_GUIDE.md:182</code> | Mô tả đúng mặc định tiếng Việt cho cài mới, 11 ngôn ngữ, theo hệ thống, fallback và tương thích cấu hình. |
| <code>docs/HEADLESS.md:64,70</code> | Thay mô tả ba ngôn ngữ lỗi thời bằng 11 ngôn ngữ cùng lựa chọn hệ thống. |
| <code>PRIVACY.md:11; localization/README.md:25</code> | Chính sách tiếng Việt là bản dịch, giữ nguyên phần tiếng Anh; cập nhật nhãn và ranh giới phần. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java:26; apps/ios/CrossDeskMobile/PrivacyViews.swift:180; apps/harmony/entry/src/main/ets/mobile/PrivacyPolicy.ets:11</code> | Theo phê duyệt: vi/mặc định/locale không hỗ trợ dùng phần Việt; locale được hỗ trợ khác kể cả zh dùng tiếng Anh. Tìm phần Anh độc lập, hỗ trợ tiêu đề cũ và CRLF. |
| <code>README_EN.md; docs/*_EN.md</code> | Đổi nhãn đích liên kết sang bản tiếng Việt; giữ văn xuôi tiếng Anh có sẵn và neo URL ngoài. |
| <code>.github/ISSUE_TEMPLATE/01-bug-report.yml; .github/ISSUE_TEMPLATE/02-feature-request.yml</code> | Biểu mẫu chào đón tiếng Việt/Anh thay tiếng Trung/Anh; giữ ID/khóa YAML, kiểu, validation và URL. |
| <code>apps/mcp/src/browser.ts; apps/mcp/ui/viewer.ts</code> | Dịch trực tiếp UI không có catalog, đặt ngôn ngữ tài liệu thành vi; không đổi giao thức/ID HTML. |
| <code>apps/desktop/src/gui/features/file_transfer/file_browser_window.cpp; apps/desktop/src/gui/ui/file_browser.slint; apps/desktop/src/gui/ui/ui_localization.h</code> | Thay nhánh chỉ Trung/Anh bằng catalog đầy đủ, cập nhật nhãn khi đổi ngôn ngữ. |
| <code>apps/desktop/scripts/macos/pkg_arm64.sh; apps/desktop/scripts/macos/pkg_x64.sh</code> | Dịch nội dung installer toàn cục và lý do quyền; không phải tài nguyên zh riêng. Chuẩn hóa LF để Bash phân tích đúng. |
| <code>apps/desktop/src/platform/macos/input/macos_keycode.h</code> | Sửa ký tự copyright bị lỗi mã hóa thành ký hiệu copyright, không đổi ghi công. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteSessionService.java:52</code> | Khởi tạo L10n trước tạo notification channel để service không phụ thuộc khởi tạo UI. |
| <code>apps/ios/CrossDeskMobile/Info.plist:30</code> | Quyền mạng cơ sở khớp bản Việt của cùng khóa catalog dùng sinh InfoPlist.strings; giữ nguyên ja/zh. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileController.ets; apps/harmony/entry/src/main/ets/mobile/ControllerModels.ets</code> | Dịch tại consumer của feedback/error/end reason native, không đổi payload; giữ nguyên clipboard, tên tệp và nội dung từ xa. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/PrivacyConsentTest.java; apps/android/test/android/java/cn/crossdesk/mobile/AppUpdateCheckerTest.java</code> | Selector UI theo catalog thay vì đòi tiếng Trung; dữ liệu legacy và kiểm thử CJK không đổi. |

Các số dòng là vị trí đích tại đợt sửa tương ứng; các hàng chỉ tên tệp mô tả thay đổi trên nhiều dòng. Neo tương thích CONNECTIONS và MCP được giữ; các liên kết Han nội bộ sang tài liệu đã dịch đã sửa. Không tìm thấy URL runtime đến neo cũ của PRIVACY, vì vậy không thêm alias neo không cần thiết; parser tiêu đề cũ vẫn hoạt động.

## 6. 73 + 5 khóa catalog mới và fallback tiếng Anh

73 khóa ở bước 6 và 5 khóa ở bước 7; cả 78 có đủ 11 trường ngôn ngữ, với vi/en/zh đầy đủ. Hai khóa sẵn có chỉ được bật thêm cho Harmony, không tính là khóa mới. Bảng dưới ghi khóa bằng chuỗi JSON để nhìn thấy cả khoảng trắng đầu/cuối. “Fallback” chỉ các giá trị thực sự sao chép tiếng Anh; bản dịch do con người biên soạn nhưng trùng chữ tiếng Anh (như Local, Home, Host, Remote, Destination) ghi cột riêng, không coi là thiếu dịch. Đã đối chiếu nguồn bảng dịch ngắn của bước 6, không chỉ so sánh chuỗi máy móc. Năm khóa native bước 7 có bản dịch thật ở mọi ngôn ngữ, không dùng fallback tiếng Anh.

| Bước | Khóa | Tiếng Việt | Ngôn ngữ dùng fallback en | Bản dịch thật trùng en |
|---|---|---|---|---|
| 6 | <code>&quot;，降序&quot;</code> | , giảm dần | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;，升序&quot;</code> | , tăng dần | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;，点击按降序排列&quot;</code> | , nhấn để sắp xếp giảm dần | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;，点击按升序排列&quot;</code> | , nhấn để sắp xếp tăng dần | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;接收目录&quot;</code> | Thư mục nhận | Không | fr |
| 6 | <code>&quot;筛选名称…&quot;</code> | Lọc theo tên… | Không | — |
| 6 | <code>&quot;打开上一级目录&quot;</code> | Mở thư mục cấp trên | Không | — |
| 6 | <code>&quot;输入路径，按回车打开&quot;</code> | Nhập đường dẫn, nhấn Enter để mở | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;浏览目录树&quot;</code> | Duyệt cây thư mục | Không | — |
| 6 | <code>&quot;选择全部&quot;</code> | Chọn tất cả | Không | — |
| 6 | <code>&quot;，部分已选&quot;</code> | , đã chọn một phần | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;修改日期&quot;</code> | Ngày sửa đổi | Không | — |
| 6 | <code>&quot;大小&quot;</code> | Kích thước | Không | — |
| 6 | <code>&quot;选择 &quot;</code> | Chọn  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;没有匹配的项目&quot;</code> | Không có mục phù hợp | Không | — |
| 6 | <code>&quot;此目录为空&quot;</code> | Thư mục này trống | Không | — |
| 6 | <code>&quot;清空筛选&quot;</code> | Xóa bộ lọc | Không | — |
| 6 | <code>&quot;无法读取目录&quot;</code> | Không thể đọc thư mục | Không | — |
| 6 | <code>&quot;打开主目录&quot;</code> | Mở thư mục chính | Không | — |
| 6 | <code>&quot;Enter 打开路径 · Esc 还原&quot;</code> | Enter để mở đường dẫn · Esc để khôi phục | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;目录不可用&quot;</code> | Thư mục không khả dụng | Không | — |
| 6 | <code>&quot;筛选结果 · &quot;</code> | Kết quả lọc ·  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;主目录&quot;</code> | Thư mục chính | Không | de |
| 6 | <code>&quot;磁盘&quot;</code> | Ổ đĩa | Không | — |
| 6 | <code>&quot;显示隐藏项&quot;</code> | Hiện mục ẩn | Không | — |
| 6 | <code>&quot;文件传输&quot;</code> | Truyền tệp | Không | — |
| 6 | <code>&quot;刷新目录&quot;</code> | Làm mới thư mục | Không | — |
| 6 | <code>&quot;刷新双方目录&quot;</code> | Làm mới thư mục hai bên | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;本机&quot;</code> | Máy này | Không | fr, es, pt-BR |
| 6 | <code>&quot;远端&quot;</code> | Máy từ xa | Không | de |
| 6 | <code>&quot;按 Enter 打开路径&quot;</code> | Nhấn Enter để mở đường dẫn | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;松手复制 · Esc 取消&quot;</code> | Thả để sao chép · Esc để hủy | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;选择文件，或拖到另一侧&quot;</code> | Chọn tệp hoặc kéo sang phía bên kia | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;目标：&quot;</code> | Đích:  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;取消选择&quot;</code> | Bỏ chọn | Không | — |
| 6 | <code>&quot;传输任务&quot;</code> | Tác vụ truyền | Không | — |
| 6 | <code>&quot;暂无任务&quot;</code> | Chưa có tác vụ | Không | — |
| 6 | <code>&quot;暂无传输任务&quot;</code> | Chưa có tác vụ truyền | Không | — |
| 6 | <code>&quot;控制端&quot;</code> | Máy điều khiển | Không | — |
| 6 | <code>&quot;被控端&quot;</code> | Máy được điều khiển | Không | de, pt-BR |
| 6 | <code>&quot;远端设备&quot;</code> | Thiết bị từ xa | Không | — |
| 6 | <code>&quot;正在读取目录…&quot;</code> | Đang đọc thư mục… | Không | — |
| 6 | <code>&quot; 项&quot;</code> |  mục | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;正在准备传输 → &quot;</code> | Đang chuẩn bị truyền →  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;远端未响应，请确认两端已更新至支持文件浏览的版本后重试。&quot;</code> | Máy từ xa không phản hồi, hãy bảo đảm hai bên đã cập nhật bản hỗ trợ duyệt tệp rồi thử lại. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;连接已断开，请重新连接后打开文件管理器。&quot;</code> | Kết nối đã bị ngắt, hãy kết nối lại rồi mở trình quản lý tệp. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;请先在控制端打开本连接的文件管理器，再浏览控制端目录。&quot;</code> | Hãy mở trình quản lý tệp của kết nối này trên máy điều khiển trước khi duyệt thư mục máy điều khiển. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;无法打开目录，请检查路径和访问权限后刷新。&quot;</code> | Không thể mở thư mục, hãy kiểm tra đường dẫn và quyền truy cập rồi làm mới. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;远端版本无法完整读取此目录，请更新远端后重试。&quot;</code> | Bản từ xa không thể đọc đầy đủ thư mục này, hãy cập nhật máy từ xa rồi thử lại. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;目录列表未能完整加载，请刷新重试。&quot;</code> | Không thể tải đầy đủ danh sách thư mục, hãy làm mới để thử lại. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;目标目录不可写，请选择其他目录。&quot;</code> | Không thể ghi vào thư mục đích, hãy chọn thư mục khác. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;所选内容包含符号链接，请选择实际文件或目录。&quot;</code> | Nội dung đã chọn có liên kết tượng trưng, hãy chọn tệp hoặc thư mục thực. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;单次最多传输 4096 个文件与目录，请分批选择。&quot;</code> | Mỗi lần truyền tối đa 4096 tệp và thư mục, hãy chia thành nhiều lượt chọn. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;文件无法读取，可能已移动或无访问权限。&quot;</code> | Không thể đọc tệp, có thể tệp đã được chuyển hoặc bạn không có quyền truy cập. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;请求较多，请稍后重试。&quot;</code> | Có nhiều yêu cầu, hãy thử lại sau. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;任务已结束，无需取消。&quot;</code> | Tác vụ đã kết thúc, không cần hủy. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;操作失败，请检查所选路径与文件权限后重试。&quot;</code> | Thao tác thất bại, hãy kiểm tra đường dẫn đã chọn và quyền tệp rồi thử lại. | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;正在取消传输…&quot;</code> | Đang hủy truyền… | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;已加入传输队列&quot;</code> | Đã thêm vào hàng đợi truyền | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;正在加载 &quot;</code> | Đang tải  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot; 项…&quot;</code> |  mục… | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;已传输 &quot;</code> | Đã truyền  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;排队中&quot;</code> | Đang xếp hàng | Không | — |
| 6 | <code>&quot;发送中 &quot;</code> | Đang gửi  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;接收中 &quot;</code> | Đang nhận  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;失败 · 请重试&quot;</code> | Thất bại · Hãy thử lại | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot; 项进行中&quot;</code> |  mục đang thực hiện | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot; 项失败&quot;</code> |  mục thất bại | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot; 项 · &quot;</code> |  mục ·  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;松手复制 &quot;</code> | Thả để sao chép  | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot; 项到此目录&quot;</code> |  mục vào thư mục này | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;远端 → 本机&quot;</code> | Máy từ xa → Máy này | ru, ja, ko, de, fr, es, pt-BR | — |
| 6 | <code>&quot;本机 → 远端&quot;</code> | Máy này → Máy từ xa | ru, ja, ko, de, fr, es, pt-BR | — |
| 7 | <code>&quot;远程会话信令连接失败&quot;</code> | Kết nối báo hiệu phiên từ xa thất bại | Không | — |
| 7 | <code>&quot;无法发起连接&quot;</code> | Không thể bắt đầu kết nối | Không | — |
| 7 | <code>&quot;服务器连接超时，请检查地址和网络&quot;</code> | Kết nối máy chủ hết thời gian chờ, hãy kiểm tra địa chỉ và mạng | Không | — |
| 7 | <code>&quot;连接超时，请确认设备在线&quot;</code> | Kết nối hết thời gian chờ, hãy xác nhận thiết bị đang trực tuyến | Không | — |
| 7 | <code>&quot;操作失败，请检查连接参数或重新连接&quot;</code> | Thao tác thất bại, hãy kiểm tra tham số kết nối hoặc kết nối lại | Không | — |

## 7. Kiểm tra đã chạy và kết quả thực tế

Dưới đây là kết quả đã lưu ở các bước 1–7, không phải tuyên bố chạy lại mọi suite ở bước 8. Bước 8 chỉ thêm báo cáo; đầu vào build/test ứng dụng không thay đổi. Không chạy lại check đã đạt chỉ để có log mới. Các lệnh quét/đối chiếu/số liệu/Markdown của bước 8 được kiểm riêng sau khi tạo báo cáo.

| Kiểm tra | Kết quả in thực tế |
|---|---|
| <code>PYTHONUTF8=1 python localization/generate.py --check (bước 7)</code> | <code>1056 strings, 11 language variants; resources verified</code> |
| <code>localization/test_generate.py</code> | <code>Ran 7 tests in 0.145s; OK (skipped=1)</code> |
| <code>Android test/localization_test.py sau sửa cuối</code> | <code>Ran 4 tests in 0.017s; OK</code> |
| <code>Android notice generator (bước 6)</code> | <code>Ran 3 tests in 0.023s; OK</code> |
| <code>Harmony Python resources (bước 7)</code> | <code>Ran 3 tests in 0.035s; OK</code> |
| <code>Version manifest (bước 6)</code> | <code>Ran 11 tests in 0.082s; OK</code> |
| <code>Windows autostart Python (bước 6)</code> | <code>Ran 0 tests in 0.006s; OK (skipped=1)</code> |
| <code>MCP npm ci (bước 6)</code> | <code>added 23 packages in 12s</code> |
| <code>MCP npm.cmd run check / npm.cmd run build (bước 6)</code> | <code>tsc --noEmit &amp;&amp; tsc -p tsconfig.ui.json; tsc &amp;&amp; tsc -p tsconfig.ui.json &amp;&amp; node scripts/build-ui.mjs &amp;&amp; node scripts/build-harness.mjs; Exit code: 0</code> |
| <code>MCP AST (bước 6)</code> | <code>PASS: MCP AST structure, identifiers, numbers, protocol literals and HTML control attributes unchanged; 180 human-text segments translated</code> |
| <code>Bash installer (bước 6)</code> | <code>PASS: bash -n pkg_arm64.sh and pkg_x64.sh</code> |
| <code>Harmony localization (bước 7)</code> | <code>PASS: 11 locale mappings, Vietnamese default/empty/error fallback, English override and placeholders</code> |
| <code>Harmony policy (bước 7)</code> | <code>PASS: privacy policy locale/default/error fallback, legacy and independent sections, empty/missing sections, CRLF and bundled policy</code> |
| <code>Harmony native-event/transfer localization (bước 7)</code> | <code>PASS: localized native feedback/errors/end reasons, file progress/empty/error states, English override, stale events and unchanged clipboard/file-name data</code> |
| <code>Harmony models (bước 7)</code> | <code>PASS: mobile viewport/gestures, shared button ownership, keyboard, video ACK ordering, network expiry, presence and announcement pagination/revisions</code> |
| <code>Harmony text input / permissions (bước 7)</code> | <code>Slint IME editing, selection, composition, detached callbacks and clipboard serialization passed</code> |
| <code>Harmony desktop window (bước 7)</code> | <code>Slint window restore/fullscreen, optional services, recreation, stale callbacks and queue cleanup passed</code> |
| <code>Tài liệu bước 5</code> | <code>RELATIVE LINKS: 409 checked; 153 anchors; 129 external skipped; REPOSITORY-WIDE INCOMING: 386 references into 33 translated documents; 162 anchors checked; Errors: []</code> |
| <code>YAML bước 5</code> | <code>YAML PARSE/KEYS/IDS/TYPES/VALIDATION/URLS: cả 3 issue-template MATCH</code> |
| <code>Cấu trúc tài liệu bước 4/5</code> | <code>12/15 bộ tuple headings, table rows, code blocks, links, images: MATCH; phần chính sách Anh không đổi</code> |
| Quét tracked bước 8, gồm báo cáo | 1113 mục Git; 1049 văn bản, 63 nhị phân, 1 gitlink; 14739 dòng Han / 103 tệp; 6 dòng có escape; 0 tên Han |
| Kiểm định báo cáo bước 8 | Đối chiếu đủ 1113 dòng danh mục tệp, 111 tệp diff (95 sửa, 16 thêm), 78 khóa, 13 danh sách tham chiếu ảnh; dải dòng (e) và tự kiểm kê khớp |
| Kiểm tra diff bước 8 | git diff --cached --check: exit 0, không có lỗi khoảng trắng |

### Lỗi có sẵn / giới hạn môi trường, không gộp thành đạt

| Suite | Lỗi/kết quả | Bằng chứng baseline và giới hạn kết luận |
|---|---|---|
| <code>Android Python discovery</code> | <code>Ran 8 tests in 0.040s; FAILED (errors=1, skipped=1); ModuleNotFoundError: No module named &#x27;fcntl&#x27;</code> | Bản HEAD chép riêng tái hiện cùng lỗi tại build_native.py:4; os.name=nt, fcntl không có trên Windows. Build script/test không thay đổi. Kết quả suite này có trước lần thêm guard thứ tư; guard cuối chạy riêng 4/4 OK, không giả ghi full suite đạt. |
| <code>iOS Python discovery</code> | <code>Ran 39 tests in 0.894s; FAILED (errors=2, skipped=31); FileNotFoundError: libopus.a; KeyError: &#x27;opus&#x27;</code> | Archive baseline của scripts/tests/licenses tái hiện Ran 6 tests; FAILED (errors=2). Hai ca: test_simulator_cache_keeps_archive_validation và test_source_only_commit_keeps_reviewed_catalog_and_uses_actual_links. verify_native_build.py:21 và third_party_licenses_test.py:90/99 không thay đổi bởi Việt hóa. |
| <code>Harmony mobile_lifecycle_test.cjs:52</code> | <code>assert(commands.some(c=&gt;c.op===&#x27;start&#x27;)); actual: false, expected: true</code> | Chép nguồn HEAD trước sửa vào thư mục baseline, chạy cùng Node/TypeScript: exit 1 cùng assertion. Có cả baseline trước các bước nguồn. |
| <code>Harmony mobile_platform_test.cjs:23; mobile_store_test.cjs:24</code> | <code>Error: ./Localization</code> | VM require stub của fixture từ chối import. Bản HEAD riêng tái hiện cả hai exit 1, cùng thông báo. |
| <code>Harmony autostart_host_test.cjs:20</code> | <code>Error: Slint startup: Cannot read properties of undefined (reading &#x27;System&#x27;)</code> | Bước 6 chép đủ desktop-port ETS của HEAD tái hiện cùng lỗi; lần đầu thiếu DesktopHost chỉ là lỗi thiết lập repro, đã bổ sung trước khi kết luận. Fixture/nguồn autostart không được sửa để che lỗi. |
| <code>.github/scripts/test_xmake_version_check.py</code> | <code>Ran 9 tests in 10.694s; FAILED (failures=37); AssertionError: 127 != 0; /bin/bash: C:UsersAdministratorcrossdesk-baseline-tests.githubactionsensure-xmakecheck-version.sh: No such file or directory</code> | Baseline archive tái hiện cùng lỗi shell/đường dẫn Windows. Đây là check đã chạy từ bước sớm; không phải lỗi bản dịch hay 37 test độc lập (có subtest). |

Không có lỗi mới do thay đổi Việt hóa được phát hiện trong các kiểm tra đã thực thi; điều này không chứng minh native build/UI đúng. Một số lỗi thao tác đã sửa trước kết quả cuối: wrapper npm Bash chọn đường dẫn WSL không hợp lệ (dùng npm.cmd), CRLF làm bash -n lỗi (chuẩn hóa hai shell script sang LF), marker ngày của fixture policy sai (đổi kỳ vọng sang marker thực có). Không đổi cấu hình Git hay cài toolchain lớn. Cảnh báo core.autocrlf về LF/CRLF được chẩn đoán là chuyển dòng làm việc, không phải lỗi nội dung.

Bằng chứng đầy đủ trên máy làm việc: `C:/Users/Administrator/mobile_vi_check_logs/`, `source_vi_check_logs/`, `mobile_vi_baseline/`, `source_vi_baseline/`, `crossdesk-baseline-tests/baseline-0.log`, `baseline-1.log`. Các thư mục này ngoài Git; thông báo lỗi và điều kiện tái hiện quan trọng đã chép trong báo cáo để không phụ thuộc chúng khi đọc sau này.

## 8. Ảnh tiếng Trung và việc còn chặn

Đã đọc `docs/images/REGENERATE.md` (đường dẫn thực trong repo). **Không khả thi trong khoảng 30 phút trên máy Windows hiện tại; không thử build/tạo lại ảnh.** Quy trình hiện tại dùng `xcrun clang++`, Objective-C++ AppKit, framework CoreFoundation/AppKit và `/usr/sbin/screencapture`, riêng macOS. PATH không có xmake, cl, clang++, g++, cmake, cargo, rustc hay xcrun; không có thư mục build, cache `.xmake`, cache package xmake, hay nội dung MiniRTC submodule. Python/Node vẫn có, nhưng không thay thế compiler, Rust/Slint và SDK/framework. Cần máy macOS đã chuẩn bị hoặc port công cụ capture sang Windows, tải/biên dịch dependency và thêm vòng capture ngôn ngữ Việt; chưa có căn cứ bảo đảm xong trong giới hạn, nên giữ nguyên ảnh như yêu cầu.

Cả 13 ảnh sau là ngoại lệ chưa Việt hóa. Danh sách tham chiếu là toàn bộ khớp tên tệp trong văn bản tracked trước báo cáo; mẫu sinh tên động trong REGENERATE cũng áp dụng cho tất cả ảnh. Báo cáo không tính liên kết tự trích dẫn của chính nó. Không đổi tên `-zh.png` thành `-vi.png` vì sẽ gây hiểu sai nội dung.

| Ảnh | Nơi tham chiếu hiện tại | Trạng thái |
|---|---|---|
| <code>docs/images/desktop-about-zh.png</code> | <code>docs/USER_GUIDE.md:244</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-deskhub-focus-zh.png</code> | <code>docs/USER_GUIDE.md:98</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-deskhub-zh.png</code> | <code>README.md:150</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-feedback-zh.png</code> | <code>docs/USER_GUIDE.md:254</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-files-zh.png</code> | <code>README.md:160</code>; <code>docs/USER_GUIDE.md:155</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-lan-password-zh.png</code> | <code>docs/USER_GUIDE.md:61</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-lan-zh.png</code> | <code>README.md:140</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-main-zh.png</code> | <code>README.md:132</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-self-hosted-zh.png</code> | <code>docs/SELF_HOSTING.md:56</code>; <code>docs/USER_GUIDE.md:399</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-settings-deskhub-zh.png</code> | <code>docs/USER_GUIDE.md:234</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-settings-network-zh.png</code> | <code>docs/USER_GUIDE.md:196</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-settings-zh.png</code> | <code>docs/USER_GUIDE.md:177</code>; <code>docs/images/README.md:15</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |
| <code>docs/images/desktop-stream-video-settings-zh.png</code> | <code>README.md:173</code>; <code>docs/USER_GUIDE.md:165</code> | Chặn: cần tạo lại ảnh UI Việt trên môi trường native phù hợp |

Các fixture raster khác, ảnh English/Web/CLI/font/PDF không được OCR hay chỉnh sửa trong đợt này; bảng toàn tệp ghi rõ giới hạn nhị phân. Không dùng build thành công hay DOM thay cho xác minh hình ảnh.

- Chưa build/chạy trực quan desktop C++/Slint, Android, iOS hay Harmony; không có bằng chứng bố cục, xuống dòng và clipping của tiếng Việt trên thiết bị thật.
- Không chạy Java compile/instrumentation vì thiếu JDK/SDK; không chạy Swift/xcrun native vì không có Apple toolchain; không compile C++ do không có compiler cấu hình. Một số test host Python tự skip đúng điều kiện, không coi skip là native pass.
- Không sửa nguyên văn giấy phép pháp lý; thông báo do dự án tự viết đã dịch, generator input và bundle đã cập nhật. Lịch sử copyright và ID giao thức được giữ.
- Giữ neo ngoài của kho server và URL ngoài, không kiểm mạng/anchor bên ngoài; xem từng dòng (d) dưới đây. Các đường dẫn MiniRTC dùng trong link check trước được đối chiếu cây GitHub ghim d10fd498df1f76a2929b68b8f8ee8bc914ee1aea (không thay thế build submodule).
- Giữ dữ liệu zh/ja và tên ngôn ngữ bản địa; đó là yêu cầu hỗ trợ đa ngôn ngữ, không phải phần UI Việt bỏ sót. Các giá trị fallback tiếng Anh ngoài vi/en/zh còn lại được công khai ở mục 6.

## 9. Han giữ lại: (a) theo tệp

| Tệp | Số dòng (a) |
|---|---:|
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AboutPages.java</code> | 35 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AnnouncementText.java</code> | 1 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AnnouncementView.java</code> | 18 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/BackgroundSession.java</code> | 3 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/LicenseCatalog.java</code> | 3 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/LocalizationKeys.java</code> | 369 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/MainActivity.java</code> | 102 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/NativeSession.java</code> | 20 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyConsentDialog.java</code> | 4 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java</code> | 1 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteDisplayResolutions.java</code> | 1 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteKeyboard.java</code> | 11 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteNetworkStatistics.java</code> | 1 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemotePreviewStore.java</code> | 2 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemotePrivacyScreen.java</code> | 14 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteSessionService.java</code> | 9 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteShortcutPanel.java</code> | 19 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteShortcuts.java</code> | 14 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVersionCheck.java</code> | 1 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVideoSettings.java</code> | 5 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVideoView.java</code> | 2 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/SessionControls.java</code> | 24 |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/VirtualMouseBar.java</code> | 4 |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/AppUpdateCheckerTest.java</code> | 3 |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/PrivacyConsentTest.java</code> | 11 |
| <code>apps/desktop/src/gui/application/gui_application.cpp</code> | 61 |
| <code>apps/desktop/src/gui/assets/localization/localization_data.h</code> | 1203 |
| <code>apps/desktop/src/gui/features/file_transfer/file_browser_window.cpp</code> | 40 |
| <code>apps/desktop/src/gui/ui/ui_localization.h</code> | 25 |
| <code>apps/harmony/entry/src/main/ets/mobile/ControllerModels.ets</code> | 4 |
| <code>apps/harmony/entry/src/main/ets/mobile/LocalizationData.ets</code> | 309 |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileController.ets</code> | 44 |
| <code>apps/harmony/entry/src/main/ets/mobile/MobilePlatform.ets</code> | 7 |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileRemoteView.ets</code> | 44 |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileStore.ets</code> | 2 |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileWidgets.ets</code> | 1 |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteDisplayResolutions.ets</code> | 1 |
| <code>apps/harmony/entry/src/main/ets/mobile/RemotePrivacyScreen.ets</code> | 14 |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteShortcutPanel.ets</code> | 25 |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteShortcuts.ets</code> | 14 |
| <code>apps/harmony/entry/src/main/ets/pages/Mobile.ets</code> | 117 |
| <code>apps/harmony/native/mobile_session.cpp</code> | 6 |
| <code>apps/ios/CrossDeskMobile/AnnouncementInboxView.swift</code> | 23 |
| <code>apps/ios/CrossDeskMobile/ContentView.swift</code> | 64 |
| <code>apps/ios/CrossDeskMobile/Input/RemoteShortcutPanel.swift</code> | 32 |
| <code>apps/ios/CrossDeskMobile/Input/RemoteShortcuts.swift</code> | 13 |
| <code>apps/ios/CrossDeskMobile/Input/RemoteSystemKeyboard.swift</code> | 8 |
| <code>apps/ios/CrossDeskMobile/Input/VirtualMouseBar.swift</code> | 2 |
| <code>apps/ios/CrossDeskMobile/PrivacyViews.swift</code> | 32 |
| <code>apps/ios/CrossDeskMobile/RecentConnection.swift</code> | 1 |
| <code>apps/ios/CrossDeskMobile/RemoteNetworkStatistics.swift</code> | 2 |
| <code>apps/ios/CrossDeskMobile/RemotePrivacyScreen.swift</code> | 14 |
| <code>apps/ios/CrossDeskMobile/RemoteSessionModel.swift</code> | 68 |
| <code>apps/ios/CrossDeskMobile/RemoteSessionView.swift</code> | 56 |
| <code>apps/ios/CrossDeskMobile/Resources/de.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/en.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/es.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/fr.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/ja.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/ko.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/pt-BR.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/pt.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/ru.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/vi.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hans.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hant.lproj/Localizable.strings</code> | 352 |
| <code>apps/ios/CrossDeskMobile/ThirdPartyLicensesView.swift</code> | 38 |
| <code>localization/catalog.json</code> | 3286 |
| <code>localization/generate.py</code> | 1 |

## 10. Han giữ lại: từng dòng (d)

Giá trị kỳ vọng Swift standalone giữ khóa Trung vì harness compile model mà không bundle resource app; không ép thay thành Việt gây sai mô hình kiểm thử. UI Android thực được đổi selector qua lookup. Dòng escape CJK được giữ nguyên vì kiểm tra Unicode/nhận diện, không phải thông báo người dùng.

| Vị trí | Nội dung nguyên gốc | Lý do |
|---|---|---|
| <code>.github/scripts/test_update_version_manifest.py:137</code> | <code>notes = &#x27;中文 release notes\n&quot;quoted&quot; $(not-a-command) $HOME `literal`&#x27;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java:30</code> | <code>if(vietnamese.isEmpty())vietnamese=section(value,&quot;中文&quot;);</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java:51</code> | <code>boolean heading=paragraph.startsWith(&quot;#&quot;),date=paragraph.startsWith(&quot;更新日期：&quot;)&#124;&#124;paragraph.startsWith(&quot;Last updated:&quot;);</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/AnnouncementInboxTest.java:33</code> | <code>if(!summary)item.put(&quot;title&quot;,i==0?&quot;欢迎使用 CrossDesk&quot;:&quot;公告 &quot;+(i+1)).put(&quot;body&quot;,&quot;安卓公告与 iOS 保持一致。\n\n访问 [CrossDesk 官网](https://crossdesk.cn) 了解更多。\n支持中文、换行与网页链接。&quot;).put(&quot;updated_at&quot;,1790812800L);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/AnnouncementInboxTest.java:67</code> | <code>body.getJSONArray(&quot;items&quot;).getJSONObject(0).put(&quot;body&quot;,&quot;中&quot;.repeat(2667));inbox.receive(body);assertTrue(inbox.failed);assertEquals(0,inbox.items.size());</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/ControllerTest.java:78</code> | <code>assertEquals(0, KeyMap.ascii(&#x27;中&#x27;));</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/PrivacyConsentTest.java:134</code> | <code>assertEquals(&quot;Legacy&quot;,new PrivacyPolicyDocument(&quot;## 中文\r\nLegacy\r\n## English\r\nEnglish&quot;,&quot;vi&quot;).text);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/PrivacyConsentTest.java:135</code> | <code>assertEquals(&quot;English&quot;,new PrivacyPolicyDocument(&quot;## 中文\nLegacy\n## English\nEnglish&quot;,&quot;zh&quot;).text);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/localization_test.py:21</code> | <code>key = &#x27;未找到可打开链接的浏览器&#x27;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/localization_test.py:26</code> | <code>self.assertNotRegex(self.resources[identifier], &#x27;[\u3400-\u9fff]&#x27;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/localization_test.py:31</code> | <code>self.assertIn(&#x27;L10n.tr(&quot;远程连接&quot;)&#x27;, source)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/privacy_policy_test.py:20</code> | <code>cases += [(&#x27;## 中文\r\nLegacy\r\n## English\r\nEnglish&#x27;, &#x27;vi&#x27;, &#x27;Legacy&#x27;),</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/android/test/privacy_policy_test.py:21</code> | <code>(&#x27;## 中文\nLegacy\n## English\nEnglish&#x27;, &#x27;zh&#x27;, &#x27;English&#x27;), (None, None, &#x27;&#x27;)]</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:247</code> | <code>&quot;无法保存设置，请重试。&quot;, &quot;Unable to save settings. Please try again.&quot;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:276</code> | <code>&quot;**新增功能与改进**\n\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:277</code> | <code>&quot;1. 增强信令快速重连与连接恢复能力；桌面端配置修改无需重新连接信令服务。\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:278</code> | <code>&quot;2. Windows 安装版新增登录前远程控制支持，被控端可随系统启动，在尚未登录时接受远程连接。\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:279</code> | <code>&quot;3. Linux 提供普通版与 Xfce 虚拟桌面版独立安装包，普通版不再强制安装 Xfce 和 Xvfb。\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:280</code> | <code>&quot;4. 被控端会话窗口改为紧凑的屏幕边缘状态栏。\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:281</code> | <code>&quot;5. 完善 iOS / Android 原生控制端，优化远程画面、触控与外接键鼠体验。\n\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:282</code> | <code>&quot;**问题修复**\n\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:283</code> | <code>&quot;6. 修复连接中断后的状态显示，改善高分辨率屏幕上的文字与按钮边界。\n&quot;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:284</code> | <code>&quot;7. 统一窗口、输入框与按钮的圆角，优化更新提示与长文本滚动体验。&quot;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:461</code> | <code>&quot;全部设备&quot;, &quot;All devices&quot;, &quot;Все устройства&quot;)}));</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp:487</code> | <code>&quot;请输入完整的 9 位设备 ID&quot;, &quot;Enter a complete 9-digit device ID&quot;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_task_test.py:30</code> | <code>L&quot;S-1-5-21-123-456-789-1001&quot;, L&quot;C:\\程序 &amp; Tools\\CrossDesk.exe&quot;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_task_test.py:31</code> | <code>L&quot;C:\\程序 &amp; Tools&quot;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_task_test.py:68</code> | <code>self.assertEqual(self.text(&quot;t:Actions/t:Exec/t:Command&quot;), r&quot;C:\程序 &amp; Tools\CrossDesk.exe&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_task_test.py:69</code> | <code>self.assertEqual(self.text(&quot;t:Actions/t:Exec/t:WorkingDirectory&quot;), r&quot;C:\程序 &amp; Tools&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_test.cpp:226</code> | <code>(name + L&quot; \u81ea\u542f &amp; spaces&quot;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/desktop/tests/windows_autostart_test.cpp:228</code> | <code>const auto alternate = temporary / L&quot;CrossDesk \u6d4b\u8bd5.exe&quot;;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/desktop-port/README.md:46</code> | <code>&#124; Nhập tiếng Trung &#124; Chọn hết/thay ô bí danh kết nối; chọn “你好” (xin chào) từ gợi ý Pinyin hệ thống `nihao`, gửi vào ô Slint gốc &#124;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>apps/harmony/entry/src/main/ets/mobile/PrivacyPolicy.ets:13</code> | <code>const vietnamese = policySection(text, &#x27;Tiếng Việt&#x27;) &#124;&#124; policySection(text, &#x27;中文&#x27;);</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/harmony/entry/src/main/ets/pages/Mobile.ets:512</code> | <code>Text(paragraph.replace(/^#+\s*/, &#x27;&#x27;).replace(/\*\*/g, &#x27;&#x27;)).fontSize(paragraph.startsWith(&#x27;更新日期：&#x27;) ? 12 : 16)</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/harmony/entry/src/main/ets/pages/Mobile.ets:514</code> | <code>.fontColor(paragraph.startsWith(&#x27;更新日期：&#x27;) ? MobileTheme.secondary : MobileTheme.ink).width(&#x27;100%&#x27;).copyOption(CopyOptions.LocalDevice)</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/harmony/tests/localization_test.cjs:23</code> | <code>assert.equal(tr(&#x27;连接&#x27;), &#x27;Kết nối&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:25</code> | <code>const saved = translations[&#x27;连接&#x27;][10];</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:26</code> | <code>translations[&#x27;连接&#x27;][10] = &#x27;&#x27;;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:27</code> | <code>assert.equal(tr(&#x27;连接&#x27;), &#x27;&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:28</code> | <code>translations[&#x27;连接&#x27;][10] = saved;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:31</code> | <code>assert.equal(tr(&#x27;{0} 台设备&#x27;, [&#x27;{1}&#x27;]), &#x27;{1} thiết bị&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:32</code> | <code>assert.equal(tr(&#x27;{0} 台设备&#x27;), &#x27;{0} thiết bị&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:34</code> | <code>assert.equal(tr(&#x27;连接&#x27;), &#x27;Connect&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:36</code> | <code>assert.equal(tr(&#x27;连接&#x27;), &#x27;Kết nối&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/localization_test.cjs:39</code> | <code>assert.equal(tr(&#x27;连接&#x27;), &#x27;Kết nối&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:30</code> | <code>assert.equal(fileTransferStatus(false, &#x27;report.txt&#x27;, 1), `Nhận report.txt · ${tr(&#x27;已完成&#x27;)}`);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:32</code> | <code>const filename = &#x27;报告{0}.txt&#x27;;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:33</code> | <code>assert.equal(fileTransferStatus(false, filename, -1), tr(&#x27;{0} 传输失败&#x27;, [filename]));</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:48</code> | <code>[&#x27;远程会话信令连接失败&#x27;, &#x27;Kết nối báo hiệu phiên từ xa thất bại&#x27;],</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:49</code> | <code>[&#x27;无法发起连接&#x27;, &#x27;Không thể bắt đầu kết nối&#x27;],</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:50</code> | <code>[&#x27;服务器连接超时，请检查地址和网络&#x27;, &#x27;Kết nối máy chủ hết thời gian chờ, hãy kiểm tra địa chỉ và mạng&#x27;],</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:51</code> | <code>[&#x27;连接超时，请确认设备在线&#x27;, &#x27;Kết nối hết thời gian chờ, hãy xác nhận thiết bị đang trực tuyến&#x27;],</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:52</code> | <code>[&#x27;操作失败，请检查连接参数或重新连接&#x27;, &#x27;Thao tác thất bại, hãy kiểm tra tham số kết nối hoặc kết nối lại&#x27;]</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:61</code> | <code>feed({ type: &#x27;feedback&#x27;, text: &#x27;已发送 Ctrl+Alt+Del&#x27; });</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:62</code> | <code>assert.equal(shown.at(-1), tr(&#x27;已发送 Ctrl+Alt+Del&#x27;));</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:67</code> | <code>feed({ type: &#x27;clipboard&#x27;, text: &#x27;复制&#x27; });</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:68</code> | <code>assert.equal(controller.state.remoteClipboard, &#x27;复制&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_localization_test.cjs:83</code> | <code>assert.equal(shown.at(-1), tr(&#x27;操作失败&#x27;));</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_models_test.cjs:48</code> | <code>assert.equal(asciiKey(&#x27;a&#x27;), 65); assert.equal(asciiKey(&#x27;A&#x27;), 321); assert.equal(asciiKey(&#x27;!&#x27;), 305); assert.equal(asciiKey(&#x27;中&#x27;), 0);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_models_test.cjs:75</code> | <code>const spans = announcementSpans(&#x27;说明\r\n[下载](HTTP://example.com/a(b))；https://example.com/end. &lt;b&gt;普通文本&lt;/b&gt;&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_models_test.cjs:76</code> | <code>assert.equal(spans.map(p=&gt;p.text).join(&#x27;&#x27;),&#x27;说明\n下载；https://example.com/end. &lt;b&gt;普通文本&lt;/b&gt;&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_models_test.cjs:79</code> | <code>assert.equal(announcementByteLength(&#x27;中&#x27;.repeat(80)),240);assert.equal(announcementByteLength(&#x27;😀&#x27;),4);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/mobile_models_test.cjs:96</code> | <code>answer(requests.at(-1),[{...entries[0],title:&#x27;中&#x27;.repeat(81)}],1);assert(inbox.failed);assert.equal(inbox.items.length,0);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/privacy_policy_test.cjs:32</code> | <code>assert.equal(selectPrivacyPolicy(&#x27;## 中文\r\nLegacy\r\n## English\r\nEnglish&#x27;, 10), &#x27;Legacy&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/privacy_policy_test.cjs:33</code> | <code>assert.equal(selectPrivacyPolicy(&#x27;## 中文\nLegacy\n## English\nEnglish&#x27;, 0), &#x27;English&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/text_input_host_test.cjs:133</code> | <code>clipboardText = &#x27;外部应用复制 😀&#x27;; host.clipboardChanged();</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/text_input_host_test.cjs:271</code> | <code>await request({ op: &#x27;ime&#x27;, id: 1, kind: 1, text: &#x27;a😀中&#x27;, anchor: 1, cursor: 4 });</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/text_input_host_test.cjs:272</code> | <code>assert.deepEqual(selection, [&#x27;a😀中&#x27;, 1, 4]);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/text_input_host_test.cjs:285</code> | <code>handlers.get(&#x27;insertText&#x27;)(&#x27;你好&#x27;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/harmony/tests/text_input_host_test.cjs:286</code> | <code>assert.deepEqual(take(), [{ op: &#x27;composition&#x27;, id: 1, imeSession: 0, text: &#x27;你好&#x27;, commit: true }]);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/CrossDeskMobile/PrivacyViews.swift:189</code> | <code>let vietnamese = section(&quot;Tiếng Việt&quot;).isEmpty ? section(&quot;中文&quot;) : section(&quot;Tiếng Việt&quot;)</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/ios/CrossDeskMobile/PrivacyViews.swift:220</code> | <code>} else if paragraph.hasPrefix(&quot;更新日期：&quot;) &#124;&#124; paragraph.hasPrefix(&quot;Last updated:&quot;) {</code> | Nhận diện tiêu đề/ngày của chính sách cũ; giữ tương thích tài liệu đã phát hành. |
| <code>apps/ios/tests/localization_test.py:9</code> | <code>PERMISSION = &#x27;CrossDesk 使用局域网发现并建立原生 P2P 远程控制连接。&#x27;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/tests/network_statistics_test.py:52</code> | <code>expect(snapshot.report?.srtpActive == true &amp;&amp; snapshot.connectionMode == &quot;TURN 中继&quot;, &quot;Actual security and route&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/tests/network_statistics_test.py:56</code> | <code>expect(state.snapshot(at: 11).connectionMode == &quot;P2P 直连&quot;, &quot;Route updates&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/tests/privacy_policy_test.py:26</code> | <code>precondition(PrivacyPolicyText.select(&quot;## 中文\r\nLegacy\r\n## English\r\nEnglish&quot;) == &quot;Legacy&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/tests/privacy_policy_test.py:27</code> | <code>precondition(PrivacyPolicyText.select(&quot;## 中文\nLegacy\n## English\nEnglish&quot;, language: &quot;zh&quot;) == &quot;English&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/ios/tests/recent_connection_test.py:39</code> | <code>expect(connection.platform.displayName == &quot;未知&quot;, &quot;Unknown has a readable placeholder&quot;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>apps/mcp/README.md:243</code> | <code>&lt;a id=&quot;从源码运行&quot;&gt;&lt;/a&gt;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/CONNECTIONS.md:16</code> | <code>&lt;a id=&quot;41-id-模式&quot;&gt;&lt;/a&gt;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/CONNECTIONS.md:30</code> | <code>&lt;a id=&quot;42-ip-模式&quot;&gt;&lt;/a&gt;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/CONNECTIONS.md:50</code> | <code>&lt;a id=&quot;43-可选可信设备认证&quot;&gt;&lt;/a&gt;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/SELF_HOSTING.md:13</code> | <code>Theo [tài liệu triển khai máy chủ](https://github.com/kunkundi/crossdesk-server#运行服务), tải `compose.yaml` và `env.example` từ [Releases](https://github.com/kunkundi/crossdesk-server/releases) của dự án đó, đặt vào cùng thư mục. Thực hiện trong thư mục này:</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/SELF_HOSTING_EN.md:13</code> | <code>Follow the [server deployment guide](https://github.com/kunkundi/crossdesk-server#运行服务). Download `compose.yaml` and `env.example` from its [Releases](https://github.com/kunkundi/crossdesk-server/releases) into one directory and run there:</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:147</code> | <code>files-&gt;set_local_role(zh ? &quot;控制端&quot; : &quot;Controller&quot;);</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:148</code> | <code>files-&gt;set_remote_role(zh ? &quot;被控端&quot; : &quot;Host&quot;);</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:155</code> | <code>files-&gt;set_local_status(zh ? &quot;4 个项目&quot; : &quot;4 items&quot;);</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:156</code> | <code>files-&gt;set_remote_status(zh ? &quot;4 个项目&quot; : &quot;4 items&quot;);</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:194</code> | <code>task.detail = zh ? &quot;发送到远端&quot; : &quot;Send to remote&quot;;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:195</code> | <code>task.status = zh ? &quot;已完成&quot; : &quot;Completed&quot;;</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>docs/images/REGENERATE.md:198</code> | <code>files-&gt;set_task_summary(zh ? &quot;1 个任务 · 已完成 1&quot; : &quot;1 task · 1 completed&quot;);</code> | ID neo tương thích/neo URL ngoài hoặc dữ liệu hướng dẫn chụp/kiểm thử IME; không phải văn xuôi chưa dịch. |
| <code>localization/test_generate.py:27</code> | <code>key = generate.resource_id(&#x27;连接&#x27;)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:48</code> | <code>del data[&#x27;strings&#x27;][&#x27;连接&#x27;][&#x27;en&#x27;]</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:54</code> | <code>data[&#x27;strings&#x27;][&#x27;连接&#x27;][&#x27;vi&#x27;] = &#x27;&#x27;</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:56</code> | <code>self.assertIn(f&#x27;name=&quot;{generate.resource_id(&quot;连接&quot;)}&quot; formatted=&quot;false&quot;&gt;&quot;&quot;&lt;/string&gt;&#x27;, output)</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:60</code> | <code>&#x27;文件传输&#x27;: &#x27;Truyền tệp&#x27;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:61</code> | <code>&#x27;此目录为空&#x27;: &#x27;Thư mục này trống&#x27;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:62</code> | <code>&#x27;目录列表未能完整加载，请刷新重试。&#x27;:</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:64</code> | <code>&#x27;已传输 &#x27;: &#x27;Đã truyền &#x27;,</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:111</code> | <code>assert(Literal(&quot;连接&quot;, 11) == &quot;Kết nối&quot;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:112</code> | <code>assert(Literal(&quot;连接&quot;, detail::ClampLanguageIndex(11)) == &quot;Kết nối&quot;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |
| <code>localization/test_generate.py:113</code> | <code>assert(Literal(&quot;连接&quot;, 1) == &quot;Connect&quot;);</code> | Dữ liệu/khóa/giá trị kỳ vọng trong kiểm thử; không phải văn bản UI chưa dịch. |

## 11. Han giữ lại: (e), mọi vị trí dữ liệu locale

Liệt kê chính xác mọi số dòng bằng các dải liên tiếp, không chỉ đếm mẫu. Ví dụ 2–4 nghĩa là ba dòng 2, 3, 4 đều thuộc (e). Cách này tránh chép lại hàng nghìn giá trị zh/ja nhưng vẫn định vị đầy đủ. Với bảng lookup sinh hỗn hợp, loại chính (a) ở mục 9 đã bao gồm cùng dòng và giữ các cột locale, không đếm trùng vào (e).

| Tệp | Các dòng (e) | Số dòng | Lý do |
|---|---|---:|---|
| <code>apps/android/app/src/main/res/values-b+zh+Hans/strings.xml</code> | 4–372 | 369 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/android/app/src/main/res/values-b+zh+Hant/strings.xml</code> | 4–372 | 369 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/android/app/src/main/res/values-ja/strings.xml</code> | 4–9, 11, 13–14, 16–36, 38–48, 50–58, 60–69, 71, 74–82, 86–99, 101, 103–106, 108–119, 121–127, 129–144, 146–147, 149, 151, 153, 155–161, 164–167, 169–183, 185–187, 190, 192–196, 199–200, 202–203, 206–211, 213, 217–219, 221, 224–229, 231–240, 242–246, 249, 251–271, 273–276, 279, 281–289, 291, 295–296, 298–300, 302–308, 310–319, 322–331, 334–337, 339, 341–351, 353–356, 358–367, 369–372 | 303 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/desktop/src/gui/assets/localization/localization.h</code> | 40–41 | 2 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/desktop/src/gui/ui/device_strings.slint</code> | 15–17 | 3 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/ios/CrossDeskMobile/Resources/ja.lproj/InfoPlist.strings</code> | 1 | 1 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hans.lproj/InfoPlist.strings</code> | 1 | 1 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hant.lproj/InfoPlist.strings</code> | 1 | 1 | Dữ liệu locale hoặc tên ngôn ngữ hiển thị, giữ nguyên tiếng gốc. |
| <code>localization/catalog.json</code> | 30, 32–33, 43, 45–46, 56, 59, 69, 72–73, 82, 84–85, 95, 97–98, 108, 110–111, 121, 123–124, 134, 136–137, 147, 149–150, 160, 162, 173, 175–176, 186, 189–190, 212, 214–215, 225, 228–229, 238, 241–242, 251, 254, 264, 266–267, 277, 279–280, 290, 292, 316, 319, 329, 332, 342, 345, 355, 358–359, 368, 371–372, 381, 384–385, 394, 397–398, 407, 410–411, 420, 423–424, 433, 436–437, 446, 449–450, 459, 461–462, 472, 475, 485, 487–488, 498, 500–501, 511, 513–514, 524, 526–527, 537, 539, 550, 553–554, 563, 565–566, 576, 578–579, 589, 592–593, 602, 604–605, 615, 618–619, 628, 630–631, 641, 643–644, 654, 657–658, 667, 670–671, 680, 683–684, 693, 696–697, 706, 709–710, 719, 722–723, 732, 734–735, 745, 748, 758, 761, 771, 774–775, 784, 787–788, 797, 800–801, 810, 813–814, 823, 826–827, 836, 839–840, 849, 852–853, 862, 864–865, 875, 878–879, 888, 890–891, 901, 904–905, 914, 917–918, 927, 930–931, 940, 942–943, 953, 955–956, 966, 969–970, 979, 982–983, 992, 994–995, 1005, 1007–1008, 1018, 1021–1022, 1031, 1033–1034, 1044, 1047–1048, 1057, 1059–1060, 1070, 1072–1073, 1083, 1085–1086, 1096, 1099–1100, 1109, 1112–1113, 1122, 1125–1126, 1135, 1138–1139, 1148, 1150, 1161, 1164–1165, 1174, 1176–1177, 1187, 1190–1191, 1200, 1203–1204, 1213, 1216–1217, 1226, 1229, 1239, 1242–1243, 1252, 1255–1256, 1265, 1268–1269, 1278, 1281, 1291, 1293–1294, 1304, 1306–1307, 1317, 1319–1320, 1330, 1332–1333, 1343, 1346–1347, 1356, 1359, 1369, 1371–1372, 1382, 1385–1386, 1395, 1398–1399, 1408, 1411–1412, 1421, 1423–1424, 1434, 1437–1438, 1447, 1450–1451, 1460, 1463–1464, 1473, 1475–1476, 1486, 1489–1490, 1499, 1502, 1512, 1515–1516, 1525, 1528–1529, 1538, 1541–1542, 1551, 1554, 1564, 1567–1568, 1577, 1580–1581, 1590, 1592–1593, 1603, 1605–1606, 1616, 1619–1620, 1629, 1632–1633, 1642, 1645–1646, 1655, 1658–1659, 1668, 1670–1671, 1681, 1684–1685, 1694, 1696–1697, 1707, 1710–1711, 1720, 1723–1724, 1733, 1735–1736, 1746, 1749, 1759, 1762–1763, 1772, 1775–1776, 1785, 1787, 1798, 1800, 1811, 1813–1814, 1824, 1827–1828, 1837, 1840–1841, 1850, 1853–1854, 1863, 1865–1866, 1876, 1878–1879, 1889, 1891–1892, 1902, 1905–1906, 1915, 1917–1918, 1928, 1931–1932, 1941, 1943–1944, 1954, 1956–1957, 1967, 1970–1971, 1980, 1983–1984, 1993, 1996–1997, 2006, 2008–2009, 2019, 2022–2023, 2032, 2035–2036, 2045, 2047–2048, 2058, 2061–2062, 2071, 2073–2074, 2084, 2086–2087, 2097, 2100–2101, 2110, 2113–2114, 2123, 2125–2126, 2136, 2138–2139, 2149, 2152–2153, 2162, 2165, 2175, 2178–2179, 2188, 2190–2191, 2201, 2204–2205, 2214, 2217–2218, 2227, 2230–2231, 2240, 2243, 2253, 2255–2256, 2266, 2268–2269, 2279, 2281–2282, 2292, 2294–2295, 2305, 2307–2308, 2318, 2320–2321, 2331, 2333–2334, 2344, 2346–2347, 2357, 2359–2360, 2370, 2372–2373, 2383, 2385–2386, 2396, 2398–2399, 2409, 2412–2413, 2422, 2425–2426, 2435, 2438–2439, 2448, 2451–2452, 2461, 2464–2465, 2474, 2476–2477, 2487, 2490, 2500, 2502–2503, 2513, 2515–2516, 2526, 2528–2529, 2539, 2541–2542, 2552, 2554, 2565, 2567, 2578, 2581, 2591, 2594–2595, 2604, 2607–2608, 2617, 2620, 2630, 2632, 2643, 2646–2647, 2656, 2659–2660, 2669, 2671–2672, 2682, 2684–2685, 2695, 2698–2699, 2708, 2711–2712, 2721, 2723–2724, 2734, 2736–2737, 2747, 2750, 2760, 2763, 2773, 2776, 2786, 2789–2790, 2799, 2801–2802, 2812, 2814–2815, 2825, 2828, 2838, 2841–2842, 2851, 2853–2854, 2864, 2867–2868, 2877, 2880–2881, 2890, 2892–2893, 2903, 2905–2906, 2916, 2918–2919, 2929, 2932–2933, 2942, 2945–2946, 2955, 2958–2959, 2968, 2971, 2981, 2983, 2994, 2996, 3007, 3009, 3020, 3023–3024, 3033, 3036–3037, 3046, 3049–3050, 3059, 3062–3063, 3072, 3075–3076, 3085, 3088–3089, 3098, 3100–3101, 3111, 3113–3114, 3124, 3126–3127, 3137, 3139–3140, 3150, 3152–3153, 3163, 3165–3166, 3176, 3178–3179, 3189, 3192–3193, 3202, 3205–3206, 3215, 3218–3219, 3228, 3231, 3241, 3244–3245, 3254, 3256–3257, 3267, 3270–3271, 3280, 3282–3283, 3293, 3296–3297, 3306, 3308–3309, 3319, 3322–3323, 3332, 3334–3335, 3345, 3348–3349, 3358, 3360–3361, 3371, 3374–3375, 3384, 3387–3388, 3397, 3400–3401, 3410, 3413–3414, 3423, 3426–3427, 3436, 3439–3440, 3449, 3451, 3462, 3464–3465, 3475, 3478–3479, 3488, 3490–3491, 3501, 3504, 3514, 3517–3518, 3527, 3529–3530, 3540, 3542–3543, 3553, 3556, 3566, 3568, 3579, 3582–3583, 3592, 3595–3596, 3605, 3608–3609, 3618, 3621–3622, 3631, 3633–3634, 3644, 3646–3647, 3657, 3660–3661, 3670, 3673–3674, 3683, 3686–3687, 3696, 3698, 3709, 3712–3713, 3722, 3724–3725, 3735, 3738–3739, 3748, 3750–3751, 3761, 3764–3765, 3774, 3777–3778, 3787, 3790–3791, 3800, 3803, 3813, 3816–3817, 3826, 3829–3830, 3839, 3842, 3852, 3854–3855, 3865, 3868–3869, 3878, 3881–3882, 3891, 3894–3895, 3904, 3906–3907, 3917, 3919, 3930, 3933–3934, 3943, 3946–3947, 3956, 3959–3960, 3969, 3972, 3982, 3985, 3995, 3997–3998, 4008, 4011–4012, 4021, 4024–4025, 4034, 4036–4037, 4047, 4050–4051, 4060, 4063–4064, 4073, 4076–4077, 4086, 4088–4089, 4099, 4102–4103, 4112, 4115–4116, 4125, 4128–4129, 4138, 4140–4141, 4151, 4153–4154, 4164, 4166–4167, 4177, 4179–4180, 4190, 4193–4194, 4203, 4206–4207, 4216, 4218–4219, 4229, 4232–4233, 4242, 4244, 4255, 4257–4258, 4268, 4271–4272, 4281, 4284–4285, 4294, 4296, 4307, 4309–4310, 4320, 4322–4323, 4333, 4336–4337, 4346, 4348–4349, 4359, 4362–4363, 4372, 4374–4375, 4385, 4387–4388, 4398, 4401–4402, 4411, 4413–4414, 4424, 4427, 4437, 4439–4440, 4450, 4453, 4463, 4466, 4476, 4479, 4489, 4492, 4502, 4505–4506, 4515, 4517–4518, 4528, 4530–4531, 4541, 4543–4544, 4554, 4556–4557, 4567, 4569–4570, 4580, 4583–4584, 4593, 4595–4596, 4606, 4609–4610, 4619, 4622–4623, 4632, 4635–4636, 4645, 4647–4648, 4658, 4660–4661, 4671, 4673–4674, 4684, 4687–4688, 4697, 4700, 4710, 4713–4714, 4723, 4726–4727, 4736, 4738–4739, 4749, 4752–4753, 4762, 4765–4766, 4775, 4777–4778, 4788, 4791–4792, 4801, 4804–4805, 4814, 4816–4817, 4827, 4829–4830, 4840, 4843–4844, 4853, 4855–4856, 4866, 4868–4869, 4879, 4882–4883, 4892, 4895–4896, 4905, 4907–4908, 4918, 4921–4922, 4931, 4934–4935, 4944, 4947–4948, 4957, 4959–4960, 4970, 4973–4974, 4983, 4986–4987, 4996, 4998–4999, 5009, 5011–5012, 5022, 5024–5025, 5035, 5038–5039, 5048, 5050–5051, 5061, 5064, 5074, 5076, 5087, 5090–5091, 5100, 5103–5104, 5113, 5116–5117, 5126, 5129–5130, 5139, 5142–5143, 5152, 5154–5155, 5165, 5168–5169, 5178, 5181–5182, 5191, 5194–5195, 5204, 5206–5207, 5217, 5220–5221, 5230, 5232–5233, 5243, 5246–5247, 5256, 5258–5259, 5269, 5272–5273, 5282, 5285–5286, 5295, 5297–5298, 5308, 5310–5311, 5321, 5323–5324, 5334, 5336–5337, 5347, 5349–5350, 5360, 5363–5364, 5373, 5376–5377, 5386, 5389–5390, 5399, 5402, 5412, 5414–5415, 5425, 5428–5429, 5438, 5441, 5451, 5453, 5464, 5466, 5477, 5480–5481, 5490, 5493–5494, 5503, 5506–5507, 5516, 5519–5520, 5529, 5532, 5542, 5545–5546, 5555, 5558–5559, 5568, 5571–5572, 5581, 5584–5585, 5594, 5596–5597, 5607, 5610–5611, 5620, 5623–5624, 5633, 5636, 5646, 5649, 5659, 5662, 5672, 5675–5676, 5685, 5688–5689, 5698, 5701–5702, 5711, 5714–5715, 5724, 5726–5727, 5737, 5740–5741, 5750, 5752, 5763, 5765–5766, 5776, 5779, 5789, 5791–5792, 5802, 5805–5806, 5815, 5818–5819, 5828, 5830–5831, 5841, 5843, 5854, 5856–5857, 5867, 5869–5870, 5880, 5882–5883, 5893, 5896–5897, 5906, 5909–5910, 5919, 5922–5923, 5932, 5935–5936, 5945, 5947, 5958, 5960–5961, 5971, 5974–5975, 5984, 5986–5987, 5997, 6000–6001, 6010, 6012–6013, 6023, 6026–6027, 6036, 6038–6039, 6049, 6052–6053, 6062, 6065–6066, 6075, 6077–6078, 6088, 6091–6092, 6101, 6104–6105, 6114, 6117–6118, 6127, 6129–6130, 6140, 6143, 6153, 6156–6157, 6166, 6169, 6179, 6181, 6192, 6194, 6205, 6207, 6218, 6221, 6231, 6233–6234, 6244, 6247, 6257, 6259–6260, 6270, 6273–6274, 6283, 6286–6287, 6296, 6299–6300, 6309, 6311–6312, 6322, 6325, 6335, 6337, 6348, 6351–6352, 6361, 6363, 6374, 6377–6378, 6387, 6389–6390, 6400, 6403, 6413, 6416, 6426, 6429–6430, 6439, 6442–6443, 6452, 6455–6456, 6465, 6468–6469, 6478, 6480–6481, 6491, 6494–6495, 6504, 6506–6507, 6517, 6520–6521, 6530, 6532–6533, 6543, 6545–6546, 6556, 6558–6559, 6569, 6572, 6582, 6585–6586, 6595, 6598–6599, 6608, 6610–6611, 6621, 6624–6625, 6634, 6636–6637, 6647, 6649–6650, 6660, 6663–6664, 6673, 6676–6677, 6686, 6688–6689, 6699, 6702–6703, 6712, 6714–6715, 6725, 6728–6729, 6738, 6740–6741, 6751, 6753–6754, 6764, 6766–6767, 6777, 6780–6781, 6790, 6792–6793, 6803, 6805–6806, 6816, 6819–6820, 6829, 6831–6832, 6842, 6844–6845, 6855, 6857–6858, 6868, 6871–6872, 6881, 6884–6885, 6894, 6896–6897, 6907, 6909–6910, 6920, 6922–6923, 6933, 6936–6937, 6946, 6948–6949, 6959, 6962–6963, 6972, 6975–6976, 6985, 6988, 6998, 7001–7002, 7011, 7014–7015, 7024, 7027–7028, 7037, 7040–7041, 7050, 7052, 7063, 7066–7067, 7076, 7079–7080, 7089, 7092–7093, 7102, 7104, 7115, 7118–7119, 7128, 7131–7132, 7141, 7144–7145, 7154, 7157–7158, 7167, 7170, 7180, 7183, 7193, 7195–7196, 7206, 7209, 7219, 7221, 7232, 7234, 7245, 7247, 7258, 7260–7261, 7271, 7274–7275, 7284, 7287, 7297, 7300–7301, 7310, 7313–7314, 7323, 7326–7327, 7336, 7339–7340, 7349, 7352–7353, 7362, 7365–7366, 7375, 7378–7379, 7388, 7391, 7401, 7404, 7414, 7417–7418, 7427, 7430–7431, 7440, 7443–7444, 7453, 7456, 7466, 7468–7469, 7479, 7482–7483, 7492, 7494, 7505, 7508–7509, 7518, 7521–7522, 7531, 7534–7535, 7544, 7547–7548, 7557, 7560–7561, 7570, 7572–7573, 7583, 7586, 7596, 7599, 7609, 7611–7612, 7622, 7624–7625, 7635, 7638–7639, 7648, 7651–7652, 7661, 7663–7664, 7674, 7676–7677, 7687, 7690–7691, 7700, 7703–7704, 7713, 7715–7716, 7726, 7728–7729, 7739, 7741–7742, 7752, 7754–7755, 7765, 7767–7768, 7778, 7780–7781, 7791, 7794, 7804, 7806–7807, 7817, 7819–7820, 7830, 7832, 7843, 7846–7847, 7856, 7858, 7869, 7872–7873, 7882, 7885, 7895, 7898, 7908, 7910–7911, 7921, 7924–7925, 7934, 7937–7938, 7947, 7950, 7960, 7962, 7973, 7976, 7986, 7989–7990, 7999, 8002, 8012, 8014–8015, 8025, 8028–8029, 8038, 8040–8041, 8051, 8054–8055, 8064, 8066–8067, 8077, 8079–8080, 8090, 8093, 8103, 8106, 8116, 8119–8120, 8129, 8131–8132, 8142, 8145, 8155, 8157, 8168, 8171–8172, 8181, 8184–8185, 8194, 8197, 8207, 8210, 8220, 8222–8223, 8233, 8236–8237, 8246, 8249–8250, 8259, 8261–8262, 8272, 8274, 8285, 8288–8289, 8298, 8300–8301, 8311, 8314, 8324, 8327, 8337, 8340, 8350, 8353, 8363, 8366–8367, 8376, 8379–8380, 8389, 8392, 8402, 8405–8406, 8415, 8417–8418, 8428, 8430–8431, 8441, 8443–8444, 8454, 8457, 8467, 8470–8471, 8480, 8483–8484, 8493, 8496–8497, 8506, 8509–8510, 8519, 8522–8523, 8532, 8535–8536, 8545, 8547–8548, 8558, 8561, 8571, 8573, 8584, 8587–8588, 8597, 8599–8600, 8610, 8612–8613, 8623, 8625–8626, 8636, 8638–8639, 8649, 8652–8653, 8662, 8665–8666, 8675, 8678–8679, 8688, 8691–8692, 8701, 8704–8705, 8714, 8717–8718, 8727, 8729–8730, 8740, 8743–8744, 8753, 8756–8757, 8766, 8769–8770, 8779, 8782–8783, 8792, 8794–8795, 8805, 8808–8809, 8818, 8820–8821, 8831, 8834–8835, 8844, 8846–8847, 8857, 8860–8861, 8870, 8873–8874, 8883, 8886–8887, 8896, 8898–8899, 8909, 8911–8912, 8922, 8925–8926, 8935, 8938–8939, 8948, 8951–8952, 8961, 8964–8965, 8974, 8977–8978, 8987, 8990–8991, 9000, 9003–9004, 9013, 9015–9016, 9026, 9028–9029, 9039, 9041–9042, 9052, 9054–9055, 9065, 9067–9068, 9078, 9081–9082, 9091, 9094–9095, 9104, 9107–9108, 9117, 9120–9121, 9130, 9133–9134, 9143, 9146–9147, 9156, 9159–9160, 9169, 9172–9173, 9182, 9185–9186, 9195, 9198–9199, 9208, 9211–9212, 9221, 9224–9225, 9234, 9237, 9247, 9250–9251, 9260, 9263, 9273, 9276–9277, 9286, 9289–9290, 9299, 9302–9303, 9312, 9315–9316, 9325, 9328–9329, 9338, 9341, 9351, 9354–9355, 9364, 9367–9368, 9377, 9379–9380, 9390, 9393–9394, 9403, 9405–9406, 9416, 9418–9419, 9429, 9431–9432, 9442, 9444–9445, 9455, 9458, 9468, 9470, 9481, 9483–9484, 9494, 9496–9497, 9507, 9509, 9520, 9522–9523, 9533, 9535–9536, 9546, 9548–9549, 9559, 9561–9562, 9572, 9574–9575, 9585, 9588, 9598, 9601–9602, 9611, 9614–9615, 9624, 9627–9628, 9637, 9639–9640, 9650, 9652–9653, 9663, 9666, 9676, 9678–9679, 9689, 9691–9692, 9702, 9705–9706, 9715, 9717–9718, 9728, 9731–9732, 9741, 9744–9745, 9754, 9756–9757, 9767, 9769–9770, 9780, 9783–9784, 9793, 9795–9796, 9806, 9808–9809, 9819, 9822–9823, 9832, 9834–9835, 9845, 9848–9849, 9858, 9860–9861, 9871, 9873–9874, 9884, 9887–9888, 9897, 9899–9900, 9910, 9913–9914, 9923, 9925–9926, 9936, 9938–9939, 9949, 9952–9953, 9962, 9965–9966, 9975, 9977–9978, 9988, 9991–9992, 10001, 10003–10004, 10014, 10017–10018, 10027, 10030–10031, 10040, 10043–10044, 10053, 10055, 10066, 10069–10070, 10079, 10082–10083, 10092, 10095–10096, 10105, 10108–10109, 10118, 10121–10122, 10131, 10134–10135, 10144, 10147–10148, 10157, 10160–10161, 10170, 10173–10174, 10183, 10185–10186, 10196, 10199–10200, 10209, 10212–10213, 10222, 10225, 10235, 10238–10239, 10248, 10251–10252, 10261, 10264–10265, 10274, 10277–10278, 10287, 10290–10291, 10300, 10302–10303, 10313, 10315–10316, 10326, 10329, 10339, 10342, 10352, 10355–10356, 10365, 10367, 10378, 10380–10381, 10391, 10393–10394, 10404, 10407–10408, 10417, 10419, 10430, 10432–10433, 10443, 10446, 10456, 10459–10460, 10469, 10472–10473, 10482, 10485–10486, 10495, 10498–10499, 10508, 10511–10512, 10521, 10523–10524, 10534, 10536–10537, 10547, 10549–10550, 10560, 10562–10563, 10573, 10576–10577, 10586, 10589–10590, 10599, 10602–10603, 10612, 10614, 10625, 10627–10628, 10638, 10640–10641, 10651, 10654–10655, 10664, 10667–10668, 10677, 10680–10681, 10690, 10693, 10703, 10706, 10716, 10719, 10729, 10731–10732, 10742, 10744–10745, 10755, 10758–10759, 10768, 10771–10772, 10781, 10784–10785, 10794, 10796, 10807, 10809–10810, 10820, 10822–10823, 10833, 10835–10836, 10846, 10848–10849, 10859, 10861–10862, 10872, 10874–10875, 10885, 10887–10888, 10898, 10900–10901, 10911, 10913, 10924, 10926, 10937, 10939–10940, 10950, 10953–10954, 10963, 10965–10966, 10976, 10978–10979, 10989, 10991–10992, 11002, 11004–11005, 11015, 11017–11018, 11028, 11031–11032, 11041, 11044–11045, 11054, 11056–11057, 11067, 11069–11070, 11080, 11082–11083, 11093, 11095–11096, 11106, 11109–11110, 11119, 11122–11123, 11132, 11135–11136, 11145, 11148–11149, 11158, 11161–11162, 11171, 11174–11175, 11184, 11187–11188, 11197, 11200–11201, 11210, 11213–11214, 11223, 11226–11227, 11236, 11239–11240, 11249, 11252–11253, 11262, 11265–11266, 11275, 11278–11279, 11288, 11291–11292, 11301, 11304–11305, 11314, 11317–11318, 11327, 11330–11331, 11340, 11343–11344, 11353, 11356–11357, 11366, 11369–11370, 11379, 11382–11383, 11392, 11395–11396, 11405, 11408–11409, 11418, 11421–11422, 11431, 11434–11435, 11444, 11447–11448, 11457, 11460–11461, 11470, 11473–11474, 11483, 11486–11487, 11496, 11499–11500, 11509, 11512–11513, 11522, 11525–11526, 11535, 11538–11539, 11548, 11551–11552, 11561, 11564–11565, 11574, 11577–11578, 11587, 11590–11591, 11600, 11603–11604, 11613, 11616–11617, 11626, 11629–11630, 11639, 11642–11643, 11652, 11655–11656, 11665, 11668–11669, 11678, 11681–11682, 11691, 11694–11695, 11704, 11707–11708, 11717, 11720–11721, 11730, 11733–11734, 11743, 11746, 11756, 11759–11760, 11769, 11772–11773, 11782, 11785–11786, 11795, 11798, 11808, 11811–11812, 11821, 11824–11825, 11834, 11837–11838, 11847, 11850–11851, 11860, 11863–11864, 11873, 11876–11877, 11886, 11889–11890, 11899, 11902–11903, 11912, 11915–11916, 11925, 11928–11929, 11938, 11941–11942, 11951, 11954–11955, 11964, 11967–11968, 11977, 11980–11981, 11990, 11992–11993, 12003, 12005–12006, 12016, 12018, 12029, 12031, 12042, 12044–12045, 12055, 12057–12058, 12068, 12070–12071, 12081, 12083–12084, 12094, 12096–12097, 12107, 12109–12110, 12120, 12122–12123, 12133, 12135–12136, 12146, 12148, 12159, 12161–12162, 12172, 12174–12175, 12185, 12187–12188, 12198, 12200–12201, 12211, 12213, 12224, 12226–12227, 12237, 12239, 12250, 12252–12253, 12263, 12265–12266, 12276, 12278–12279, 12289, 12291–12292, 12302, 12304–12305, 12315, 12317–12318, 12328, 12330–12331, 12341, 12343–12344, 12354, 12356–12357, 12367, 12369–12370, 12380, 12382, 12393, 12395–12396, 12406, 12408–12409, 12419, 12421–12422, 12432, 12434, 12445, 12447–12448, 12458, 12460–12461, 12471, 12473–12474, 12484, 12486–12487, 12497, 12499–12500, 12510, 12512–12513, 12523, 12525–12526, 12536, 12538–12539, 12549, 12551–12552, 12562, 12564, 12575, 12577–12578, 12588, 12590–12591, 12601, 12603–12604, 12614, 12616–12617, 12627, 12630–12631, 12640, 12643–12644, 12653, 12656–12657, 12666, 12669–12670, 12679, 12682, 12692, 12695–12696, 12705, 12708–12709, 12718, 12721–12722, 12731, 12734, 12744, 12747, 12757, 12760, 12770, 12773, 12783, 12786–12787, 12796, 12799–12800, 12809, 12812–12813, 12822, 12825, 12835, 12838–12839, 12848, 12851–12852, 12861, 12864, 12874, 12877–12878, 12887, 12890, 12900, 12903, 12913, 12916–12917, 12926, 12929–12930, 12939, 12942, 12952, 12955–12956, 12965, 12968–12969, 12978, 12981, 12991, 12994–12995, 13004, 13007, 13017, 13020, 13030, 13033, 13043, 13046–13047, 13056, 13059–13060, 13069, 13072–13073, 13082, 13085, 13095, 13098, 13108, 13111, 13121, 13124, 13134, 13137, 13147, 13150, 13160, 13163, 13173, 13176–13177, 13186, 13189–13190, 13199, 13202, 13212, 13215–13216, 13225, 13228, 13238, 13241, 13251, 13254, 13264, 13267–13268, 13277, 13280, 13290, 13293, 13303, 13306, 13316, 13319, 13329, 13332, 13342, 13345, 13355, 13358, 13368, 13371, 13381, 13384, 13394, 13397, 13407, 13410, 13420, 13423, 13433, 13436, 13446, 13449, 13459, 13462, 13472, 13475, 13485, 13488, 13498, 13501, 13511, 13514, 13524, 13527, 13537, 13540–13541, 13550, 13553, 13563, 13566, 13576, 13579, 13589, 13592, 13602, 13605, 13615, 13618, 13628, 13631, 13641, 13644, 13654, 13657, 13667, 13670, 13680, 13683–13684, 13693, 13696–13697, 13706, 13709–13710, 13719, 13722–13723, 13732, 13735–13736 | 2950 | Giá trị locale zh/ja được giữ nguyên. |

## 12. Danh mục từng tệp Git và trạng thái

Bảng này bao gồm toàn bộ cây tracked, kể cả hidden, không chỉ tệp sửa. Nhãn “giữ nguyên” không đồng nghĩa đã dịch tiếng Anh có sẵn. Đối với binary/gitlink, chỉ rà tên/tham chiếu; không khẳng định đã kiểm văn bản trong ảnh hay mã kho con.

| Tệp | Trạng thái | Quan hệ với ff369cd / lý do |
|---|---|---|
| <code>.dockerignore</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.gitattributes</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/FUNDING.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/ISSUE_TEMPLATE/01-bug-report.yml</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>.github/ISSUE_TEMPLATE/02-feature-request.yml</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>.github/ISSUE_TEMPLATE/config.yml</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>.github/actions/ensure-xmake/action.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/actions/ensure-xmake/check-version.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/scripts/cli-dependency-cache.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/scripts/setup_macos_signing.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/scripts/test_update_version_manifest.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>.github/scripts/test_xmake_version_check.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/scripts/update_version_manifest.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build-android.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build-cli.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build-ios.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build-linux-image.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build-mcp.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/build.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/check-versions.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/close-issue.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/release-cli.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/release-mcp.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/release-mobile.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/sign-android.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.github/workflows/update-version-json.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.gitignore</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>.gitmodules</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>LICENSE</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>PRIVACY.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>README_EN.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>VIETNAMESE_LOCALIZATION_REPORT.md</code> | đã dịch | Báo cáo tiếng Việt mới, tính 1 trong diff. |
| <code>apps/android/.gitattributes</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/.gitignore</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/app/build.gradle</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/AndroidManifest.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AboutPages.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AnnouncementInbox.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AnnouncementText.java</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AnnouncementView.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AppUpdateChecker.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/AudioPlayer.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/BackgroundSession.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/CrossDeskApplication.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/HomeLayout.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/KeyMap.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/L10n.java</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/LicenseCatalog.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/LocalizationKeys.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/MainActivity.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/MobileUi.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/NativeSession.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PageHost.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyConsentDialog.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/PrivacyPolicyDocument.java</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RecentConnectionPresence.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RecentConnections.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteDisplayResolutions.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteKeyboard.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteKeyboardInput.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteMouseInput.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteNetworkStatistics.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteOrientation.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemotePictureInPicture.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemotePreviewStore.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemotePrivacyScreen.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteResolutionStore.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteSessionService.java</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteShortcutPanel.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteShortcuts.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVersionCheck.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVideoSettings.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteVideoView.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/RemoteViewport.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/SecretStore.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/SegmentedControl.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/SessionControls.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/SessionDocuments.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/java/cn/crossdesk/mobile/VirtualMouseBar.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/res/drawable-nodpi/mobile_brand.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/android/app/src/main/res/drawable/ic_check_circle.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/drawable/ic_remote_session.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/mipmap-anydpi-v26/ic_launcher.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-b+zh+Hans/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/res/values-b+zh+Hant/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/res/values-de/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-en/strings.xml</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/app/src/main/res/values-es/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-fr/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-ja/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/app/src/main/res/values-ko/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-pt/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-ru/strings.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/values-vi/strings.xml</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/app/src/main/res/values/strings.xml</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/app/src/main/res/values/styles.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/xml/data_extraction_rules.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/app/src/main/res/xml/locales_config.xml</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/app/src/main/res/xml/network_security_config.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/build.gradle</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/gradle.properties</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/gradle/wrapper/gradle-wrapper.jar</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/android/gradle/wrapper/gradle-wrapper.properties</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/gradlew</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/gradlew.bat</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/licenses/OPEN_SOURCE_NOTICE.txt</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/licenses/additional-notices.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/controller_protocol.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/file_transfer_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/latest_frame_worker.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/rtc_bridge.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/video_picture.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/video_timing.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/native/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/scripts/build_native.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/scripts/generate_notices.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/scripts/release_version.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/settings.gradle</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/assets/tls/root.pem</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/assets/tls/valid.p12</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/android/test/android/assets/tls/wrong-host.p12</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/AnnouncementInboxTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/AppUpdateCheckerTest.java</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/ControllerTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/NetworkStatisticsTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/PrivacyConsentTest.java</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/RecentConnectionPresenceTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/RemotePreviewStoreTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/RemoteVersionCheckTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/RemoteViewportTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/TlsTrustTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/android/java/cn/crossdesk/mobile/VideoSettingsTest.java</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/build_native_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/generate_notices_test.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/test/localization_test.py</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/android/test/native/controller_protocol_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/native/video_pipeline_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/privacy_policy_test.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/android/test/run_native.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/android/test/verify_apk.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/agent.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/agent.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/automation_protocol.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/automation_protocol.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/docs/automation-protocol.md</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/docs/bridge-protocol.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/docs/npm-distribution.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/docs/source-provenance.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/docs/terminal-client.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/docs/validation.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/cli/main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/npm/cli.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/npm/platforms.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/package.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/plugins/pi/index.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/plugins/pi/package.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/scripts/pack-npm.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/scripts/prepare-deps.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/scripts/release-npm.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/terminal_io.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/terminal_io_posix.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/terminal_io_windows.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/video_server.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/video_server.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/cli/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/linux/crossdesk_128x128.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_16x16.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_24x24.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_256x256.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_32x32.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_48x48.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_512x512.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_64x64.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/linux/crossdesk_96x96.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/macos/crossdesk.icns</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/crossdesk.ico</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/crossdesk.manifest</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_debug.manifest</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_portable.manifest</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_portable.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_privacy_window.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_service.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_session_helper.manifest</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_session_helper.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_updater.manifest</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/crossdesk_updater.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/License.txt</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/Win32/usbmmIdd.dll</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/deviceinstaller.exe</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/deviceinstaller64.exe</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/idd_instructions.txt</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/usbmmIdd.inf</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/usbmmidd.bat</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/usbmmidd.cat</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/usbmmidd_v2/x64/usbmmIdd.dll</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/resources/windows/version_info.rcinc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/resources/windows/wgc_plugin.rc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/linux/package_deb.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/linux/pkg_amd64.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/linux/pkg_arm64.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/bundle_app.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/entitlements.plist</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/notarize_pkg.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/pkg_arm64.sh</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/scripts/macos/pkg_x64.sh</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/scripts/macos/sign_app.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/tcc_migration_preinstall.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/macos/verify_app.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/windows/nsProcess.dll</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/scripts/windows/nsis_script.nsi</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/windows/pkg_x64.ps1</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/scripts/windows/signpath-installer.xml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/app/main.cpp</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/autostart/autostart.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/autostart/autostart.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/credential_cache.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/desktop_automation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/desktop_automation.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/display_info.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/filesystem_utf8.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/nv12_scaler.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/nv12_scaler.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/platform.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/private_file.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/private_file.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/screen_lock.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/shared_cursor_state.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/shared_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/task_queue_lock_free.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/common/task_queue_lock_free.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/config_center/SimpleIni.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/config_center/config_center.cpp</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/config_center/config_center.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/config_center/signal_server_settings.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/device_controller/device_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/device_controller/device_controller_factory.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/device_controller/keyboard_converter.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/application_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/gui_application.cpp</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/application/gui_application.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/server_window_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/signal_connection_indicator.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/window_geometry.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/application/window_presentation.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/assets/fonts/fa_solid_900.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/assets/fonts/generate_message_spacing_font.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/assets/fonts/message_spacing.ttf</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/src/gui/assets/localization/localization.h</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/gui/assets/localization/localization_data.h</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/gui/assets/localization/system_language.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/features/about/about_catalog.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/about/about_catalog.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/clipboard/clipboard_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/clipboard/clipboard_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/clipboard/clipboard_file_transfer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/clipboard/clipboard_file_transfer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/clipboard/clipboard_files_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/devices/screen_sharing_power.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/devices/screen_sharing_power.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/devices/session_device_manager.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/devices/session_device_manager.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/feedback/diagnostic_export.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/feedback/diagnostic_export.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/file_transfer/file_browser_window.cpp</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/features/file_transfer/file_browser_window.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/features/file_transfer/file_transfer_manager.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/file_transfer/file_transfer_manager.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/file_transfer/transfer_notification_layout.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/file_transfer/transfer_notification_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_fullscreen_transition.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_storage.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_storage.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_stream_settings.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_window_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/gallery_workspace.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/preview_bandwidth.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/gallery/preview_resolution.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/captured_keyboard_queue.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/input_outcome.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/keyboard_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/keyboard_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/pending_key_releases.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/remote_input_dispatcher.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/remote_input_dispatcher.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/remote_mouse_buttons.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/scroll_accumulator.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/input/stream_keyboard_focus.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/settings/remote_resolution_store.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/settings/remote_resolution_store.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/settings/settings_manager.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/features/settings/settings_manager.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/render.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/render.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/rendering/slint_video_presenter.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/rendering/slint_video_presenter.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/rendering/video_frame_diagnostics.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/rendering/video_geometry.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/rendering/video_latency.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/announcement_inbox.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/announcement_inbox.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/automation_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/connection_auth_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/connection_latency.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/connection_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/cursor_position.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/cursor_presentation.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/cursor_state_provider.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/cursor_state_provider.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_identity_store.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_identity_store.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_login.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_login.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_presence_cache.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_presence_request.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/device_trust_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/disconnect_lock_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/display_resolution_restoration.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/display_resolution_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/display_resolution_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/gui_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/gui_runtime.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/gui_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/lan_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/local_host_status.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/net_traffic_stats_cache.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_data_callbacks.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_event_handler.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_event_handler.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_media_callbacks.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_transport.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/peer_transport.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/preview_subscriptions.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/privacy_control_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/privacy_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/privacy_session_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/received_video_stats.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/remote_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/runtime_state.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/runtime/session_lifecycle.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/session_lifecycle.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/trusted_devices.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/runtime/trusted_devices.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/about_panel.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/announcement_text.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/clipboard_file_notification.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/common.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/crossdesk_ui.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/device_card_actions.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/device_group_dialog.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/device_strings.slint</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/gui/ui/feedback_panel.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/file_browser.slint</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/ui/file_transfer_notification.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/gallery_menu_button.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/gallery_network_dialog.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/gallery_settings.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/gallery_view.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/group_filter.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/arrow-right-white.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/arrow-right.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/check.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/chevron-down.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/chevron-left.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/chevron-right.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/chevron-up.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/circle-white.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/close-white.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/close.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/compress.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/copy.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/expand.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/eye-slash.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/eye.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/file-up.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/keyboard.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/maximize.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/menu.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/minimize.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/monitor.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/mouse-off.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/mouse.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/pen-white.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/pen.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/sponsors/SOURCES.md</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/sponsors/afdian.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/src/gui/ui/icons/sponsors/github.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/sponsors/kofi.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/desktop/src/gui/ui/icons/stats.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/trash-white.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/trash.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/volume-off.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/icons/volume.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/lan_strings.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/main_window.slint</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/ui/message_panel.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/model_update.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/release_notes_text.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/remote_cursor.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/rounded_line_edit.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/server_window.slint</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/gui/ui/settings_controls.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/stream_window.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/trusted_devices.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/ui_localization.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/gui/ui/ui_radius.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/ui_surface.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/gui/ui/workbench_controls.slint</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/log/build_identity.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/log/rd_log.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/log/rd_log.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_auth_limiter.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_auth_limiter.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_identity.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_identity.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_password.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_password.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_session_manager.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_session_manager.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_transport.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/network/lan/lan_transport.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/path_manager/path_manager.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/path_manager/path_manager.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/autostart_backend.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/clipboard_backend.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/cuda_driver.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/cuda_driver.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/cuda_opengl_interop.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/cuda_opengl_interop.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/native_video_frame_ref.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/opengl_video_renderer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/opengl_video_renderer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/service_status_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/gui/video_renderer_factory_opengl.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/input/keyboard_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/input/windows_key_metadata.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/posix_instance_activation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/posix_single_instance.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/common/updater_posix.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/harmony/gui/tray/harmony_tray.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/harmony/gui/window_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/harmony/harmony_autostart.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/harmony/input/system_shortcuts.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/instance_activation.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/autostart.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/autostart.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/clipboard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/clipboard_file_offer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/clipboard_fuse2_mount.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/clipboard_fuse3_mount.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/clipboard_fuse_mount.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/linux_cursor_shape.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/linux_cursor_shape.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/linux_pointer_presence.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/linux_pointer_presence.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/wayland_portal_shared.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/wayland_portal_shared.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/common/x11_error_trap.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/desktop_automation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/cuda_graphics.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/cuda_graphics.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/tray/linux_tray.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/tray/linux_tray.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/wayland_activation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/gui/wayland_activation.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/headless/desktop_session_selection.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/headless/headless_console.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/headless/headless_console.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/platform/linux/headless/headless_session.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/headless/headless_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/headless/headless_settings.cpp</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/platform/linux/headless/headless_settings.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/device_controller_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/keyboard/keyboard_capturer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/keyboard/keyboard_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/keyboard/keyboard_capturer_wayland.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/linux_evdev_keycode.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/linux_keycode.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/mouse/mouse_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/mouse/mouse_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/input/mouse/mouse_controller_wayland.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/path_backend.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/privacy/privacy_backend_linux.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/privacy/privacy_guard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/privacy/privacy_guard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/runtime_daemon.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/runtime_daemon.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/display_resolution_linux.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/display_resolution_linux.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_drm.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_drm.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_linux.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_linux.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_wayland.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_wayland.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_wayland_build.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_wayland_pipewire.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_wayland_portal.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_x11.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_capturer/screen_capturer_x11.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/screen_lock.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/speaker_capturer/speaker_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/speaker_capturer/speaker_capturer_linux.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/speaker_capturer/speaker_capturer_linux.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/system_info.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/linux/updater/update_platform.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/autostart.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/clipboard.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/clipboard_file_offer.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/desktop_automation.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/metal_video_renderer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/metal_video_renderer.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/runtime/cursor_state_provider_mac.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/runtime/mac_permission_migration.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/runtime/mac_permission_migration.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/runtime/mac_permission_runtime.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/tray/mac_tray.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/tray/mac_tray.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/video_frame_scheduler.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/video_frame_scheduler.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/video_renderer_factory_metal.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/gui/window_drag_mac.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/device_controller_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/keyboard/keyboard_capturer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/keyboard/keyboard_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/mac_injected_event.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/macos_keyboard_modifier_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/macos_keycode.h</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/desktop/src/platform/macos/input/mouse/mouse_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/input/mouse/mouse_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/path_backend.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_backend_mac.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_capture_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_cover_view.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_cover_view.mm</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_cursor_mac.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/privacy/privacy_cursor_mac.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/runtime_daemon.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/runtime_daemon.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_capturer/display_resolution_mac.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_capturer/screen_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_capturer/screen_capturer_sck.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_capturer/screen_capturer_sck.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_capturer/screen_capturer_sck_impl.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/screen_lock.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/speaker_capturer/speaker_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/speaker_capturer/speaker_capturer_macosx.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/speaker_capturer/speaker_capturer_macosx.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/speaker_capturer/speaker_capturer_macosx.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/system_info.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/macos/updater/update_platform.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/path_backend.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/single_instance.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/video_renderer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/window_drag.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/autostart.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/autostart_task.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/clipboard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/clipboard_file_offer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/desktop_automation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/executable_path.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/application/local_host_application.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/application/portable_service_integration.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/application/unattended_application.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/instance_activation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/runtime/windows_service_runtime.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/session_end_observer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/session_end_observer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/slint_backend.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/slint_backend.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/slint_renderer_probe.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/tray/win_tray.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/tray/win_tray.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/window_icons.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/window_icons.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/window_taskbar.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/gui/window_taskbar.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/device_controller_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/keyboard/keyboard_capturer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/keyboard/keyboard_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/mouse/mouse_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/mouse/mouse_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_cursor_shape.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_input_injector.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_input_injector.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_input_marker.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_keycode.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_raw_keyboard_guard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/input/windows_text_input.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/path_backend.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/band_module/main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/band_module/privacy_cover_renderer.cpp</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/desktop/src/platform/windows/privacy/band_module/privacy_cover_renderer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/cursor_helper/main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_backend_win.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_band_guard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_band_guard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_band_ipc.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_capture_exclusion.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_cursor_api.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_cursor_guard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_cursor_guard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_cursor_process.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_input_guard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_input_guard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_topology.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/privacy/privacy_window_visibility.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/runtime_daemon.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/runtime_daemon.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/capture_backend_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/captured_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/cursor_draw.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/cursor_frame_compositor.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/display_label.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/display_presence.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/display_resolution_win.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/dxgi_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/headless_recovery.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_dxgi.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_dxgi.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_gdi.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_gdi.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_wgc.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_wgc.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_win.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/screen_capturer_win.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_dxgi_capture.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_dxgi_cursor_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_frame_pacer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_frame_schedule.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_nv12_cache.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_status_poller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/secure_desktop_wgc_capture.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/stable_display_slots.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/virtual_display_binding.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_plugin_api.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_plugin_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_plugin_entry.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_session_impl.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_capturer/wgc_session_impl.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/screen_lock.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/autostart_control.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/autostart_control.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/autostart_state.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/desktop_transition_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/interactive_desktop.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/interactive_state.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/local_host_channel.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/local_host_channel.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/named_pipe_deadline.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/rdp_session_handoff.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/rdp_session_handoff.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/secure_text_input.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/service_host.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/service_host.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/service_lifetime.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/service_query_worker.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/session_helper_main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/session_helper_shared.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/shared_capture_configuration.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/shared_capture_lifecycle.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/shared_identity.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/shared_identity.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/shutdown_order.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/unattended_config.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/unattended_config.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/service/unattended_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/single_instance.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/speaker_capturer/speaker_capturer_factory.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/speaker_capturer/speaker_capturer_wasapi.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/speaker_capturer/speaker_capturer_wasapi.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/system_info.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/updater/update_platform.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/virtual_display/usbmmidd_policy.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/virtual_display/usbmmidd_virtual_display.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/virtual_display/usbmmidd_virtual_display.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/virtual_display/virtual_display_provisioner.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/virtual_display/virtual_display_provisioner.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/windows_thread_dpi.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/platform/windows/winrt_apartment.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/privacy/privacy_backend.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/privacy/privacy_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/privacy/privacy_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/screen_capturer/captured_nv12_frame.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/screen_capturer/captured_nv12_frame.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/screen_capturer/screen_capture_method.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/screen_capturer/screen_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/screen_capturer/screen_capturer_factory.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/speaker_capturer/speaker_capture_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/speaker_capturer/speaker_capture_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/speaker_capturer/speaker_capturer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/speaker_capturer/speaker_capturer_factory.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/thumbnail/stb_image.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/thumbnail/stb_image_write.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/thumbnail/thumbnail.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/thumbnail/thumbnail.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/clipboard.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/clipboard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/clipboard_file_promise.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/clipboard_file_promise.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/clipboard_file_uri.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/file_transfer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/tools/file_transfer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/main.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/update_controller.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/update_controller.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/update_core.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/update_core.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/updater/update_platform.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/linux_update_download.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/linux_update_download.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/update_checker.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/update_checker.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/version_checker.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/src/version_checker/version_checker.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/build_define_scope_test.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/connection_status_protocol_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/cursor_position_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/linux_cuda_graphics_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/linux_x11_capture_recovery_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/macos_keyboard_modifier_state_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/path_manager_portable_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/repository_structure_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/slint_popup_render_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/slint_render_test_platform.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/slint_stream_render_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/slint_ui_smoke_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/desktop/tests/update_checker_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/version_checker_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/video_callback_lifetime_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_autostart_task_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/desktop/tests/windows_autostart_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/desktop/tests/windows_keyboard_capturer_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_manifest_resource_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_mouse_controller_safety_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_sas_guard_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_service_lifetime_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_service_mouse_ipc_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_unattended_policy_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/tests/windows_unattended_security_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/options.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/platform.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/rules/about_resources.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/rules/build_identity.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/rules/slint.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/desktop/xmake/targets.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/.clang-tidy</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/.clangd</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/.gitignore</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/AppScope/app.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/AppScope/resources/base/element/string.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/AppScope/resources/base/media/app_icon.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/harmony/MOBILE_PARITY.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/build-profile.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/AutostartHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/CMakeLists.txt</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/DesktopAccess.ets</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/desktop-port/DesktopHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/DisplayHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/EntryAbility.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/FileHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/Index.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/KeyboardHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/desktop-port/TextInputHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/TrayHost.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/bridge.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/cursor_shape.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/desktop.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/devices.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/displays.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/font-discovery.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/host_access.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_audio.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_clipboard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_error.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_events.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_init.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/SDL3/SDL_timer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/include/tinyfiledialogs.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/index.d.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/keyboard.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/keyboard_hook.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/keyboard_hook.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/os-services.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/pointer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/services.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/surface.inc</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/tray.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/window-properties.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/desktop-port/window_mode.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/build-profile.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/hvigorfile.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/oh-package-lock.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/oh-package.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/cpp/CMakeLists.txt</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/cpp/napi_init.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/cpp/types/libcrossdesk/index.d.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/cpp/types/libcrossdesk/oh-package.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/ets/mobile/AnnouncementInbox.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/ets/mobile/AnnouncementText.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/ets/mobile/ControllerModels.ets</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/harmony/entry/src/main/ets/mobile/EntryAbility.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/ets/mobile/Localization.ets</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/harmony/entry/src/main/ets/mobile/LocalizationData.ets</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileController.ets</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobilePlatform.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileRemoteView.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileStore.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/MobileWidgets.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/PrivacyPolicy.ets</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteDisplayResolutions.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/RemotePrivacyScreen.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteShortcutPanel.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/mobile/RemoteShortcuts.ets</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/entry/src/main/ets/pages/Mobile.ets</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/harmony/entry/src/main/module.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/resources/base/element/color.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/entry/src/main/resources/base/media/mobile_brand.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/harmony/entry/src/main/resources/base/profile/main_pages.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/hvigor/hvigor-config.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/hvigorfile.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/audio.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/audio.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/capture.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/capture.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/capture_surface.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/capture_surface.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/input.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/input.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/key_mapping.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/media.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/media.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/mobile_session.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/harmony/native/mobile_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/native/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/oh-package.json5</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/scripts/build.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/scripts/build_desktop.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/scripts/generate_mobile_resources.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/autostart_host_test.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/build_mobile_fixture.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/capture_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/desktop_devices_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/desktop_keyboard_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/desktop_platform_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/desktop_window_host_test.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/generate_mobile_resources_test.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/input_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/keyboard_hook_mock.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/keyboard_hook_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/localization_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/mobile_file_transfer_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/mobile_lifecycle_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/mobile_localization_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/mobile_models_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/mobile_platform_test.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/mobile_store_test.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/mobile_synthetic_host.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/harmony/tests/privacy_policy_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/run.py</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/harmony/tests/text_input_host_test.cjs</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/harmony/tests/verify_hap.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile.xcodeproj/project.pbxproj</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/ios/CrossDeskMobile.xcodeproj/xcshareddata/xcodecloud/manifest.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile.xcodeproj/xcshareddata/xcschemes/CrossDeskMobile.xcscheme</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/AnnouncementInbox.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/AnnouncementInboxView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/AnnouncementText.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/AppUpdateChecker.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Assets.xcassets/AppIcon.appiconset/Contents.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Assets.xcassets/AppIcon.appiconset/CrossDeskAppIcon-1024.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/ios/CrossDeskMobile/Assets.xcassets/Contents.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Bridge/CrossDeskRTCBridge.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Bridge/CrossDeskRTCBridge.mm</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/ContentView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/CrossDeskMobile-Bridging-Header.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/CrossDeskMobileApp.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Info.plist</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/CrossDeskMobile/Input/RemoteMouseInput.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Input/RemoteShortcutPanel.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Input/RemoteShortcuts.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Input/RemoteSystemKeyboard.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Input/RemoteTouchInputView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Input/VirtualMouseBar.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/PrivacyInfo.xcprivacy</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/PrivacyPreferences.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/PrivacyViews.swift</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/CrossDeskMobile/RecentConnection.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/RemoteNetworkStatistics.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/RemotePrivacyScreen.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/RemoteSessionModel.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/RemoteSessionView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/RemoteVideoSettings.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Rendering/NativeMetalVideoView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/ThirdPartyLicenses.json</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/CrossDeskMobile/Resources/de.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/de.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/en.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/en.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/es.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/es.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/fr.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/fr.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/ja.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/ja.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/ko.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/ko.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/pt-BR.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/pt-BR.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/pt.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/pt.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/ru.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/CrossDeskMobile/Resources/ru.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/vi.lproj/InfoPlist.strings</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/CrossDeskMobile/Resources/vi.lproj/Localizable.strings</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hans.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hans.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hant.lproj/InfoPlist.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/Resources/zh-Hant.lproj/Localizable.strings</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/ThirdPartyLicensesView.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/CrossDeskMobile/ThumbnailFileStore.swift</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/ExportOptions-AppStore.plist</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/OpenSSL/PrivacyInfo.xcprivacy</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/ci_scripts/ci_post_clone.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/ci_scripts/ci_pre_xcodebuild.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/licenses/OPEN_SOURCE_NOTICE.txt</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/licenses/sources.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/build_minirtc_ios.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/generate_openssl_symbols.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/generate_third_party_licenses.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/package_openssl_ios.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/prepare_source_metadata.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/verify_app_bundle.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/scripts/verify_native_build.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/tests/app_update_checker_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/tests/build_minirtc_ios_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/tests/localization_test.py</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>apps/ios/tests/network_statistics_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/tests/privacy_controls_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/tests/privacy_policy_test.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/ios/tests/recent_connection_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Han có lý do tại mục 9–11. |
| <code>apps/ios/tests/third_party_licenses_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/ios/tests/video_settings_test.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/.gitignore</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/DEVELOPMENT.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/cordis.patch.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/integrations/deepseek-harness/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/integrations/deepseek-harness/cordis.patch.yml</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/integrations/deepseek-harness/package.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/integrations/deepseek-harness/src/client.js</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/integrations/deepseek-harness/src/host.js</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/integrations/deepseek-harness/src/plugin.js</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/package-lock.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/package.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/scripts/build-harness.mjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/scripts/build-ui.mjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/scripts/pack.mjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/scripts/release-npm.cjs</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/browser-host.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/browser.ts</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/src/common.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/environment.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/main.ts</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/src/native.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/server.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/sessions.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/store.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/terminal-view.ts</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>apps/mcp/src/terminal.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/ui-actions.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/src/version.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/tsconfig.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/tsconfig.ui.json</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/assets/crossdesk.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>apps/mcp/ui/audio-player.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/human-input.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/preview-player.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/video-player.ts</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/viewer.css</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>apps/mcp/ui/viewer.ts</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>deps/submodules/minirtc</code> | ngoại lệ có lý do | Gitlink chưa khởi tạo; chỉ kiểm tên/mã ghim, không có nội dung submodule để quét. |
| <code>deps/submodules/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/cpp-httplib/patches/v0.26.0/fix-mingw.diff</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/cpp-httplib/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/libsdl3/patches/3.2.26-x11-ime-lifetime.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/libsdl3/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/openssl/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/rust/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/slint/patches/1.17.1-macos-accesskit-kvo.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/slint/patches/1.17.1-per-window-xdg-app-id.patch</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/slint/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>deps/thirdparty/xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docker/linux-build/Dockerfile</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docker/linux-build/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docker/linux-build/install-pipewire-sdk.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docker/linux-build/verify-build-image.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docker/linux-build/verify-glibc-baseline.sh</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docs/BUILD.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/BUILD_EN.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/CONNECTIONS.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/FAQ.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/HEADLESS.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/HEADLESS_EN.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/SELF_HOSTING.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/SELF_HOSTING_EN.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/UPDATER.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/USER_GUIDE.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/USER_GUIDE_EN.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/VERSIONING.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/connection-architecture.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/deskhub.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/gui-architecture.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/images/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/images/REGENERATE.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>docs/images/cli-ansi-overview.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/cli-kitty-overview.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/crossdesk-banner.svg</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>docs/images/desktop-about-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-about-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-deskhub-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-deskhub-focus-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-deskhub-focus-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-deskhub-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-feedback-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-feedback-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-files-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-files-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-lan-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-lan-password-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-lan-password-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-lan-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-main-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-main-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-self-hosted-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-self-hosted-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-settings-deskhub-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-settings-deskhub-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-settings-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-settings-network-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-settings-network-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-settings-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/desktop-stream-video-settings-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/desktop-stream-video-settings-zh.png</code> | ngoại lệ có lý do | Ảnh UI tiếng Trung chưa tạo lại; thiếu toolchain/capture native, xem mục 8. |
| <code>docs/images/fixtures/build-terminal.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/fixtures/cpu-history-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/fixtures/cpu-history.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/fixtures/source.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/fixtures/website-en.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/fixtures/website.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>docs/images/web-client.png</code> | ngoại lệ có lý do | Nhị phân: kiểm tên/loại/tham chiếu, không OCR hay thay đổi nội dung. |
| <code>libs/controller/include/controller_diagnostics.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/controller_files.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/controller_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/file_browser.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/file_transfer_core.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/frame_image.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/mobile_controller_protocol.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/mobile_device_trust.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/mobile_file_transfer_session.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/include/mobile_video_timing.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/controller_audio.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/controller_diagnostics.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/controller_files.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/controller_requests.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/controller_session.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/encoded_video.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/file_browser.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/frame_image.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/frame_observation.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/latest_frame.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/controller/src/mobile_device_trust.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/include/terminal_renderer.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/src/input.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/src/sixel_renderer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/src/terminal_metrics.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/src/terminal_renderer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/terminal/src/terminal_status.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/app_version.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/display_resolution.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/display_resolution_preferences.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/display_stream_id.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/file_transfer_format.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/preview_subscription.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/remote_action.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/remote_cursor_shape.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/stream_names.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/include/web_clipboard_protocol.h</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/src/app_version.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/src/file_transfer.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/src/remote_action.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/tests/keyboard_state_wire_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>libs/wire/tests/wire_test.cpp</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>localization/README.md</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>localization/catalog.json</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>localization/generate.py</code> | đã rà soát | Có trong diff: tích hợp locale/catalog/default, sửa selector/kiểm thử hoặc bảo toàn ghi công; không thêm bản dịch tại chỗ. |
| <code>localization/test_generate.py</code> | đã dịch | Có trong diff: thêm/cập nhật nội dung tiếng Việt trực tiếp. |
| <code>scripts/version.py</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
| <code>xmake.lua</code> | đã rà soát | Giữ nguyên; quét tĩnh toàn văn. Không phát hiện Han literal/escape. |
