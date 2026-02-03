#pragma once
#include <string>
#include <set>
#include <filesystem>
#include <atomic>
#include <thread>
#include <chrono>
#include <ShlObj_core.h>
#include <windows.h>
#include "FileUtils.h"
#include "Logger.h"

class StartupFolderMonitor {
public:
    enum class FolderType { User, Admin };

    /* @brief Constructor which calls after creating Handles load lastsnapshot which creates a snapshot or reads the last one.
    */
    StartupFolderMonitor(FolderType type);

    /* @brief Destructor of the StartupFolderMonitor Object, also triggers the StopMonitoring
    */
    ~StartupFolderMonitor();
    
    /* @brief Activates the Monitoring at the Start of the PGM Functionality which keeps Track of the Admin & User Startup Folder for Persistence
    */
    void StartMonitoring();

    /* @brief Gets called when Program exits and stops the Monitoring Threads by setting the running var to false and joins the Thread
    */
    void StopMonitoring();

private:
    FolderType folderType;
    std::filesystem::path folderPath;
    std::set<std::string> previousFiles;
    std::atomic<bool> running;
    std::thread monitorThread;

    /* @brief Monitor Loop which looks for changes event based
    */
    void monitorLoop();

    /* @brief Used to generate a new Snapshot Path when a new File is detected in the Startup Path  
    *  @return Returns a Path concatenated with a new Timestamp and the needed suffix
    */
    std::filesystem::path getSnapshotPath() const;

    /* @brief Gets called in the Constructor and loads & reads the last Snapshot. If its not existing he creates one
    */
    void loadLastSnapshot();
};
