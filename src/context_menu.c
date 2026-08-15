// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "context_menu.h"
#include "strings.h"
#include "menu.h"

BOOL IsContextMenuInstalled(void) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey); return TRUE;
    }
    return FALSE;
}

BOOL InstallContextMenu(void) {
    WCHAR szExePath[MAX_PATH]; GetModuleFileName(NULL, szExePath, MAX_PATH);
    HKEY hKey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    const WCHAR* displayName = GetStr(STR_TITLE);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)displayName, (wcslen(displayName)+1)*sizeof(WCHAR));
    WCHAR iconPath[MAX_PATH+10]; _snwprintf(iconPath, MAX_PATH+10, L"%s,0", szExePath);
    RegSetValueEx(hKey, L"Icon", 0, REG_SZ, (BYTE*)iconPath, (wcslen(iconPath)+1)*sizeof(WCHAR));
    RegCloseKey(hKey);
    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad\\command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) return FALSE;
    WCHAR cmdLine[MAX_PATH+10]; _snwprintf(cmdLine, MAX_PATH+10, L"\"%s\" \"%%1\"", szExePath);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)cmdLine, (wcslen(cmdLine)+1)*sizeof(WCHAR));
    RegCloseKey(hKey);
    return TRUE;
}

BOOL UninstallContextMenu(void) {
    RegDeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad");
    return TRUE;
}

void ToggleContextMenu(AppState* state) {
    if (state->contextMenuInstalled) {
        if (UninstallContextMenu()) { state->contextMenuInstalled = FALSE; MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_REMOVED), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION); }
        else MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_REMOVE_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONERROR);
    } else {
        if (InstallContextMenu()) { state->contextMenuInstalled = TRUE; MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_ADDED), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION); }
        else MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_ADD_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONERROR);
    }
    SetMenu(state->hwndMain, BuildMenu(state)); DrawMenuBar(state->hwndMain);
}