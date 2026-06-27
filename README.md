# 📋 ClipboardFixer

A cross-platform C++ tool that repairs broken clipboard managers. Fixes the frozen/unresponsive Win+V panel on Windows 10/11.

---

## ⚡ Quick Start

Clone the repo and build from source.

**Windows (MSVC - Visual Studio Developer Command Prompt):**
```cmd
cl fix_clipboard.cpp /Fe:fix_clipboard.exe /link advapi32.lib shell32.lib
fix_clipboard.exe
```

**Windows (g++ via MinGW/MSYS2):**
```bash
g++ fix_clipboard.cpp -o fix_clipboard.exe -ladvapi32 -lshell32
./fix_clipboard.exe
```

**macOS:**
```bash
g++ fix_clipboard.cpp -o fix_clipboard
./fix_clipboard
```

**Linux:**
```bash
g++ fix_clipboard.cpp -o fix_clipboard
./fix_clipboard
```

> **Windows users:** Right-click the `.exe` and select *Run as administrator* for full repairs.

---

## 🖥️ Platform Support

| Platform        | Support    | What it does |
|-----------------|------------|--------------|
| Windows 10/11   | ✅ Full     | Force-kills frozen UI host, clears corrupt cache, restarts service, fixes registry |
| macOS           | ✅ Full     | Restarts `pboard` daemon, clears clipboard |
| Linux (X11)     | ✅ Full     | Detects clipboard tools, restarts Klipper on KDE |
| Linux (Wayland) | ✅ Partial  | Detects `wl-clipboard`, advises on missing tools |
| Android         | ⚠️ Manual   | See [Android section](#android) |
| iOS             | ❌ N/A      | iOS does not allow system clipboard access from scripts |

---

## 🪟 Windows - What Gets Fixed

The Win+V clipboard panel freezes (opens but won't scroll, click, or respond) when `TextInputHost.exe` gets stuck. Restarting the clipboard service alone does not help because the frozen UI process is still running on top of it.

The tool fixes this in order:

| Step | Root Cause | Fix Applied |
|------|-----------|-------------|
| 1 | Frozen `TextInputHost.exe` UI host | Force-killed via Windows process API |
| 2 | Corrupt or locked clipboard cache | Cache directory cleared while process is dead |
| 3 | Corrupt `cbdhsvc.dll` registration | Re-registered via `regsvr32` |
| 4 | Stalled clipboard service | Stopped and restarted via Windows Service API |
| 5 | Registry flag unset | `EnableClipboardHistory` set to `1` in `HKCU` |

### If it's still frozen after running:
1. Sign out and back in - this restarts `TextInputHost.exe` cleanly
2. Go to Settings, System, Clipboard and toggle Clipboard history off, wait 5 seconds, then back on
3. Run `sfc /scannow` in an admin Command Prompt to repair system files

---

## 🍎 macOS

Restarts the `pboard` pasteboard daemon and clears clipboard contents.

macOS has no built-in clipboard history. Recommended free options:
- **[Maccy](https://github.com/p0deje/Maccy)** - lightweight, open source
- **[Raycast](https://www.raycast.com/)** - full launcher with clipboard history

---

## 🐧 Linux

What the tool does:
- Detects X11 vs Wayland session
- Confirms a clipboard tool is installed (`xclip`, `xsel`, `wl-clipboard`)
- Restarts Klipper on KDE
- Advises GNOME users on next steps

Install a clipboard tool if missing:
```bash
sudo apt install xclip          # X11
sudo apt install wl-clipboard   # Wayland
sudo apt install parcellite     # GNOME lightweight manager
```

---

## 🤖 Android <a name="android"></a>

Android's sandbox prevents any script from modifying the system clipboard manager.

Manual fix:
1. Go to Settings, Apps, your keyboard app, Storage, and clear the cache
2. Force-stop the keyboard app and reopen it
3. Samsung users: open Samsung Keyboard Settings, go to Clipboard, and clear history
4. Consider switching to **[Gboard](https://play.google.com/store/apps/details?id=com.google.android.inputmethod.latin)**

---

## 📱 iOS

iOS does not expose system clipboard management to scripts or third-party apps. If your clipboard isn't working:
- Restart the app you're pasting into
- Restart your device
- Check Settings, Privacy and Security, Paste from Other Apps

---

## 🛠️ Requirements

- C++11 or later (`g++`, `clang++`, or MSVC)
- Windows: `advapi32` and `shell32` libs (included with any Windows SDK)
- Linux: `g++` via `sudo apt install build-essential`
- macOS: Xcode Command Line Tools via `xcode-select --install`
- Run as Administrator on Windows for full repairs

---

## 🤝 Contributing

Pull requests welcome. Open an issue with:
- Your OS and version
- What Win+V does (frozen, crashes, won't open, etc.)
- Output of the tool

---

## 📄 License

MIT
