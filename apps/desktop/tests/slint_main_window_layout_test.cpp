// Do fork sở hữu: kiểm tra chữ trên cửa sổ chính ở mọi ngôn ngữ, nhiều mức scale
// DPI và vùng làm việc nhỏ, render MainWindow thật bằng software renderer. Thanh
// công cụ (thanh chuyển chế độ ID/IP, DeskHub, More) và nút Kết nối phải chừa lề
// quanh chữ; nhãn, placeholder và lời gợi ý không được chạm mép khung (Text của
// Slint cắt chữ ở mép khung); tiêu đề và phụ đề thẻ máy này không bị rút gọn bằng
// dấu "…". Khi đặt CROSSDESK_MAIN_WINDOW_EVIDENCE_DIR, test xuất thêm ảnh PPM của
// cửa sổ chính ở các trạng thái thường gặp để soát chữ tràn bằng mắt.
#include "slint_layout_test_support.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

namespace {

using namespace crossdesk::layout_test;

constexpr const char* kDisplayEnv = "CROSSDESK_MAIN_WINDOW_DISPLAY";
constexpr const char* kEvidenceEnv = "CROSSDESK_MAIN_WINDOW_EVIDENCE_DIR";

// Hình học thanh công cụ trên canvas thiết kế (khớp main_window.slint): các nút cao
// 40px, đặt cách mép trên vùng nội dung 9px.
constexpr float kToolbarTop = 9.0f;
constexpr float kToolbarHeight = 40.0f;
constexpr float kToolbarInset = 28.0f;
constexpr float kMenuWidth = 106.0f;
// Hàng quét viền nút nằm trên chữ nhưng dưới viền trên của nút.
constexpr float kProbeOffset = 4.0f;
// Khoảng trống tối thiểu giữa chữ và mép ô, đơn vị canvas.
constexpr float kLabelMargin = 8.0f;
// Chữ canh trái bị cắt ở mép khung thì nét cuối nằm sát mép; chữ trọn vẹn dừng
// trước mép ít nhất chừng này.
constexpr float kClipMargin = 4.0f;
// Chữ wrap: word-wrap tự xuống dòng ở ranh giới từ nên một dòng vừa khít có thể dừng rất
// sát mép mà không mất nét nào; chỉ coi là bị cắt khi nét chạm hẳn mép khung.
constexpr float kWrapMargin = 1.0f;

// Hàng nhập ID trong hero, tính từ mép trên vùng nội dung (khớp main_window.slint).
constexpr float kHeroLeft = 28.0f;
constexpr float kHeroTop = 70.0f;
constexpr float kConnectRowTop = kHeroTop + 126.0f;
constexpr float kConnectRowHeight = 60.0f;
constexpr float kConnectMinWidth = 188.0f;
constexpr float kConnectGap = 8.0f;
constexpr float kRemoteLabelLeft = kHeroLeft + 16.0f;
constexpr float kRemoteLabelWidth = 150.0f;
constexpr float kRemoteLabelTop = kHeroTop + 132.0f;
constexpr float kRemoteLabelHeight = 52.0f;
constexpr float kInputLeft = kHeroLeft + 174.0f;
constexpr float kInputPadding = 12.0f;

// Thẻ máy này nằm sát mép phải hero; danh sách gần đây chiếm phần còn lại bên trái.
constexpr float kLocalCardWidth = 330.0f;
constexpr float kLocalCardTop = kHeroTop + 252.0f;
constexpr float kLocalTextInset = 16.0f;
constexpr float kLocalTitleSize = 25.0f;
constexpr float kLocalSubtitleSize = 11.0f;
// Hàng nhãn địa chỉ LAN với hai nút "Đổi IP", "Đổi cổng"; nhãn luôn còn ít nhất 64px.
constexpr float kLanRowTop = 62.0f;
constexpr float kLanRowHeight = 36.0f;
constexpr float kLanLabelReserve = 64.0f;
constexpr float kLanGap = 6.0f;
constexpr float kLanLabelSize = 13.0f;
// Tiêu đề lớn của hero: cỡ chữ 62px trong khung cao 76px, ngay dưới dòng CROSSDESK.
constexpr float kHeadingTop = kHeroTop + 28.0f;
constexpr float kHeadingHeight = 76.0f;
constexpr float kHeadingSize = 62.0f;
// Khung danh sách gần đây: panel rộng bằng canvas trừ 380px, danh sách thụt 20px
// mỗi bên; thẻ trống có cột chữ bắt đầu sau biểu tượng.
constexpr float kRecentPanelLeft = 8.0f;
constexpr float kRecentPanelReserve = 380.0f;
constexpr float kRecentListInset = 20.0f;
constexpr float kEmptyCardTop = 322.0f + 50.0f + 6.0f;
constexpr float kEmptyCardHeight = 124.0f;
constexpr float kEmptyTextLeft = 84.0f;
constexpr float kEmptyTextRightInset = 20.0f;
// Tiêu đề và lời gợi ý của thẻ trống cách nhau spacing 14px nên khoảng trống giữa hai khối
// chữ luôn từ 13px trở lên, còn các dòng trong cùng một khối chỉ cách nhau 0 tới 2px.
constexpr float kEmptyParagraphGap = 8.0f;
constexpr float kEmptyHeadingSize = 14.0f;
constexpr float kEmptyHintSize = 12.0f;

const std::vector<DisplayCase>& DisplayCases() {
  static const std::vector<DisplayCase> cases = SelectDisplayCases(kDisplayEnv);
  return cases;
}

// Bốn ngôn ngữ của tiêu chí nghiệm thu: chữ ngắn, chữ có dấu, chữ dài và chữ CJK.
const std::vector<LanguageCase>& EvidenceLanguages() {
  static const std::vector<LanguageCase> languages = {
      {"en", LanguageIndex("en-US")}, {"vi", LanguageIndex("vi")},
      {"de", LanguageIndex("de")},    {"zh-Hans", LanguageIndex("zh-CN")}};
  return languages;
}

class Fixture {
 public:
  Fixture(ScaledPlatform& platform, const DisplayCase& display, int language)
      : display_(display), main_(crossdesk::ui::MainWindow::create()) {
    screen_ = platform.adapter;
    fit_scale_ = PrepareMainWindow(main_, *screen_, display, language);
    ConfigureData();
    main_->show();
    screen_->Settle();
    ui_scale_ = main_->get_ui_scale();
    titlebar_ = main_->get_design_height() - 650.0f;
  }
  ~Fixture() { main_->hide(); }

  crossdesk::ui::MainWindow& main() { return *main_; }
  ScaledWindow& screen() { return *screen_; }

  float Logical(float canvas) const { return canvas * ui_scale_; }
  float Physical(float canvas) const {
    return canvas * ui_scale_ * display_.scale * fit_scale_;
  }
  // Mép trên vùng nội dung, ngay dưới thanh tiêu đề tự vẽ (nếu có).
  float ContentTop() const { return titlebar_; }
  float DesignWidth() const { return main_->get_design_width(); }

  void Click(float canvas_x, float canvas_y) {
    const slint::LogicalPosition point({Logical(canvas_x), Logical(canvas_y)});
    main_->window().dispatch_pointer_move_event(point);
    main_->window().dispatch_pointer_press_event(point, slint::PointerEventButton::Left);
    main_->window().dispatch_pointer_release_event(point, slint::PointerEventButton::Left);
    screen_->Settle();
  }
  // Đưa con trỏ vào khoảng trống giữa thanh công cụ và hero để không dính hover.
  void ParkPointer() {
    main_->window().dispatch_pointer_move_event(slint::LogicalPosition(
        {Logical(DesignWidth() / 2), Logical(ContentTop() + 62.0f)}));
    screen_->Settle();
  }

  void SetRecentConnections(bool lan) {
    std::vector<crossdesk::ui::RecentConnection> entries;
    struct Sample { const char* id; const char* name; const char* host; bool online; };
    const Sample samples[] = {
        {"987654321", "Studio fixture workstation", "studio-fixture", true},
        {"123123123", "Office fixture", "office-fixture-laptop", false},
        {"555666777", "Lab fixture", "lab-fixture", true},
    };
    for (const auto& sample : samples) {
      crossdesk::ui::RecentConnection entry{};
      entry.remote_id = sample.id;
      entry.display_name = sample.name;
      entry.host_name = sample.host;
      entry.online = sample.online;
      entry.lan = lan;
      entry.endpoint = lan ? "192.168.1.20:52888" : "";
      entry.platform = "windows";
      entries.push_back(entry);
    }
    main_->set_recent_connections(
        std::make_shared<slint::VectorModel<crossdesk::ui::RecentConnection>>(entries));
  }
  void ClearRecentConnections() {
    main_->set_recent_connections(
        std::make_shared<slint::VectorModel<crossdesk::ui::RecentConnection>>());
  }

 private:
  void ConfigureData() {
    main_->set_ip_direct_enabled(true);
    main_->set_local_id("123 456 789");
    main_->set_local_password("A1b2C3");
    main_->set_signal_connected(true);
    main_->set_current_version("1.7.7");
    main_->set_lan_interfaces(std::make_shared<slint::VectorModel<slint::SharedString>>(
        std::vector<slint::SharedString>{"192.168.1.20", "10.0.0.8"}));
    main_->set_lan_listener_ip("192.168.1.20");
    main_->set_lan_listening(true);
    main_->set_lan_new_password("Q7r8S9");
    SetRecentConnections(false);
  }

  DisplayCase display_;
  slint::ComponentHandle<crossdesk::ui::MainWindow> main_;
  ScaledWindow* screen_ = nullptr;
  float fit_scale_ = 1.0f;
  float ui_scale_ = 1.0f;
  float titlebar_ = 0.0f;
};

// Viền nút Postcard (#8f9088) là màu xám trung tính; nền canvas, nền ô và màu
// nhấn (#bd4118) đều không thỏa điều kiện này.
bool OutlineGray(const slint::Rgb8Pixel& pixel) {
  const int r = pixel.r, g = pixel.g, b = pixel.b;
  return r < 200 && std::abs(r - g) < 24 && std::abs(g - b) < 24 && std::abs(r - b) < 24;
}

bool AccentFill(const slint::Rgb8Pixel& pixel) {
  return pixel.r > 150 && pixel.r - pixel.g > 80 && pixel.g - pixel.b > 10;
}

bool InkOn(const slint::Rgb8Pixel& pixel, const slint::Rgb8Pixel& background) {
  return std::abs(int(pixel.r) - int(background.r)) +
             std::abs(int(pixel.g) - int(background.g)) +
             std::abs(int(pixel.b) - int(background.b)) >
         90;
}

struct Span {
  int begin = 0;
  int end = 0;  // bao gồm cả end
  int Width() const { return end - begin + 1; }
};

// Các đoạn liên tiếp thỏa predicate trên một hàng pixel.
std::vector<Span> Runs(const ScaledWindow& screen, int y, int x0, int x1,
                       const std::function<bool(const slint::Rgb8Pixel&)>& predicate) {
  std::vector<Span> runs;
  for (int x = x0; x <= x1;) {
    if (!predicate(screen.At(x, y))) { ++x; continue; }
    Span run{x, x};
    while (x <= x1 && predicate(screen.At(x, y))) run.end = x++;
    runs.push_back(run);
  }
  return runs;
}

// Đo chữ trong một ô: tìm mọi pixel khác hẳn màu nền của ô trong vùng [x0, x1] x
// [y0, y1] và yêu cầu khối chữ cách hai mép trái, phải ít nhất kLabelMargin.
bool VerifyLabelInside(Fixture& fixture, const std::string& where, Span cell, int y0, int y1,
                       int background_y) {
  auto& screen = fixture.screen();
  const float unit = fixture.Physical(1.0f);
  const auto background = screen.At((cell.begin + cell.end) / 2, background_y);
  // Cột sát viền hay sát khối màu nhấn là pixel pha trộn khi scale lẻ, không phải chữ.
  const int edge = int(std::ceil(unit));
  int ink_left = -1, ink_right = -1;
  for (int y = y0; y <= y1; ++y)
    for (int x = cell.begin + edge; x <= cell.end - edge; ++x)
      if (InkOn(screen.At(x, y), background)) {
        if (ink_left < 0 || x < ink_left) ink_left = x;
        if (x > ink_right) ink_right = x;
      }
  if (!Check(ink_left >= 0, where + ": the label must be visible")) return false;
  const float left_gap = (ink_left - cell.begin) / unit;
  const float right_gap = (cell.end - ink_right) / unit;
  return Check(left_gap >= kLabelMargin && right_gap >= kLabelMargin,
               where + ": the label must keep " + std::to_string(int(kLabelMargin)) +
                   "px clear of both cell edges (left " + std::to_string(left_gap) +
                   ", right " + std::to_string(right_gap) + ", cell " +
                   std::to_string(cell.Width() / unit) + ")");
}

// Khối chữ (mọi pixel khác hẳn màu nền) trong một vùng chữ nhật pixel vật lý.
struct InkBox {
  int left = -1, right = -1, top = -1, bottom = -1;
  bool Found() const { return left >= 0; }
};

InkBox FindInk(const ScaledWindow& screen, int x0, int x1, int y0, int y1,
               const slint::Rgb8Pixel& background) {
  InkBox box;
  for (int y = y0; y <= y1; ++y)
    for (int x = x0; x <= x1; ++x)
      if (InkOn(screen.At(x, y), background)) {
        if (!box.Found() || x < box.left) box.left = x;
        if (x > box.right) box.right = x;
        if (box.top < 0) box.top = y;
        box.bottom = y;
      }
  return box;
}

int Px(const Fixture& fixture, float canvas) { return int(std::lround(fixture.Physical(canvas))); }

// Các dải hàng có nét chữ trong vùng [x0, x1] x [y0, y1], tách nhau bởi ít nhất `min_gap`
// hàng trống liên tiếp. Với min_gap = 1 mỗi dải thường là một dòng chữ; hai dòng có dấu
// chồng cao sát nhau có thể dính thành một dải.
std::vector<Span> InkRowBands(const ScaledWindow& screen, int x0, int x1, int y0, int y1,
                              const slint::Rgb8Pixel& background, int min_gap) {
  std::vector<Span> bands;
  int blank = 0;
  for (int y = y0; y <= y1; ++y) {
    bool ink = false;
    for (int x = x0; x <= x1 && !ink; ++x) ink = InkOn(screen.At(x, y), background);
    if (!ink) { ++blank; continue; }
    if (bands.empty() || blank >= min_gap) bands.push_back({y, y});
    else bands.back().end = y;
    blank = 0;
  }
  return bands;
}

// Dấu "…" của chữ bị rút gọn là ba chấm nhỏ cách đều nhau, nằm cùng một dải hàng ở cuối
// dòng, phía trên chúng không có nét nào. Chữ kết thúc bằng một dấu chấm thường (ko) chỉ
// có một chấm như vậy, còn nét chữ cái luôn cao hơn dải chấm.
bool EndsWithEllipsis(const ScaledWindow& screen, int x0, int x1, int y0, int y1,
                      const slint::Rgb8Pixel& background, float font_px) {
  const auto column_has_ink = [&](int x, int top, int bottom) {
    for (int y = top; y <= bottom; ++y)
      if (InkOn(screen.At(x, y), background)) return true;
    return false;
  };
  int right = x1;
  while (right >= x0 && !column_has_ink(right, y0, y1)) --right;
  if (right < x0) return false;
  int x = right;
  while (x - 1 >= x0 && column_has_ink(x - 1, y0, y1)) --x;
  int low = y1, high = y0;
  for (int y = y0; y <= y1; ++y)
    for (int column = x; column <= right; ++column)
      if (InkOn(screen.At(column, y), background)) {
        low = std::min(low, y);
        high = std::max(high, y);
      }
  const float dot_limit = std::max(2.0f, 0.25f * font_px);
  if (high - low + 1 > dot_limit) return false;
  // Chấm luôn có khoảng trống phía trên trong vùng quét; khối chạm hàng đầu của vùng là phần
  // đáy của một chữ cái bị vùng quét cắt ngang.
  if (low <= y0) return false;
  // Tách khối theo cột chỉ trong dải hàng của chấm, để móc của chữ cái đứng trước
  // (chữ "r" trong "Ver…") không dính vào chấm đầu tiên.
  const int band_top = std::max(y0, low - 1);
  const int band_bottom = std::min(y1, high + 1);
  const auto ink_above = [&](int column) {
    return band_top - 1 >= y0 && column_has_ink(column, y0, band_top - 1);
  };
  // Hai chấm cuối luôn tách rời nhau và khỏi chữ cái.
  Span dots[2];
  int found = 0;
  for (x = right; x >= x0 && found < 2;) {
    if (!column_has_ink(x, band_top, band_bottom)) { --x; continue; }
    const int end = x;
    while (x - 1 >= x0 && column_has_ink(x - 1, band_top, band_bottom)) --x;
    if (end - x + 1 > std::max(2.0f, 0.3f * font_px)) return false;
    for (int column = x; column <= end; ++column)
      if (ink_above(column)) return false;
    dots[found++] = {x, end};
    --x;
  }
  if (found < 2) return false;
  // Chấm đầu có thể dính vào móc chân của chữ cái đứng trước trong dải chấm (chữ "t"
  // trong "Port…"), nên chỉ xét cột giữa của nó, cách chấm giữa đúng một bước như giữa
  // hai chấm cuối: cột đó có nét trong dải chấm và không có nét phía trên. Cho lệch một
  // cột vì vị trí chấm được làm tròn tới pixel.
  const int pitch = dots[0].begin - dots[1].begin;
  const int center = (dots[1].begin + dots[1].end) / 2 - pitch;
  for (int column = center - 1; column <= center + 1; ++column)
    if (column >= x0 && column_has_ink(column, band_top, band_bottom) && !ink_above(column))
      return true;
  return false;
}

// Chữ word-wrap có overflow: elide kết thúc dòng cuối bằng "…" khi hết chỗ. Xét từng dòng
// (dải hàng có nét) vì dòng trên có thể dài hơn dòng cuối. Chân chữ dòng trên có thể chạm
// dấu của dòng dưới làm hai dòng dính thành một dải; khi đó xét thêm phần đáy cao 0.6em của
// dải. Dấu "…" nằm trên đường chân chữ, cách đáy dải nhiều nhất chừng 0.25em (phần chân của
// g, p, y), còn chân chữ dòng trên dừng cao hơn đường chân chữ dòng dưới ít nhất chừng 0.9em
// nên không lọt vào phần đáy đó và không che được dấu "…" của một dòng cuối ngắn hơn.
// Khi xét cả dòng, vùng quét lấy thêm hàng trống ngay trên dải: dòng chỉ còn mỗi dấu "…" có
// chấm chạm hàng đầu của dải, và EndsWithEllipsis sẽ coi đó là đáy chữ cái bị cắt.
bool LinesEndWithEllipsis(const ScaledWindow& screen, int x0, int x1, int y0, int y1,
                          const slint::Rgb8Pixel& background, float font_px) {
  bool elided = false;
  for (const Span& line : InkRowBands(screen, x0, x1, y0, y1, background, 1)) {
    elided |= EndsWithEllipsis(screen, x0, x1, std::max(y0, line.begin - 1), line.end,
                               background, font_px);
    if (line.Width() > 1.5f * font_px)
      elided |= EndsWithEllipsis(screen, x0, x1, line.end - int(std::lround(0.6f * font_px)),
                                 line.end, background, font_px);
  }
  return elided;
}

// Một khối chữ (dải hàng do InkRowBands tách với khoảng cách đoạn) có kết thúc bằng "…" không.
// Vùng quét bắt đầu từ hàng trống ngay trên khối, không vượt mép trên `area_top` của vùng đã
// tách khối: khối chỉ còn mỗi dấu "…" có chấm nằm ở hàng đầu khối, và LinesEndWithEllipsis
// không nhìn lên quá mép trên vùng quét nên sẽ coi chấm đó là đáy chữ cái bị cắt. Hàng đó luôn
// trống khi khối không chạm mép trên vùng, vì InkRowBands tách khối ngay tại hàng có nét đầu.
bool BlockEndsWithEllipsis(const ScaledWindow& screen, int x0, int x1, int area_top,
                           const Span& block, const slint::Rgb8Pixel& background, float font_px) {
  return LinesEndWithEllipsis(screen, x0, x1, std::max(area_top, block.begin - 1), block.end,
                              background, font_px);
}

// Tự kiểm bộ dò "…" trên hình vẽ tay: hai dòng dính nhau, chân chữ dòng trên nằm bên phải
// dấu "…" của dòng cuối ngắn hơn. Không có dấu "…" thì bộ dò không được báo nhầm. Dòng cuối
// chỉ còn mỗi dấu "…" (không có chữ cái nào vừa chỗ) cũng phải bị phát hiện. Phần sau kiểm
// đúng đường quét của thẻ trống: vùng bị cắt (không bắt đầu ở hàng 0), tách thành hai khối
// theo khoảng cách đoạn, và một trong hai khối chỉ còn mỗi dấu "…".
bool VerifyEllipsisDetector() {
  static constexpr int kWidth = 120, kHeight = 32;
  static constexpr float kFontPx = 13.0f;
  const slint::Rgb8Pixel paper{255, 255, 255};
  const auto detect = [&paper](bool lower_text, bool with_dots) {
    ScaledWindow screen;
    screen.extent = slint::PhysicalSize({kWidth, kHeight});
    screen.pixels.assign(std::size_t(kWidth) * kHeight, paper);
    const auto draw = [&screen](int x0, int x1, int y0, int y1) {
      for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) screen.pixels[std::size_t(y) * kWidth + x] = {0, 0, 0};
    };
    draw(2, 90, 2, 11);    // thân chữ dòng trên
    draw(88, 90, 12, 15);  // chân chữ dòng trên, ở bên phải dấu "…"
    if (lower_text) {
      draw(10, 12, 14, 16);  // dấu của dòng cuối chạm chân chữ dòng trên
      draw(2, 50, 17, 26);   // thân chữ dòng cuối, đường chân chữ ở hàng 26
    }
    if (with_dots)
      for (const int x : {54, 58, 62}) draw(x, x + 1, 25, 26);
    return LinesEndWithEllipsis(screen, 0, kWidth - 1, 0, kHeight - 1, paper, kFontPx);
  };
  bool ok = Check(detect(true, true),
                  "The ellipsis detector must find a trailing ellipsis under a merged line");
  ok &= Check(!detect(true, false),
              "The ellipsis detector must not report an ellipsis that is not there");
  ok &= Check(detect(false, true),
              "The ellipsis detector must find a last line that holds only an ellipsis");

  // Thẻ giả: vùng tách khối là hàng 4 tới 47, giống VerifyEmptyState quét từ mép trên thẻ chứ
  // không từ đầu ảnh. dots_first đặt khối chỉ có "…" lên trên (tiêu đề), còn không thì xuống
  // dưới (lời gợi ý); khối còn lại là chữ thường không có "…".
  static constexpr int kAreaTop = 4, kAreaBottom = 47, kGap = 8;
  const auto detect_blocks = [&paper](bool dots_first) {
    ScaledWindow screen;
    screen.extent = slint::PhysicalSize({kWidth, kAreaBottom + 1});
    screen.pixels.assign(std::size_t(kWidth) * (kAreaBottom + 1), paper);
    const auto draw = [&screen](int x0, int x1, int y0, int y1) {
      for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) screen.pixels[std::size_t(y) * kWidth + x] = {0, 0, 0};
    };
    const int dots_top = dots_first ? 10 : 34;
    for (const int x : {54, 58, 62}) draw(x, x + 1, dots_top, dots_top + 1);
    if (dots_first) draw(2, 60, 22, 31);
    else draw(2, 60, 10, 19);
    const auto blocks = InkRowBands(screen, 0, kWidth - 1, kAreaTop, kAreaBottom, paper, kGap);
    std::vector<bool> elided;
    for (const Span& block : blocks)
      elided.push_back(BlockEndsWithEllipsis(screen, 0, kWidth - 1, kAreaTop, block, paper,
                                             kFontPx));
    return elided;
  };
  ok &= Check(detect_blocks(true) == std::vector<bool>{true, false},
              "The ellipsis detector must find an empty-state heading that holds only an "
              "ellipsis");
  ok &= Check(detect_blocks(false) == std::vector<bool>{false, true},
              "The ellipsis detector must find an empty-state hint that holds only an ellipsis");
  return ok;
}

// Chữ một dòng có overflow: elide: phải hiện ra, không kết thúc bằng "…" và không bị
// cắt ở mép phải khung.
bool VerifySingleLine(Fixture& fixture, const std::string& where, float left, float right,
                      float top, float bottom, const slint::Rgb8Pixel& background,
                      float font_size) {
  auto& screen = fixture.screen();
  const float unit = fixture.Physical(1.0f);
  const int x0 = Px(fixture, left), x1 = Px(fixture, right) - 1;
  const int y0 = Px(fixture, top), y1 = Px(fixture, bottom) - 1;
  const auto ink = FindInk(screen, x0, x1, y0, y1, background);
  if (!Check(ink.Found(), where + ": the text must be visible")) return false;
  bool ok = Check(!EndsWithEllipsis(screen, x0, x1, y0, y1, background, font_size * unit),
                  where + ": the text must not be elided");
  const float gap = (x1 - ink.right) / unit;
  ok &= Check(gap >= kClipMargin, where + ": the text must end " +
                                      std::to_string(int(kClipMargin)) +
                                      "px before the right edge of its box (gap " +
                                      std::to_string(gap) + ")");
  return ok;
}

// Chữ canh trái trong khung [left, right) x [top, bottom) phải hiện ra và dừng trước
// mép phải ít nhất `margin` (canvas). Text của Slint mặc định cắt chữ tại mép khung,
// nên chữ bị cắt có nét chạm sát mép; chữ có overflow: elide dùng VerifySingleLine.
// Chữ canh giữa theo chiều dọc mà xuống quá nhiều dòng thì tràn cả mép trên lẫn mép
// dưới, nên nét chữ cũng không được nằm trên hàng pixel đầu hay cuối của khung.
bool VerifyTextEndsInside(Fixture& fixture, const std::string& where, float left, float right,
                          float top, float bottom, const slint::Rgb8Pixel& background,
                          float margin) {
  auto& screen = fixture.screen();
  const float unit = fixture.Physical(1.0f);
  const int y0 = Px(fixture, top), y1 = Px(fixture, bottom) - 1;
  const auto ink = FindInk(screen, Px(fixture, left), Px(fixture, right) - 1, y0, y1, background);
  if (!Check(ink.Found(), where + ": the text must be visible")) return false;
  const float gap = (Px(fixture, right) - 1 - ink.right) / unit;
  bool ok = Check(gap >= margin, where + ": the text must end " + std::to_string(int(margin)) +
                                     "px before the right edge of its box (gap " +
                                     std::to_string(gap) + ")");
  ok &= Check(ink.top > y0 && ink.bottom < y1,
              where + ": the text must not run past the top or bottom of its box");
  return ok;
}

// Nút Kết nối nằm sát mép phải hero; quét từ mép đó sang trái tới khi gặp lại màu nền hero.
Span FindConnectButton(Fixture& fixture, const slint::Rgb8Pixel& hero_background) {
  auto& screen = fixture.screen();
  const int y = Px(fixture, fixture.ContentTop() + kConnectRowTop + kProbeOffset);
  const int right = Px(fixture, fixture.DesignWidth() - kHeroLeft) - 1;
  int x = Px(fixture, fixture.DesignWidth() - kHeroLeft - 2.0f);
  while (x > 0 && InkOn(screen.At(x - 1, y), hero_background)) --x;
  return {x, right};
}

bool VerifyConnectButton(Fixture& fixture, const std::string& where,
                         const slint::Rgb8Pixel& hero_background, Span* button_out = nullptr) {
  const float unit = fixture.Physical(1.0f);
  const float top = fixture.ContentTop() + kConnectRowTop;
  const Span button = FindConnectButton(fixture, hero_background);
  if (button_out) *button_out = button;
  bool ok = Check(button.Width() / unit >= kConnectMinWidth - 1.5f,
                  where + ": the Connect button must keep at least its design width");
  ok &= VerifyLabelInside(fixture, where + " Connect", button,
                          int(std::ceil(fixture.Physical(top + 3.0f))),
                          int(std::floor(fixture.Physical(top + kConnectRowHeight - 3.0f))),
                          Px(fixture, top + kProbeOffset));
  return ok;
}

// Hàng địa chỉ LAN: hai nút có viền nằm sát mép phải thẻ, chữ trong nút chừa lề, và
// nhãn bên trái vẫn hiện ra trọn vẹn (không có "…") với ít nhất kLanLabelReserve.
bool VerifyLanRow(Fixture& fixture, const std::string& where, float card_left, float card_top,
                  const slint::Rgb8Pixel& background) {
  auto& screen = fixture.screen();
  const float unit = fixture.Physical(1.0f);
  const float top = card_top + kLanRowTop;
  const float text_left = card_left + kLocalTextInset;
  const float text_right = card_left + kLocalCardWidth - kLocalTextInset;
  const int probe_y = Px(fixture, top + kProbeOffset);
  // Hàng dò nằm dưới viền trên của nút và trên chữ; nhãn bên trái (nếu chạm tới) nằm
  // ngoài bốn đoạn viền ngoài cùng bên phải.
  const auto runs = Runs(screen, probe_y, Px(fixture, text_left), Px(fixture, text_right + 1.0f),
                         OutlineGray);
  if (!Check(runs.size() >= 4, where + ": the LAN row must show two outlined buttons"))
    return false;
  const Span switch_ip{runs[runs.size() - 4].begin, runs[runs.size() - 3].end};
  const Span edit_port{runs[runs.size() - 2].begin, runs[runs.size() - 1].end};
  const int text_top = int(std::ceil(fixture.Physical(top + 2.0f)));
  const int text_bottom = int(std::floor(fixture.Physical(top + kLanRowHeight - 2.0f)));
  bool ok = VerifyLabelInside(fixture, where + " Switch IP", switch_ip, text_top, text_bottom,
                              probe_y);
  ok &= VerifyLabelInside(fixture, where + " Edit port", edit_port, text_top, text_bottom,
                          probe_y);
  const float label_width = switch_ip.begin / unit - kLanGap - text_left;
  ok &= Check(label_width >= kLanLabelReserve - 1.5f,
              where + ": the LAN address label must keep " +
                  std::to_string(int(kLanLabelReserve)) + "px beside the buttons (width " +
                  std::to_string(label_width) + ")");
  const int label_x0 = Px(fixture, text_left), label_x1 = switch_ip.begin - 1;
  const int label_y0 = Px(fixture, top), label_y1 = Px(fixture, top + kLanRowHeight) - 1;
  const auto label = FindInk(screen, label_x0, label_x1, label_y0, label_y1, background);
  ok &= Check(label.Found(), where + ": the LAN address label must be visible");
  // Nhãn có word-wrap và overflow: elide: khi hai dòng không đủ chỗ thì dòng cuối kết thúc
  // bằng "…".
  ok &= Check(!LinesEndWithEllipsis(screen, label_x0, label_x1, label_y0, label_y1, background,
                                    kLanLabelSize * unit),
              where + ": the LAN address label must not be elided");
  return ok;
}

// Thẻ máy này: tiêu đề và phụ đề một dòng không bị rút gọn hay cắt; ở chế độ IP thì
// hàng địa chỉ LAN cũng vừa thẻ.
bool VerifyLocalCard(Fixture& fixture, const std::string& where, bool lan_mode) {
  auto& screen = fixture.screen();
  const float top = fixture.ContentTop() + kLocalCardTop;
  const float left = fixture.DesignWidth() - kHeroLeft - kLocalCardWidth;
  const float text_left = left + kLocalTextInset;
  const float text_right = left + kLocalCardWidth - kLocalTextInset;
  const auto background = screen.At(Px(fixture, left + 6.0f), Px(fixture, top + 6.0f));
  // Chỉ quét thân chữ tiêu đề (tới quanh đường chân chữ, nơi có dấu "…"): phụ đề CJK
  // ngay bên dưới có nét cao chạm lên vài hàng cuối của khung tiêu đề.
  bool ok = VerifySingleLine(fixture, where + " local card title", text_left, text_right,
                             top + 14.0f, top + 40.0f, background, kLocalTitleSize);
  // Bỏ hai hàng pixel đầu của phụ đề vì chân chữ tiêu đề có thể chạm tới đó.
  ok &= VerifySingleLine(fixture, where + " local card subtitle", text_left, text_right,
                         top + 45.0f, top + 61.0f, background, kLocalSubtitleSize);
  if (lan_mode) ok &= VerifyLanRow(fixture, where, left, top, background);
  return ok;
}

// Thẻ trống của danh sách gần đây: tiêu đề và lời gợi ý đều hiện ra, xuống dòng trong
// cột chữ, không chạm mép phải cột và không lọt ra dưới đáy thẻ.
bool VerifyEmptyState(Fixture& fixture, bool lan_mode, int group, const std::string& label) {
  auto& screen = fixture.screen();
  fixture.main().set_lan_mode(lan_mode);
  fixture.main().set_recent_group_index(group);
  fixture.ParkPointer();
  const std::string where = label + (lan_mode ? " IP mode" : " ID mode") + " empty group " +
                            std::to_string(group);
  const float top = fixture.ContentTop() + kEmptyCardTop;
  const float list_left = kRecentPanelLeft + kRecentListInset;
  const float list_right =
      kRecentPanelLeft + fixture.DesignWidth() - kRecentPanelReserve - kRecentListInset;
  const float text_left = list_left + kEmptyTextLeft;
  const float text_right = list_right - kEmptyTextRightInset;
  const auto background = screen.At(Px(fixture, text_right + 4.0f), Px(fixture, top + 6.0f));
  bool ok = VerifyTextEndsInside(fixture, where + " empty state", text_left, text_right, top,
                                 top + kEmptyCardHeight, background, kWrapMargin);
  // Chữ phải nằm gọn trong thẻ, chừa hàng viền dưới.
  const auto ink = FindInk(screen, Px(fixture, text_left), Px(fixture, text_right) - 1,
                           Px(fixture, top), Px(fixture, top + kEmptyCardHeight + 12.0f),
                           background);
  ok &= Check(ink.Found() && ink.bottom < Px(fixture, top + kEmptyCardHeight - 4.0f),
              where + ": the empty state text must stay inside its card");
  // Kiểm tra gộp ở trên vẫn đạt khi chỉ còn tiêu đề, nên lời gợi ý phải hiện thành một
  // khối chữ riêng bên dưới tiêu đề.
  const auto blocks = InkRowBands(screen, Px(fixture, text_left), Px(fixture, text_right) - 1,
                                  Px(fixture, top), Px(fixture, top + kEmptyCardHeight) - 1,
                                  background, Px(fixture, kEmptyParagraphGap));
  ok &= Check(blocks.size() >= 2, where + ": the empty state hint must show below its heading");
  // Hai khối chữ chỉ có word-wrap nên Slint không vẽ "…"; vẫn xét để test bắt được khi ai đó
  // thêm overflow: elide mà khung không đủ chỗ, kể cả khi khối chỉ còn mỗi dấu "…".
  for (std::size_t i = 0; i < blocks.size() && i < 2; ++i) {
    const float font_px = fixture.Physical(i == 0 ? kEmptyHeadingSize : kEmptyHintSize);
    ok &= Check(!BlockEndsWithEllipsis(screen, Px(fixture, text_left),
                                       Px(fixture, text_right) - 1, Px(fixture, top), blocks[i],
                                       background, font_px),
                where + (i == 0 ? ": the empty state heading" : ": the empty state hint") +
                    " must not be elided");
  }
  if (!ok) WriteFailureFrame(screen, kEvidenceEnv, where);
  return ok;
}

// Hàng nhập ID của hero: nút Kết nối chừa lề quanh chữ, nhãn bên trái và
// placeholder trong ô nhập không chạm mép khung. button_out nhận vị trí nút để so với
// trạng thái đang kết nối.
bool VerifyConnectRow(Fixture& fixture, bool lan_mode, const std::string& label,
                      Span* button_out = nullptr) {
  auto& screen = fixture.screen();
  fixture.main().set_lan_mode(lan_mode);
  fixture.ParkPointer();
  const float top = fixture.ContentTop();
  const std::string where = label + (lan_mode ? " IP mode" : " ID mode");
  // Khoảng trống giữa tiêu đề lớn và hàng nhập, bên trái nhãn.
  const auto hero_background = screen.At(Px(fixture, kHeroLeft + 8.0f),
                                         Px(fixture, top + kConnectRowTop - 6.0f));
  Span button{};
  bool ok = VerifyConnectButton(fixture, where, hero_background, &button);
  if (button_out) *button_out = button;
  ok &= VerifyTextEndsInside(fixture, where + " remote address label", kRemoteLabelLeft,
                             kRemoteLabelLeft + kRemoteLabelWidth, top + kRemoteLabelTop,
                             top + kRemoteLabelTop + kRemoteLabelHeight, hero_background,
                             kWrapMargin);
  const float input_right = button.begin / fixture.Physical(1.0f) - kConnectGap;
  const auto field_background = screen.At(Px(fixture, kInputLeft + 4.0f),
                                          Px(fixture, top + kConnectRowTop + 6.0f));
  ok &= VerifyTextEndsInside(fixture, where + " placeholder", kInputLeft + kInputPadding,
                             input_right - kInputPadding, top + kConnectRowTop + 4.0f,
                             top + kConnectRowTop + kConnectRowHeight - 4.0f, field_background,
                             kClipMargin);
  // Nền hero giữa dòng CROSSDESK và tiêu đề lớn.
  const auto heading_background =
      screen.At(Px(fixture, kHeroLeft + 4.0f), Px(fixture, top + kHeroTop + 24.0f));
  ok &= VerifySingleLine(fixture, where + " heading", kHeroLeft,
                         fixture.DesignWidth() - kHeroLeft, top + kHeadingTop,
                         top + kHeadingTop + kHeadingHeight, heading_background, kHeadingSize);
  ok &= VerifyLocalCard(fixture, where, lan_mode);
  if (!ok) WriteFailureFrame(screen, kEvidenceEnv, where + " connect row");
  return ok;
}

// Nhãn "Đang kết nối…" thay cho "Kết nối" khi nút bị khóa trong lúc chờ: connection-pending
// ở chế độ ID, lan-connecting ở chế độ IP. Nút rộng sẵn cho nhãn dài hơn trong hai nhãn, nên
// bấm Kết nối không được làm nút đổi bề rộng; mép phải cố định nên mép trái cũng phải đứng
// yên. Chừa 1px cho khử răng cưa ở mép.
bool VerifyConnecting(Fixture& fixture, bool lan_mode, const Span& idle_button,
                      const std::string& label) {
  fixture.main().set_lan_mode(lan_mode);
  if (lan_mode)
    fixture.main().set_lan_connecting(true);
  else
    fixture.main().set_connection_pending(true);
  fixture.ParkPointer();
  const std::string where = label + (lan_mode ? " IP mode" : " ID mode") + " connecting";
  const auto hero_background = fixture.screen().At(
      Px(fixture, kHeroLeft + 8.0f), Px(fixture, fixture.ContentTop() + kConnectRowTop - 6.0f));
  Span button{};
  bool ok = VerifyConnectButton(fixture, where, hero_background, &button);
  ok &= Check(std::abs(button.begin - idle_button.begin) <= 1 &&
                  std::abs(button.end - idle_button.end) <= 1,
              where + ": the Connect button must keep its width (idle " +
                  std::to_string(idle_button.Width()) + "px, connecting " +
                  std::to_string(button.Width()) + "px)");
  if (!ok) WriteFailureFrame(fixture.screen(), kEvidenceEnv, where);
  fixture.main().set_lan_connecting(false);
  fixture.main().set_connection_pending(false);
  return ok;
}

bool VerifyToolbar(Fixture& fixture, bool lan_mode, const std::string& label) {
  auto& screen = fixture.screen();
  fixture.main().set_lan_mode(lan_mode);
  fixture.ParkPointer();
  const float unit = fixture.Physical(1.0f);
  const float top = fixture.ContentTop() + kToolbarTop;
  const int probe_y = int(std::lround(fixture.Physical(top + kProbeOffset)));
  // Chừa viền trên, viền dưới (1px) của nút khi quét chữ.
  const int text_top = int(std::ceil(fixture.Physical(top + 2.0f)));
  const int text_bottom = int(std::floor(fixture.Physical(top + kToolbarHeight - 2.0f)));
  const int scan_left = int(std::floor(fixture.Physical(kToolbarInset - 8.0f)));
  const int scan_right =
      std::min(int(std::ceil(fixture.Physical(fixture.DesignWidth() - kToolbarInset + 8.0f))),
               int(screen.extent.width) - 1);
  const std::string where = label + (lan_mode ? " IP mode" : " ID mode");

  // Sáu viền dọc theo thứ tự: thanh chuyển chế độ, DeskHub, More.
  const auto outlines = Runs(screen, probe_y, scan_left, scan_right, OutlineGray);
  if (!Check(outlines.size() == 6,
             where + ": expected the three toolbar buttons to show six vertical borders, found " +
                 std::to_string(outlines.size()))) {
    WriteFailureFrame(screen, kEvidenceEnv, where);
    return false;
  }
  const Span mode_switch{outlines[0].end + 1, outlines[1].begin - 1};
  const Span gallery{outlines[2].end + 1, outlines[3].begin - 1};
  const Span menu{outlines[4].end + 1, outlines[5].begin - 1};
  bool ok = true;
  ok &= Check(std::fabs(outlines[0].begin / unit - kToolbarInset) <= 1.5f,
              where + ": the mode switch must start at the toolbar inset");
  ok &= Check((outlines[2].begin - outlines[1].end - 1) / unit >= 6.0f,
              where + ": DeskHub must not touch the mode switch");
  // Bề rộng giữa hai viền là bề rộng nút trừ 2px viền.
  ok &= Check(menu.Width() / unit >= kMenuWidth - 2.0f - 1.5f,
              where + ": the More button must keep at least its design width");

  // Nửa đang chọn tô màu nhấn; ranh giới hai nửa là mép của khối màu nhấn.
  const auto accents = Runs(screen, probe_y, mode_switch.begin, mode_switch.end, AccentFill);
  Span accent{};
  for (const auto& run : accents)
    if (run.Width() > accent.Width()) accent = run;
  if (!Check(accent.Width() / unit >= 40.0f,
             where + ": the selected mode must be filled with the accent color")) {
    WriteFailureFrame(screen, kEvidenceEnv, where);
    return false;
  }
  Span id_half{}, ip_half{};
  if (lan_mode) {
    ip_half = {accent.begin, mode_switch.end};
    id_half = {mode_switch.begin, accent.begin - 1};
  } else {
    id_half = {mode_switch.begin, accent.end};
    ip_half = {accent.end + 1, mode_switch.end};
  }
  ok &= Check(std::abs(id_half.Width() - ip_half.Width()) <= int(std::ceil(2 * unit)) + 1,
              where + ": both mode tabs must be the same width (ID " +
                  std::to_string(id_half.Width() / unit) + ", IP " +
                  std::to_string(ip_half.Width() / unit) + ")");
  ok &= VerifyLabelInside(fixture, where + " ID tab", id_half, text_top, text_bottom, probe_y);
  ok &= VerifyLabelInside(fixture, where + " IP tab", ip_half, text_top, text_bottom, probe_y);
  ok &= VerifyLabelInside(fixture, where + " DeskHub", gallery, text_top, text_bottom, probe_y);
  ok &= VerifyLabelInside(fixture, where + " More", menu, text_top, text_bottom, probe_y);
  if (!ok) WriteFailureFrame(screen, kEvidenceEnv, where);
  return ok;
}

struct EvidenceState {
  const char* name;
  std::function<void(Fixture&)> arrange;
};

// Các trạng thái thường gặp của cửa sổ chính để soát chữ tràn bằng mắt.
const std::vector<EvidenceState>& EvidenceStates() {
  static const std::vector<EvidenceState> states = {
      {"id-mode", [](Fixture&) {}},
      {"ip-mode", [](Fixture& f) {
         f.SetRecentConnections(true);
         f.main().set_lan_mode(true);
       }},
      {"id-empty-disconnected", [](Fixture& f) {
         f.ClearRecentConnections();
         f.main().set_signal_connected(false);
       }},
      {"ip-empty", [](Fixture& f) {
         f.ClearRecentConnections();
         f.main().set_lan_mode(true);
       }},
      {"connecting", [](Fixture& f) {
         f.main().set_remote_id_input("987 654 321");
         f.main().set_connection_pending(true);
       }},
      {"id-input-error", [](Fixture& f) {
         f.main().set_remote_id_input("123 456");
         f.main().set_connection_input_error(
             "The device ID must have 9 digits. Check the ID and try again.");
       }},
      {"controlled-menu", [](Fixture& f) {
         f.main().set_controllers(
             std::make_shared<slint::VectorModel<crossdesk::ui::ControllerEntry>>(
                 std::vector<crossdesk::ui::ControllerEntry>{
                     {"987654321", "Studio fixture"}, {"123123123", "Office fixture"}}));
         f.main().set_update_available(true);
         f.screen().Settle();
         f.Click(f.DesignWidth() - kToolbarInset - kMenuWidth / 2,
                 f.ContentTop() + kToolbarTop + kToolbarHeight / 2);
       }},
  };
  return states;
}

bool WriteEvidence(ScaledPlatform& platform, const std::filesystem::path& root,
                   const DisplayCase& display, const LanguageCase& language) {
  bool ok = true;
  for (const auto& state : EvidenceStates()) {
    // Mỗi trạng thái dùng một cửa sổ mới để trạng thái trước không lọt sang.
    Fixture fixture(platform, display, language.index);
    fixture.ParkPointer();
    state.arrange(fixture);
    fixture.screen().Settle(24);
    ok &= WritePpm(fixture.screen(),
                   root / display.name / language.name / (std::string(state.name) + ".ppm"));
  }
  return ok;
}

}  // namespace

int main() {
  auto owned = std::make_unique<ScaledPlatform>();
  auto* platform = owned.get();
  slint::platform::set_platform(std::move(owned));

  // Tên cấu hình gõ sai thì danh sách rỗng: báo lỗi thay vì "đạt" mà không kiểm gì.
  if (DisplayCases().empty()) {
    ReportUnknownDisplay(kDisplayEnv);
    return 1;
  }

  bool ok = VerifyEllipsisDetector();
  for (const auto& display : DisplayCases()) {
    for (const auto& language : AllLanguages()) {
      Fixture fixture(*platform, display, language.index);
      const std::string label = std::string(display.name) + "/" + language.name;
      ok &= VerifyToolbar(fixture, false, label);
      ok &= VerifyToolbar(fixture, true, label);
      Span id_button{}, ip_button{};
      ok &= VerifyConnectRow(fixture, false, label, &id_button);
      ok &= VerifyConnectRow(fixture, true, label, &ip_button);
      ok &= VerifyConnecting(fixture, false, id_button, label);
      ok &= VerifyConnecting(fixture, true, ip_button, label);
      fixture.ClearRecentConnections();
      ok &= VerifyEmptyState(fixture, false, 0, label);
      ok &= VerifyEmptyState(fixture, false, 1, label);
      ok &= VerifyEmptyState(fixture, true, 0, label);
    }
  }
  if (ok)
    std::cout << "Main window labels fit their boxes at every scale and language.\n";

  if (const char* directory = std::getenv(kEvidenceEnv); directory && *directory) {
    const std::filesystem::path root(directory);
    for (const auto& display : DisplayCases())
      for (const auto& language : EvidenceLanguages())
        ok &= WriteEvidence(*platform, root, display, language);
    // Mọi ngôn ngữ ở cấu hình mặc định (100%) để soát chữ dài hay bị cắt, kể cả khi
    // CROSSDESK_MAIN_WINDOW_DISPLAY chọn cấu hình khác cho phần trên.
    for (const auto& language : AllLanguages())
      ok &= WriteEvidence(*platform, root / "all-languages", AllDisplayCases().front(), language);
  }
  return ok ? 0 : 1;
}
