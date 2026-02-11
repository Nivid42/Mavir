#include "StartupFolderMonitor.h"
#include "VbsProtector.h"
#include "TaskSchedulerMonitor.h"
#include "TaskSchedulerFolderMonitor.h"
#include "Logger.h"
#include "Menu.h"

std::filesystem::path g_exeDirectory; // Holds the current working directory of the Exe, used for creating Logfiles 
std::atomic<bool> g_running(true);  // Atomic because we have multiple Threads which accesses this Thread Loop Variable

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
        g_running = false;
        return TRUE;
    default:
        return FALSE;
    }
}

int main() {
    wchar_t exePath[MAX_PATH]; // Buffer for our EXE path we get from GetModuleFilename in Unicode because it returns paths as UTF-16
    GetModuleFileNameW(NULL, exePath, MAX_PATH);// Retrieves current EXE path as Unicode so special characters work and stores it 
    g_exeDirectory = std::filesystem::path(exePath).parent_path();

    Logger::InitializeLogger(g_exeDirectory);
    SetConsoleCtrlHandler(ConsoleHandler, TRUE); // Needed to handle inproper closing

    std::cout << "[+] Starting Mavir Antivirus Background-Scans...\n";

    {
        TaskSchedulerMonitor taskSchedulerMonitor;
        TaskSchedulerFolderMonitor taskSchedulerFolderMonitor;
        StartupFolderMonitor userStartupFolderMonitor(StartupFolderMonitor::FolderType::User);
        StartupFolderMonitor adminStartupFolderMonitor(StartupFolderMonitor::FolderType::Admin);
        VbsProtector vbsUser(VbsProtector::KeyType::User);
        VbsProtector vbsSystem(VbsProtector::KeyType::System);


        Menu::ShowMenu();
    }  

    std::cout << "[+] Mavir Antivirus terminated.\n";
    PLOG_INFO << "[+] Mavir Antivirus terminated cleanly.";

    return 0;
}
