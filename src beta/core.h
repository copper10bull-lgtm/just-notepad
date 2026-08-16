// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "common.h"
#include "appstate.h"

#define SUBCLASS_RICHEDIT 1
#define SUBCLASS_GUTTER   2

void AddTab(AppState* state, const WCHAR* title, const WCHAR* filepath);
void RemoveTab(AppState* state, int index);
void SwitchToTab(AppState* state, int index);
void UpdateTabTitle(AppState* state, int index);
void UpdateWindowTitle(const AppState* state);
void ResizeControls(AppState* state);
void OnCloseTab(AppState* state);

BOOL LoadFileToTab(AppState* state, int index, const WCHAR* filepath, int encoding);
BOOL SaveTab(AppState* state, int index, BOOL forceSaveAs);
BOOL SwitchHexMode(AppState* state, int index);
void UpdateStatusBar(const AppState* state, int index);
void ShowFileHash(HWND hwnd, const BYTE* data, DWORD size, ALG_ID algId);

void ShowFindDialog(AppState* state);
void DoFind(HWND hwnd, FINDREPLACE* fr);

void UpdateLineNumbers(const AppState* state, int index);
void RecalcGutterWidth(AppState* state, int index);

LRESULT CALLBACK RichEditSubclassProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);
LRESULT CALLBACK GutterSubclassProc(HWND, UINT, WPARAM, LPARAM, UINT_PTR, DWORD_PTR);