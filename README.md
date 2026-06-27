# 📋 Clipboard Fix Tool

A cross-platform Python script that repairs broken clipboard managers - with a focus on fixing the **Windows+V crash** on Windows 10/11.

---

## ⚡ Quick Start

```bash
# Clone
git clone https://github.com/VortexDQ/ClipboardFixer.git
cd ClipboardFixer

# Run
python fix_clipboard.py
```

> **Windows users:** Right-click → *Run as administrator* for full repairs.

---

## 🖥️ Platform Support

| Platform       | Supported | What it does |
|----------------|-----------|--------------|
| Windows 10/11  | ✅ Full    | Re-registers DLL, restarts service, clears corrupt cache, fixes registry |
| macOS          | ✅ Full    | Restarts `pboard` daemon, clears clipboard |
| Linux (X11)    | ✅ Full    | Detects clipboard tools, restarts Klipper (KDE) |
| Linux (Wayland)| ✅ Partial | Detects `wl-clipboard`, advises on fixes |
| Android        | ⚠️ Manual  | Script runs but can't modify system clipboard — see [Android section](#android) |
| iOS            | ❌ N/A     | iOS does not allow clipboard system access from scripts |

> **Android & iOS note:** Mobile operating systems sandbox all apps, including terminals. No script can reach the system clipboard manager. Manual steps are provided instead.

---

## 🪟 Windows — What Gets Fixed

The `Win+V` clipboard history menu commonly crashes due to:

| Root Cause | Fix Applied |
|---|---|
| Corrupt `cbdhsvc.dll` registration | Re-registered via `regsvr32` |
| Stalled clipboard service | Service stopped + restarted |
| Corrupt clipboard history database | Cache directory cleared |
| Registry flag unset | `EnableClipboardHistory` set to `1` |

### If it still crashes after running:

1. Open **Settings → System → Clipboard**
2. Toggle *Clipboard history* **off**, wait 5 seconds, toggle **on**
3. Run `sfc /scannow` in an admin Command Prompt to repair system files
4. As a last resort: **Settings → Apps → Optional Features → clipboard** — uninstall and reinstall

---

## 🍎 macOS

The script restarts the `pboard` (pasteboard) daemon and clears clipboard contents.

macOS has **no built-in clipboard history** like Windows. If you want one:

- **[Maccy](https://github.com/p0deje/Maccy)** — free, open source, lightweight
- **[Raycast](https://www.raycast.com/)** — free, full launcher with clipboard history

---

## 🐧 Linux

Clipboard behavior on Linux depends heavily on your desktop environment.

**What the script does:**
- Detects X11 vs Wayland session
- Confirms a clipboard tool (`xclip`, `xsel`, `wl-clipboard`) is installed
- Restarts **Klipper** on KDE if found
- Advises GNOME users on next steps

**Install a clipboard tool if missing:**

```bash
# X11
sudo apt install xclip

# Wayland
sudo apt install wl-clipboard

# KDE clipboard manager
sudo apt install klipper

# GNOME lightweight alternative
sudo apt install parcellite
```

---

## 🤖 Android <a name="android"></a>

The script detects Android (via Termux environment variables) and prints manual instructions. It **cannot** modify the system clipboard manager — Android's sandbox prevents this.

**Manual fix:**

1. Go to **Settings → Apps → [Your Keyboard App] → Storage → Clear Cache**
2. Force-stop the keyboard app, then reopen it
3. If using Samsung keyboard, check *Samsung Keyboard Settings → Clipboard* and clear history there
4. Consider switching to **[Gboard](https://play.google.com/store/apps/details?id=com.google.android.inputmethod.latin)** which has a reliable built-in clipboard panel

---

## 📱 iOS

iOS does not expose system clipboard management to third-party apps or scripts. There is no fix script possible. If your clipboard isn't working:

- Restart the app you're pasting into
- Restart your iPhone/iPad
- Check **Settings → Privacy & Security → Paste from Other Apps** for permission issues

---

## 🛠️ Requirements

- **Python 3.7+** (no third-party packages required — stdlib only)
- Windows: Run as Administrator for full repairs
- Linux: `xclip` / `wl-clipboard` recommended

---

## 🤝 Contributing

Pull requests welcome. If you hit a clipboard issue not covered here, open an issue with:

- Your OS + version
- What Win+V / clipboard does (or doesn't do)
- Output of `python fix_clipboard.py`

---

## 📄 License

MIT — do whatever you want with it.
