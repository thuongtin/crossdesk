/*
 * @Author: DI JUNKUN
 * @Date: 2025-11-11
 * Copyright (c) 2025 by DI JUNKUN, All Rights Reserved.
 */

#include "version_checker.h"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "rd_log.h"
#include "updater/update_source.h"
#ifndef __OHOS__
#include "updater/update_core.h"
#include "updater/update_platform.h"
#endif

#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace crossdesk {

namespace {

void LogHttpError(const httplib::Result& result) {
  LOG_WARN("Failed to fetch version.json: error={}, message={}",
           static_cast<int>(result.error()), httplib::to_string(result.error()));
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  LOG_WARN("version.json SSL error={}, OpenSSL error={}", result.ssl_error(),
           result.ssl_openssl_error());
#endif
}

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT) && defined(__linux__)
bool PathExists(const std::string& path) {
  if (path.empty()) {
    return false;
  }

  std::error_code ec;
  return std::filesystem::exists(path, ec);
}

std::string GetEnvPathIfExists(const char* key) {
  const char* value = std::getenv(key);
  if (!value) {
    return "";
  }

  const std::string path = value;
  return PathExists(path) ? path : "";
}

std::string FindFirstExistingPath(
    const std::vector<std::string>& candidates) {
  for (const auto& candidate : candidates) {
    if (PathExists(candidate)) {
      return candidate;
    }
  }
  return "";
}

void ConfigureLinuxCaCerts(httplib::Client* cli) {
  const std::string ca_file = [&]() {
    const std::string env_path = GetEnvPathIfExists("SSL_CERT_FILE");
    if (!env_path.empty()) {
      return env_path;
    }

    return FindFirstExistingPath({
        "/etc/ssl/certs/ca-certificates.crt",
        "/etc/pki/tls/certs/ca-bundle.crt",
        "/etc/ssl/cert.pem",
        "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
    });
  }();

  const std::string ca_dir = [&]() {
    const std::string env_path = GetEnvPathIfExists("SSL_CERT_DIR");
    if (!env_path.empty()) {
      return env_path;
    }

    return FindFirstExistingPath({
        "/etc/ssl/certs",
        "/etc/pki/tls/certs",
        "/etc/openssl/certs",
    });
  }();

  if (ca_file.empty() && ca_dir.empty()) {
    LOG_WARN("No Linux CA bundle found for version.json request; relying on OpenSSL defaults");
    return;
  }

  cli->set_ca_cert_path(ca_file, ca_dir);
  LOG_DEBUG("Configured version.json TLS CA bundle: file={}, dir={}",
            ca_file.empty() ? "<none>" : ca_file,
            ca_dir.empty() ? "<none>" : ca_dir);
}
#endif

}  // namespace

#ifndef __OHOS__
std::optional<VersionInfo> ParseSignedUpdateManifest(
    const nlohmann::json& manifest, const std::string& platform,
    const std::string& public_key) {
  try {
    const auto& download = manifest.at("downloads").at(platform);
    const auto envelope = download.at("update").dump();
    const auto artifact = updater::VerifyReleaseEnvelope(envelope, public_key, platform);
    // Order by the signed version, never by unsigned top-level patch metadata.
    auto info = ParseVersionInfo(download);
    if (!info || info->version != artifact.version) return std::nullopt;
    const auto notes = ParseVersionInfo(manifest);
    if (notes && notes->version == artifact.version) {
      info->release_name = notes->release_name;
      info->release_notes = notes->release_notes;
    }
    info->patch = -1;
    info->download_url = artifact.url;
    info->update_envelope = envelope;
    return info;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}
#endif

std::optional<VersionInfo> ParsePlatformUpdateManifest(
    const nlohmann::json& manifest, const std::vector<std::string>& platforms) {
  try {
    const auto& downloads = manifest.at("downloads");
    for (const auto& platform : platforms) {
      if (platform.empty()) continue;
      const auto entry = downloads.find(platform);
      if (entry == downloads.end()) continue;
      auto info = ParseVersionInfo(*entry);
      if (!info) return std::nullopt;
      // Ghi chú phát hành ở cấp cao nhất chỉ dùng khi chúng nói về đúng phiên bản này.
      const auto notes = ParseVersionInfo(manifest);
      if (notes && notes->version == info->version) {
        info->release_name = notes->release_name;
        info->release_notes = notes->release_notes;
      }
      return info;
    }
  } catch (const std::exception&) {
  }
  return std::nullopt;
}

std::optional<VersionInfo> CheckUpdate() {
  httplib::Client cli(updater::kManifestOrigin);
  cli.set_connection_timeout(5);
  cli.set_read_timeout(5);
  cli.set_write_timeout(5);
  cli.set_max_timeout(15000);
  cli.set_follow_location(false);

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT) && defined(__linux__)
  ConfigureLinuxCaCerts(&cli);
#endif

  constexpr std::size_t kMaxManifestSize = 256 * 1024;
  std::string body;
  int status = 0;
  const auto res = cli.Get(updater::kManifestPath,
      [&](const httplib::Response& headers) {
        status = headers.status;
        return status == 200;
      },
      [&](const char* data, std::size_t length) {
        if (length > kMaxManifestSize - body.size()) return false;
        body.append(data, length);
        return true;
      });
  if (!res || status != 200) {
    if (status != 0 && status != 200)
      LOG_WARN("Fork update manifest HTTP status={}", status);
    else
      LogHttpError(res);
    return std::nullopt;
  }
  try {
    const auto manifest = nlohmann::json::parse(body);
#if defined(__OHOS__)
    // Bản HarmonyOS chỉ mở trang release, nên chỉ báo khi tag mới thật sự có gói HarmonyOS.
    // Phiên bản cấp cao nhất có thể thuộc bản phát hành chỉ có gói macOS, Windows hay Linux.
    auto info = ParsePlatformUpdateManifest(manifest, {kHarmonyUpdatePlatform});
    if (info) info->download_url = updater::kReleasePage;
#elif CROSSDESK_PORTABLE
    // Bản portable không tự cài đặt mà chỉ mở trang release, nên chỉ báo khi tag mới thật sự có
    // gói cho hệ điều hành và kiến trúc này. Bản fork chỉ phát hành gói cài đặt, vì vậy khi chưa
    // có mục portable thì dùng mục gói cài đặt của cùng nền tảng.
    const auto executable = updater::ExecutablePath();
    auto info = ParsePlatformUpdateManifest(
        manifest, {updater::PlatformKey(executable, true), updater::PlatformKey(executable)});
    if (info) info->download_url = updater::kReleasePage;
#else
    const auto platform = updater::PlatformKey(updater::ExecutablePath());
    auto info = ParseSignedUpdateManifest(manifest, platform, updater::PublicKey());
#endif
    if (info) {
      LOG_INFO("Fetched fork update manifest: latest_version={}", info->version);
    } else {
      LOG_WARN("Fork update manifest contains no valid release for this installation");
    }
    return info;
  } catch (const std::exception& e) {
    LOG_WARN("Failed to parse fork update manifest: {}", e.what());
    return std::nullopt;
  }
}

}  // namespace crossdesk
