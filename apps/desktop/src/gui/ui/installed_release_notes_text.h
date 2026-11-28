// Do fork sở hữu: chuyển Markdown release notes nhúng trong bản build thành các khối của hộp thoại
// Release Notes, hiển thị giống release_notes_text::Parse của lời nhắc cập nhật.

#ifndef CROSSDESK_INSTALLED_RELEASE_NOTES_TEXT_H_
#define CROSSDESK_INSTALLED_RELEASE_NOTES_TEXT_H_

#include <slint.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "release_notes_text.h"

namespace crossdesk::release_notes_text {

// Parse đọc từng dòng riêng lẻ và bỏ mọi dòng tiêu đề vì hộp thoại cập nhật đã có tiêu đề riêng.
// Notes nhúng đã bỏ tiêu đề `## Changes`, nên hàm này khác Parse ở hai chỗ:
// - Tiêu đề còn lại như "### Linux" là nội dung, hiện thành dòng in đậm.
// - Mọi dòng trong code fence, kể cả hai dòng fence, giữ nguyên chữ: "---", "- x" hay "# ..."
//   trong khối code không bị đọc thành đường kẻ, danh sách hay tiêu đề rồi mất đi. Dòng trống
//   trong fence thành khoảng cách như dòng trống bên ngoài.
inline std::vector<Block> InstalledBlocks(std::string_view markdown) {
  constexpr std::string_view kBlank = " \t";
  std::vector<Block> blocks;
  bool section_gap = false;
  const auto append = [&](slint::StyledText content) {
    Block block;
    block.content = std::move(content);
    block.section_gap = section_gap;
    blocks.push_back(std::move(block));
    section_gap = false;
  };
  const auto append_markdown = [&](std::string_view line) {
    if (auto parsed = slint::StyledText::from_markdown(line)) {
      append(std::move(*parsed));
    } else {
      // Cú pháp ngoài tập Markdown của Slint vẫn là nội dung, giữ nguyên chữ như Parse.
      append(slint::StyledText::from_plain_text(line));
    }
  };
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

    // Dòng trống (và đường kẻ ngang ngoài fence) thành khoảng cách giữa các khối, như Parse.
    if (line.find_first_not_of(" \t\v\f\r") == std::string_view::npos ||
        (fence_char == 0 && IsThematicBreak(line))) {
      section_gap = !blocks.empty();
      continue;
    }

    // Markdown chỉ nhận fence và tiêu đề thụt vào tối đa ba dấu cách.
    const size_t indent = line.find_first_not_of(' ');
    const std::string_view body = indent > 3 ? std::string_view{} : line.substr(indent);
    size_t run = 0;
    if (!body.empty() && (body[0] == '`' || body[0] == '~')) {
      while (run < body.size() && body[run] == body[0]) ++run;
      // Dòng mở fence bằng backtick không được chứa backtick trong phần info.
      const bool backtick_info = body[0] == '`' && fence_char == 0 &&
                                 body.find('`', run) != std::string_view::npos;
      if (run < 3 || backtick_info) run = 0;
    }

    if (fence_char != 0) {
      if (run >= fence_length && body[0] == fence_char &&
          body.find_first_not_of(kBlank, run) == std::string_view::npos) {
        fence_char = 0;
      }
      append(slint::StyledText::from_plain_text(line));
      continue;
    }
    if (run > 0) {
      fence_char = body[0];
      fence_length = run;
      append(slint::StyledText::from_plain_text(line));
      continue;
    }

    if (!body.empty() && body[0] == '#') {
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
        if (!text.empty()) append_markdown("**" + std::string(text) + "**");
        continue;
      }
    }
    append_markdown(line);
  }
  return blocks;
}

}  // namespace crossdesk::release_notes_text

#endif
