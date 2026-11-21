# Cài CLI và phân phối npm

[Về hướng dẫn CLI](../README.md) · [Thao tác terminal](terminal-client.md) · [Đóng gói](#tạo-gói) · [CI và phát hành](#ci-và-phát-hành)

## Cài qua npm

Chuẩn bị **Node.js 22 trở lên** và npm, rồi chạy:

```sh
npm install -g @chunkdash/crossdesk-cli@latest
crossdesk-cli doctor --json
crossdesk-cli 123456789
```

Thay `123456789` bằng ID thiết bị 9 chữ số của máy tính, nhập mật khẩu ẩn theo lời nhắc terminal. `doctor` chỉ kiểm tra môi trường cục bộ, không kết nối từ xa.

npm tải chương trình native phù hợp, không cần cài công cụ biên dịch. Hãy cài gói đầu vào chung `@chunkdash/crossdesk-cli`, giữ phụ thuộc tùy chọn, không dùng `--omit=optional`. Chỉ cài gói nền tảng không đăng ký lệnh `crossdesk-cli`.

Nếu không cần cài toàn cục, có thể dùng:

```sh
npx @chunkdash/crossdesk-cli@latest doctor --json
npx @chunkdash/crossdesk-cli@latest 123456789
```

Lần chạy đầu tải gói tương ứng.

## Yêu cầu hệ thống và terminal

| Hệ thống | Kiến trúc | Yêu cầu chạy |
| --- | --- | --- |
| macOS | Apple Silicon / Intel | Phiên bản tối thiểu theo `build-info.json` của gói đã tải |
| Linux | x64 / arm64 | glibc, libstdc++, zlib; phiên bản cần thiết theo `build-info.json` trong gói |
| Windows | x64 | Windows 10+, terminal hỗ trợ Console / ConPTY và đầu ra VT |

Alpine / musl không có gói biên dịch sẵn. Gói nền tảng chọn theo hệ thống và kiến trúc Node đang chạy; nếu sai kiến trúc, kiểm tra Node và hệ điều hành trước.

Terminal thường dùng hình ký tự ANSI. Hình pixel Kitty / Sixel cần terminal hỗ trợ và vượt kiểm tra khả năng; nếu báo không hỗ trợ, đổi sang `--renderer ansi`. SSH cần cấp PTY (`ssh -t`), tmux / screen dùng ANSI. Git Bash Windows nên chạy trong Windows Terminal.

Xem tương tác không hỗ trợ `TERM=dumb`, cũng không được chuyển đầu ra vào tệp. Phạm vi chạy đã xác nhận từng nền tảng xem [tương thích](validation.md); cài thành công không có nghĩa mọi terminal đều hiển thị được hình pixel.

## Cập nhật, chuyển đổi bản cũ và gỡ cài đặt

Để cập nhật, chạy lại:

```sh
npm install -g @chunkdash/crossdesk-cli@latest
```

Gói không có scope cũ `crossdesk-cli` không tự chuyển đổi. Nếu từng cài, gỡ điểm vào cũ trước rồi cài mới:

```sh
npm uninstall -g crossdesk-cli
npm install -g @chunkdash/crossdesk-cli@latest
```

Gỡ bản mới:

```sh
npm uninstall -g @chunkdash/crossdesk-cli
```

## Cài thất bại hoặc lệnh không khả dụng

| Hiện tượng | Cách xử lý |
| --- | --- |
| Không tìm thấy `crossdesk-cli` | Xác nhận cài gói đầu vào chung; sau cài toàn cục, kiểm tra thư mục lệnh npm toàn cục trong PATH hoặc dùng `npx` |
| Không chạy trực tiếp được sau cài cục bộ | Lệnh cục bộ nằm ở `node_modules/.bin`, có thể dùng `npx crossdesk-cli` trong thư mục cài |
| Thiếu gói native theo nền tảng | Bỏ thiết lập lược phụ thuộc tùy chọn, cài lại; kiểm tra mạng tải và phiên bản chọn có nền tảng tương ứng |
| Báo không hỗ trợ phiên bản Node | Nâng lên Node.js 22 trở lên rồi cài lại |
| Lỗi `GLIBC` / `GLIBCXX` hoặc phiên bản hệ thống | Kiểm tra yêu cầu trong gói, dùng hệ thống đáp ứng hoặc [tự biên dịch](terminal-client.md#biên-dịch-và-cài-đặt) chương trình phù hợp |
| Không hỗ trợ chế độ hình ảnh | Dùng `--renderer ansi`, hình pixel cần terminal tự hỗ trợ Kitty hoặc Sixel |
| `doctor` bình thường nhưng không kết nối được | Kiểm tra tiếp trực tuyến, ID, mật khẩu và máy chủ từ xa; xem [khắc phục kết nối](terminal-client.md#khắc-phục-sự-cố-và-giới-hạn) |

Khi phản hồi, kèm hệ điều hành, kiến trúc CPU, phiên bản Node, tên terminal và lỗi cụ thể. Không ghi mật khẩu kết nối vào lệnh hoặc phản hồi.

## Tạo gói


Biên dịch release theo [hướng dẫn](terminal-client.md) trước. macOS cần ký trước đóng gói, CI dùng chữ ký ad-hoc; cục bộ có thể chạy lệnh sau (Intel thay `arm64` bằng `x86_64`):

```sh
codesign --force --sign - build/macosx/arm64/release/crossdesk-cli
```

Sau đó tạo gói npm:

```sh
npm --prefix apps/cli run pack:npm
```

Script đọc phiên bản `apps/cli/package.json` và `apps/cli/xmake.lua`, kiểm tra khớp nhau; kiểm tra định dạng nhị phân, CPU, phụ thuộc liên kết động của nền tảng hiện tại, chạy `doctor --json` ngoại tuyến. macOS chỉ nhận thư viện động hệ thống, Linux chỉ nhận thư viện nền hệ thống đã liệt kê, Windows từ chối DLL chưa xử lý trong sản phẩm và runtime MSVC động. Đóng gói Windows chạy trong môi trường phát triển Visual Studio có `dumpbin.exe`. Script kiểm tra trước hai tệp đầu ra có tồn tại không, hoàn tất cả hai gói trong thư mục tạm rồi mới ghi đầu ra; nếu ghi thất bại, dọn gói vừa ghi lần này, không ghi đè sản phẩm trùng tên có sẵn.

Mặc định đọc chương trình native từ `build/<platform>/<arch>/release/`, ghi hai `.tgz` vào `build/npm/`. Có thể chỉ định đường dẫn:

```sh
npm --prefix apps/cli run pack:npm -- --binary /absolute/path/crossdesk-cli --out-dir /absolute/path/output
```

Mỗi máy biên dịch tạo gói native theo nền tảng hiện tại và gói khởi chạy chung cùng phiên bản. Ví dụ macOS arm64, phiên bản 0.1.0: tên là `chunkdash-crossdesk-cli-darwin-arm64-0.1.0.tgz` và `chunkdash-crossdesk-cli-0.1.0.tgz`; npm đặt tên tarball `@scope/name` thành `scope-name-version.tgz`. Mỗi trong năm artifact CI chứa một gói đầu vào, cuối cùng cần năm gói nền tảng và một gói đầu vào, tổng sáu tarball.

`apps/cli/npm/platforms.json` tập trung định nghĩa năm nền tảng; script kiểm tra tên gói hợp lệ, không trùng. Gói khởi chạy dùng `optionalDependencies` phiên bản chính xác để npm chỉ cài gói native khớp hệ thống và CPU. Cả sáu gói sinh ra có tác giả, kho nguồn, trang chủ, địa chỉ phản hồi và cấu hình phát hành công khai cho registry npm chính thức. Gói native chứa nhị phân, yêu cầu chạy, SHA-256 và ảnh chụp nguồn ứng dụng điều khiển hiện tại cùng MiniRTC. Giấy phép, nguồn tương ứng và tài liệu liên kết lại của phụ thuộc Xmake ngoài cần kèm bản chính thức; xem [nguồn gốc mã](source-provenance.md). Ảnh chụp nguồn đã gồm MiniRTC, giải nén không cần chạy `prepare-deps.sh`; trong `source/`, tắt máy tính và bật CLI tường minh theo hướng dẫn trên rồi biên dịch `crossdesk-cli`. Gói nguồn chỉ chứa CLI và phụ thuộc, không chứa ứng dụng máy tính hay di động.

`private: true` trong `apps/cli/package.json` chỉ thư mục nguồn không phải sản phẩm phát hành. Chỉ phát hành tarball do script tạo; gói sinh ra không mang private.

## Kiểm chứng cài đặt cục bộ

Ví dụ macOS arm64, dùng đường dẫn tuyệt đối gói đã tạo, chạy trong thư mục tạm ngoài dự án:

```sh
npx --yes --offline \
  --package /absolute/path/chunkdash-crossdesk-cli-darwin-arm64-0.1.0.tgz \
  --package /absolute/path/chunkdash-crossdesk-cli-0.1.0.tgz \
  -- crossdesk-cli doctor --json
```

Cũng có thể cài hai tarball cục bộ bằng `npm install --ignore-scripts` trước rồi dùng `crossdesk-cli` trong thư mục cài. Kiểm chứng xem tương tác cần TTY thực và máy được điều khiển đã cấp quyền; doctor không kết nối từ xa.

Ứng dụng tích hợp như Pi nên khởi động trực tiếp chương trình native để điều khiển tiến trình và mô tả mật khẩu riêng đáng tin cậy. Sau cài toàn cục, lấy đường dẫn qua `crossdesk-cli --print-binary-path`, đặt vào `CROSSDESK_CLI`; không đặt cả lệnh npx vào biến này.

## CI và phát hành

`.github/workflows/build-cli.yml` cấu hình biên dịch release native, doctor, đóng gói năm nền tảng và tải tarball lên. Push thường, PR và biên dịch thủ công chỉ tạo artifact; `.github/workflows/release-cli.yml` dùng lại quy trình đó rồi tập hợp, phát hành sáu gói npm. Nhãn `cli-v*` chỉ vào quy trình phát hành CLI, không kích hoạt trùng biên dịch CLI riêng hay phát hành máy tính. Khi đổi tham chiếu MiniRTC, bắt buộc đẩy commit tương ứng lên remote submodule trước rồi mới đẩy CrossDesk; nếu không, checkout mới và CI thất bại khi tải submodule.

### Phát hành tự động GitHub Actions

| Thao tác | Kết quả |
| --- | --- |
| Đẩy commit thường / gửi PR | Biên dịch năm nền tảng, tải artifact, không phát hành npm |
| Actions → Publish CLI to npm → Run workflow, giữ chọn `dry_run` | Biên dịch đầy đủ và diễn tập phát hành sáu gói, không ghi npm |
| Đẩy `cli-v0.1.1` | Kiểm tra nguồn phiên bản `0.1.1`, biên dịch và phát hành vào `latest` |
| Đẩy `cli-v0.2.0-rc.1` | Kiểm tra nguồn phiên bản `0.2.0-rc.1`, biên dịch và phát hành vào `next` |
| Chạy thủ công trên nhãn `cli-v*` tương ứng, bỏ `dry_run` | Phát hành thật; trên nhánh không được tắt diễn tập |

Chuẩn bị bản tiếp theo bằng `python3 scripts/version.py set cli <phiên-bản>` tại gốc kho, đồng bộ CLI, MCP, plugin DSH và phụ thuộc native của MCP, rồi chạy `python3 scripts/version.py check cli`. `set mcp <phiên-bản>` có cùng phạm vi sửa, xem [quản lý phiên bản thành phần](../../../docs/VERSIONING.md). Phát hành gói nền tảng và đầu vào CLI trước, rồi MCP cùng phiên bản. Commit và đẩy mã rồi tạo nhãn cùng phiên bản. Ví dụ nguồn đã lên `0.2.1`:

```sh
git tag cli-v0.2.1
git push origin cli-v0.2.1
```

Gói `0.1.0` đã tải lên không được ghi đè bằng bản biên dịch mới; lần phát hành chương trình native sau nên dùng phiên bản mới. Diễn tập thủ công dùng `npm pack --dry-run` và kiểm tra nội dung, không gọi `npm publish` hay trao đổi thông tin xác thực OIDC; có thể chạy trên nhánh và kiểm tra bản đã phát hành, nhưng không kiểm chứng quyền phát hành npm. Chạy nhãn luôn kiểm tra phiên bản khớp. Mục chạy thủ công GitHub cần quy trình đã có trong nhánh mặc định.

Job phát hành chỉ tải năm artifact `npm-*` của lần chạy hiện tại, giữ thư mục riêng. `release-npm.cjs` kiểm tra tên gói, phiên bản, nền tảng, phụ thuộc đầu vào, SHA-256 chương trình native và SHA-512 tarball; nội dung năm gói đầu vào bắt buộc giống nhau (cho phép khác CRLF Windows), thực dùng đầu vào sinh bởi Linux x64, giữ quyền khởi chạy Unix. Trước tải thật, kiểm tra trạng thái registry mọi gói; dừng nếu cùng phiên bản khác nội dung hoặc dist-tag đích đã trỏ bản mới hơn.

Thứ tự tải là năm gói nền tảng → chờ truy vấn được phiên bản chính xác, tính toàn vẹn và dist-tag → gói đầu vào → xác nhận cuối. Nếu npm xử lý bất đồng bộ, truy vấn mỗi 15 giây, mỗi đợt chờ tối đa 15 phút; không tải lặp chỉ vì chưa truy vấn thấy phiên bản. Phát hành thật có provenance, dùng xác thực OIDC ngắn hạn, chỉ job phát hành có `id-token: write`, toàn quy trình không cần `NPM_TOKEN`. Job phát hành chạy tuần tự, không ngắt phiên bản đang tải.

Nếu job phát hành thất bại, ưu tiên **Re-run failed jobs** của GitHub để dùng lại artifact biên dịch thành công lần này. Gói đã tồn tại, tính toàn vẹn và dist-tag đích khớp sẽ bỏ qua; cùng phiên bản khác nội dung, gói giữ chỗ vẫn `0.0.0-stage`, lỗi 403 hoặc mạng đều dừng và nêu lý do. Chờ npm xử lý xong hoặc sửa quyền rồi chạy lại, không chạy lại toàn bộ biên dịch ngay; biên dịch lại có thể sinh byte khác. Nếu dist-tag bản có sẵn khác đích lần này, cần kiểm tra và chỉnh nhãn thủ công trước. Quy trình không tự sửa dist-tag phiên bản có sẵn, cũng không có thao tác nâng `next` lên `latest`.

### Liên kết tài khoản npm một lần

Trong npm **Settings → Trusted Publisher** của từng gói trong sáu gói sau, chọn **GitHub Actions**, không phải cấp quyền GitHub trong menu tài khoản npm:

- `@chunkdash/crossdesk-cli`
- `@chunkdash/crossdesk-cli-darwin-arm64`
- `@chunkdash/crossdesk-cli-darwin-x64`
- `@chunkdash/crossdesk-cli-linux-arm64`
- `@chunkdash/crossdesk-cli-linux-x64`
- `@chunkdash/crossdesk-cli-windows-x64`

Sáu gói dùng cùng cấu hình:

| Trường npm | Giá trị |
| --- | --- |
| Organization or user | `kunkundi` (chủ kho GitHub, không phải scope npm) |
| Repository | `crossdesk` |
| Workflow filename | `release-cli.yml` (chỉ điền tên tệp) |
| Environment name | `npm` |
| Allowed actions | Chọn **Allow npm publish**; quy trình không cần **Allow npm dist-tag** |

Quy trình dùng runner GitHub-hosted, Node.js 24 và npm 12.2.1. Tên environment `npm` của kho GitHub bắt buộc khớp chính xác cấu hình npm; có thể giới hạn chỉ nhãn `cli-v*` được triển khai trong Settings → Environments. Nếu giới hạn vậy, diễn tập nhánh cũng bị chặn, nên chọn nhãn tương ứng để diễn tập. Có thể đặt required reviewers theo nhóm; đây là phê duyệt phát hành GitHub, khác npm 2FA.

Sau khi tạo Trusted Publisher, nên **phát hành thật lần đầu trong 2 ngày**; nếu không cấu hình hết hạn, cần xóa và tạo lại, diễn tập không tính là phát hành thành công. Vì vậy nên đẩy quy trình, diễn tập xong rồi tạo tin cậy npm, sau đó phát hành phiên bản mới. npm có thể yêu cầu tài khoản hiện tại xác thực trình duyệt / 2FA khi thiết lập, GitHub Secrets không thay thế được. Nếu gói đầu vào có scope chưa tồn tại, cần phát hành cục bộ một lần theo phần dưới rồi mới cấu hình Trusted Publisher. Liên kết này chỉ xử lý xác thực, không miễn duyệt tên gói và chống spam npm.

### Bộ nhớ đệm biên dịch

Biên dịch CLI lưu riêng công cụ Xmake và phụ thuộc native đã cài. Phụ thuộc native dùng thư mục tạm runner do `XMAKE_PKG_INSTALLDIR` chỉ định, không lưu cấu hình `.xmake` hay sản phẩm `build` dự án. Khóa đệm gồm loại runner, kiến trúc đích, phiên bản ảnh, Xmake và nội dung cấu hình, công thức phụ thuộc, bản vá; không gồm số lần chạy, không mất hiệu lực vì sửa C++ CLI, README hay siêu dữ liệu npm. Cập nhật ảnh có thể gồm trình biên dịch／SDK, nên sẽ tạo lại đệm; nếu cấu hình phụ thuộc đổi, chỉ nhận khóa đầy đủ mới, không dùng lại thư viện công thức cũ.

`Restore native dependencies` hiển thị có trúng bộ đệm không. `xmake f` từng nền tảng cài phụ thuộc thành công xong là chạy `Save native dependencies` ngay; CLI biên dịch, ký hay đóng gói thất bại sau đó vẫn tái dùng lần sau; cài phụ thuộc thất bại không lưu đệm thiếu. Linux chỉ cập nhật và cài APT cho công cụ hệ thống thiếu trong ảnh. Lần đầu, đệm bị xóa, ảnh/cấu hình phụ thuộc đổi vẫn cần tải／biên dịch; trúng đệm không có nghĩa bỏ biên dịch và đóng gói CLI.

### Phát hành cục bộ lần đầu và khôi phục lỗi

Trước dùng `@chunkdash` lần đầu, xác nhận tài khoản npm thuộc tổ chức và có quyền phát hành gói đích. Phát hành cục bộ cần 2FA đã bật hoặc xác thực phát hành theo yêu cầu npm; vai trò owner tổ chức không thay xác thực phát hành.

```sh
npm whoami --registry=https://registry.npmjs.org/
npm org ls chunkdash --json --registry=https://registry.npmjs.org/
```

Biên dịch lại gói cả năm nền tảng trước; tarball `crossdesk-cli-<nền-tảng>` cũ không tự thành gói có scope, chỉ đổi tên tệp không đổi tên trong gói. Gói nền tảng cũ đã phát hành giữ nguyên, đầu vào mới chỉ phụ thuộc gói `@chunkdash`. Mỗi lần phát hành cập nhật đồng bộ `apps/cli/package.json` và `apps/cli/xmake.lua`; nếu một gói đích đã phát hành phiên bản đó, không được ghi đè bằng nội dung mới.

Nếu năm gói nền tảng `@chunkdash/crossdesk-cli-*` phiên bản `0.1.0` đã phát hành, lần này chỉ chuyển đầu vào `crossdesk-cli` sang `@chunkdash/crossdesk-cli`, có thể giữ `0.1.0` cho đầu vào mới, phụ thuộc năm phiên bản nền tảng cũ. Xác nhận tính toàn vẹn các phiên bản nền tảng trong registry rồi chỉ phát hành `chunkdash-crossdesk-cli-0.1.0.tgz` mới; không phát hành lại phiên bản nền tảng có sẵn. Cập nhật chương trình native sau vẫn cần nâng cả bộ và biên dịch lại.

Tải và giải nén sáu tarball cùng commit, cùng phiên bản, phát hành trong thư mục đó. Dưới đây lấy 0.1.0 làm ví dụ, kiểm tra nội dung đóng gói trước, rồi lần lượt phát hành năm gói nền tảng vào `next`:

```sh
(
  set -e
  for target in darwin-arm64 darwin-x64 linux-arm64 linux-x64 windows-x64; do
    npm publish "./chunkdash-crossdesk-cli-${target}-0.1.0.tgz" \
      --dry-run --ignore-scripts --access public --tag next --registry=https://registry.npmjs.org/
  done
  npm publish ./chunkdash-crossdesk-cli-0.1.0.tgz \
    --dry-run --ignore-scripts --access public --tag next --registry=https://registry.npmjs.org/
)
```

`--dry-run` chỉ diễn tập phát hành cục bộ, không chứng minh registry chấp nhận tên gói. Kiểm tra xong thì phát hành gói nền tảng:

```sh
(
  set -e
  for target in darwin-arm64 darwin-x64 linux-arm64 linux-x64 windows-x64; do
    npm publish "./chunkdash-crossdesk-cli-${target}-0.1.0.tgz" \
      --ignore-scripts --access public --tag next --registry=https://registry.npmjs.org/
  done
)
```

Nếu npm trả đang xử lý (HTTP 202), chờ truy vấn được đúng phiên bản, không tải lặp ngay. Nếu gặp `403 Package name triggered spam detection`, dừng, giữ gói thành công, gửi tên gói, địa chỉ kho, phiên bản, nhật ký lỗi cho [hỗ trợ npm](https://www.npmjs.com/support) để xin xem xét; không tự thử lại cả lượt. Khôi phục bằng truy vấn đúng phiên bản mỗi gói trước, so `dist.integrity` registry với `integrity` của `npm pack <tep-cuc-bo.tgz> --dry-run --json --ignore-scripts`, chỉ phát hành bù gói chưa thành công; cùng phiên bản khác nội dung thì dừng và nâng cả bộ.

Chỉ phát hành đầu vào chung sau khi truy vấn được mọi gói native và tính toàn vẹn khớp:

```sh
(
  set -e
  for target in darwin-arm64 darwin-x64 linux-arm64 linux-x64 windows-x64; do
    npm view "@chunkdash/crossdesk-cli-${target}@0.1.0" version dist.integrity \
      --registry=https://registry.npmjs.org/
  done
  npm publish ./chunkdash-crossdesk-cli-0.1.0.tgz \
    --ignore-scripts --access public --tag next --registry=https://registry.npmjs.org/
)
```

Xác nhận lại phụ thuộc và tính toàn vẹn `@chunkdash/crossdesk-cli@0.1.0`, kiểm chứng cài trên năm nền tảng bằng `npx @chunkdash/crossdesk-cli@0.1.0 doctor --json`; kết nối tương tác vẫn cần nghiệm thu máy thật. Sau nghiệm thu, trỏ `latest` của sáu gói vào bản này theo nhu cầu. Không chỉ dựa `--tag next` mà cho rằng gói phát hành lần đầu không có `latest`, nên truy vấn `dist-tags` thực tế. Nếu thiếu gói nền tảng, trình khởi chạy báo lỗi rõ, không tải chương trình nền tảng khác hay chuyển sang biên dịch nguồn.

Cơ sở: [npm bin / optionalDependencies / os / cpu](https://docs.npmjs.com/cli/v11/configuring-npm/package-json/), [npm scopes](https://docs.npmjs.com/about-scopes/), [npm publish](https://docs.npmjs.com/cli/v11/commands/npm-publish/), [Trusted Publishing](https://docs.npmjs.com/trusted-publishers/), [npx](https://docs.npmjs.com/cli/v11/commands/npx/), [nền tảng runner GitHub](https://docs.github.com/en/actions/reference/runners/github-hosted-runners), [chế độ Windows Console](https://learn.microsoft.com/en-us/windows/console/getconsolemode).
