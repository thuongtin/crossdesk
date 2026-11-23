#ifndef CROSSDESK_UPDATE_SOURCE_H_
#define CROSSDESK_UPDATE_SOURCE_H_

#include <optional>
#include <string>
#include <utility>

namespace crossdesk::updater {
inline constexpr char kManifestOrigin[] = "https://raw.githubusercontent.com";
inline constexpr char kManifestPath[] =
    "/thuongtin/crossdesk/desktop-updates/crossdesk-desktop-update.json";
inline constexpr char kReleaseOrigin[] = "https://github.com";
inline constexpr char kReleasePath[] = "/thuongtin/crossdesk/releases";
inline constexpr char kReleasePage[] = "https://github.com/thuongtin/crossdesk/releases";

inline std::string ReleaseAssetPath(const std::string& version, const std::string& filename) {
  return std::string(kReleasePath) + "/download/v" + version + "/" + filename;
}

inline std::string ReleaseAssetUrl(const std::string& version, const std::string& filename) {
  return kReleaseOrigin + ReleaseAssetPath(version, filename);
}

// GitHub returns a temporary HTTPS URL on its release-asset CDN. Do not
// silently follow redirects to arbitrary hosts, HTTP, or out-of-repo URLs.
inline std::optional<std::pair<std::string, std::string>> ReleaseAssetRedirect(
    const std::string& url) {
  if (url.compare(0, 8, "https://") != 0 ||
      url.find_first_of("\\ \t\r\n#") != std::string::npos) {
    return std::nullopt;
  }
  const auto path = url.find('/', 8);
  if (path == std::string::npos) return std::nullopt;
  auto origin = url.substr(0, path);
  if (origin != "https://release-assets.githubusercontent.com" &&
      origin != "https://objects.githubusercontent.com") {
    return std::nullopt;
  }
  return std::make_pair(std::move(origin), url.substr(path));
}
}  // namespace crossdesk::updater

#endif
