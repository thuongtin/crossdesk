/*
 * @Author: DI JUNKUN
 * @Date: 2026-10-21
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _ABOUT_CATALOG_H_
#define _ABOUT_CATALOG_H_

#include <string_view>

#include "crossdesk_ui.h"

namespace crossdesk::about {

// Markdown release notes embedded in this build; empty when none were configured.
std::string_view InstalledReleaseNotes();
bool HasInstalledReleaseNotes();
void SetInstalledReleaseNotes(const slint::ComponentHandle<ui::MainWindow>& window);
void Populate(const slint::ComponentHandle<ui::MainWindow>& window);

}  // namespace crossdesk::about

#endif
