#include "FileUtils.h" // Documentation in here

std::set<std::string> FileUtils::getFilesInFolder(const std::filesystem::path& path) {
    std::set<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) files.insert(entry.path().string());
        }
    }
    catch (const std::exception& ex) {
        PLOG_ERROR << "Error scanning folder " << path << ": " << ex.what();
    }
    return files;
}

std::set<std::string> FileUtils::readSnapshot(const std::filesystem::path& path) {
    std::set<std::string> files;
    std::ifstream inFile(path);
    if (!inFile) {
        PLOG_WARNING << "Could not read snapshot: " << path;
        return files;
    }
    std::string line;
    while (std::getline(inFile, line)) {
        if (!line.empty()) files.insert(line);
    }
    return files;
}

void FileUtils::saveSnapshot(const std::filesystem::path& path, const std::set<std::string>& files) {
    std::ofstream outFile(path);
    if (!outFile) {
        PLOG_ERROR << "Could not save snapshot: " << path;
        return;
    }
    for (const auto& file : files) outFile << file << "\n";
    PLOG_INFO << "Snapshot saved: " << path.filename() << " (" << files.size() << " entries)";
}