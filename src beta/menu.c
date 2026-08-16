// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "menu.h"
#include "strings.h"

HMENU BuildMenu(AppState* state) {
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_STRING, IDM_NEW,      GetStr(STR_MENU_NEW));
    AppendMenu(hFileMenu, MF_STRING, IDM_OPEN,     GetStr(STR_MENU_OPEN));
    AppendMenu(hFileMenu, MF_STRING, IDM_SAVE,     GetStr(STR_MENU_SAVE));
    AppendMenu(hFileMenu, MF_STRING, IDM_SAVEAS,   GetStr(STR_MENU_SAVEAS));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    HMENU hEncMenu = CreatePopupMenu();
    AppendMenu(hEncMenu, MF_STRING, IDM_ENC_UTF8,     GetStr(STR_ENC_UTF8));
    AppendMenu(hEncMenu, MF_STRING, IDM_ENC_UTF16LE,  GetStr(STR_ENC_UTF16LE));
    AppendMenu(hEncMenu, MF_STRING, IDM_ENC_UTF16BE,  GetStr(STR_ENC_UTF16BE));
    AppendMenu(hEncMenu, MF_STRING, IDM_ENC_WIN1251,  GetStr(STR_ENC_WIN1251));
    AppendMenu(hEncMenu, MF_STRING, IDM_ENC_WIN1252,  GetStr(STR_ENC_WIN1252));
    AppendMenu(hFileMenu, MF_POPUP, (UINT_PTR)hEncMenu, GetStr(STR_MENU_ENCODING));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_CLOSE_TAB,GetStr(STR_MENU_CLOSETAB));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_EXIT,     GetStr(STR_MENU_EXIT));
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    HMENU hHashMenu = CreatePopupMenu();
    AppendMenu(hHashMenu, MF_STRING, IDM_HASH_MD5,   GetStr(STR_MENU_HASH_MD5));
    AppendMenu(hHashMenu, MF_STRING, IDM_HASH_SHA1,  GetStr(STR_MENU_HASH_SHA1));
    AppendMenu(hHashMenu, MF_STRING, IDM_HASH_SHA256,GetStr(STR_MENU_HASH_SHA256));
    AppendMenu(hFileMenu, MF_POPUP, (UINT_PTR)hHashMenu, GetStr(STR_MENU_HASH));

    HMENU hEditMenu = CreatePopupMenu();
    AppendMenu(hEditMenu, MF_STRING, IDM_UNDO,     GetStr(STR_MENU_UNDO));
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, MF_STRING, IDM_CUT,      GetStr(STR_MENU_CUT));
    AppendMenu(hEditMenu, MF_STRING, IDM_COPY,     GetStr(STR_MENU_COPY));
    AppendMenu(hEditMenu, MF_STRING, IDM_PASTE,    GetStr(STR_MENU_PASTE));
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, MF_STRING, IDM_SELECTALL,GetStr(STR_MENU_SELECTALL));
    AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEditMenu, MF_STRING, IDM_FIND,     GetStr(STR_MENU_FIND));

    HMENU hViewMenu = CreatePopupMenu();
    AppendMenu(hViewMenu, MF_STRING, IDM_DARKTHEME,GetStr(STR_MENU_DARKTHEME));
    AppendMenu(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hViewMenu, MF_STRING, IDM_FONT,     GetStr(STR_MENU_FONT));
    AppendMenu(hViewMenu, MF_STRING, IDM_HEXMODE,  GetStr(STR_MENU_HEXMODE));
    AppendMenu(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hViewMenu, MF_STRING, IDM_CONTEXTMENU,
               state->contextMenuInstalled ? GetStr(STR_MENU_CONTEXTMENU_REMOVE) : GetStr(STR_MENU_CONTEXTMENU_ADD));

    HMENU hLangMenu = CreatePopupMenu();
    AppendMenu(hLangMenu, MF_STRING, IDM_LANG_EN, GetStr(STR_MENU_LANG_EN));
    AppendMenu(hLangMenu, MF_STRING, IDM_LANG_RU, GetStr(STR_MENU_LANG_RU));

    HMENU hMenu = CreateMenu();
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, GetStr(STR_MENU_FILE));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, GetStr(STR_MENU_EDIT));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, GetStr(STR_MENU_VIEW));
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLangMenu, GetStr(STR_MENU_LANGUAGE));
    return hMenu;
}

HACCEL CreateAccels() {
    ACCEL acc[5] = {
        { FCONTROL | FVIRTKEY, 'W',    ID_ACCEL_CLOSE_TAB },
        { FCONTROL | FVIRTKEY, VK_TAB, ID_ACCEL_NEXT_TAB },
        { FCONTROL | FSHIFT | FVIRTKEY, VK_TAB, ID_ACCEL_PREV_TAB },
        { FCONTROL | FVIRTKEY, 'H',    ID_ACCEL_HEXMODE },
        { FCONTROL | FVIRTKEY, 'F',    IDM_FIND }
    };
    return CreateAcceleratorTable(acc, 5);
}