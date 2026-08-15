// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "appstate.h"
#include "strings.h"
#include "settings.h"
#include "theme.h"
#include "core.h"
#include "menu.h"
#include "context_menu.h"

static void ProcessCommandLine(AppState* state, LPWSTR lpCmdLine);
static UINT WM_FINDMSG = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppState* state = (AppState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (msg == WM_FINDMSG && state) {
        FINDREPLACE* fr = (FINDREPLACE*)lParam;
        if (fr->Flags & FR_FINDNEXT) {
            DoFind(hwnd, fr);
        }
        if (fr->Flags & FR_DIALOGTERM) {
            state->findActive = FALSE;
            state->hwndFind = NULL;
        }
        return 0;
    }

    switch (msg) {
        case WM_CREATE: {
            WM_FINDMSG = RegisterWindowMessage(FINDMSGSTRING);
            state = malloc(sizeof(AppState));
            if (!state) return -1;
            memset(state, 0, sizeof(AppState));
            state->hwndMain = hwnd;
            state->currentTab = -1;
            HDC hdc = GetDC(NULL);
            int ptSize = 10;
            state->lf.lfHeight = -MulDiv(ptSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
            ReleaseDC(NULL, hdc);
            state->lf.lfWeight = FW_NORMAL;
            state->lf.lfCharSet = DEFAULT_CHARSET;
            state->lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
            wcscpy(state->lf.lfFaceName, L"Consolas");
            state->hFont = CreateFontIndirect(&state->lf);
            state->gutterWidth = 60;

            LoadSettings(state);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
            DragAcceptFiles(hwnd, TRUE);

            state->hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0,0,0,0, hwnd, (HMENU)IDC_STATUS, GetModuleHandle(NULL), NULL);
            state->hTab = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE | TCS_FOCUSNEVER | TCS_FIXEDWIDTH, 0,0,0,0, hwnd, (HMENU)IDC_TAB, GetModuleHandle(NULL), NULL);

            SetMenu(hwnd, BuildMenu(state));
            AddTab(state, GetStr(STR_UNTITLED), NULL);
            state->hAccel = CreateAccels();
            ApplyTheme(state);

            LPWSTR cmdLine = GetCommandLineW();
            if (*cmdLine == L'"') {
                cmdLine++; while (*cmdLine && *cmdLine != L'"') cmdLine++;
                if (*cmdLine == L'"') cmdLine++;
            } else {
                while (*cmdLine && *cmdLine != L' ' && *cmdLine != L'\t') cmdLine++;
            }
            ProcessCommandLine(state, cmdLine);
            break;
        }

        case WM_ERASEBKGND: {
            if (state && state->darkTheme) {
                HDC hdc = (HDC)wParam;
                RECT rc; GetClientRect(hwnd, &rc);
                SetBkColor(hdc, RGB(25,25,25));
                ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
                return 1;
            }
            break;
        }

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            WCHAR path[MAX_PATH];
            if (DragQueryFile(hDrop, 0, path, MAX_PATH)) {
                TabInfo* cur = &state->tabs[state->currentTab];
                BOOL needNewTab = (cur->modified || cur->filepath[0] != L'\0');
                if (needNewTab) { AddTab(state, GetStr(STR_LOADING), NULL); SwitchToTab(state, state->tabCount - 1); }
                if (!LoadFileToTab(state, state->currentTab, path, -1)) { if (needNewTab) RemoveTab(state, state->currentTab); }
            }
            DragFinish(hDrop);
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HWND hStatic = (HWND)lParam; HDC hdc = (HDC)wParam; int id = GetDlgCtrlID(hStatic);
            if (id == IDC_LINENUMBERS) { SetTextColor(hdc, state->lineNumberTextColor); SetBkMode(hdc, TRANSPARENT); return (INT_PTR)state->hBkBrush; }
            else if (id == IDC_STATUS) { SetTextColor(hdc, state->statusTextColor); SetBkColor(hdc, state->statusBkColor); return (INT_PTR)state->hStatusBrush; }
            break;
        }

        case WM_SIZE: {
            RECT rc; GetClientRect(hwnd, &rc); int statHeight = 20;
            SetWindowPos(state->hStatus, NULL, 0, rc.bottom - statHeight, rc.right, statHeight, SWP_NOZORDER);
            SetWindowPos(state->hTab, NULL, 0, 0, rc.right, rc.bottom - statHeight, SWP_NOZORDER);
            if (state->tabCount > 0) ResizeControls(state);
            break;
        }

        case WM_NOTIFY: {
            NMHDR* nm = (NMHDR*)lParam;
            if (nm->idFrom == IDC_TAB && nm->code == TCN_SELCHANGE) { int sel = TabCtrl_GetCurSel(state->hTab); if (sel >= 0) SwitchToTab(state, sel); }
            break;
        }

        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            switch (id) {
                case IDM_NEW: AddTab(state, GetStr(STR_UNTITLED), NULL); SwitchToTab(state, state->tabCount - 1); break;
                case IDM_OPEN: {
                    WCHAR openFilter[] = L"All files (*.*)\0*.*\0";
                    OPENFILENAME ofn = {0}; ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFilter = openFilter;
                    ofn.lpstrFile = malloc(MAX_PATH * sizeof(WCHAR)); ofn.lpstrFile[0] = L'\0'; ofn.nMaxFile = MAX_PATH; ofn.nFilterIndex = 1; ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                    if (GetOpenFileName(&ofn)) {
                        const TabInfo* cur = &state->tabs[state->currentTab];
                        BOOL needNewTab = (cur->modified || cur->filepath[0] != L'\0');
                        if (needNewTab) { AddTab(state, GetStr(STR_LOADING), NULL); SwitchToTab(state, state->tabCount - 1); }
                        if (!LoadFileToTab(state, state->currentTab, ofn.lpstrFile, -1)) { if (needNewTab) RemoveTab(state, state->currentTab); }
                    }
                    free(ofn.lpstrFile); break;
                }
                case IDM_ENC_UTF8:    state->defaultEncoding = 0; SaveSettings(state); if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab); break;
                case IDM_ENC_UTF16LE: state->defaultEncoding = 1; SaveSettings(state); if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab); break;
                case IDM_ENC_UTF16BE: state->defaultEncoding = 2; SaveSettings(state); if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab); break;
                case IDM_ENC_WIN1251: state->defaultEncoding = 3; SaveSettings(state); if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab); break;
                case IDM_ENC_WIN1252: state->defaultEncoding = 4; SaveSettings(state); if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab); break;
                case IDM_SAVE: if (state->tabCount > 0) SaveTab(state, state->currentTab, FALSE); break;
                case IDM_SAVEAS: if (state->tabCount > 0) SaveTab(state, state->currentTab, TRUE); break;
                case IDM_CLOSE_TAB: case ID_ACCEL_CLOSE_TAB: OnCloseTab(state); break;
                case IDM_EXIT: DestroyWindow(hwnd); break;
                case IDM_HASH_MD5: if (state->tabCount > 0) ShowFileHash(hwnd, state->tabs[state->currentTab].rawData, state->tabs[state->currentTab].rawSize, CALG_MD5); break;
                case IDM_HASH_SHA1: if (state->tabCount > 0) ShowFileHash(hwnd, state->tabs[state->currentTab].rawData, state->tabs[state->currentTab].rawSize, CALG_SHA1); break;
                case IDM_HASH_SHA256: if (state->tabCount > 0) ShowFileHash(hwnd, state->tabs[state->currentTab].rawData, state->tabs[state->currentTab].rawSize, CALG_SHA_256); break;
                case IDM_UNDO: if (state->tabCount > 0) SendMessage(state->tabs[state->currentTab].hRichEdit, WM_UNDO, 0, 0); break;
                case IDM_CUT: if (state->tabCount > 0) SendMessage(state->tabs[state->currentTab].hRichEdit, WM_CUT, 0, 0); break;
                case IDM_COPY: if (state->tabCount > 0) SendMessage(state->tabs[state->currentTab].hRichEdit, WM_COPY, 0, 0); break;
                case IDM_PASTE: if (state->tabCount > 0) SendMessage(state->tabs[state->currentTab].hRichEdit, WM_PASTE, 0, 0); break;
                case IDM_SELECTALL: if (state->tabCount > 0) SendMessage(state->tabs[state->currentTab].hRichEdit, EM_SETSEL, 0, -1); break;
                case IDM_FIND: ShowFindDialog(state); break;
                case IDM_DARKTHEME: state->darkTheme = !state->darkTheme; ApplyTheme(state); break;
                case IDM_FONT: {
                    CHOOSEFONT cf = {0}; cf.lStructSize = sizeof(cf); cf.hwndOwner = hwnd; cf.lpLogFont = &state->lf; cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
                    if (ChooseFont(&cf)) { if (state->hFont) DeleteObject(state->hFont); state->hFont = CreateFontIndirect(&state->lf); ApplyFont(state); } break;
                }
                case IDM_HEXMODE: case ID_ACCEL_HEXMODE: if (state->tabCount > 0) SwitchHexMode(state, state->currentTab); break;
                case IDM_CONTEXTMENU: ToggleContextMenu(state); break;
                case IDM_LANG_EN: currentLang = 0; SaveSettings(state); { HMENU old = GetMenu(hwnd); if (old) DestroyMenu(old); SetMenu(hwnd, BuildMenu(state)); DrawMenuBar(hwnd); } ApplyTheme(state); if (state->tabCount > 0) UpdateWindowTitle(state); break;
                case IDM_LANG_RU: currentLang = 1; SaveSettings(state); { HMENU old = GetMenu(hwnd); if (old) DestroyMenu(old); SetMenu(hwnd, BuildMenu(state)); DrawMenuBar(hwnd); } ApplyTheme(state); if (state->tabCount > 0) UpdateWindowTitle(state); break;
                case ID_ACCEL_NEXT_TAB: if (state->tabCount > 1) SwitchToTab(state, (state->currentTab + 1) % state->tabCount); break;
                case ID_ACCEL_PREV_TAB: if (state->tabCount > 1) SwitchToTab(state, (state->currentTab - 1 + state->tabCount) % state->tabCount); break;
            }
            if (state->tabCount > 0) UpdateStatusBar(state, state->currentTab);
            break;
        }

        case WM_MOUSEWHEEL: case WM_VSCROLL: case WM_HSCROLL:
            if (state->tabCount > 0) { UpdateLineNumbers(state, state->currentTab); UpdateStatusBar(state, state->currentTab); }
            break;

        case WM_CLOSE: {
            for (int i = 0; i < state->tabCount; i++) {
                if (state->tabs[i].modified) {
                    SwitchToTab(state, i);
                    WCHAR msgClose[512]; _snwprintf(msgClose, 511, GetStr(STR_EXIT_SAVE_PROMPT), state->tabs[i].title);
                    int ret = MessageBox(hwnd, msgClose, GetStr(STR_TITLE), MB_YESNOCANCEL | MB_ICONQUESTION);
                    if (ret == IDCANCEL) return 0;
                    if (ret == IDYES) { if (!SaveTab(state, i, FALSE)) return 0; }
                }
            }
            SaveSettings(state); DestroyWindow(hwnd); break;
        }

        case WM_DESTROY:
            if (state) {
                DestroyAcceleratorTable(state->hAccel);
                for (int i = 0; i < state->tabCount; i++) {
                    RemoveWindowSubclass(state->tabs[i].hRichEdit, RichEditSubclassProc, SUBCLASS_RICHEDIT);
                    RemoveWindowSubclass(state->tabs[i].hLineNumbers, GutterSubclassProc, SUBCLASS_GUTTER);
                    DestroyWindow(state->tabs[i].hRichEdit); DestroyWindow(state->tabs[i].hSeparator);
                    DestroyWindow(state->tabs[i].hLineNumbers); DestroyWindow(state->tabs[i].hContainer);
                    free(state->tabs[i].rawData);
                }
                free(state->tabs);
                if (state->hFont) DeleteObject(state->hFont);
                if (state->hBkBrush) DeleteObject(state->hBkBrush);
                if (state->hSepBrush) DeleteObject(state->hSepBrush);
                if (state->hStatusBrush) DeleteObject(state->hStatusBrush);
                if (state->hMainBgBrush) DeleteObject(state->hMainBgBrush);
                free(state);
            }
            PostQuitMessage(0); break;

        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static void ProcessCommandLine(AppState* state, LPWSTR lpCmdLine) {
    while (*lpCmdLine == L' ' || *lpCmdLine == L'\t') lpCmdLine++;
    if (*lpCmdLine == L'\0') return;
    WCHAR path[MAX_PATH] = L"";
    if (lpCmdLine[0] == L'"') { lpCmdLine++; const WCHAR* end = wcschr(lpCmdLine, L'"'); if (end) { wcsncpy(path, lpCmdLine, end - lpCmdLine); path[end - lpCmdLine] = L'\0'; } else wcscpy(path, lpCmdLine); }
    else { const WCHAR* end = wcspbrk(lpCmdLine, L" \t"); if (end) { wcsncpy(path, lpCmdLine, end - lpCmdLine); path[end - lpCmdLine] = L'\0'; } else wcscpy(path, lpCmdLine); }
    if (path[0] == L'\0') return;
    TabInfo* cur = &state->tabs[state->currentTab];
    if (cur->modified || cur->filepath[0] != L'\0') { AddTab(state, GetStr(STR_LOADING), NULL); SwitchToTab(state, state->tabCount - 1); }
    if (!LoadFileToTab(state, state->currentTab, path, -1)) { if (state->tabCount > 0 && wcscmp(state->tabs[state->currentTab].title, GetStr(STR_LOADING)) == 0) RemoveTab(state, state->currentTab); }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    InitCommonControls();
    LoadLanguageSetting();
    if (currentLang == -1) {
        currentLang = ShowLanguageDialog(NULL);
        WCHAR path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        WCHAR* p = wcsrchr(path, L'\\');
        if (p) *(p+1) = L'\0';
        wcscat(path, L"settings.ini");
        WCHAR buf[16];
        _snwprintf(buf, 16, L"%d", currentLang);
        WritePrivateProfileString(L"Settings", L"Language", buf, path);
    }

    int argc; LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        if (argc > 1 && wcscmp(argv[1], L"/install") == 0) { MessageBox(NULL, InstallContextMenu() ? GetStr(STR_CONTEXTMENU_ADDED) : GetStr(STR_CONTEXTMENU_ADD_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION); LocalFree(argv); return 0; }
        if (argc > 1 && wcscmp(argv[1], L"/uninstall") == 0) { MessageBox(NULL, UninstallContextMenu() ? GetStr(STR_CONTEXTMENU_REMOVED) : GetStr(STR_CONTEXTMENU_REMOVE_FAIL), GetStr(STR_TITLE), MB_OK | MB_ICONINFORMATION); LocalFree(argv); return 0; }
        LocalFree(argv);
    }

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"JNClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(L"JNClass", L"", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1000, 650,
                             NULL, NULL, hInstance, NULL);
    if (!hwnd) return 1;

    AppState* state = (AppState*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (currentLang != -1) {
        HMENU hMenu = BuildMenu(state);
        SetMenu(hwnd, hMenu);
        DrawMenuBar(hwnd);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        if (!TranslateAccelerator(hwnd, state->hAccel, &message)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
    return (int)message.wParam;
}