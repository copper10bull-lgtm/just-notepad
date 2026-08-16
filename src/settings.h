// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "common.h"
#include "appstate.h"

void LoadLanguageSetting(void);
void LoadSettings(AppState* state);
void SaveSettings(const AppState* state);
int  ShowLanguageDialog(HWND hwndParent);