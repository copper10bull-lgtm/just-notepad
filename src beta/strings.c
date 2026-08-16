// Copyright (c) 2026 https://github.com/copper10bull-lgtm
// SPDX-License-Identifier: Apache-2.0
#include "common.h"
#include "strings.h"

int currentLang = -1;

const WCHAR* strings[2][STR_COUNT] = {
    {
        L"&File", L"&New\tCtrl+N", L"&Open...\tCtrl+O", L"&Save\tCtrl+S", L"Save &As...",
        L"&Close Tab\tCtrl+W", L"E&xit", L"&Hash", L"MD&5", L"SHA-&1", L"SHA-&256",
        L"&Edit", L"&Undo\tCtrl+Z", L"Cu&t\tCtrl+X", L"&Copy\tCtrl+C", L"&Paste\tCtrl+V",
        L"Select &All\tCtrl+A", L"&Find...\tCtrl+F",
        L"&View", L"&Dark Theme\tF11", L"&Font...", L"&HEX Mode\tCtrl+H",
        L"&Language", L"&English", L"&Russian",
        L"Add to Context Menu", L"Remove from Context Menu",
        L"Untitled", L"Loading...", L"Failed to open file!", L"Failed to open file from command line!",
        L"The file is too large (over 500 MB). Please open a smaller file.",
        L"Save changes to \"%s\"?", L"Save changes to \"%s\"?",
        L"Ln: %d, Col: %d", L"UTF-8 | Size: %llu bytes", L"Just Notepad",
        L"Choose language:", L"English", L"Русский", L"Hash", L"No data to hash.",
        L"Context menu item installed.", L"Context menu item removed.",
        L"Failed to install context menu item.", L"Failed to remove context menu item.",
        L"Unsaved changes will be lost when switching to HEX mode. Save first?",
        L"Msftedit.dll not found!", L"Could not create RichEdit control!",
        L"Not enough memory to open the file.", L"Error reading file.",
        L"Not enough memory to display HEX.", L"... HEX display truncated (file too large) ...",
        L"&Encoding", L"UTF-8", L"UTF-16 LE", L"UTF-16 BE",
        L"Windows-1251", L"Windows-1252",
        L"%s | Size: %llu bytes",
        L"Find", L"Find what:", L"String not found."
    },
    {
        L"&Файл", L"&Новый\tCtrl+N", L"&Открыть...\tCtrl+O", L"&Сохранить\tCtrl+S", L"Сохранить &как...",
        L"&Закрыть вкладку\tCtrl+W", L"В&ыход", L"&Хеш", L"MD&5", L"SHA-&1", L"SHA-&256",
        L"&Правка", L"&Отменить\tCtrl+Z", L"&Вырезать\tCtrl+X", L"&Копировать\tCtrl+C", L"&Вставить\tCtrl+V",
        L"Выделить &всё\tCtrl+A", L"&Найти...\tCtrl+F",
        L"&Вид", L"&Тёмная тема\tF11", L"&Шрифт...", L"&HEX-режим\tCtrl+H",
        L"&Язык", L"&Английский", L"&Русский",
        L"Добавить в контекстное меню", L"Удалить из контекстного меню",
        L"Безымянный", L"Загрузка...", L"Не удалось открыть файл!", L"Не удалось открыть файл из командной строки!",
        L"Файл слишком большой (более 500 МБ). Пожалуйста, откройте файл меньшего размера.",
        L"Сохранить изменения в \"%s\"?", L"Сохранить изменения в \"%s\"?",
        L"Стр: %d, Стлб: %d", L"UTF-8 | Размер: %llu байт", L"Just Notepad",
        L"Выберите язык:", L"English", L"Русский", L"Хеш", L"Нет данных для хеширования.",
        L"Пункт контекстного меню установлен.", L"Пункт контекстного меню удалён.",
        L"Не удалось установить пункт контекстного меню.", L"Не удалось удалить пункт контекстного меню.",
        L"Несохранённые изменения будут потеряны при переключении в HEX-режим. Сохранить сначала?",
        L"Msftedit.dll не найден!", L"Не удалось создать RichEdit!",
        L"Недостаточно памяти для открытия файла.", L"Ошибка чтения файла.",
        L"Недостаточно памяти для отображения HEX.", L"... HEX отображение усечено (файл слишком большой) ...",
        L"&Кодировка", L"UTF-8", L"UTF-16 LE", L"UTF-16 BE",
        L"Windows-1251", L"Windows-1252",
        L"%s | Размер: %llu байт",
        L"Найти", L"Что искать:", L"Строка не найдена."
    }
};

const WCHAR* GetStr(enum StringId id) {
    return strings[currentLang == 1 ? 1 : 0][id];
}