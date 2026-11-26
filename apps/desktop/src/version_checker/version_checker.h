/*
 * @Author: DI JUNKUN
 * @Date: 2025-11-11
 * Copyright (c) 2025 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _VERSION_CHECKER_H_
#define _VERSION_CHECKER_H_

#include <nlohmann/json_fwd.hpp>
#include <app_version.h>

#include <optional>
#include <string>
#include <vector>

namespace crossdesk {

// Khoá nền tảng của gói HarmonyOS trong `downloads`, giống đường kiểm phiên bản của bản di động.
inline constexpr char kHarmonyUpdatePlatform[] = "harmony-arm64";

std::optional<VersionInfo> ParseVersionInfo(const nlohmann::json& json);
std::optional<VersionInfo> ParseSignedUpdateManifest(
    const nlohmann::json& manifest, const std::string& platform,
    const std::string& public_key);
// Phiên bản mới nhất của nền tảng đầu tiên có mặt trong `downloads`, theo thứ tự ưu tiên của
// `platforms`. Không có nền tảng nào thì không có bản cập nhật, kể cả khi phiên bản cấp cao nhất
// mới hơn: phiên bản đó có thể thuộc một bản phát hành chỉ có gói của nền tảng khác.
std::optional<VersionInfo> ParsePlatformUpdateManifest(
    const nlohmann::json& manifest, const std::vector<std::string>& platforms);
std::optional<VersionInfo> CheckUpdate();

}  // namespace crossdesk

#endif
