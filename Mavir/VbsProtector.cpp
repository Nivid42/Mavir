#include "VbsProtector.h"

VbsProtector::VbsProtector(KeyType type) : keyType(type), running(false) {
    initializeKey();
}

VbsProtector::~VbsProtector() { StopMonitoring(); }

void VbsProtector::StartMonitoring() {
    running = true;
    monitorThread = std::thread(&VbsProtector::monitorLoop, this);
}

void VbsProtector::StopMonitoring() {
    running = false;
    if (monitorThread.joinable()) monitorThread.join();
    if (hKey) RegCloseKey(hKey);
}

void VbsProtector::initializeKey() {
    LONG result = RegOpenKeyEx(
        (keyType == KeyType::User) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE,
        L"Software\\\\Microsoft\\\\Windows Script Host\\\\Settings",
        0, KEY_READ | KEY_WRITE | KEY_NOTIFY, &hKey
    );

    if (result != ERROR_SUCCESS) { PLOG_ERROR << "Registry access failed: " << result; return; }

    DWORD value = 0, type = REG_DWORD, size = sizeof(value);
    if (RegQueryValueEx(hKey, L"Enabled", nullptr, &type, reinterpret_cast<LPBYTE>(&value), &size) != ERROR_SUCCESS || value != 0) {
        DWORD newValue = 0;
        RegSetValueEx(hKey, L"Enabled", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&newValue), sizeof(newValue));
    }
}

void VbsProtector::monitorLoop() {
    DWORD type = REG_DWORD, size = sizeof(DWORD);
    DWORD currValue = 0;

    while (running) {
        RegNotifyChangeKeyValue(hKey, FALSE, REG_NOTIFY_CHANGE_LAST_SET, NULL, FALSE);
        // OnTaskChange();  // Will be basically the handling of the suspicious Change
        if (RegQueryValueEx(hKey, L"Enabled", nullptr, &type, reinterpret_cast<LPBYTE>(&currValue), &size) == ERROR_SUCCESS) {
            if (currValue != 0) {
                DWORD newValue = 0;
                RegSetValueEx(hKey, L"Enabled", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&newValue), sizeof(newValue));
                MessageBoxW(NULL,
                    (keyType == KeyType::User) ?
                    L"USER VBS Registry Key was modified!" :
                    L"SYSTEM VBS Registry Key was modified!",
                    L"Mavir Antivirus - Warning",
                    MB_OK | MB_ICONWARNING
                );
                PLOG_WARNING << ".vbs Files were disabled";

                std::this_thread::sleep_for(std::chrono::milliseconds(1)); // was earlier 50 ms but this can be  a condition where malicious code executes exactly then
            }
        }
    }
}
