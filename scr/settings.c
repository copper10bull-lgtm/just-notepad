// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "settings.h"
#include "strings.h"
#include "context_menu.h"

static HHOOK g_hHook = NULL;
static WCHAR g_yesText[32], g_noText[32];

LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam) {
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
    wcsncpy(g_yesText, GetStr(STR_LANGUAGE_ENGLISH), 31);
    wcsncpy(g_noText,  GetStr(STR_LANGUAGE_RUSSIAN), 31);
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
    GetModuleFileName(NULL, path, MAX_PATH);
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    wcscat(path, L"settings.ini");
    currentLang = GetPrivateProfileInt(L"Settings", L"Language", -1, path);
}

void LoadSettings(AppState* state) {
    LoadLanguageSetting();
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    wcscat(path, L"settings.ini");
    state->darkTheme = GetPrivateProfileInt(L"Settings", L"DarkTheme", 0, path);
    state->contextMenuInstalled = IsContextMenuInstalled();
    state->defaultEncoding = GetPrivateProfileInt(L"Settings", L"Encoding", 0, path);
}

void SaveSettings(const AppState* state) {
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    WCHAR* p = wcsrchr(path, L'\\');
    if (p) *(p+1) = L'\0';
    wcscat(path, L"settings.ini");
    WCHAR buf[32];
    _snwprintf(buf, 32, L"%d", currentLang);
    WritePrivateProfileString(L"Settings", L"Language", buf, path);
    _snwprintf(buf, 32, L"%d", state->darkTheme);
    WritePrivateProfileString(L"Settings", L"DarkTheme", buf, path);
    _snwprintf(buf, 32, L"%d", state->defaultEncoding);
    WritePrivateProfileString(L"Settings", L"Encoding", buf, path);
}