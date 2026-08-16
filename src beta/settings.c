// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "settings.h"
#include "strings.h"
#include "context_menu.h"

static HHOOK g_hHook = NULL;
static WCHAR g_yesText[32], g_noText[32];

static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HCBT_ACTIVATE) {
        HWND hDlg = (HWND)wParam;
        HWND hYes = GetDlgItem(hDlg, IDYES);
        HWND hNo  = GetDlgItem(hDlg, IDNO);
        if (hYes) SetWindowTextW(hYes, g_yesText);
        if (hNo)  SetWindowTextW(hNo, g_noText);
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

int ShowLanguageDialog(HWND hwndParent) {
    wcsncpy_s(g_yesText, _countof(g_yesText), GetStr(STR_LANGUAGE_ENGLISH), _TRUNCATE);
    wcsncpy_s(g_noText,  _countof(g_noText),  GetStr(STR_LANGUAGE_RUSSIAN), _TRUNCATE);
    g_hHook = SetWindowsHookEx(WH_CBT, CBTProc, GetModuleHandle(NULL), GetCurrentThreadId());
    int ret = MessageBoxW(hwndParent, GetStr(STR_LANGUAGE_SELECT), GetStr(STR_TITLE),
                          MB_YESNO | MB_ICONINFORMATION);
    if (g_hHook) { UnhookWindowsHookEx(g_hHook); g_hHook = NULL; }
    if (ret == IDYES) return 0;
    if (ret == IDNO)  return 1;
    return 0;
}

void LoadLanguageSetting(void) {
    WCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
        currentLang = 0;
        return;
    }
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    if (FAILED(StringCchCat(path, _countof(path), L"settings.ini"))) {
        currentLang = 0;
        return;
    }
    currentLang = (int)GetPrivateProfileInt(L"Settings", L"Language", -1, path);
}

void LoadSettings(AppState* state) {
    LoadLanguageSetting();
    WCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
        state->darkTheme = 0;
        state->defaultEncoding = 0;
        state->contextMenuInstalled = FALSE;
        return;
    }
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    if (FAILED(StringCchCat(path, _countof(path), L"settings.ini"))) {
        state->darkTheme = 0;
        state->defaultEncoding = 0;
        state->contextMenuInstalled = FALSE;
        return;
    }
    state->darkTheme = (int)GetPrivateProfileInt(L"Settings", L"DarkTheme", 0, path) != 0;
    state->contextMenuInstalled = IsContextMenuInstalled();
    state->defaultEncoding = (int)GetPrivateProfileInt(L"Settings", L"Encoding", 0, path);
}

void SaveSettings(const AppState* state) {
    WCHAR path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0) return;
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    if (FAILED(StringCchCat(path, _countof(path), L"settings.ini"))) return;
    WCHAR buf[32];
    StringCchPrintf(buf, _countof(buf), L"%d", currentLang);
    WritePrivateProfileString(L"Settings", L"Language", buf, path);
    StringCchPrintf(buf, _countof(buf), L"%d", state->darkTheme);
    WritePrivateProfileString(L"Settings", L"DarkTheme", buf, path);
    StringCchPrintf(buf, _countof(buf), L"%d", state->defaultEncoding);
    WritePrivateProfileString(L"Settings", L"Encoding", buf, path);
}