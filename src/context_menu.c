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
        RegCloseKey(hKey);
        return TRUE;
    }
    return FALSE;
}

BOOL InstallContextMenu(void) {
    WCHAR szExePath[MAX_PATH];
    if (GetModuleFileName(NULL, szExePath, MAX_PATH) == 0)
        return FALSE;

    HKEY hKey;
    LSTATUS status = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad",
                                    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (status != ERROR_SUCCESS)
        return FALSE;

    const WCHAR* displayName = GetStr(STR_TITLE);
    DWORD displayNameBytes = (DWORD)((wcslen(displayName) + 1) * sizeof(WCHAR));
    status = RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)displayName, displayNameBytes);
    if (status != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return FALSE;
    }

    WCHAR iconPath[MAX_PATH + 10];
    StringCchPrintf(iconPath, _countof(iconPath), L"%s,0", szExePath);
    DWORD iconPathBytes = (DWORD)((wcslen(iconPath) + 1) * sizeof(WCHAR));
    status = RegSetValueEx(hKey, L"Icon", 0, REG_SZ, (const BYTE*)iconPath, iconPathBytes);
    RegCloseKey(hKey);
    if (status != ERROR_SUCCESS)
        return FALSE;

    status = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad\\command",
                            0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (status != ERROR_SUCCESS)
        return FALSE;

    WCHAR cmdLine[MAX_PATH + 10];
    StringCchPrintf(cmdLine, _countof(cmdLine), L"\"%s\" \"%%1\"", szExePath);
    DWORD cmdLineBytes = (DWORD)((wcslen(cmdLine) + 1) * sizeof(WCHAR));
    status = RegSetValueEx(hKey, NULL, 0, REG_SZ, (const BYTE*)cmdLine, cmdLineBytes);
    RegCloseKey(hKey);
    return (status == ERROR_SUCCESS);
}

BOOL UninstallContextMenu(void) {
    return (RegDeleteTree(HKEY_CURRENT_USER, L"Software\\Classes\\*\\shell\\JustNotepad") == ERROR_SUCCESS);
}

void ToggleContextMenu(AppState* state) {
    if (state->contextMenuInstalled) {
        if (UninstallContextMenu()) {
            state->contextMenuInstalled = FALSE;
            MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_REMOVED), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_REMOVE_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONERROR);
        }
    } else {
        if (InstallContextMenu()) {
            state->contextMenuInstalled = TRUE;
            MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_ADDED), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION);
        } else {
            MessageBox(state->hwndMain, GetStr(STR_CONTEXTMENU_ADD_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONERROR);
        }
    }
    SetMenu(state->hwndMain, BuildMenu(state));
    DrawMenuBar(state->hwndMain);
}