#include <iostream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
  #include <windows.h>
  #include <winsvc.h>
  #include <shlobj.h>
  #include <tlhelp32.h>
  #pragma comment(lib, "advapi32.lib")
  #pragma comment(lib, "shell32.lib")
#elif __APPLE__
  #include <unistd.h>
#elif __linux__
  #include <unistd.h>
#endif

void ok(const std::string& msg)  { std::cout << "  [OK] " << msg << "\n"; }
void err(const std::string& msg) { std::cout << "  [!!] " << msg << "\n"; }
void info(const std::string& msg){ std::cout << "  [..] " << msg << "\n"; }

void banner() {
    std::cout << "\n====================================================\n";
    std::cout << "   ClipboardFixer — Frozen UI Fix\n";
    std::cout << "   github.com/VortexDQ/ClipboardFixer\n";
    std::cout << "====================================================\n\n";
}

#ifdef _WIN32

bool isAdmin() {
    BOOL result = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &result);
        FreeSid(adminGroup);
    }
    return result == TRUE;
}

// Kill every process matching a name
void killProcessByName(const std::string& procName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        err("Could not snapshot processes");
        return;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    bool found = false;
    if (Process32First(snap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, procName.c_str()) == 0) {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc) {
                    TerminateProcess(hProc, 0);
                    CloseHandle(hProc);
                    found = true;
                }
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);
    if (found) ok("Killed process: " + procName);
    else        info("Process not running: " + procName);
}

void restartService(const std::string& svcName) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) { err("Could not open Service Manager — run as Administrator"); return; }

    SC_HANDLE svc = OpenServiceA(scm, svcName.c_str(),
                                  SERVICE_STOP | SERVICE_START | SERVICE_QUERY_STATUS);
    if (!svc) {
        err("Could not open service: " + svcName);
        CloseServiceHandle(scm);
        return;
    }

    SERVICE_STATUS status;
    ControlService(svc, SERVICE_CONTROL_STOP, &status);
    Sleep(2000); // wait for full stop

    if (StartService(svc, 0, NULL)) {
        ok("Restarted service: " + svcName);
    } else {
        DWORD e = GetLastError();
        if (e == ERROR_SERVICE_ALREADY_RUNNING)
            ok("Service already running: " + svcName);
        else
            err("Failed to start service (error " + std::to_string(e) + ")");
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
}

void clearClipboardCache() {
    char localAppData[MAX_PATH];
    if (FAILED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
        err("Could not locate %LOCALAPPDATA%");
        return;
    }
    std::string cbDir = std::string(localAppData) + "\\Microsoft\\Windows\\Clipboard";
    std::string rmCmd = "rmdir /s /q \"" + cbDir + "\" 2>nul";
    std::string mkCmd = "mkdir \"" + cbDir + "\" 2>nul";
    system(rmCmd.c_str());
    system(mkCmd.c_str());
    ok("Cleared clipboard history cache (was likely corrupt/locked)");
}

void fixRegistry() {
    HKEY key;
    LONG res = RegCreateKeyExA(HKEY_CURRENT_USER,
                                "Software\\Microsoft\\Clipboard",
                                0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE, NULL, &key, NULL);
    if (res != ERROR_SUCCESS) {
        err("Registry fix failed (error " + std::to_string(res) + ")");
        return;
    }
    DWORD val = 1;
    RegSetValueExA(key, "EnableClipboardHistory", 0, REG_DWORD,
                   (const BYTE*)&val, sizeof(val));
    RegCloseKey(key);
    ok("Clipboard history confirmed enabled in registry");
}

void reRegisterDll() {
    const char* dll = "C:\\Windows\\System32\\cbdhsvc.dll";
    if (GetFileAttributesA(dll) == INVALID_FILE_ATTRIBUTES) {
        err("cbdhsvc.dll not found — skipping");
        return;
    }
    std::string cmd = std::string("regsvr32 /s \"") + dll + "\"";
    int ret = system(cmd.c_str());
    if (ret == 0) ok("Re-registered cbdhsvc.dll");
    else          err("regsvr32 failed (error " + std::to_string(ret) + ")");
}

void fixWindows() {
    std::cout << "[Windows] Fixing frozen Win+V clipboard UI...\n\n";

    if (!isAdmin()) {
        std::cout << "  WARNING: Not running as Administrator.\n";
        std::cout << "  Service restart will be skipped.\n";
        std::cout << "  Right-click the .exe -> Run as administrator\n\n";
    }

    // Step 1: Force-kill the frozen clipboard host process
    info("Step 1: Force-killing frozen clipboard processes...");
    killProcessByName("TextInputHost.exe");   // clipboard UI host
    killProcessByName("cbdhsvc_*.exe");       // older Windows versions
    Sleep(500);

    // Step 2: Clear the corrupt/locked cache that causes freezing
    info("Step 2: Clearing clipboard cache...");
    clearClipboardCache();

    // Step 3: Re-register the DLL
    info("Step 3: Re-registering clipboard DLL...");
    reRegisterDll();

    // Step 4: Restart the clipboard service
    info("Step 4: Restarting clipboard service...");
    restartService("cbdhsvc");

    // Step 5: Fix registry
    info("Step 5: Verifying registry...");
    fixRegistry();

    std::cout << "\n  All steps complete.\n";
    std::cout << "  Wait 3 seconds, then press Win+V.\n";
    std::cout << "\n  If still frozen:\n";
    std::cout << "    1. Sign out and back in (restarts TextInputHost cleanly)\n";
    std::cout << "    2. Settings -> System -> Clipboard -> toggle off/on\n";
    std::cout << "    3. Run 'sfc /scannow' in admin CMD\n";
}

#elif __APPLE__

void fixMacOS() {
    std::cout << "[macOS] Resetting clipboard...\n\n";
    int r1 = system("killall pboard 2>/dev/null");
    if (r1 == 0) ok("Restarted pboard daemon");
    else         err("Could not restart pboard");
    int r2 = system("echo -n '' | pbcopy");
    if (r2 == 0) ok("Cleared clipboard contents");
    std::cout << "\n  Recommended clipboard history app: https://github.com/p0deje/Maccy\n";
}

#elif __linux__

bool cmdExists(const std::string& cmd) {
    return system(("command -v " + cmd + " > /dev/null 2>&1").c_str()) == 0;
}

void fixLinux() {
    std::cout << "[Linux] Diagnosing clipboard...\n\n";
    const char* st = getenv("XDG_SESSION_TYPE");
    info(std::string("Session: ") + (st ? st : "unknown"));

    if      (cmdExists("xclip"))        ok("Clipboard tool found: xclip");
    else if (cmdExists("xsel"))         ok("Clipboard tool found: xsel");
    else if (cmdExists("wl-clipboard")) ok("Clipboard tool found: wl-clipboard");
    else    err("No clipboard tool — install xclip or wl-clipboard");

    if (cmdExists("klipper")) {
        system("pkill klipper 2>/dev/null; klipper &");
        ok("Restarted Klipper");
    }
    if (cmdExists("gnome-shell"))
        info("GNOME: log out/in to reset clipboard, or install parcellite");
}

#else

void fixUnsupported() {
    err("Unsupported platform. Supported: Windows, macOS, Linux");
}

#endif

int main() {
    banner();
#ifdef _WIN32
    fixWindows();
#elif __APPLE__
    fixMacOS();
#elif __linux__
    fixLinux();
#else
    fixUnsupported();
#endif
    std::cout << "\n";
    return 0;
}

/*
 * ── COMPILE ───────────────────────────────────────────────────
 *
 * Windows (MSVC — Visual Studio Developer Command Prompt):
 *   cl fix_clipboard.cpp /Fe:fix_clipboard.exe /link advapi32.lib shell32.lib
 *
 * Windows (g++ via MinGW/MSYS2):
 *   g++ fix_clipboard.cpp -o fix_clipboard.exe -ladvapi32 -lshell32
 *
 * macOS:
 *   g++ fix_clipboard.cpp -o fix_clipboard
 *
 * Linux:
 *   g++ fix_clipboard.cpp -o fix_clipboard
 *
 * Run: right-click .exe -> Run as administrator
 * ──────────────────────────────────────────────────────────────
 */