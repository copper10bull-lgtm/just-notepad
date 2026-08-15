Just Notepad

A lightweight, tabbed text and hex viewer for Windows, written in pure C with WinAPI.  
It supports multiple encodings, a full-featured search, dark mode, and minimalistic design.

FEATURES

File Management
- Multi-tab interface (unlimited tabs)
- New document (Ctrl+N)
- Open file via dialog (Ctrl+O) or drag & drop
- Open file from command line
- Save (Ctrl+S) and Save As… (UTF-8 without BOM)
- Close tab (Ctrl+W) with unsaved changes prompt
- File size limit: 500 MB (larger files are rejected)
- Automatic text/binary detection: binary files open in read-only HEX mode
- HEX mode (Ctrl+H): address, hex dump, ASCII representation (truncated at 100 MB)

Text Editing
- RichEdit 5.0 (Msftedit.dll) with advanced typography
- Undo (Ctrl+Z), Redo (Ctrl+Y)
- Cut, Copy, Paste, Select All
- Modified indicator (asterisk in title and tab)

Encoding Support
- UTF-8 (with or without BOM)
- UTF-16 Little Endian / Big Endian (with or without BOM)
- Windows-1251 (Cyrillic)
- Windows-1252 (Western European)
- Encoding pre-selection via Encoding menu (saved in settings.ini)
- Status bar shows current encoding and file size

Search
- Find dialog (Ctrl+F) with direction and case-sensitivity options
- Wraps around at document boundaries
- "String not found" notification

User Interface
- Line number gutter (fully synchronised with scrolling)
- Status bar (line, column, encoding, file size)
- Resizable window, proper control scaling
- DPI-aware (Per Monitor V2 via manifest)
- Customisable monospaced font (default: Consolas, 10 pt) via Font…
- Minimalistic design

Themes
- Dark theme (toggle via View → Dark Theme or F11)
- Full dark mode: main window, tabs, status bar, gutter, scrollbars
- Light theme (default)

Localisation
- Full interface translation: English and Russian
- Language selection on first launch
- Switch at any time via Language → English / Russian
- All strings translated: menus, dialogs, status bar, error messages

Tools
- File hashing (Windows CryptoAPI):
  - MD5
  - SHA-1
  - SHA-256
- Context menu integration:
  - Add "Just Notepad" to Explorer right-click menu
  - Remove when not needed
  - Command-line switches /install and /uninstall

Settings
- settings.ini next to the executable stores:
  - Interface language
  - Dark theme on/off
  - Default encoding for BOM-less files
- Automatically loaded/saved

Technical Details
- Pure C + WinAPI, no external dependencies (.NET, Java, Electron)
- Native performance, low memory usage
- Fully portable – no installation required
- Unicode throughout (all wide-char API calls)
- Compact size after stripping and optional UPX compression

BUILD INSTRUCTIONS

cd scr
gcc *.c -o JustNotepad.exe -lcomctl32 -luxtheme -ldwmapi -ladvapi32 -mwindows

USAGE

Run JustNotepad.exe.
On the first launch, you will be prompted to choose the interface language.
Settings are saved automatically to settings.ini.

To integrate into the Explorer context menu:
JustNotepad.exe /install

To remove:
JustNotepad.exe /uninstall

(No administrator privileges required; writes to HKCU.)

LICENSE

This project is licensed under the Apache License 2.0.
Copyright © 2026 Alex (https://github.com/copper10bull-lgtm)
