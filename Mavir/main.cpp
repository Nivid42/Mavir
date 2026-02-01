#include "StartupFolderMonitor.h"
#include "VbsProtector.h"
#include "Logger.h"
#include "Menu.h"
#include <filesystem>
#include <windows.h>
#include <iostream>
#include <atomic>

std::filesystem::path g_exeDirectory; // Holds the current working directory of the Exe, used for creating Logfiles 
std::atomic<bool> g_running(true); // Atomic because we have multiple Threads which accesses this Thread Loop Variable

/* @brief  Makes sure that unexpected termination of the Program exits cleanly by closing the Threads
   @param  Signal which references to closing the Program by X, ALT+F4 or Taskmanager
   @return TRUE if the signal was handled, FALSE otherwise
*/
BOOL WINAPI ConsoleHandler(DWORD signal) {
    switch (signal) {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        g_running = false; // Managed the Event and exits cleanly
        return TRUE;
    default:
        return FALSE;
    }
}

int main() {
    wchar_t exePath[MAX_PATH]; // Buffer for our EXE path we get from GetModuleFilename in Unicode because it returns paths as UTF-16
    GetModuleFileNameW(NULL, exePath, MAX_PATH); // Retrieves current EXE path as Unicode so special characters work and stores it 
    g_exeDirectory = std::filesystem::path(exePath).parent_path(); //

    Logger::InitializeLogger(g_exeDirectory);

    SetConsoleCtrlHandler(ConsoleHandler, TRUE); // Needed to handle inproper closing

    std::cout << "[+] Starting Background-Scans...\n";

    
    StartupFolderMonitor userStartupFolderMonitor(StartupFolderMonitor::FolderType::User, 10);
    StartupFolderMonitor adminStartupFolderMonitor(StartupFolderMonitor::FolderType::Admin, 10);
    VbsProtector vbsUser(VbsProtector::KeyType::User);
    VbsProtector vbsSystem(VbsProtector::KeyType::System);

    userStartupFolderMonitor.StartMonitoring();
    adminStartupFolderMonitor.StartMonitoring();
    vbsUser.StartMonitoring();
    vbsSystem.StartMonitoring();

    // Menü blockiert, bis User Exit wählt
    Menu::ShowMenu();

    // Threads stoppen (falls Menu endet)
    std::cout << "[+] Stoppe Hintergrund-Scans...\n";
    userStartupFolderMonitor.StopMonitoring();
    adminStartupFolderMonitor.StopMonitoring();
    vbsUser.StopMonitoring();
    vbsSystem.StopMonitoring();

    PLOG_INFO << "[+] Mavir Antivirus beendet";
    std::cout << "[+] Mavir Antivirus beendet. Auf Wiedersehen!\n";

    return 0;
}
