// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "theme.h"
#include "core.h"

void AllowDarkModeForWindow(HWND hWnd, BOOL allow) {
    HMODULE hUxtheme = LoadLibraryEx(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme) {
        typedef BOOL (WINAPI *pfnAllowDarkModeForWindow)(HWND, BOOL);
        pfnAllowDarkModeForWindow fn = (pfnAllowDarkModeForWindow)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (fn) fn(hWnd, allow);
        FreeLibrary(hUxtheme);
    }
    HMODULE hDwm = LoadLibraryEx(L"dwmapi.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hDwm) {
        typedef HRESULT (WINAPI *pfnDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
        pfnDwmSetWindowAttribute DwmSetWindowAttribute = (pfnDwmSetWindowAttribute)GetProcAddress(hDwm, "DwmSetWindowAttribute");
        if (DwmSetWindowAttribute) {
            BOOL value = allow;
            DwmSetWindowAttribute(hWnd, 20, &value, sizeof(value));
        }
        FreeLibrary(hDwm);
    }
}

void ApplyTheme(AppState* state) {
    COLORREF bg = state->darkTheme ? RGB(30,30,30) : RGB(255,255,255);
    COLORREF fg = state->darkTheme ? RGB(220,220,220) : RGB(0,0,0);
    COLORREF lnBg = state->darkTheme ? RGB(50,50,50) : RGB(240,240,240);
    COLORREF lnFg = state->darkTheme ? RGB(180,180,180) : RGB(100,100,100);
    COLORREF tabBg = state->darkTheme ? RGB(40,40,40) : RGB(250,250,250);
    COLORREF statusBg = state->darkTheme ? RGB(45,45,45) : RGB(235,235,235);
    COLORREF sepColor = state->darkTheme ? RGB(100,100,100) : RGB(180,180,180);
    COLORREF statusFg = state->darkTheme ? RGB(220,220,220) : RGB(30,30,30);
    COLORREF mainBg = state->darkTheme ? RGB(25,25,25) : RGB(240,240,240);

    state->lineNumberTextColor = lnFg; state->separatorColor = sepColor;
    state->statusTextColor = statusFg; state->statusBkColor = statusBg;

    if (state->hMainBgBrush) DeleteObject(state->hMainBgBrush);
    state->hMainBgBrush = CreateSolidBrush(mainBg);
    SetClassLongPtr(state->hwndMain, GCLP_HBRBACKGROUND, (LONG_PTR)state->hMainBgBrush);
    RedrawWindow(state->hwndMain, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

    AllowDarkModeForWindow(state->hwndMain, state->darkTheme);
    AllowDarkModeForWindow(state->hTab, state->darkTheme);
    AllowDarkModeForWindow(state->hStatus, state->darkTheme);

    SendMessage(state->hTab, TCM_SETBKCOLOR, 0, (LPARAM)tabBg);
    SetWindowTheme(state->hTab, state->darkTheme ? L"DarkMode_Explorer" : NULL, NULL);

    if (state->hStatusBrush) DeleteObject(state->hStatusBrush);
    state->hStatusBrush = CreateSolidBrush(statusBg);
    SetClassLongPtr(state->hStatus, GCLP_HBRBACKGROUND, (LONG_PTR)state->hStatusBrush);
    SetWindowTheme(state->hStatus, state->darkTheme ? L"DarkMode_Explorer" : NULL, NULL);
    RedrawWindow(state->hStatus, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

    if (state->hBkBrush) DeleteObject(state->hBkBrush);
    state->hBkBrush = CreateSolidBrush(lnBg);
    if (state->hSepBrush) DeleteObject(state->hSepBrush);
    state->hSepBrush = CreateSolidBrush(sepColor);

    for (int i = 0; i < state->tabCount; i++) {
        AllowDarkModeForWindow(state->tabs[i].hRichEdit, state->darkTheme);
        AllowDarkModeForWindow(state->tabs[i].hLineNumbers, state->darkTheme);
        SendMessage(state->tabs[i].hRichEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)bg);
        CHARFORMAT2 cf = {0}; cf.cbSize = sizeof(cf); cf.dwMask = CFM_COLOR; cf.crTextColor = fg;
        SendMessage(state->tabs[i].hRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        SetWindowTheme(state->tabs[i].hRichEdit, state->darkTheme ? L"DarkMode_Explorer" : NULL, NULL);
        InvalidateRect(state->tabs[i].hLineNumbers, NULL, TRUE);
    }
}

void ApplyFont(AppState* state) {
    if (state->hFont) {
        for (int i = 0; i < state->tabCount; i++) {
            SendMessage(state->tabs[i].hRichEdit, WM_SETFONT, (WPARAM)state->hFont, TRUE);
            SendMessage(state->tabs[i].hLineNumbers, WM_SETFONT, (WPARAM)state->hFont, TRUE);
        }
        ResizeControls(state);
    }
}