#include "StartupFolderMonitor.h"
#include "FileUtils.h"
#include "Logger.h"
#include <ShlObj_core.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <optional>

extern std::filesystem::path g_exeDirectory; // EXE-Ordner, hier speichern wir Snapshots#

StartupFolderMonitor::StartupFolderMonitor(FolderType type, int interval)
    : folderType(type), intervalSeconds(interval), running(false)
{
    PWSTR path = nullptr;
    HRESULT hr = SHGetKnownFolderPath(
        (folderType == FolderType::User) ? FOLDERID_Startup : FOLDERID_CommonStartup,
        0, nullptr, &path
    );
    if (FAILED(hr) || !path) {
        PLOG_ERROR << "Konnte Startup-Ordner nicht ermitteln!";
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
    if (monitorThread.joinable()) monitorThread.join();
}

void StartupFolderMonitor::loadLastSnapshot() {
    std::string suffix = (folderType == FolderType::User) ? "_startup_user.txt" : "_startup_admin.txt";
    std::optional<std::filesystem::path> latestSnapshot;
    std::filesystem::file_time_type latestTime;

    // Iterates through all TXT Files
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

void StartupFolderMonitor::detectChanges(const std::set<std::string>& currentFiles) {
    bool changed = false;

    for (const auto& file : currentFiles)
        if (previousFiles.find(file) == previousFiles.end()) {
            changed = true;
            PLOG_WARNING << "Neue Datei: " << file;
        }

    for (const auto& file : previousFiles)
        if (currentFiles.find(file) == currentFiles.end()) {
            changed = true;
            PLOG_WARNING << "Datei entfernt: " << file;
        }

    if (changed) {
        FileUtils::saveSnapshot(getSnapshotPath(), currentFiles);

        MessageBox(NULL,
            (folderType == FolderType::User) ?
            L"Der USER Startup Ordner wurde verändert!" :
            L"Der ADMIN Startup Ordner wurde verändert!",
            L"Mavir Antivirus - Warnung",
            MB_OK | MB_ICONWARNING
        );

        previousFiles = currentFiles;
    }
}

void StartupFolderMonitor::monitorLoop() {
    while (running) {
        auto currentFiles = FileUtils::getFilesInFolder(folderPath);
        detectChanges(currentFiles);
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
    }
}
