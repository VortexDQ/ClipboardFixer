# 📋 ClipboardFixer

Fixes the frozen/unresponsive Win+V clipboard panel on Windows 10/11. Also supports macOS and Linux.

---

## ⚡ Installation

Open **Windows Terminal** and run these commands one by one:

**1. Install a C++ compiler:**
```
winget install -e --id MSYS2.MSYS2
```

**2. Close and reopen Windows Terminal, then set up g++:**
```
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-gcc"
```

**3. Clone the repo:**
```
git clone https://github.com/VortexDQ/ClipboardFixer.git
cd ClipboardFixer
```

**4. Compile:**
```
C:\msys64\mingw64\bin\g++.exe fix_clipboard.cpp -o fix_clipboard.exe -ladvapi32 -lshell32
```

**5. Run as administrator:**
```
Start-Process ./fix_clipboard.exe -Verb RunAs
```

That's it. Win+V should work after it finishes.

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

The Win+V clipboard panel freezes when `TextInputHost.exe` gets stuck. Restarting the clipboard service alone does not fix it because the frozen UI process is still running on top of it.

The tool fixes this in order:

| Step | Root Cause | Fix Applied |
|------|-----------|-------------|
| 1 | Frozen `TextInputHost.exe` UI host | Force-killed via Windows process API |
| 2 | Corrupt or locked clipboard cache | Cache cleared while process is dead |
| 3 | Corrupt `cbdhsvc.dll` registration | Re-registered via `regsvr32` |
| 4 | Stalled clipboard service | Stopped and restarted via Windows Service API |
| 5 | Registry flag unset | `EnableClipboardHistory` set to `1` in `HKCU` |

### If it's still frozen after running:
1. Sign out and back in - this restarts `TextInputHost.exe` cleanly
2. Go to Settings, System, Clipboard and toggle Clipboard history off, wait 5 seconds, then back on
3. Run `sfc /scannow` in an admin Command Prompt to repair system files

---

## 🍎 macOS

```bash
g++ fix_clipboard.cpp -o fix_clipboard && ./fix_clipboard
```

macOS has no built-in clipboard history. Recommended free options:
- **[Maccy](https://github.com/p0deje/Maccy)** - lightweight, open source
- **[Raycast](https://www.raycast.com/)** - full launcher with clipboard history

---

## 🐧 Linux

```bash
g++ fix_clipboard.cpp -o fix_clipboard && ./fix_clipboard
```

Install a clipboard tool if missing:
```bash
sudo apt install xclip          # X11
sudo apt install wl-clipboard   # Wayland
sudo apt install parcellite     # GNOME
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

## 🤝 Contributing

Pull requests welcome. Open an issue with:
- Your OS and version
- What Win+V does (frozen, crashes, won't open, etc.)
- Output of the tool

---

## 📄 License

MIT
