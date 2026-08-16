// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#pragma once

#define MAX_FILE_SIZE (500 * 1024 * 1024)

#ifndef EM_GETLINEHEIGHT
#define EM_GETLINEHEIGHT (WM_USER + 91)
#endif

#ifndef TCM_SETBKCOLOR
#define TCM_SETBKCOLOR 0x1300
#endif

#define IDC_TAB            1001
#define IDC_LINENUMBERS    1002
#define IDC_SEPARATOR      1004
#define IDC_STATUS         1003

#define IDM_NEW            40001
#define IDM_OPEN           40002
#define IDM_SAVE           40003
#define IDM_SAVEAS         40004
#define IDM_CLOSE_TAB      40005
#define IDM_EXIT           40006
#define IDM_UNDO           40007
#define IDM_CUT            40008
#define IDM_COPY           40009
#define IDM_PASTE          40010
#define IDM_SELECTALL      40011
#define IDM_DARKTHEME      40012
#define IDM_FONT           40013
#define IDM_HEXMODE        40014
#define IDM_LANG_EN        40015
#define IDM_LANG_RU        40016
#define IDM_CONTEXTMENU    40017
#define IDM_HASH_MD5       40018
#define IDM_HASH_SHA1      40019
#define IDM_HASH_SHA256    40020

#define IDM_ENC_UTF8       40021
#define IDM_ENC_UTF16LE    40022
#define IDM_ENC_UTF16BE    40023
#define IDM_ENC_WIN1251    40024
#define IDM_ENC_WIN1252    40025

#define IDM_FIND           40026

#define ID_ACCEL_CLOSE_TAB 5001
#define ID_ACCEL_NEXT_TAB  5002
#define ID_ACCEL_PREV_TAB  5003
#define ID_ACCEL_HEXMODE   5004