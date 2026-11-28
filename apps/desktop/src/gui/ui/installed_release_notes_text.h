// Do fork sở hữu: chuẩn bị Markdown release notes nhúng trong bản build cho hộp thoại
// Release Notes trước khi đưa qua release_notes_text::Parse.

#ifndef CROSSDESK_INSTALLED_RELEASE_NOTES_TEXT_H_
#define CROSSDESK_INSTALLED_RELEASE_NOTES_TEXT_H_

#include <cstddef>
#include <string>
#include <string_view>

namespace crossdesk::release_notes_text {

// Parse bỏ mọi dòng tiêu đề vì hộp thoại cập nhật đã có tiêu đề riêng. Notes nhúng đã bỏ tiêu
// đề `## Changes`, nên tiêu đề còn lại như "### Linux" là nội dung: hàm đổi nó thành dòng in
// đậm. Trong code fence, dòng bắt đầu bằng '#' (ví dụ chú thích shell) được thoát thành "\#"
// để Parse giữ lại thành chữ thay vì coi là tiêu đề.
inline std::string InstalledMarkdown(std::string_view markdown) {
  constexpr std::string_view kBlank = " \t";
  std::string result;
  result.reserve(markdown.size() + 16);
  // Ký tự và độ dài của fence đang mở; 0 khi đang ở ngoài fence.
  char fence_char = 0;
  size_t fence_length = 0;
  size_t line_start = 0;
  while (line_start < markdown.size()) {
    const size_t newline = markdown.find('\n', line_start);
    const size_t line_end = newline == std::string_view::npos ? markdown.size() : newline;
    std::string_view line = markdown.substr(line_start, line_end - line_start);
    line_start = line_end + 1;
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);

    // Markdown chỉ nhận fence và tiêu đề thụt vào tối đa ba dấu cách.
    const size_t indent = line.find_first_not_of(' ');
    const std::string_view body = indent == std::string_view::npos || indent > 3
                                      ? std::string_view{}
                                      : line.substr(indent);
    size_t run = 0;
    if (!body.empty() && (body[0] == '`' || body[0] == '~')) {
      while (run < body.size() && body[run] == body[0]) ++run;
      // Dòng mở fence bằng backtick không được chứa backtick trong phần info.
      const bool backtick_info = body[0] == '`' && fence_char == 0 &&
                                 body.find('`', run) != std::string_view::npos;
      if (run < 3 || backtick_info) run = 0;
    }

    if (fence_char == 0) {
      if (run > 0) {
        fence_char = body[0];
        fence_length = run;
      } else if (!body.empty() && body[0] == '#') {
        size_t level = 0;
        while (level < body.size() && body[level] == '#') ++level;
        if (level <= 6 && (level == body.size() || kBlank.find(body[level]) != std::string_view::npos)) {
          std::string_view text = body.substr(level);
          const size_t first = text.find_first_not_of(kBlank);
          text = first == std::string_view::npos ? std::string_view{} : text.substr(first);
          text = text.substr(0, text.find_last_not_of(kBlank) + 1);
          // Bỏ chuỗi '#' đóng tiêu đề kiểu "### Linux ###".
          const size_t closing = text.find_last_not_of('#');
          if (closing == std::string_view::npos) {
            text = {};
          } else if (closing + 1 < text.size() && kBlank.find(text[closing]) != std::string_view::npos) {
            text = text.substr(0, text.find_last_not_of(kBlank, closing) + 1);
          }
          if (!text.empty()) {
            result += "**";
            result.append(text);
            result += "**\n";
          }
          continue;
        }
      }
    } else if (run >= fence_length && body[0] == fence_char &&
               body.find_first_not_of(kBlank, run) == std::string_view::npos) {
      fence_char = 0;
    } else if (!line.empty() && line[0] == '#') {
      result += '\\';
    }
    result.append(line);
    result += '\n';
  }
  return result;
}

}  // namespace crossdesk::release_notes_text

#endif
