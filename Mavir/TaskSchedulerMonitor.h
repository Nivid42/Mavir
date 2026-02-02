#pragma once
#include <atomic>
#include <thread>
#include <windows.h>
#include <chrono>
#include "Logger.h"

class TaskSchedulerMonitor {
public:

    /* @brief Constructor which calls StartMonitoring() to actively monitor the Taskscheduler Tree.
    */
    TaskSchedulerMonitor();

    /* @brief Destructor of the TaskSchedulerMonitor Object, also triggers the StopMonitoring
    */
    ~TaskSchedulerMonitor();

    /* @brief Activates the Monitoring at the Start of the PGM  which keeps Track of the Admin & User Taskscheduler Registry Key for Persistence
    */
    void StartMonitoring();

    /* @brief Stops the Monitoring at the End of the PGM, releases all threads / handles
    */
    void StopMonitoring();

private:

    /* Opens handles to HKCU & HKLM for MonitorLoop()
    */
    void initializeKeys();

    /* Monitors the Registry Keys of the Taskscheduler Tree 
    */
    void monitorLoop();

    HKEY hKeyUser = nullptr;
    HKEY hKeySystem = nullptr;

    std::atomic<bool> running{ false };
    std::thread monitorThread;
};
