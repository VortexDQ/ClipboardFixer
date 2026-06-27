#!/usr/bin/env python3
"""
fix_clipboard.py — Cross-platform clipboard repair tool.
Fixes Windows+V clipboard history crashes on Windows.
Provides clipboard diagnostics and reset on macOS and Linux.
Android is not supported (see README).
"""

import sys
import os
import platform
import subprocess
import shutil


def banner():
    print("=" * 52)
    print("   Clipboard Fix Tool")
    print("   github.com/YOUR_USERNAME/clipboard-fix")
    print("=" * 52)
    print()


# ── WINDOWS ──────────────────────────────────────────
def fix_windows():
    print("[Windows] Diagnosing Win+V clipboard history crash...\n")

    fixes_applied = []
    errors = []

    # 1. Re-register the clipboard DLL
    dll_path = r"C:\Windows\System32\cbdhsvc.dll"
    if os.path.exists(dll_path):
        try:
            result = subprocess.run(
                ["regsvr32", "/s", dll_path],
                capture_output=True, text=True
            )
            if result.returncode == 0:
                fixes_applied.append("Re-registered cbdhsvc.dll")
            else:
                errors.append(f"regsvr32 failed: {result.stderr.strip()}")
        except FileNotFoundError:
            errors.append("regsvr32 not found — skipping DLL re-registration.")
    else:
        errors.append(f"DLL not found at {dll_path} — skipping.")

    # 2. Restart the Clipboard User Data service
    try:
        subprocess.run(["net", "stop", "cbdhsvc"], capture_output=True)
        result = subprocess.run(["net", "start", "cbdhsvc"], capture_output=True, text=True)
        if result.returncode == 0:
            fixes_applied.append("Restarted cbdhsvc (Clipboard User Data Service)")
        else:
            errors.append(f"Could not restart cbdhsvc: {result.stderr.strip()}")
    except FileNotFoundError:
        errors.append("'net' command not found — skipping service restart.")

    # 3. Clear corrupt clipboard history DB
    appdata = os.environ.get("LOCALAPPDATA", "")
    cb_db = os.path.join(
        appdata,
        r"Microsoft\Windows\Clipboard\*"
    )
    cb_dir = os.path.join(appdata, "Microsoft", "Windows", "Clipboard")
    if os.path.isdir(cb_dir):
        try:
            shutil.rmtree(cb_dir)
            os.makedirs(cb_dir)
            fixes_applied.append("Cleared corrupt clipboard history cache")
        except PermissionError:
            errors.append(
                "Could not clear clipboard cache — close all apps and retry as Administrator."
            )
    else:
        fixes_applied.append("Clipboard cache directory not found — likely already clean")

    # 4. Re-enable clipboard history via registry
    try:
        import winreg
        key_path = r"Software\Microsoft\Clipboard"
        try:
            key = winreg.OpenKey(
                winreg.HKEY_CURRENT_USER, key_path,
                0, winreg.KEY_SET_VALUE
            )
        except FileNotFoundError:
            key = winreg.CreateKey(winreg.HKEY_CURRENT_USER, key_path)
        winreg.SetValueEx(key, "EnableClipboardHistory", 0, winreg.REG_DWORD, 1)
        winreg.CloseKey(key)
        fixes_applied.append("Verified/enabled clipboard history in registry")
    except ImportError:
        errors.append("winreg not available — skipping registry fix.")
    except Exception as e:
        errors.append(f"Registry fix failed: {e}")

    # ── Summary ──
    print("Results:")
    for f in fixes_applied:
        print(f"  ✓ {f}")
    for e in errors:
        print(f"  ✗ {e}")

    if not errors:
        print("\n✅ All fixes applied. Press Win+V to test.")
    else:
        print("\n⚠️  Some fixes failed. Try running as Administrator:")
        print("   Right-click fix_clipboard.py → Run as administrator")

    print("\nIf the issue persists:")
    print("  1. Settings → System → Clipboard → toggle 'Clipboard history' off/on")
    print("  2. sfc /scannow  (run in admin CMD to repair system files)")


# ── macOS ─────────────────────────────────────────────
def fix_macos():
    print("[macOS] Resetting clipboard...\n")
    fixes = []
    errors = []

    # Kill and restart pboard (pasteboard daemon)
    try:
        subprocess.run(["killall", "pboard"], capture_output=True)
        fixes.append("Restarted pboard (pasteboard daemon)")
    except Exception as e:
        errors.append(f"Could not restart pboard: {e}")

    # Clear clipboard content
    try:
        subprocess.run(["pbcopy"], input=b"", check=True)
        fixes.append("Cleared clipboard contents")
    except Exception as e:
        errors.append(f"pbcopy failed: {e}")

    for f in fixes:
        print(f"  ✓ {f}")
    for e in errors:
        print(f"  ✗ {e}")

    print("\nmacOS clipboard shortcuts:")
    print("  Cmd+C / Cmd+V — standard copy/paste")
    print("  No built-in clipboard history — use Raycast or Maccy (free)")
    print("  https://github.com/p0deje/Maccy")


# ── LINUX ─────────────────────────────────────────────
def fix_linux():
    print("[Linux] Diagnosing clipboard...\n")
    fixes = []
    errors = []

    desktop = os.environ.get("XDG_SESSION_TYPE", "unknown").lower()
    print(f"  Session type: {desktop}")

    # Try restarting xclip/xsel daemon or relevant service
    for tool in ["xclip", "xsel", "wl-clipboard"]:
        if shutil.which(tool):
            fixes.append(f"Found clipboard tool: {tool}")
            break
    else:
        errors.append(
            "No clipboard tool found. Install one:\n"
            "    sudo apt install xclip   # X11\n"
            "    sudo apt install wl-clipboard  # Wayland"
        )

    # KDE Plasma — restart klipper
    if shutil.which("klipper"):
        try:
            subprocess.run(["pkill", "klipper"], capture_output=True)
            subprocess.Popen(["klipper"])
            fixes.append("Restarted Klipper (KDE clipboard manager)")
        except Exception as e:
            errors.append(f"Klipper restart failed: {e}")

    # GNOME — restart gnome-shell isn't safe, just advise
    if shutil.which("gnome-shell"):
        fixes.append(
            "GNOME detected — if clipboard is broken, log out and back in,\n"
            "    or install Parcellite: sudo apt install parcellite"
        )

    for f in fixes:
        print(f"  ✓ {f}")
    for e in errors:
        print(f"  ✗ {e}")


# ── ANDROID ───────────────────────────────────────────
def fix_android():
    print("[Android] This tool cannot modify Android's clipboard manager.")
    print()
    print("To fix clipboard issues on Android:")
    print("  1. Clear the cache of your keyboard app")
    print("     Settings → Apps → [Your Keyboard] → Storage → Clear Cache")
    print("  2. Try Gboard (reliable clipboard built in):")
    print("     https://play.google.com/store/apps/details?id=com.google.android.inputmethod.latin")
    print("  3. Force-stop and restart your launcher if clipboard bar is missing")


# ── MAIN ──────────────────────────────────────────────
def main():
    banner()

    system = platform.system().lower()

    # Android detection (Termux sets this)
    if "android" in os.environ.get("PREFIX", "").lower() or \
       "com.termux" in os.environ.get("HOME", "").lower():
        fix_android()
        return

    if system == "windows":
        # Check for admin
        try:
            is_admin = os.getuid() == 0
        except AttributeError:
            import ctypes
            is_admin = ctypes.windll.shell32.IsUserAnAdmin()

        if not is_admin:
            print("⚠️  Not running as Administrator. Some fixes may be skipped.")
            print("   Re-run: right-click → 'Run as administrator'\n")

        fix_windows()

    elif system == "darwin":
        fix_macos()

    elif system == "linux":
        fix_linux()

    else:
        print(f"Unsupported platform: {platform.system()}")
        print("Supported: Windows, macOS, Linux")
        print("Android: see README for manual steps")
        sys.exit(1)


if __name__ == "__main__":
    main()
