// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "resource.h"
#include "core.h"
#include "strings.h"
#include "theme.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static BOOL IsTextFile(const BYTE* data, DWORD size) {
    if (size == 0) return TRUE;
    if (size >= 2 && data[0] == 0xFF && data[1] == 0xFE) return TRUE;
    if (size >= 2 && data[0] == 0xFE && data[1] == 0xFF) return TRUE;
    for (DWORD i = 0; i < size; i++) if (data[i] == 0) return FALSE;
    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCCH)data, size, NULL, 0);
    return !(wlen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
}

static void FillRichEditFromRawData(TabInfo* tab, int encoding) {
    if (!tab->rawData || tab->rawSize == 0) {
        SetWindowText(tab->hRichEdit, L"");
        return;
    }
    if (!tab->hexMode) {
        int bomOff = 0;
        if (encoding == CP_UTF8 && tab->rawSize >= 3 && tab->rawData[0]==0xEF && tab->rawData[1]==0xBB && tab->rawData[2]==0xBF) bomOff = 3;
        else if (encoding == 1200 && tab->rawSize >= 2 && tab->rawData[0]==0xFF && tab->rawData[1]==0xFE) bomOff = 2;
        else if (encoding == 1201 && tab->rawSize >= 2 && tab->rawData[0]==0xFE && tab->rawData[1]==0xFF) bomOff = 2;

        if (encoding == 1251 || encoding == 1252) {
            int wlen = MultiByteToWideChar(encoding, 0, (LPCCH)(tab->rawData + bomOff), tab->rawSize - bomOff, NULL, 0);
            if (wlen > 0) {
                WCHAR* wbuf = malloc((size_t)(wlen + 1) * sizeof(WCHAR));
                if (wbuf) {
                    int conv = MultiByteToWideChar(encoding, 0, (LPCCH)(tab->rawData + bomOff), tab->rawSize - bomOff, wbuf, wlen);
                    if (conv > 0) {
                        wbuf[conv] = L'\0';
                        SetWindowText(tab->hRichEdit, wbuf);
                    } else {
                        SetWindowText(tab->hRichEdit, L"");
                    }
                    free(wbuf);
                } else {
                    SetWindowText(tab->hRichEdit, L"");
                }
            } else {
                SetWindowText(tab->hRichEdit, L"");
            }
        } else if (encoding == 1200) {
            int wlen = (tab->rawSize - bomOff) / 2;
            if (wlen > 0) {
                WCHAR* wbuf = malloc((size_t)(wlen + 1) * sizeof(WCHAR));
                if (wbuf) {
                    memcpy(wbuf, tab->rawData + bomOff, wlen * sizeof(WCHAR));
                    wbuf[wlen] = L'\0';
                    SetWindowText(tab->hRichEdit, wbuf);
                    free(wbuf);
                } else {
                    SetWindowText(tab->hRichEdit, L"");
                }
            } else {
                SetWindowText(tab->hRichEdit, L"");
            }
        } else if (encoding == 1201) {
            int wlen = (tab->rawSize - bomOff) / 2;
            if (wlen > 0) {
                WCHAR* wbuf = malloc((size_t)(wlen + 1) * sizeof(WCHAR));
                if (wbuf) {
                    BYTE* src = tab->rawData + bomOff;
                    for (int i = 0; i < wlen; i++) wbuf[i] = (src[i*2] << 8) | src[i*2 + 1];
                    wbuf[wlen] = L'\0';
                    SetWindowText(tab->hRichEdit, wbuf);
                    free(wbuf);
                } else {
                    SetWindowText(tab->hRichEdit, L"");
                }
            } else {
                SetWindowText(tab->hRichEdit, L"");
            }
        } else {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)(tab->rawData + bomOff), tab->rawSize - bomOff, NULL, 0);
            if (wlen > 0) {
                WCHAR* wbuf = malloc((size_t)(wlen + 1) * sizeof(WCHAR));
                if (wbuf) {
                    int conv = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)(tab->rawData + bomOff), tab->rawSize - bomOff, wbuf, wlen);
                    if (conv > 0) {
                        wbuf[conv] = L'\0';
                        SetWindowText(tab->hRichEdit, wbuf);
                    } else {
                        SetWindowText(tab->hRichEdit, L"");
                    }
                    free(wbuf);
                } else {
                    SetWindowText(tab->hRichEdit, L"");
                }
            } else {
                SetWindowText(tab->hRichEdit, L"");
            }
        }
    } else {
        DWORD maxHexDump = 100 * 1024 * 1024, dumpSize = min(tab->rawSize, maxHexDump);
        DWORD lines = (dumpSize + 15) / 16, bufSize = lines * 80 + 1;
        WCHAR* buf = malloc(bufSize * sizeof(WCHAR));
        if (!buf) {
            SetWindowText(tab->hRichEdit, GetStr(STR_HEX_MEMORY_ERROR));
            return;
        }
        WCHAR* ptr = buf;
        for (DWORD i = 0; i < dumpSize; i += 16) {
            ptr += _snwprintf(ptr, 20, L"%08X  ", i);
            for (int j = 0; j < 16; j++) {
                if (i + j < dumpSize) ptr += _snwprintf(ptr, 4, L"%02X ", tab->rawData[i + j]);
                else ptr += _snwprintf(ptr, 4, L"   ");
                if (j == 7) *ptr++ = L' ';
            }
            *ptr++ = L' '; *ptr++ = L'|';
            for (int j = 0; j < 16 && (i + j) < dumpSize; j++) {
                BYTE c = tab->rawData[i + j];
                *ptr++ = (c >= 32 && c < 127) ? c : L'.';
            }
            *ptr++ = L'|'; *ptr++ = L'\r'; *ptr++ = L'\n';
        }
        *ptr = L'\0';
        SetWindowText(tab->hRichEdit, buf);
        free(buf);
        if (dumpSize < tab->rawSize) {
            SendMessage(tab->hRichEdit, EM_SETSEL, -1, -1);
            SendMessage(tab->hRichEdit, EM_REPLACESEL, FALSE, (LPARAM)GetStr(STR_HEX_TRUNCATED));
        }
    }
}

void AddTab(AppState* state, const WCHAR* title, const WCHAR* filepath) {
    if (state->tabCount >= state->tabCapacity) {
        state->tabCapacity = state->tabCapacity == 0 ? 8 : state->tabCapacity * 2;
        state->tabs = realloc(state->tabs, state->tabCapacity * sizeof(TabInfo));
    }
    int idx = state->tabCount++;
    memset(&state->tabs[idx], 0, sizeof(TabInfo));
    StringCchCopy(state->tabs[idx].title, 256, title);
    if (filepath) StringCchCopy(state->tabs[idx].filepath, MAX_PATH, filepath);
    state->tabs[idx].modified = FALSE;
    state->tabs[idx].rawData = NULL;
    state->tabs[idx].rawSize = 0;
    state->tabs[idx].hexMode = FALSE;

    state->tabs[idx].hContainer = CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, 0,0,0,0, state->hTab, NULL, GetModuleHandle(NULL), NULL);
    state->tabs[idx].hLineNumbers = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY, 0,0,60,0, state->tabs[idx].hContainer, (HMENU)IDC_LINENUMBERS, GetModuleHandle(NULL), NULL);
    if (state->hFont) SendMessage(state->tabs[idx].hLineNumbers, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    SetWindowSubclass(state->tabs[idx].hLineNumbers, GutterSubclassProc, SUBCLASS_GUTTER, (DWORD_PTR)&state->tabs[idx]);

    state->tabs[idx].hSeparator = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_ETCHEDFRAME, 0,0,1,0, state->tabs[idx].hContainer, (HMENU)IDC_SEPARATOR, GetModuleHandle(NULL), NULL);

    static HMODULE hRichEdit = NULL;
    if (!hRichEdit) {
        hRichEdit = LoadLibrary(L"Msftedit.dll");
        if (!hRichEdit) { MessageBox(state->hwndMain, GetStr(STR_MSFTEDIT_NOTFOUND), L"Fatal Error", MB_ICONERROR); exit(1); }
    }
    state->tabs[idx].hRichEdit = CreateWindowEx(0, L"RICHEDIT50W", NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL | WS_VSCROLL | WS_HSCROLL | ES_SAVESEL, 0,0,0,0, state->tabs[idx].hContainer, NULL, GetModuleHandle(NULL), NULL);
    if (!state->tabs[idx].hRichEdit) { MessageBox(state->hwndMain, GetStr(STR_CREATE_RICHEDIT_FAIL), L"Fatal Error", MB_ICONERROR); exit(1); }
    if (state->hFont) SendMessage(state->tabs[idx].hRichEdit, WM_SETFONT, (WPARAM)state->hFont, TRUE);
    SendMessage(state->tabs[idx].hRichEdit, EM_SETTYPOGRAPHYOPTIONS, TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY);
    SetWindowSubclass(state->tabs[idx].hRichEdit, RichEditSubclassProc, SUBCLASS_RICHEDIT, (DWORD_PTR)&state->tabs[idx]);

    TCITEM tie = {0}; tie.mask = TCIF_TEXT; tie.pszText = (LPWSTR)title;
    TabCtrl_InsertItem(state->hTab, idx, &tie);

    if (state->currentTab == -1) SwitchToTab(state, idx);
    else ShowWindow(state->tabs[idx].hContainer, SW_HIDE);
    RecalcGutterWidth(state, idx);
    ApplyTheme(state);
}

void RemoveTab(AppState* state, int index) {
    if (index < 0 || index >= state->tabCount) return;
    TabInfo* tab = &state->tabs[index];
    RemoveWindowSubclass(tab->hRichEdit, RichEditSubclassProc, SUBCLASS_RICHEDIT);
    RemoveWindowSubclass(tab->hLineNumbers, GutterSubclassProc, SUBCLASS_GUTTER);
    DestroyWindow(tab->hRichEdit); DestroyWindow(tab->hSeparator);
    DestroyWindow(tab->hLineNumbers); DestroyWindow(tab->hContainer);
    free(tab->rawData);
    tab->rawData = NULL;
    TabCtrl_DeleteItem(state->hTab, index);
    if (index < state->tabCount - 1)
        memmove(&state->tabs[index], &state->tabs[index + 1], (state->tabCount - index - 1) * sizeof(TabInfo));
    state->tabCount--;
    if (state->tabCount == 0) {
        AddTab(state, GetStr(STR_UNTITLED), NULL);
        state->currentTab = 0; SwitchToTab(state, 0);
    } else if (index <= state->currentTab) {
        int newCur = state->currentTab;
        if (newCur >= state->tabCount) newCur = state->tabCount - 1;
        SwitchToTab(state, newCur);
    }
    UpdateWindowTitle(state);
}

void SwitchToTab(AppState* state, int index) {
    if (index < 0 || index >= state->tabCount) return;
    if (state->currentTab >= 0 && state->currentTab < state->tabCount)
        ShowWindow(state->tabs[state->currentTab].hContainer, SW_HIDE);
    state->currentTab = index;
    ShowWindow(state->tabs[index].hContainer, SW_SHOW);
    SetFocus(state->tabs[index].hRichEdit);
    TabCtrl_SetCurSel(state->hTab, index);
    ResizeControls(state);
    UpdateLineNumbers(state, index);
    UpdateWindowTitle(state);
    UpdateStatusBar(state, index);
}

void UpdateTabTitle(AppState* state, int index) {
    WCHAR title[270];
    if (state->tabs[index].modified)
        StringCchPrintf(title, 270, L"*%s", state->tabs[index].title);
    else
        StringCchCopy(title, 270, state->tabs[index].title);
    TCITEM tie = {0}; tie.mask = TCIF_TEXT; tie.pszText = title;
    TabCtrl_SetItem(state->hTab, index, &tie);
    if (index == state->currentTab) { UpdateWindowTitle(state); UpdateStatusBar(state, index); }
}

void UpdateWindowTitle(const AppState* state) {
    if (state->tabCount == 0) return;
    const TabInfo* tab = &state->tabs[state->currentTab];
    WCHAR caption[512], fname[256];
    if (tab->filepath[0]) {
        const WCHAR* p = wcsrchr(tab->filepath, L'\\');
        if (p) StringCchCopy(fname, 256, p + 1);
        else StringCchCopy(fname, 256, tab->filepath);
    } else {
        StringCchCopy(fname, 256, tab->title);
    }
    StringCchPrintf(caption, 512, L"%s%s - %s", tab->modified ? L"*" : L"", fname, GetStr(STR_TITLE));
    SetWindowText(state->hwndMain, caption);
}

void ResizeControls(AppState* state) {
    if (state->tabCount == 0) return;
    TabInfo* tab = &state->tabs[state->currentTab];
    RECT rcTab, rcCont;
    GetClientRect(state->hTab, &rcTab);
    TabCtrl_AdjustRect(state->hTab, FALSE, &rcTab);
    SetWindowPos(tab->hContainer, NULL, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, SWP_NOZORDER);
    GetClientRect(tab->hContainer, &rcCont);
    int gutterW = state->gutterWidth, sepW = 2;
    SetWindowPos(tab->hLineNumbers, NULL, 0, 0, gutterW, rcCont.bottom, SWP_NOZORDER);
    SetWindowPos(tab->hSeparator, NULL, gutterW, 0, sepW, rcCont.bottom, SWP_NOZORDER);
    SetWindowPos(tab->hRichEdit, NULL, gutterW + sepW, 0, rcCont.right - gutterW - sepW, rcCont.bottom, SWP_NOZORDER);
    UpdateLineNumbers(state, state->currentTab);
}

void OnCloseTab(AppState* state) {
    if (state->tabCount == 0) return;
    int idx = state->currentTab;
    if (state->tabs[idx].modified) {
        WCHAR msg[512]; StringCchPrintf(msg, 512, GetStr(STR_CLOSE_SAVE_PROMPT), state->tabs[idx].title);
        int ret = MessageBox(state->hwndMain, msg, GetStr(STR_MENU_CLOSETAB), MB_YESNOCANCEL | MB_ICONQUESTION);
        if (ret == IDCANCEL) return;
        if (ret == IDYES) { if (!SaveTab(state, idx, FALSE)) return; }
    }
    RemoveTab(state, idx);
}

BOOL SwitchHexMode(AppState* state, int index) {
    TabInfo* tab = &state->tabs[index];
    if (tab->hexMode) {
        tab->hexMode = FALSE; SendMessage(tab->hRichEdit, EM_SETREADONLY, FALSE, 0); FillRichEditFromRawData(tab, state->defaultEncoding);
    } else {
        if (tab->modified && tab->rawData) {
            int ret = MessageBox(state->hwndMain, GetStr(STR_HEX_WARNING), GetStr(STR_TITLE), MB_YESNO | MB_ICONWARNING);
            if (ret == IDYES) { if (!SaveTab(state, index, FALSE)) return FALSE; }
            else if (ret == IDNO) tab->modified = FALSE;
            else return FALSE;
        }
        tab->hexMode = TRUE; SendMessage(tab->hRichEdit, EM_SETREADONLY, TRUE, 0); FillRichEditFromRawData(tab, 0);
    }
    UpdateTabTitle(state, index);
    RecalcGutterWidth(state, index);
    UpdateLineNumbers(state, index);
    UpdateStatusBar(state, index);
    SendMessage(tab->hRichEdit, EM_SETSEL, 0, 0);
    return TRUE;
}

BOOL LoadFileToTab(AppState* state, int index, const WCHAR* filepath, int encoding) {
    TabInfo* tab = &state->tabs[index];
    free(tab->rawData);
    tab->rawData = NULL;
    tab->rawSize = 0;

    HANDLE hFile = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBox(state->hwndMain, GetStr(STR_ERROR_OPEN_FILE), GetStr(STR_TITLE), MB_ICONERROR);
        return FALSE;
    }
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > MAX_FILE_SIZE) {
        CloseHandle(hFile);
        MessageBox(state->hwndMain, GetStr(STR_FILE_TOO_BIG), GetStr(STR_TITLE), MB_ICONWARNING);
        return FALSE;
    }
    if (fileSize == 0) {
        CloseHandle(hFile);
        SetWindowText(tab->hRichEdit, L"");
        StringCchCopy(tab->filepath, MAX_PATH, filepath);
        const WCHAR* p = wcsrchr(filepath, L'\\');
        if (p) StringCchCopy(tab->title, 256, p + 1);
        else StringCchCopy(tab->title, 256, filepath);
        tab->modified = FALSE;
        tab->hexMode = FALSE;
        SendMessage(tab->hRichEdit, EM_SETREADONLY, FALSE, 0);
        UpdateTabTitle(state, index);
        RecalcGutterWidth(state, index);
        ResizeControls(state);
        return TRUE;
    }
    tab->rawData = malloc(fileSize);
    if (!tab->rawData) {
        CloseHandle(hFile);
        MessageBox(state->hwndMain, GetStr(STR_NOT_ENOUGH_MEMORY), GetStr(STR_TITLE), MB_ICONERROR);
        return FALSE;
    }
    DWORD read = 0;
    BOOL ok = ReadFile(hFile, tab->rawData, fileSize, &read, NULL);
    CloseHandle(hFile);
    if (!ok || read != fileSize) {
        free(tab->rawData);
        tab->rawData = NULL;
        MessageBox(state->hwndMain, GetStr(STR_ERROR_READ_FILE), GetStr(STR_TITLE), MB_ICONERROR);
        return FALSE;
    }
    tab->rawSize = fileSize;
    BOOL isText = IsTextFile(tab->rawData, tab->rawSize);
    if (!isText) {
        tab->hexMode = TRUE;
        SendMessage(tab->hRichEdit, EM_SETREADONLY, TRUE, 0);
        FillRichEditFromRawData(tab, 0);
    } else {
        BOOL hasBOM = (tab->rawSize >= 3 && tab->rawData[0]==0xEF && tab->rawData[1]==0xBB && tab->rawData[2]==0xBF) ||
                      (tab->rawSize >= 2 && tab->rawData[0]==0xFF && tab->rawData[1]==0xFE) ||
                      (tab->rawSize >= 2 && tab->rawData[0]==0xFE && tab->rawData[1]==0xFF);
        if (!hasBOM) {
            if (encoding == -1) encoding = state->defaultEncoding;
        } else {
            if (tab->rawData[0]==0xFF && tab->rawData[1]==0xFE) encoding = 1200;
            else if (tab->rawData[0]==0xFE && tab->rawData[1]==0xFF) encoding = 1201;
            else encoding = CP_UTF8;
        }
        state->defaultEncoding = encoding;
        tab->hexMode = FALSE;
        SendMessage(tab->hRichEdit, EM_SETREADONLY, FALSE, 0);
        FillRichEditFromRawData(tab, encoding);
    }
    StringCchCopy(tab->filepath, MAX_PATH, filepath);
    const WCHAR* p = wcsrchr(filepath, L'\\');
    if (p) StringCchCopy(tab->title, 256, p + 1);
    else StringCchCopy(tab->title, 256, filepath);
    tab->modified = FALSE;
    UpdateTabTitle(state, index);
    RecalcGutterWidth(state, index);
    ResizeControls(state);
    return TRUE;
}

BOOL SaveTab(AppState* state, int index, BOOL forceSaveAs) {
    TabInfo* tab = &state->tabs[index];
    WCHAR path[MAX_PATH] = L"";
    if (!forceSaveAs && tab->filepath[0]) {
        StringCchCopy(path, MAX_PATH, tab->filepath);
    } else {
        WCHAR saveFilter[] = L"Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
        OPENFILENAME ofn = {0};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = state->hwndMain;
        ofn.lpstrFilter = saveFilter;
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.nFilterIndex = 2;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (!GetSaveFileName(&ofn)) return FALSE;
        StringCchCopy(tab->filepath, MAX_PATH, path);
        const WCHAR* p = wcsrchr(path, L'\\');
        if (p) StringCchCopy(tab->title, 256, p + 1);
        else StringCchCopy(tab->title, 256, path);
    }
    GETTEXTEX gtx = {0};
    gtx.cb = GetWindowTextLength(tab->hRichEdit);
    if (gtx.cb == 0) {
        HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
        tab->modified = FALSE;
        UpdateTabTitle(state, index);
        return TRUE;
    }
    gtx.cb++;
    WCHAR* wbuf = malloc((size_t)gtx.cb * sizeof(WCHAR));
    if (!wbuf) return FALSE;
    gtx.flags = GT_DEFAULT;
    gtx.codepage = 1200;
    SendMessage(tab->hRichEdit, EM_GETTEXTEX, (WPARAM)&gtx, (LPARAM)wbuf);
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, NULL, 0, NULL, NULL);
    if (utf8len <= 0) {
        free(wbuf);
        return FALSE;
    }
    char* utf8buf = malloc((size_t)utf8len);
    if (!utf8buf) {
        free(wbuf);
        return FALSE;
    }
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, utf8buf, utf8len, NULL, NULL);
    free(wbuf);
    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        free(utf8buf);
        return FALSE;
    }
    DWORD written = 0;
    BOOL ok = WriteFile(hFile, utf8buf, utf8len - 1, &written, NULL);
    CloseHandle(hFile);
    free(utf8buf);
    if (!ok || written != (DWORD)(utf8len - 1)) return FALSE;
    tab->modified = FALSE;
    UpdateTabTitle(state, index);
    return TRUE;
}

void UpdateStatusBar(const AppState* state, int index) {
    const TabInfo* tab = &state->tabs[index];
    WCHAR text[512];
    int line = SendMessage(tab->hRichEdit, EM_LINEFROMCHAR, -1, 0) + 1;
    LRESULT sel = SendMessage(tab->hRichEdit, EM_GETSEL, 0, 0);
    int start = LOWORD(sel), lineIndex = SendMessage(tab->hRichEdit, EM_LINEINDEX, line - 1, 0);
    int col = start - lineIndex + 1;
    ULONGLONG fileSize = 0;
    if (tab->filepath[0]) {
        HANDLE hFile = CreateFile(tab->filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) { GetFileSizeEx(hFile, (PLARGE_INTEGER)&fileSize); CloseHandle(hFile); }
    }
    const WCHAR* encNames[] = { GetStr(STR_ENC_UTF8), GetStr(STR_ENC_UTF16LE), GetStr(STR_ENC_UTF16BE), GetStr(STR_ENC_WIN1251), GetStr(STR_ENC_WIN1252) };
    const WCHAR* enc = (state->defaultEncoding >=0 && state->defaultEncoding <=4) ? encNames[state->defaultEncoding] : L"???";
    WCHAR lineColPart[128], sizePart[256];
    StringCchPrintf(lineColPart, 128, GetStr(STR_STATUS_LINE_COL), line, col);
    StringCchPrintf(sizePart, 256, GetStr(STR_STATUS_ENC_SIZE), enc, fileSize);
    StringCchPrintf(text, 512, L"  %s  |  %s", lineColPart, sizePart);
    SetWindowText(state->hStatus, text);
}

void ShowFileHash(HWND hwnd, const BYTE* data, DWORD size, ALG_ID algId) {
    if (!data || size == 0) { MessageBox(hwnd, GetStr(STR_HASH_EMPTY), GetStr(STR_HASH_TITLE), MB_OK | MB_ICONINFORMATION); return; }
    HCRYPTPROV hProv = 0; HCRYPTHASH hHash = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) return;
    if (!CryptCreateHash(hProv, algId, 0, 0, &hHash)) { CryptReleaseContext(hProv, 0); return; }
    if (!CryptHashData(hHash, data, size, 0)) { CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0); return; }
    DWORD hashLen = 0, hashLenSize = sizeof(hashLen);
    if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&hashLen, &hashLenSize, 0)) { CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0); return; }
    BYTE* hash = malloc(hashLen); if (!hash) { CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0); return; }
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) { free(hash); CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0); return; }
    CryptDestroyHash(hHash); CryptReleaseContext(hProv, 0);
    int hexLen = hashLen * 2 + 1; WCHAR* szHash = malloc(hexLen * sizeof(WCHAR));
    if (szHash) {
        WCHAR* ptr = szHash;
        for (DWORD i = 0; i < hashLen; i++) { ptr += _snwprintf(ptr, 3, L"%02X", hash[i]); }
        *ptr = L'\0'; MessageBox(hwnd, szHash, GetStr(STR_HASH_TITLE), MB_OK | MB_ICONINFORMATION); free(szHash);
    }
    free(hash);
}

static UINT_PTR CALLBACK FindHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
    if (uiMsg == WM_INITDIALOG) {
        AppState* state = (AppState*)((FINDREPLACE*)lParam)->lCustData;
        SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)state);
        return TRUE;
    }
    return FALSE;
}

void ShowFindDialog(AppState* state) {
    if (state->hwndFind && IsWindow(state->hwndFind)) {
        SetForegroundWindow(state->hwndFind);
        return;
    }
    ZeroMemory(&state->fr, sizeof(FINDREPLACE));
    state->fr.lStructSize = sizeof(FINDREPLACE);
    state->fr.hwndOwner = state->hwndMain;
    state->fr.lpstrFindWhat = state->findWhat;
    state->fr.wFindWhatLen = 128;
    state->fr.lCustData = (LPARAM)state;
    state->fr.Flags = FR_DOWN;
    state->fr.lpfnHook = FindHookProc;
    state->hwndFind = FindTextW(&state->fr);
    if (state->hwndFind) state->findActive = TRUE;
}

void DoFind(HWND hwnd, FINDREPLACE* fr) {
    AppState* state = (AppState*)fr->lCustData;
    if (!state || state->tabCount == 0) return;
    TabInfo* tab = &state->tabs[state->currentTab];
    if (tab->hexMode) return;
    HWND hEdit = tab->hRichEdit;

    DWORD findFlags = 0;
    if (fr->Flags & FR_DOWN) findFlags = FR_DOWN;
    if (fr->Flags & FR_MATCHCASE) findFlags |= FR_MATCHCASE;

    FINDTEXTEX ft = {0};
    if (fr->Flags & FR_DOWN) {
        ft.chrg.cpMin = (LONG)(SendMessage(hEdit, EM_GETSEL, 0, 0) & 0xFFFF) + 1;
        ft.chrg.cpMax = -1;
    } else {
        ft.chrg.cpMin = 0;
        ft.chrg.cpMax = (LONG)(SendMessage(hEdit, EM_GETSEL, 0, 0) & 0xFFFF);
    }
    ft.lpstrText = fr->lpstrFindWhat;
    LRESULT pos = SendMessage(hEdit, EM_FINDTEXTEX, findFlags, (LPARAM)&ft);
    if (pos == -1) {
        if (fr->Flags & FR_DOWN) {
            ft.chrg.cpMin = 0;
            ft.chrg.cpMax = -1;
        } else {
            ft.chrg.cpMin = 0;
            ft.chrg.cpMax = -1;
        }
        pos = SendMessage(hEdit, EM_FINDTEXTEX, findFlags, (LPARAM)&ft);
    }
    if (pos != -1) {
        SendMessage(hEdit, EM_SETSEL, pos, pos + wcslen(fr->lpstrFindWhat));
        SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
    } else {
        MessageBox(state->hwndMain, GetStr(STR_FIND_NOT_FOUND), GetStr(STR_FIND_TITLE), MB_ICONINFORMATION);
    }
}

void UpdateLineNumbers(const AppState* state, int index) {
    if (index >= 0 && index < state->tabCount) {
        InvalidateRect(state->tabs[index].hLineNumbers, NULL, FALSE);
        UpdateWindow(state->tabs[index].hLineNumbers);
    }
}

void RecalcGutterWidth(AppState* state, int index) {
    TabInfo* tab = &state->tabs[index];
    int totalLines = SendMessage(tab->hRichEdit, EM_GETLINECOUNT, 0, 0);
    int digits = 1, temp = totalLines;
    while (temp >= 10) { temp /= 10; digits++; }
    if (digits < 3) digits = 3;
    int newWidth = 8 + digits * 8;
    if (newWidth < 40) newWidth = 40;
    if (newWidth != state->gutterWidth) {
        state->gutterWidth = newWidth;
        if (index == state->currentTab) ResizeControls(state);
    } else {
        if (index == state->currentTab) UpdateLineNumbers(state, index);
    }
}

LRESULT CALLBACK RichEditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uIdSubclass == SUBCLASS_RICHEDIT) {
        TabInfo* tab = (TabInfo*)dwRefData;
        if (tab) {
            HWND hMain = GetAncestor(hWnd, GA_ROOT);
            AppState* state = (AppState*)GetWindowLongPtr(hMain, GWLP_USERDATA);
            if (state) {
                int idx = -1;
                for (int i = 0; i < state->tabCount; i++) if (&state->tabs[i] == tab) { idx = i; break; }
                switch (uMsg) {
                    case WM_COMMAND:
                        if (HIWORD(wParam) == EN_CHANGE && !tab->hexMode) {
                            tab->modified = TRUE;
                            if (idx != -1) { UpdateTabTitle(state, idx); UpdateStatusBar(state, idx); RecalcGutterWidth(state, idx); }
                        }
                        break;
                    case WM_VSCROLL:
                    case WM_HSCROLL:
                    case WM_MOUSEWHEEL:
                    case WM_KEYDOWN:
                        if (idx != -1) {
                            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                            UpdateLineNumbers(state, idx);
                            UpdateStatusBar(state, idx);
                            return res;
                        }
                        break;
                }
            }
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GutterSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uIdSubclass == SUBCLASS_GUTTER && uMsg == WM_PAINT) {
        TabInfo* tab = (TabInfo*)dwRefData;
        if (!tab) return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps); RECT rc; GetClientRect(hWnd, &rc);
        HWND hMain = GetAncestor(hWnd, GA_ROOT);
        AppState* state = (AppState*)GetWindowLongPtr(hMain, GWLP_USERDATA);
        if (!state) { EndPaint(hWnd, &ps); return 0; }

        FillRect(hdc, &rc, state->hBkBrush);
        HPEN hPen = CreatePen(PS_SOLID, 1, state->separatorColor);
        HPEN oldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, rc.right - 1, 0, NULL); LineTo(hdc, rc.right - 1, rc.bottom);
        SelectObject(hdc, oldPen); DeleteObject(hPen);

        HWND hEdit = tab->hRichEdit;
        int totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);
        if (totalLines > 0) {
            int firstLine = SendMessage(hEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
            if (firstLine < 0) firstLine = 0;
            int charIndex = SendMessage(hEdit, EM_LINEINDEX, firstLine, 0);
            POINT pt = {0};
            if (charIndex >= 0) SendMessage(hEdit, EM_POSFROMCHAR, (WPARAM)&pt, charIndex);
            int topOffset = pt.y;
            int lineHeight = SendMessage(hEdit, EM_GETLINEHEIGHT, 0, 0);
            if (lineHeight <= 0) {
                TEXTMETRIC tm; HFONT hFont = state->hFont; HFONT oldFont = hFont ? SelectObject(hdc, hFont) : NULL;
                GetTextMetrics(hdc, &tm); lineHeight = tm.tmHeight + tm.tmExternalLeading;
                if (oldFont) SelectObject(hdc, oldFont);
            }
            HFONT hFont = state->hFont; HFONT oldFont = hFont ? SelectObject(hdc, hFont) : NULL;
            SetTextColor(hdc, state->lineNumberTextColor); SetBkMode(hdc, TRANSPARENT);
            int visibleLines = (rc.bottom - topOffset + lineHeight - 1) / lineHeight + 1;
            if (visibleLines < 0) visibleLines = 0;
            RECT lineRect = rc;
            lineRect.left += 2; lineRect.right -= 4;
            for (int i = 0; i < visibleLines && (firstLine + i) < totalLines; i++) {
                int lineNum = firstLine + i + 1; WCHAR buf[20]; StringCchPrintf(buf, 20, L"%d", lineNum);
                lineRect.top = topOffset + i * lineHeight; lineRect.bottom = lineRect.top + lineHeight;
                DrawText(hdc, buf, -1, &lineRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
            }
            if (oldFont) SelectObject(hdc, oldFont);
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}