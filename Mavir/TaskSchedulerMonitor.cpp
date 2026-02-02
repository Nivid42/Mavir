#include "TaskSchedulerMonitor.h"

TaskSchedulerMonitor::TaskSchedulerMonitor() : running(false) {
    StartMonitoring();
}

TaskSchedulerMonitor::~TaskSchedulerMonitor() { StopMonitoring(); }

void TaskSchedulerMonitor::StartMonitoring() {
    running = true;
    monitorThread = std::thread(&TaskSchedulerMonitor::monitorLoop, this);
}

void TaskSchedulerMonitor::StopMonitoring() {
    running = false;
    if (monitorThread.joinable()) monitorThread.join();

    if (hKeyUser) { RegCloseKey(hKeyUser); hKeyUser = nullptr; }
    if (hKeySystem) { RegCloseKey(hKeySystem); hKeySystem = nullptr; }
}

void TaskSchedulerMonitor::initializeKeys() {
    const wchar_t* path = L"SOFTWARE\\\\Microsoft\\\\Windows NT\\\\CurrentVersion\\\\Schedule\\\\TaskCache\\\\Tree";

    LONG resUser = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        path,
        0,
        KEY_NOTIFY,
        &hKeyUser
    );
    if (resUser == ERROR_SUCCESS) {
        PLOG_INFO << "[TaskSchedulerMonitor] Monitoring HKCU TaskCache Tree";
    }
    else {
        hKeyUser = nullptr;
        PLOG_WARNING << "[TaskSchedulerMonitor] HKCU OpenKey failed: " << resUser;
    }

    LONG resSystem = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        path,
        0,
        KEY_NOTIFY,
        &hKeySystem
    );
    if (resSystem == ERROR_SUCCESS) {
        PLOG_INFO << "[TaskSchedulerMonitor] Monitoring HKLM TaskCache Tree";
    }
    else {
        hKeySystem = nullptr;
        PLOG_WARNING << "[TaskSchedulerMonitor] HKLM OpenKey failed: " << resSystem;
    }
}

void TaskSchedulerMonitor::monitorLoop() {
    initializeKeys();
    if (!hKeyUser && !hKeySystem) {
        PLOG_ERROR << "[TaskSchedulerMonitor] No keys opened — monitoring aborted.";
        return;
    }

    HANDLE hEventUser = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    HANDLE hEventSystem = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    HANDLE handles[2];
    DWORD handleCount = 0;

    if (hKeyUser) {
        RegNotifyChangeKeyValue(hKeyUser, FALSE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
            hEventUser, TRUE);
        handles[handleCount++] = hEventUser;
    }
    if (hKeySystem) {
        RegNotifyChangeKeyValue(hKeySystem, FALSE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
            hEventSystem, TRUE);
        handles[handleCount++] = hEventSystem;
    }

    while (running) {
        DWORD waitStatus = WaitForMultipleObjects(handleCount, handles, FALSE, 500);

        if (!running) break;

        if (waitStatus >= WAIT_OBJECT_0 && waitStatus < WAIT_OBJECT_0 + handleCount) {
            const wchar_t* source =
                (waitStatus == WAIT_OBJECT_0) ?
                L"User TaskCache (HKCU)" : L"System TaskCache (HKLM)";

            MessageBoxW(NULL,
                (std::wstring(L"A scheduled task was added or modified: ") + source).c_str(),
                L"Mavir Antivirus - Warning",
                MB_OK | MB_ICONWARNING);

            PLOG_WARNING << "[TaskSchedulerMonitor] TaskCache change detected at: " << source;

            // Re-arm notifications
            if (hKeyUser)
                RegNotifyChangeKeyValue(hKeyUser, FALSE,
                    REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                    hEventUser, TRUE);
            if (hKeySystem)
                RegNotifyChangeKeyValue(hKeySystem, FALSE,
                    REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                    hEventSystem, TRUE);
        }
    }

    if (hEventUser) CloseHandle(hEventUser);
    if (hEventSystem) CloseHandle(hEventSystem);
}
