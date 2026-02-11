#include "TaskSchedulerFolderMonitor.h"

extern std::filesystem::path g_exeDirectory; // EXE folder for snapshots

TaskSchedulerFolderMonitor::TaskSchedulerFolderMonitor() : running(false) {
    folderPath = L"C:\\Windows\\System32\\Tasks";
    loadLastSnapshot();  
    StartMonitoring();
}

TaskSchedulerFolderMonitor::~TaskSchedulerFolderMonitor() {
    StopMonitoring();
}

void TaskSchedulerFolderMonitor::StartMonitoring() {
    if (!running) {
        running = true;
        monitorThread = std::thread(&TaskSchedulerFolderMonitor::monitorLoop, this);
    }
}

void TaskSchedulerFolderMonitor::StopMonitoring() {
    running = false;
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

void TaskSchedulerFolderMonitor::loadLastSnapshot() {
    std::string suffix = "_taskscheduler.txt";
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

    if (latestSnapshot) {
        previousFiles = FileUtils::readSnapshot(latestSnapshot.value());
        PLOG_INFO << "Loaded last Task Scheduler snapshot: " << previousFiles.size() << " tasks.";
    }
    else {
        previousFiles = FileUtils::getFilesInFolder(folderPath);
        PLOG_INFO << "Created initial Task Scheduler snapshot: " << previousFiles.size() << " tasks.";
    }
}

std::filesystem::path TaskSchedulerFolderMonitor::getSnapshotPath() const {
    time_t t;
    time(&t);
    return g_exeDirectory / (std::to_string(t) + "_taskscheduler.txt");
}

void TaskSchedulerFolderMonitor::monitorLoop() {
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
        PLOG_ERROR << "Failed to open Task Scheduler folder " << folderPath;
        return;
    }

    char buffer[8192];
    DWORD bytesReturned;

    while (running) {
        BOOL success = ReadDirectoryChangesW(
            hDir,
            buffer,
            sizeof(buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
            &bytesReturned,
            NULL,
            NULL
        );

        if (success && bytesReturned > 0) {
            DWORD offset = 0;
            std::set<std::string> currentFiles = previousFiles;
            std::string lastChangedFile;

            while (offset < bytesReturned) {
                FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer + offset);

                std::wstring fileNameW(info->FileName, info->FileName + info->FileNameLength / sizeof(WCHAR));

                int size_needed = WideCharToMultiByte(CP_UTF8, 0, fileNameW.c_str(), (int)fileNameW.size(), NULL, 0, NULL, NULL);
                std::string fileName(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, fileNameW.c_str(), (int)fileNameW.size(), &fileName[0], size_needed, NULL, NULL);

                lastChangedFile = fileName;  

                switch (info->Action) {
                case FILE_ACTION_ADDED:
                    currentFiles.insert(fileName);
                    PLOG_WARNING << "New Task Scheduler file: " << fileName;
                    break;
                case FILE_ACTION_REMOVED:
                    currentFiles.erase(fileName);
                    PLOG_WARNING << "Task Scheduler file removed: " << fileName;
                    break;
                case FILE_ACTION_MODIFIED:
                    PLOG_INFO << "Task Scheduler file modified: " << fileName;
                    break;
                case FILE_ACTION_RENAMED_OLD_NAME:
                    currentFiles.erase(fileName);
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    currentFiles.insert(fileName);
                    break;
                }

                if (info->NextEntryOffset == 0) break;
                offset += info->NextEntryOffset;
            }

            if (currentFiles != previousFiles) {
                FileUtils::saveSnapshot(getSnapshotPath(), currentFiles);

                std::wstring message = L"Task Scheduler folder changed!\nFile: " +
                    std::wstring(lastChangedFile.begin(), lastChangedFile.end());

                MessageBoxW(
                    NULL,
                    message.c_str(),
                    L"Mavir Antivirus - Task Scheduler Warning",
                    MB_OK | MB_ICONWARNING
                );

                previousFiles = currentFiles;
            }
        }
        else {
            PLOG_ERROR << "Error monitoring Task Scheduler folder! Error: " << GetLastError();
            Sleep(1000);
        }
    }

    CloseHandle(hDir);
}
