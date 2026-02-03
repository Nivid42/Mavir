#include "StartupFolderMonitor.h"


extern std::filesystem::path g_exeDirectory; // EXE folder for snapshots

StartupFolderMonitor::StartupFolderMonitor(FolderType type)
    : folderType(type), running(false)
{
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(
        (folderType == FolderType::User) ? FOLDERID_Startup : FOLDERID_CommonStartup,
        0, nullptr, &path
    );

    if (FAILED(hr) || !path) {
        PLOG_ERROR << "Failed to retrieve Startup folder!";
        return;
    }

    folderPath = path;
    CoTaskMemFree(path);

    loadLastSnapshot();
}

StartupFolderMonitor::~StartupFolderMonitor() {
    StopMonitoring();
}

void StartupFolderMonitor::StartMonitoring() {
    running = true;
    monitorThread = std::thread(&StartupFolderMonitor::monitorLoop, this);
}

void StartupFolderMonitor::StopMonitoring() {
    running = false;
    if (monitorThread.joinable())
        monitorThread.join();
}

void StartupFolderMonitor::loadLastSnapshot() {
    std::string suffix = (folderType == FolderType::User) ? "_startup_user.txt" : "_startup_admin.txt";
    std::optional<std::filesystem::path> latestSnapshot;
    std::filesystem::file_time_type latestTime;

    for (const auto& entry : std::filesystem::directory_iterator(g_exeDirectory)) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();
        if (filename.find(suffix) == std::string::npos) continue;

        auto t = std::filesystem::last_write_time(entry);
        if (!latestSnapshot || t > latestTime) {
            latestSnapshot = entry.path();
            latestTime = t;
        }
    }

    if (latestSnapshot)
        previousFiles = FileUtils::readSnapshot(latestSnapshot.value());
    else
        previousFiles = FileUtils::getFilesInFolder(folderPath);
}

std::filesystem::path StartupFolderMonitor::getSnapshotPath() const {
    time_t t; time(&t);
    std::string suffix = (folderType == FolderType::User) ? "_startup_user.txt" : "_startup_admin.txt";
    return g_exeDirectory / (std::to_string(t) + suffix);
}

void StartupFolderMonitor::monitorLoop() {
    HANDLE hDir = CreateFileW(
        folderPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        PLOG_ERROR << "Failed to open Startup folder!";
        return;
    }

    char buffer[1024];
    DWORD bytesReturned;

    while (running) {
        if (ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            FALSE, // monitor only this folder
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            NULL,
            NULL
        )) {
            DWORD offset = 0;
            std::set<std::string> currentFiles = previousFiles;

            while (offset < bytesReturned) {
                FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer + offset);

                std::wstring fileNameW(info->FileName, info->FileName + info->FileNameLength / sizeof(WCHAR));

                // UTF-8 safe conversion
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, fileNameW.c_str(), (int)fileNameW.size(), NULL, 0, NULL, NULL);
                std::string fileName(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, fileNameW.c_str(), (int)fileNameW.size(), fileName.data(), size_needed, NULL, NULL);

                switch (info->Action) {
                case FILE_ACTION_ADDED:
                    currentFiles.insert(fileName);
                    PLOG_WARNING << "New file: " << fileName;
                    break;
                case FILE_ACTION_REMOVED:
                    currentFiles.erase(fileName);
                    PLOG_WARNING << "File removed: " << fileName;
                    break;
                case FILE_ACTION_MODIFIED:
                    PLOG_INFO << "File modified: " << fileName;
                    break;
                }

                if (info->NextEntryOffset == 0) break;
                offset += info->NextEntryOffset;
            }

            // Save snapshot and notify if changed
            if (currentFiles != previousFiles) {
                FileUtils::saveSnapshot(getSnapshotPath(), currentFiles);

                MessageBox(
                    NULL,
                    (folderType == FolderType::User) ?
                    L"The USER Startup folder has changed!" :
                    L"The ADMIN Startup folder has changed!",
                    L"Mavir Antivirus - Warning",
                    MB_OK | MB_ICONWARNING
                );

                previousFiles = currentFiles;
            }
        }
        else {
            PLOG_ERROR << "Error monitoring the Startup folder!";
            Sleep(1000); // Windows sleep 1 sec
        }
    }

    CloseHandle(hDir);
}
