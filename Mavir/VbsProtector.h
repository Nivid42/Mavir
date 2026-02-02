#pragma once
#include <atomic>
#include <thread>
#include <windows.h>
#include <chrono>
#include "FileUtils.h"
#include "Logger.h"

class VbsProtector {
public:
    enum class KeyType { User, System };

    /* @brief Initializes Class and Sets Value of Registry Key of Subsystem to 0. The Class also checks if the Value is changed and if so, announces the User
    */
    VbsProtector(KeyType type);

    /* @brief Destructor which handles closing of the Thread + releases used handles
    */
    ~VbsProtector();
    
    /* @brief Activates the Monitoring at the Start of the PGM Functionality which keeps Track of the Admin & User Registry key for the vbs Subsystem
    */
    void StartMonitoring();

    /* @brief Gets called when Program exits and stops the Monitoring Threads by setting the running var to false and joins the Thread
   */
    void StopMonitoring();

private:
    KeyType keyType;
    HKEY hKey;
    std::atomic<bool> running;
    std::thread monitorThread;

    /* @brief Deactivates .vbs Scripts by Setting both Registry Key to 0.
    */
    void initializeKey();

    /* @brief Loop to Check if the Value of the vbs Subsystem RegistryKey got changed
    */
    void monitorLoop();
};
