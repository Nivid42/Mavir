#pragma once
#include <filesystem>
#include <set>
#include <atomic>
#include <thread>
#include <windows.h>
#include "FileUtils.h"
#include "Logger.h"

extern std::filesystem::path g_exeDirectory;

class TaskSchedulerFolderMonitor {
public:
    /* @brief Constructor which calls after creating Handles load lastsnapshot which creates a snapshot or reads the last one.
    */
    TaskSchedulerFolderMonitor();

    /* @brief Destructor of the TaskSchedulerFolderMonitor Object, also triggers the StopMonitoring Function
    */
    ~TaskSchedulerFolderMonitor();

    /* @brief Activates the Monitoring at the Start of the PGM Functionality which keeps Track of the Scheduled Tasks Folder for Persistence
    */
    void StartMonitoring();

    /* @brief Gets called when Program exits and stops the Monitoring Threads by setting the running var to false and joins the Thread
    */
    void StopMonitoring();

private:
    std::filesystem::path folderPath{ L"C:\\Windows\\System32\\Tasks" }; 
    std::set<std::string> previousFiles;
    std::atomic<bool> running {false};
    std::thread monitorThread; 


    /* @brief Monitor Loop which looks for changes event based
    */
    void monitorLoop();

    /* @brief Used to get a new Snapshot Path when a new File is detected in the Task Scheduler Path
    *  @return Returns a Path concatenated with a new Timestamp and the needed suffix
    */
    std::filesystem::path getSnapshotPath() const;

    /* @brief Gets called in the Constructor and loads & reads the last Snapshot. If its not existing he creates one
    */
    void loadLastSnapshot();  
};
