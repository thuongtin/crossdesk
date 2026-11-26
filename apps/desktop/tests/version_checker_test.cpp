#include "version_checker.h"
#include "updater/update_core.h"
#include "updater/update_source.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

bool ExpectEqual(const std::string& name, bool actual, bool expected) {
  if (actual == expected) {
    return true;
  }

  std::cerr << name << " mismatch\n"
            << "  expected: " << expected << "\n"
            << "  actual:   " << actual << "\n";
  return false;
}

}  // namespace

int main(int argc, char** argv) {
  bool ok = true;

  ok &= ExpectEqual("new patch-before-date is newer",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-20260529", "v1.3.5-1-20260529", "", -1),
                    true);
  ok &= ExpectEqual("larger patch wins regardless of date",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-2-20260530", "v1.3.5-3-20260529", "", -1),
                    true);
  ok &= ExpectEqual("smaller patch loses regardless of date",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-3-20260529", "v1.3.5-2-20260530", "", -1),
                    false);
  ok &= ExpectEqual("old date-before-patch remains supported",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-20260529-1", "v1.3.5-20260529-2", "", -1),
                    true);
  ok &= ExpectEqual("metadata patch overrides date",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-9-20260530", "v1.3.5", "2026-05-31", 10),
                    true);
  ok &= ExpectEqual("date alone does not update same version",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-20260529", "v1.3.5-20260530", "", -1),
                    false);
  ok &= ExpectEqual("numeric version still wins",
                    crossdesk::IsNewerVersionWithMetadata(
                        "v1.3.5-9-20260529", "v1.3.6-1-20260529", "", -1),
                    true);

  const auto info =
      crossdesk::ParseVersionInfo({{"latest_version", "v1.3.5-20260529"},
                                   {"patch", "10"},
                                   {"releaseName", "Release"},
                                   {"releaseNotes", "Changes"},
                                   {"releaseDate", "2026-05-29"}});
  ok &= ExpectEqual("valid metadata is normalized and retained",
                    info && info->version == "1.3.5-20260529" &&
                        info->patch == 10 && info->release_name == "Release" &&
                        info->release_notes == "Changes" &&
                        info->release_date == "2026-05-29",
                    true);
  const auto legacy = crossdesk::ParseVersionInfo({{"version", "1.3.6"}});
  ok &= ExpectEqual("legacy version field is supported",
                    legacy && legacy->version == "1.3.6" && legacy->patch == -1,
                    true);
  for (const auto& json :
       std::vector<nlohmann::json>{nullptr,
                                   nlohmann::json::array(),
                                   nlohmann::json::object(),
                                   {{"version", false}},
                                   {{"version", ""}},
                                   {{"version", "unknown"}},
                                   {{"version", "1..2"}},
                                   {{"version", ".1.2"}},
                                   {{"version", "1.2."}},
                                   {{"version", "1.2-"}},
                                   {{"version", "999999999999999999999.2"}}}) {
    ok &= ExpectEqual("invalid response is a failed check: " + json.dump(),
                      crossdesk::ParseVersionInfo(json).has_value(), false);
  }
  const auto malformed_optional = crossdesk::ParseVersionInfo(
      {{"version", "1.3.5"},
       {"patch", std::numeric_limits<uint64_t>::max()},
       {"releaseName", nullptr},
       {"releaseNotes", 123}});
  ok &= ExpectEqual("bad optional metadata does not leak into later checks",
                    malformed_optional && malformed_optional->patch == -1 &&
                        malformed_optional->release_name.empty() &&
                        malformed_optional->release_notes.empty(),
                    true);
  ok &= ExpectEqual("plain comparison has no hidden metadata from prior checks",
                    crossdesk::IsNewerVersion("1.3.5", "1.3.5"), false);

  const auto release = crossdesk::ParseVersionInfoJSON(
      R"({"latest_version":"v1.5.3-20260928","patch":2})");
  ok &= ExpectEqual("shared release JSON parser",
                    release.has_value(), true);
  if (release) {
    for (const auto& version : {"1.5.3-2-20260927", "1.5.3-3-20260928", "1.6.0"}) {
      ok &= ExpectEqual(std::string("no remote update for ") + version,
                        crossdesk::AvailableAppUpdate(version, *release).has_value(),
                        false);
    }
    for (const auto& version : {"", " ", "unknown", "ios-native", "1..5", "1.5-"}) {
      ok &= ExpectEqual(std::string("unavailable peer version is legacy: ") + version,
                        !crossdesk::IsValidAppVersion(version) &&
                            crossdesk::AvailableAppUpdate(version, *release) == "1.5.3-2",
                        true);
    }
    for (const auto& version : {"1.5.2", "v1.5.3-20260928", "1.5.3-1-20260929",
                               "1.5.3-20260928-1"}) {
      ok &= ExpectEqual(std::string("remote update includes metadata patch for ") + version,
                        crossdesk::AvailableAppUpdate(version, *release) == "1.5.3-2",
                        true);
    }
  }
  ok &= ExpectEqual("legacy peer cannot supply a missing latest release label",
                    crossdesk::AvailableAppUpdate("", crossdesk::VersionInfo{}).has_value(),
                    false);
  for (const auto& response : {"", "not JSON", "{}", "[]",
                               R"({"version":null})", R"({"version":"unknown"})"}) {
    ok &= ExpectEqual("invalid release response gives no update",
                      crossdesk::ParseVersionInfoJSON(response).has_value(), false);
  }

  const auto mobile_manifest = R"({"latest_version":"99.0.0","patch":99,
    "downloads":{"ios-arm64":{"version":"0.0.2-20261002"},
                 "android-arm64":{"version":"1.6.0-2-20261002"}}})";
  ok &= ExpectEqual("iOS uses its own release", crossdesk::CheckPlatformAppUpdate(
      "0.0.1", mobile_manifest, "ios-arm64") == "0.0.2", true);
  ok &= ExpectEqual("Android uses its own release and inline patch",
      crossdesk::CheckPlatformAppUpdate("1.6.0", mobile_manifest, "android-arm64") == "1.6.0-2", true);
  for (const auto& version : {"1.6.0-2", "1.6.0-3", "1.7.0"}) {
    ok &= ExpectEqual("desktop patch cannot affect mobile comparison",
        crossdesk::CheckPlatformAppUpdate(version, mobile_manifest, "android-arm64") == "", true);
  }
  ok &= ExpectEqual("mobile build date does not count as update",
      crossdesk::CheckPlatformAppUpdate("0.0.2-20261001", mobile_manifest, "ios-arm64") == "", true);
  ok &= ExpectEqual("invalid installed version is a failed local check",
      crossdesk::CheckPlatformAppUpdate("", mobile_manifest, "ios-arm64").has_value(), false);
  for (const auto& response : {"not JSON", "[]", R"({"version":"99.0.0"})",
      R"({"downloads":[]})", R"({"downloads":{"ios-arm64":null}})",
      R"({"downloads":{"android-arm64":{"version":"99.0.0"}}})",
      R"({"downloads":{"ios-arm64":{"url":"https://example.com/app.zip"}}})",
      R"({"downloads":{"ios-arm64":{"version":"invalid"}}})"}) {
    ok &= ExpectEqual("missing mobile metadata never falls back to desktop",
        crossdesk::CheckPlatformAppUpdate("0.0.1", response, "ios-arm64").has_value(), false);
  }
  ok &= ExpectEqual("platform patch metadata follows desktop rules",
      crossdesk::CheckPlatformAppUpdate("1.6.0-2",
          R"({"downloads":{"android-arm64":{"version":"1.6.0","patch":3}}})",
          "android-arm64") == "1.6.0-3", true);

  using namespace crossdesk::updater;
  ok &= ExpectEqual("asset is scoped to the release tag on the fork",
      ReleaseAssetUrl("1.7.6-7-20261123", "crossdesk-macos-arm64-v1.7.6-7-20261123.pkg") ==
          "https://github.com/thuongtin/crossdesk/releases/download/v1.7.6-7-20261123/crossdesk-macos-arm64-v1.7.6-7-20261123.pkg",
      true);
  const auto cdn = ReleaseAssetRedirect("https://release-assets.githubusercontent.com/a?x=1&y=2");
  ok &= ExpectEqual("GitHub CDN URL retains its signed query",
      cdn && cdn->first == "https://release-assets.githubusercontent.com" &&
          cdn->second == "/a?x=1&y=2", true);
  for (const auto* url : {"http://release-assets.githubusercontent.com/a",
                         "https://release-assets.githubusercontent.com.evil.example/a",
                         "https://user@release-assets.githubusercontent.com/a",
                         "https://release-assets.githubusercontent.com:443/a",
                         "https://github.com/kunkundi/crossdesk/releases/download/v1/a",
                         "https://release-assets.githubusercontent.com/a#fragment",
                         "https://release-assets.githubusercontent.com/a\r\nInjected: header",
                         "/relative"}) {
    ok &= ExpectEqual(std::string("blocked redirect: ") + url,
        ReleaseAssetRedirect(url).has_value(), false);
  }
  ok &= ExpectEqual("unsigned manifest does not offer an install",
      crossdesk::ParseSignedUpdateManifest({{"latest_version", "1.7.6-7-20261123"}},
                                           "macos-arm64", "").has_value(), false);
  {
    // Bản phát hành chỉ có gói macOS đẩy phiên bản cấp cao nhất lên, còn mục Windows giữ bản cũ.
    const nlohmann::json feed = {
        {"version", "9.0.0-1-20261201"},
        {"releaseNotes", "macOS only"},
        {"downloads",
         {{"macos-arm64", {{"version", "9.0.0-1-20261201"}}},
          {"windows-x64", {{"version", "1.7.7-3-20261126"}}}}}};
    const auto windows = crossdesk::ParsePlatformUpdateManifest(
        feed, {"windows-x64-portable", "windows-x64"});
    ok &= ExpectEqual("portable check reads its own platform version",
        windows && windows->version == "1.7.7-3-20261126" && windows->release_notes.empty(),
        true);
    const auto mac = crossdesk::ParsePlatformUpdateManifest(feed, {"macos-arm64"});
    ok &= ExpectEqual("matching platform keeps the release notes",
        mac && mac->version == "9.0.0-1-20261201" && mac->release_notes == "macOS only", true);
    ok &= ExpectEqual("platform without an entry offers no update",
        crossdesk::ParsePlatformUpdateManifest(feed, {"linux-amd64", ""}).has_value(), false);
    ok &= ExpectEqual("HarmonyOS ignores a release without a Harmony package",
        crossdesk::ParsePlatformUpdateManifest(feed, {crossdesk::kHarmonyUpdatePlatform})
            .has_value(),
        false);
    auto with_harmony = feed;
    with_harmony["downloads"][crossdesk::kHarmonyUpdatePlatform] = {
        {"version", "1.7.7-3-20261126"}};
    const auto harmony = crossdesk::ParsePlatformUpdateManifest(
        with_harmony, {crossdesk::kHarmonyUpdatePlatform});
    ok &= ExpectEqual("HarmonyOS reads its own platform version",
        harmony && harmony->version == "1.7.7-3-20261126", true);
    ok &= ExpectEqual("manifest without downloads offers no update",
        crossdesk::ParsePlatformUpdateManifest({{"version", "9.0.0"}}, {"windows-x64"})
            .has_value(),
        false);
  }
  if (argc == 3 && std::string(argv[1]) == "--live") {
    try {
      const auto info = crossdesk::CheckUpdate();
      if (!info) throw std::runtime_error("Live fork manifest unavailable or invalid");
      const auto platform = PlatformKey(argv[0]);
      const auto artifact = VerifyReleaseEnvelope(info->update_envelope, PublicKey(), platform);
      if (crossdesk::IsNewerVersion(BuildVersion(), artifact.version))
        throw std::runtime_error("The live feed changed during this release smoke test");
      std::atomic<int> progress{0};
      const std::atomic<bool> cancel{false};
      DownloadPackage(artifact, argv[2], progress, cancel);
      if (progress != 100) throw std::runtime_error("Live download did not complete");
      std::cout << "PASS: live signed manifest and GitHub/CDN download; version="
                << artifact.version << " SHA256=" << artifact.sha256
                << "; no installation performed.\n";
      return ok ? 0 : 1;
    } catch (const std::exception& e) {
      std::cerr << "Failed: " << e.what() << '\n';
      return 1;
    }
  }
  if (argc == 4 || argc == 6) {
    const std::string platform = "macos-arm64";
    const auto pub = argc == 6 ? std::string(argv[4]) : PublicKey();
    try {
      auto manifest = nlohmann::json::parse(ReadText(argv[1], 256 * 1024));
      const auto info = crossdesk::ParseSignedUpdateManifest(manifest, platform, pub);
      if (!info) throw std::runtime_error("Signed fixture could not be parsed");
      const auto artifact = VerifyEnvelope(info->update_envelope, pub, platform, argv[3]);
      VerifyPackage(argv[2], artifact);
      auto rejects = [&](const std::string& current) {
        try {
          VerifyEnvelope(info->update_envelope, pub, platform, current);
          return false;
        } catch (const std::exception&) { return true; }
      };
      ok &= ExpectEqual("same version cannot install", rejects(artifact.version), true);
      ok &= ExpectEqual("downgrade cannot install", rejects("99.1.0"), true);
      ok &= ExpectEqual("wrong architecture cannot install",
          crossdesk::ParseSignedUpdateManifest(manifest, "macos-x64", pub).has_value(), false);
      ok &= ExpectEqual("wrong key cannot offer an install",
          crossdesk::ParseSignedUpdateManifest(manifest, platform,
                                               std::string(64, '0')).has_value(), false);
      manifest["patch"] = 9001;
      manifest["downloads"][platform]["patch"] = 9001;
      const auto patch = crossdesk::ParseSignedUpdateManifest(manifest, platform, pub);
      ok &= ExpectEqual("unsigned patch cannot override signed version",
          patch && patch->patch == -1 && patch->version == artifact.version, true);
      manifest["downloads"][platform]["version"] = "99.0.0";
      ok &= ExpectEqual("unsigned version mismatch cannot offer an install",
          crossdesk::ParseSignedUpdateManifest(manifest, platform, pub).has_value(), false);
      manifest["downloads"][platform]["version"] = artifact.version;
      auto& sig = manifest["downloads"][platform]["update"]["signature"];
      auto tampered = sig.get<std::string>();
      tampered[0] = tampered[0] == '0' ? '1' : '0';
      sig = tampered;
      ok &= ExpectEqual("tampered signature cannot offer an install",
          crossdesk::ParseSignedUpdateManifest(manifest, platform, pub).has_value(), false);
      auto altered = artifact;
      altered.sha256 = std::string(64, '0');
      bool reject_hash = false;
      try { VerifyPackage(argv[2], altered); } catch (const std::exception&) { reject_hash = true; }
      ok &= ExpectEqual("altered package hash fails verification", reject_hash, true);
      if (argc == 6) {
        const auto wrong_origin = nlohmann::json::parse(ReadText(argv[5], 256 * 1024));
        ok &= ExpectEqual("valid signature cannot authorize an out-of-fork URL",
            crossdesk::ParseSignedUpdateManifest(wrong_origin, platform, pub).has_value(), false);
      }
      if (ok) std::cout << "PASS: signed fork manifest and package; invalid signature, key, platform, version, downgrade and digest blocked.\n";
    } catch (const std::exception& e) {
      std::cerr << "Failed: " << e.what() << '\n';
      return 1;
    }
  } else if (argc != 1) {
    return 2;
  }
  return ok ? 0 : 1;
}
