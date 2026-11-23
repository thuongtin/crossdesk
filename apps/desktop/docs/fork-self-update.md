# CrossDesk Postcard: phát hành self-update cho macOS ARM64

- Repo duy nhất: `thuongtin/crossdesk`. Không dùng workflow phát hành, máy chủ update hoặc khóa của upstream.
- Manifest công khai:
  `https://raw.githubusercontent.com/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json`.
- Gói phát hành: GitHub Release của đúng tag `v<VERSION>` trong repo trên, với tên asset
  `crossdesk-macos-arm64-v<VERSION>.pkg`. Chỉ manifest cần nhánh riêng; mỗi asset giữ nguyên theo tag.
- Manifest không chứa khóa riêng hay token. Giữ private Ed25519 key ngoài repo; máy build nhúng đúng
  public key qua `CROSSDESK_UPDATE_PUBLIC_KEY`. Giữ nguyên key giữa các bản; mất hoặc đổi key cần
  phân phối bootstrap mới. Không dùng nhánh nội dung `desktop-updates` để chạy workflow.
- Bản .6 hoặc cũ hơn phải cài bootstrap có kênh mới một lần. Từ đó, app kiểm tra theo kênh fork,
  chỉ chấp nhận package đúng nền tảng, đúng phiên bản, chữ ký Ed25519, kích thước và SHA-256.
  macOS vẫn yêu cầu người dùng xác nhận nâng cấp và có thể hỏi quyền quản trị.

## Quy trình phát hành

1. Build bản mới với phiên bản trong app dạng `MAJOR.MINOR.PATCH-BUILD-YYYYMMDD` và public key cố
   định. Số `BUILD` phải tăng nếu giữ cùng phiên bản ngữ nghĩa; đổi ngày đơn thuần không tăng version.
   Ký Developer ID, notarize và staple gói. Giữ signaling mặc định `crossdesk.ecom.io.vn:9099`.
2. Sinh manifest trên máy ký, không truyền nội dung private key trên command line:

   ```bash
   python3 apps/desktop/scripts/macos/make_fork_update_manifest.py \
     --package "$PACKAGE" --tag "$TAG" --notes "$RELEASE_NOTES" \
     --public-key "$PUBLIC_KEY_FILE" --private-key "$PRIVATE_KEY_FILE" \
     --output crossdesk-desktop-update.json
   ```

3. Sau khi được duyệt phát hành: push source/tag đúng commit vào **repo của bạn**, tạo release từ tag đó
   và tải gói đã notarize lên. Chỉ sau khi kiểm readback tên asset và SHA-256 tại đúng tag, cập nhật
   `crossdesk-desktop-update.json` ở gốc nhánh `desktop-updates` của repo đó. Không trỏ manifest đến
   draft release. Nhánh manifest có thể chỉ chứa manifest, không kèm source hay GitHub Actions.
4. Kiểm tra máy đang dùng bản bootstrap cũ hơn: Check for updates, xem release notes, xác nhận tải/cài,
   đọc lại phiên bản và kiểm kết nối tới server nội bộ. Xác minh asset thật, không dùng binary test
   làm bằng chứng cài đặt. Không viết đè một release asset đã công bố; phát hành số build mới.
5. Nếu cần dừng cung cấp update, phục hồi commit manifest đã biết tốt. Thao tác này không tự hạ phiên
   bản đã cài vì updater chặn downgrade; cài bản phục hồi riêng khi cần.
