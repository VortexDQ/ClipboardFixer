# 📋 ClipboardFixer

Fixes the frozen/unresponsive Win+V clipboard panel on Windows 10/11. Also supports macOS and Linux.

---

## ⚡ Installation

Open **Windows Terminal** and run these commands one by one:

**1. Install Git:**

```
winget install -e --id Git.Git
```

**2. Install a C++ compiler:**

```
winget install -e --id MSYS2.MSYS2
```

**3. Close and reopen Windows Terminal, then set up g++:**

```
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-x86_64-gcc"
```

**4. Clone the repo:**

```
git clone https://github.com/VortexDQ/ClipboardFixer.git
cd ClipboardFixer
```

**5. Put the compiler on your PATH, then compile:**

MinGW's `g++` needs `C:\msys64\mingw64\bin` on your PATH   without it, the
compiler's own helper processes fail to load and `g++` exits **with no error
message at all**. Add it first, then compile:

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
g++ fix_clipboard.cpp -o fix_clipboard.exe -static -static-libgcc -static-libstdc++ -ladvapi32 -lshell32
```

> **Why `-static ...`?** These flags bundle the C++ runtime into the `.exe` so it
> has no external DLL dependencies. Without them the build succeeds but the
> program fails to start with `0xC0000135 (DLL not found)`   especially when run
> as administrator, because the elevated process can't see your PATH and can't
> find the MinGW runtime DLLs. The static build runs anywhere, elevated or not.

**6. Run it:**

**If you're a standard (non-admin) user**   just run it normally. Do **not** use
`Run as administrator`; the UAC prompt would ask for admin credentials you don't
have. The user-level fix (kill the frozen UI host, clear the cache, set the
history flag) runs fine without elevation:

```
./fix_clipboard.exe
```

**If you're an administrator** and want the full fix (also re-registers the
clipboard DLL and restarts the clipboard service):

```
Start-Process ./fix_clipboard.exe -Verb RunAs
```

That's it. Win+V should work after it finishes.

> **Non-admin note:** Steps 3 (re-register DLL) and 4 (restart service) require
> admin rights and will be skipped when you run without elevation   you'll see a
> warning for those two, which is normal. The freeze is almost always fixed by
> the steps that *do* run: killing the stuck `TextInputHost.exe`, clearing the
> corrupt cache, and confirming the history flag in `HKCU`. If Win+V is still
> frozen afterward and you can get an admin to run it, the elevated run adds the
> service restart.

> ⚠️ **Note:** Step 2 (clearing the cache) wipes your current clipboard history,
> including pinned items. This is expected   it's what unsticks the frozen panel.

> 🔒 **Locked-down machines:** If your organization enforces AppLocker/WDAC, running
> an unsigned `.exe` from `%TEMP%` or `%LOCALAPPDATA%` may be blocked
> (*"application control policy has blocked this file"*). Compile and run from the
> cloned repo folder (as above) rather than a temp directory.

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
