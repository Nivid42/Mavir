#pragma once
#include <set>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Logger.h"

class FileUtils {
public:
    /* @brief Scans Files in Startup Folder and is only used if no existing Snapshot is available
    *  @param Takes Path to the Folder which is adressed to be scanned
    *  @return returns Set of scanned files std::set<std::string>
    */
    static std::set<std::string> getFilesInFolder(const std::filesystem::path& path);

    /* @brief Reads the existing Snapshot and is only executed if a Snapshot is available (pendant to getFilesinFolder)
    *  @param Takes Path to Snapshot file which gets read
    *  @return returns Set of scanned files std::set<std::string>
    */
    static std::set<std::string> readSnapshot(const std::filesystem::path& path);

    /* @brief Gets called when there are Changes to the Startup Folder and saves the new Snapshot with the changes.
    *  @param Path: Path to Folder where the Snapshots are stored, Set with the changed snapshot which needs to be saved
    */
    static void saveSnapshot(const std::filesystem::path& path, const std::set<std::string>& files);
};
