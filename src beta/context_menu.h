// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "common.h"
#include "appstate.h"

BOOL IsContextMenuInstalled(void);
BOOL InstallContextMenu(void);
BOOL UninstallContextMenu(void);
void ToggleContextMenu(AppState* state);