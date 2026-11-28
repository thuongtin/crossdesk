#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "lan_hosts_file.h"

namespace {

using crossdesk::lan::FindHostsFileAddresses;
using crossdesk::lan::SelectHostsFileAddresses;

bool Expect(bool condition, const char* message) {
  if (condition) return true;
  std::cerr << message << '\n';
  return false;
}

std::vector<std::string> Find(const std::string& hosts, const char* host) {
  std::istringstream in(hosts);
  return FindHostsFileAddresses(in, host);
}

// Thay cho asio::ip::make_address và DialableAddress của lan_transport.cpp:
// chuỗi "not-an-ip" là sai cú pháp, địa chỉ "::ffff:" là IPv4-mapped.
std::optional<std::string> ParseFake(const std::string& text) {
  if (text == "not-an-ip") return std::nullopt;
  return text;
}
bool DialableFake(const std::string& address) {
  return address.rfind("::ffff:", 0) != 0;
}
std::vector<std::string> Select(const std::string& hosts, const char* host) {
  return SelectHostsFileAddresses(Find(hosts, host), ParseFake, DialableFake);
}

}  // namespace

int main() {
  // Mẫu giống /etc/hosts thật: comment, alias, IPv6, dòng CRLF và dòng hỏng.
  const std::string hosts =
      "# local overrides\n"
      "127.0.0.1\tlocalhost\n"
      "192.168.1.20 desk.home.arpa desk   # office PC\n"
      "fd00::20     desk.home.arpa\n"
      "192.168.1.30 Studio.Local\r\n"
      "192.168.1.40 desk.example\n"
      "not-an-ip    broken.lan\n"
      "#192.168.1.50 hidden.lan\n"
      "192.168.1.20 DESK.HOME.ARPA.\n"
      "\n"
      "   \t\n"
      "192.168.1.60\n";

  bool ok = true;
  // Tên chính khớp đúng, gom mọi dòng theo thứ tự file, không trùng lặp.
  ok &= Expect(Find(hosts, "desk.home.arpa") ==
                   std::vector<std::string>{"192.168.1.20", "fd00::20"},
               "primary name must return every matching line once, in order");
  // Alias khớp, nhưng không khớp một phần với tên dài hơn.
  ok &= Expect(Find(hosts, "desk") == std::vector<std::string>{"192.168.1.20"},
               "an alias must match exactly, not as a prefix of desk.example");
  // Không phân biệt hoa thường; bỏ dấu chấm cuối ở cả hai phía; CRLF vẫn đúng.
  ok &= Expect(Find(hosts, "studio.local") ==
                   std::vector<std::string>{"192.168.1.30"},
               "names must match case-insensitively across CRLF lines");
  ok &= Expect(Find(hosts, "Desk.Home.Arpa.") ==
                   std::vector<std::string>{"192.168.1.20", "fd00::20"},
               "a trailing dot on the query must not change the match");
  // Không có search suffix: "home.arpa" không phải "desk.home.arpa".
  ok &= Expect(Find(hosts, "home.arpa").empty(),
               "a suffix of a hosts name must not match");
  // Phần sau '#' bị bỏ qua, kể cả cả dòng bị comment.
  ok &= Expect(Find(hosts, "hidden.lan").empty(),
               "commented-out entries must be ignored");
  ok &= Expect(Find(hosts, "office").empty(),
               "words inside a trailing comment must not match");
  // Chuỗi địa chỉ được trả nguyên văn; người gọi tự loại địa chỉ hỏng.
  ok &= Expect(Find(hosts, "broken.lan") ==
                   std::vector<std::string>{"not-an-ip"},
               "the address text must be returned verbatim for the caller");
  ok &= Expect(Find(hosts, "").empty() && Find(hosts, ".").empty(),
               "an empty name must never match");
  ok &= Expect(Find(hosts, "missing.lan").empty(),
               "an unknown name must return nothing");

  // Chọn địa chỉ thay cho DNS: kết quả rỗng nghĩa là vẫn hỏi resolver.
  const std::string resolver_hosts =
      "::ffff:192.168.1.70 mapped.lan\n"
      "::ffff:192.168.1.80 mixed.lan\n"
      "192.168.1.80        mixed.lan\n"
      "fd00::80            mixed.lan\n"
      "not-an-ip           broken.lan\n"
      "not-an-ip           partly-broken.lan\n"
      "192.168.1.90        partly-broken.lan\n";
  ok &= Expect(Select(resolver_hosts, "mapped.lan").empty(),
               "a name with only IPv4-mapped entries must still ask DNS");
  ok &= Expect(Select(resolver_hosts, "mixed.lan") ==
                   std::vector<std::string>{"192.168.1.80", "fd00::80"},
               "mixed entries must keep only dialable addresses, in file order");
  ok &= Expect(Select(resolver_hosts, "broken.lan").empty(),
               "a name with only malformed entries must still ask DNS");
  ok &= Expect(Select(resolver_hosts, "partly-broken.lan") ==
                   std::vector<std::string>{"192.168.1.90"},
               "a malformed entry must not hide a valid one");
  ok &= Expect(Select(resolver_hosts, "missing.lan").empty(),
               "a name missing from the hosts file must ask DNS");

  if (!ok) return 1;
  std::cout << "lan_hosts_file_test passed\n";
  return 0;
}
