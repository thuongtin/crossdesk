# Quản lý phiên bản và phát hành

[Về trang chủ](../README.md) · [Tự biên dịch](BUILD.md)

## Xem phiên bản đã cài

Trên máy tính, mở **Menu → Giới thiệu**; trên thiết bị di động, mở **Cài đặt → Giới thiệu**. Khi báo cáo sự cố, cung cấp phiên bản cả máy điều khiển và máy được điều khiển; nếu dùng CLI hoặc MCP, nên cung cấp cả phiên bản bộ công cụ.

Phiên bản trên trang tải, phiên bản ứng dụng hiển thị và mã nguồn hiện tại của kho mã không nhất thiết giống nhau. Khi cần tái hiện một gói cài, ưu tiên tag hoặc commit nguồn ghi ở trang giới thiệu hay thông tin nguồn mở, thay vì tải thẳng nhánh mới nhất.

Chỉ đổi ngày biên dịch không kích hoạt nhắc nâng cấp máy tính; cập nhật so sánh phiên bản số và số bản vá. Xem cách cập nhật trong [hướng dẫn cập nhật](UPDATER.md).

## Quản lý phiên bản mã nguồn

Dùng `python3 scripts/version.py` ở thư mục gốc kho mã để xem, sửa và kiểm tra phiên bản; cần Python 3.9 hoặc mới hơn, không cần phụ thuộc bên thứ ba. Phiên bản chia ba nhóm: máy tính, Android, iOS và HarmonyOS dùng chung phiên bản ứng dụng; CLI, MCP và plugin DSH dùng chung phiên bản công cụ; phần mở rộng Pi có số riêng. Script chỉ sửa cấu hình nguồn, không tạo tag Git, commit, biên dịch hay phát hành gói cài.

## Xem và kiểm tra

```sh
python3 scripts/version.py show
python3 scripts/version.py show client
python3 scripts/version.py show mcp
python3 scripts/version.py show --json
python3 scripts/version.py show desktop --field version
python3 scripts/version.py check
```

`check` xác minh định dạng phiên bản, số bản dựng di động, trường trùng và thống nhất phiên bản nhóm. Nếu không khớp, trả mã thoát khác không và chỉ rõ thành phần; dùng `set client <phiên-bản>` để thống nhất ứng dụng, `set cli <phiên-bản>` hoặc `set mcp <phiên-bản>` để thống nhất CLI, MCP, DSH và phụ thuộc gốc. Kiểm tra riêng bất kỳ ứng dụng nào cũng kiểm tra toàn bộ ứng dụng; `check cli` và `check mcp` đều kiểm tra cả nhóm công cụ.

GitHub Action `Check component versions` chạy mọi kiểm tra khi push, PR và chạy thủ công. Quy trình biên dịch và phát hành npm cũng kiểm tra nhóm tương ứng, dừng nếu không khớp. Script phiên bản không truy vấn npm hay cửa hàng ứng dụng; phát hành chính thức vẫn dùng kiểm tra trước phát hành của quy trình tương ứng.

## Thành phần có thể quản lý

| Tham số thành phần | Nguồn phiên bản và phạm vi đồng bộ |
| --- | --- |
| `client` | Quản lý thống nhất phiên bản mọi ứng dụng, giữ riêng số bản dựng Android, iOS, HarmonyOS; `mobile` là bí danh cùng nhóm, cũng đồng bộ phiên bản máy tính |
| `desktop` | Sửa phiên bản đồng bộ mọi ứng dụng; ghi vào giá trị mặc định `CROSSDESK_VERSION` trong `apps/desktop/xmake/options.lua`, dùng chung Windows, macOS, Linux |
| `android` | Sửa phiên bản đồng bộ mọi ứng dụng; `--build` và `bump android build` chỉ đặt `crossdeskVersionCode` của Gradle |
| `ios` | Sửa phiên bản đồng bộ mọi ứng dụng; `--build` và `bump ios build` chỉ đặt `CURRENT_PROJECT_VERSION`, đồng bộ Debug và Release |
| `harmony` | Sửa phiên bản đồng bộ mọi ứng dụng; `--build` và `bump harmony build` chỉ đặt `versionCode` của `AppScope/app.json5` |
| `cli` | Đồng bộ CLI, MCP, DSH và phụ thuộc CLI của MCP; CLI ghi vào `apps/cli/package.json` và `apps/cli/xmake.lua`, năm gói npm gốc nền tảng và gói đầu vào kế thừa phiên bản này khi đóng gói |
| `mcp` | Cùng phạm vi sửa với `cli`; MCP ghi vào `apps/mcp/package.json`, phiên bản gốc và gói gốc trong tệp khóa, phiên bản gói thư mục tích hợp DSH và `NATIVE_VERSION` trong `src/native.ts` |
| `pi` | Phiên bản phần mở rộng trong `apps/cli/plugins/pi/package.json` |

Nếu thành phần npm có `package-lock.json`, script đồng bộ phiên bản gốc và `packages[""].version`, không đổi phiên bản gói phụ thuộc. DSH phân phối cùng MCP, không cần đặt phiên bản phát hành DSH riêng. Mô tả mô-đun HarmonyOS, `modelVersion` SDK và phiên bản thư viện kiểu cục bộ không thuộc phiên bản phát hành ứng dụng, giữ mục đích riêng.

## Đặt phiên bản

Các lệnh sau đều là ví dụ; thay bằng phiên bản chuẩn bị phát hành:

```sh
python3 scripts/version.py set client 1.7.7 --android-build 5 --ios-build 7 --harmony-build 10002
python3 scripts/version.py set cli 0.3.0
python3 scripts/version.py set mcp 0.3.0
python3 scripts/version.py set pi 0.1.1
```

Mọi lệnh sửa hỗ trợ `--dry-run`, hiện trước khác biệt tệp sắp sửa:

```sh
python3 scripts/version.py set mcp 0.3.0 --dry-run
```

Tham số phiên bản là `X.Y.Z`, không gồm tiền tố tag `v`, ngày hoặc siêu dữ liệu biên dịch. CLI, MCP, Pi còn hỗ trợ bản tiền phát hành như `X.Y.Z-rc.N`. Mỗi phần phiên bản chung ứng dụng giới hạn `0–65535` để phù hợp tài nguyên phiên bản Windows.

`set client` có thể nhận riêng `--android-build`, `--ios-build`, `--harmony-build`, không nhận `--build` mơ hồ. `set android 1.7.7 --build 5` cũng đặt phiên bản phát hành mọi ứng dụng thành `1.7.7`, nhưng `--build 5` chỉ đặt số bản dựng Android, các nền tảng di động khác tăng theo số riêng. `set ios`, `set harmony` xử lý tương tự; `set desktop` cũng đồng bộ mọi ứng dụng nhưng không nhận `--build`.

Khi phiên bản phát hành một nền tảng thay đổi mà không chỉ định số bản dựng, số đó tự tăng một; nếu cùng phiên bản và chưa chỉ định số bản dựng thì không sửa tệp. Ví dụ, từ mọi ứng dụng `1.7.6`, số bản dựng Android / iOS / HarmonyOS lần lượt `4` / `6` / `10001`, chạy `set client 1.7.7` cho kết quả mọi ứng dụng `1.7.7`, số bản dựng `5` / `7` / `10002`. Khi sửa lệch phiên bản, chỉ nền tảng thực sự đổi phiên bản tự tăng số bản dựng. Số chỉ định rõ không được giảm; khi đổi phiên bản ứng dụng, số bản dựng bắt buộc tăng. Script thống nhất dùng số nguyên dương: Android tối đa `2100000000`, iOS tối đa `9999`, HarmonyOS tối đa `2147483647`.

Trước khi sửa, xác minh mọi trường đích; nếu thất bại không ghi tệp. Ghi giữ định dạng và quyền tệp, thay thế nguyên tử từng tệp. Nếu ghi lỗi, thử khôi phục tệp đã ghi; sau sửa vẫn nên kiểm tra `git diff`.

## Tăng phiên bản

```sh
python3 scripts/version.py bump cli patch
python3 scripts/version.py bump mcp minor
python3 scripts/version.py bump client patch
python3 scripts/version.py bump client major --dry-run
python3 scripts/version.py bump android build
```

`major`, `minor`, `patch` tăng số tương ứng, đặt các phần thấp hơn về không; ví dụ `1.2.3` tăng `minor` thành `1.3.0`. `bump client patch` và `bump ... patch` trên bất kỳ ứng dụng nào đều đồng bộ tăng phiên bản phát hành mọi ứng dụng, tăng riêng số bản dựng di động. `bump cli` và `bump mcp` đều đồng bộ CLI, MCP, DSH và phụ thuộc gốc MCP. Nếu trong nhóm vốn không khớp, dùng `set` để chỉ rõ phiên bản thống nhất trước.

`bump android build`, `bump ios build`, `bump harmony build` chỉ tăng số bản dựng nền tảng chỉ định, giữ nguyên các nền tảng khác; không có `bump client build` hay `bump mobile build`. Khi đang là bản tiền phát hành, dùng `set` chỉ rõ phiên bản tiếp theo, ví dụ `set mcp 0.2.0-rc.2` hoặc `set mcp 0.2.0`.

## Phiên bản chung của CLI, MCP và DSH

Phiên bản CLI, MCP, siêu dữ liệu tích hợp DSH và phụ thuộc gốc MCP phải giống hoàn toàn, hiện đều là `0.2.0`. Chạy `set cli 0.3.0` hoặc `set mcp 0.3.0` sẽ đặt tất cả thành `0.3.0`, chỉ cần một trong hai lệnh. Hậu tố tiền phát hành cũng phải khớp. Không có tham số đặt riêng phiên bản phụ thuộc gốc.

Gói npm MCP chung tạo phụ thuộc chính xác cho năm gói nền tảng từ `NATIVE_VERSION`; `--version` và bắt tay giao thức MCP đọc phiên bản gói. DSH phân phối cùng gói MCP, không phát hành phiên bản plugin riêng.

Trước hết phát hành năm gói gốc nền tảng và đầu vào CLI cùng phiên bản qua quy trình phát hành CLI, rồi phát hành MCP. Dù chỉ sửa MCP, vẫn cần chuẩn bị cùng phiên bản mới cho cả hai và phát hành theo thứ tự này. CLI và MCP vẫn dùng quy trình và tiền tố tag riêng; ví dụ cùng lần cập nhật dùng `cli-v0.3.0` và `mcp-v0.3.0`, nên trỏ cùng commit.

Quy trình biên dịch CLI tạo kèm gói cài `crossdesk-mcp-<nền-tảng>` cho từng nền tảng. Nếu phiên bản nguồn không khớp, biên dịch, đóng gói MCP và kiểm tra phát hành báo lỗi; gói cài nền tảng còn yêu cầu phiên bản gói gốc truyền vào khớp. Phiên bản công cụ không ảnh hưởng phiên bản ứng dụng và phần mở rộng Pi.

## Phối hợp biên dịch và phát hành

Phiên bản ứng dụng đồng thời điều khiển mặc định Xmake cục bộ và phiên bản mặc định khi biên dịch nhánh GitHub Actions không nhập `desktop_version`. Cấu hình Xmake hiện có có thể giữ bộ nhớ đệm, nên cấu hình lại; `--CROSSDESK_VERSION` tường minh, `desktop_version` Actions và tag phát hành tiếp tục ghi đè mặc định theo quy trình cũ. Phát hành chính thức nên khớp phiên bản cơ sở tag với phiên bản nguồn ứng dụng. Ngày và hậu tố hotfix vẫn do quy trình phát hành xử lý, script này không sửa định dạng tag hiện có. Các nền tảng có thể biên dịch và phát hành riêng nhưng dùng cùng phiên bản phát hành nguồn.

Sau sửa và kiểm tra, rà soát, commit và push nguồn rồi tạo tag tương ứng phiên bản thực: CLI dùng `cli-v<phiên-bản>`, MCP dùng `mcp-v<phiên-bản>`. Di động tiếp tục dùng `android-v<phiên-bản>-YYYYMMDD`, `ios-v<phiên-bản>-YYYYMMDD`. Không ghi đè bản đã phát hành bằng nội dung khác; script sửa phiên bản không xác định hộ bạn phiên bản đã được dùng trên registry hay chưa.

Tài liệu liên quan: [biên dịch máy tính](BUILD.md) · [phát hành CLI](../apps/cli/docs/npm-distribution.md#ci-và-phát-hành) · [phát hành MCP](../apps/mcp/DEVELOPMENT.md#phát-hành-lên-npm-qua-github-actions).
