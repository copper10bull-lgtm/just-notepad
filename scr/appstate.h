// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "common.h"

typedef struct {
    HWND hRichEdit;
    HWND hLineNumbers;
    HWND hSeparator;
    HWND hContainer;
    WCHAR filepath[MAX_PATH];
    BOOL  modified;
    WCHAR title[256];
    BYTE* rawData;
    DWORD rawSize;
    BOOL  hexMode;
} TabInfo;

typedef struct {
    HWND      hwndMain;
    HWND      hTab;
    HWND      hStatus;
    TabInfo*  tabs;
    int       tabCount;
    int       tabCapacity;
    int       currentTab;
    HACCEL    hAccel;
    BOOL      darkTheme;
    HFONT     hFont;
    LOGFONT   lf;
    HBRUSH    hBkBrush;
    HBRUSH    hSepBrush;
    HBRUSH    hStatusBrush;
    HBRUSH    hMainBgBrush;
    COLORREF  lineNumberTextColor;
    COLORREF  separatorColor;
    COLORREF  statusTextColor;
    COLORREF  statusBkColor;
    int       gutterWidth;
    BOOL      contextMenuInstalled;
    int       defaultEncoding;
    FINDREPLACE fr;
    WCHAR      findWhat[128];
    BOOL       findActive;
    HWND       hwndFind;
} AppState;