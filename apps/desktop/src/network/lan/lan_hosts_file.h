#ifndef _LAN_HOSTS_FILE_H_
#define _LAN_HOSTS_FILE_H_

#include <algorithm>
#include <cctype>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace crossdesk::lan {

// Chuẩn hoá tên theo hosts(5): không phân biệt hoa thường, bỏ một dấu chấm
// cuối, để "Desk.Home.Arpa." và "desk.home.arpa" là cùng một tên.
inline std::string NormalizeHostsName(std::string_view name) {
  if (!name.empty() && name.back() == '.') name.remove_suffix(1);
  std::string normalized(name);
  std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return normalized;
}

// Trả về các địa chỉ (dạng chữ, theo thứ tự trong file, không trùng) của
// những dòng có tên chính hoặc alias trùng đúng `host`. Không ghép search
// suffix và không khớp một phần, nên "desk" không khớp "desk.example".
// Địa chỉ chưa được kiểm tra cú pháp; người gọi tự phân tích.
inline std::vector<std::string> FindHostsFileAddresses(std::istream& in,
                                                       std::string_view host) {
  std::vector<std::string> addresses;
  const std::string wanted = NormalizeHostsName(host);
  if (wanted.empty()) return addresses;
  std::string line;
  while (std::getline(in, line)) {
    if (const auto hash = line.find('#'); hash != std::string::npos)
      line.erase(hash);
    std::istringstream fields(line);
    std::string address, name;
    if (!(fields >> address)) continue;
    while (fields >> name) {
      if (NormalizeHostsName(name) != wanted) continue;
      if (std::find(addresses.begin(), addresses.end(), address) ==
          addresses.end())
        addresses.push_back(address);
      break;
    }
  }
  return addresses;
}

// Đọc cơ sở dữ liệu hosts cục bộ. Chỉ dùng trên hệ POSIX, nơi backend files
// của glibc không khớp "name." với /etc/hosts; trên Windows giữ nguyên hành vi
// của resolver hệ thống.
inline std::vector<std::string> ReadHostsFileAddresses(std::string_view host) {
#if defined(_WIN32)
  (void)host;
  return {};
#else
  std::ifstream file("/etc/hosts");
  if (!file) return {};
  return FindHostsFileAddresses(file, host);
#endif
}

}  // namespace crossdesk::lan

#endif  // _LAN_HOSTS_FILE_H_
