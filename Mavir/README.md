# Mavir
Lightweight User-Mode antivirus engine inspired by cybersecurity research.

# Responsibilities
- FileUtils: Responsible for File Operation Tasks
- Logger: Initializes Logger
- Menu: Responsible for displaying a responsive Menu while the AV is running
- StartupFolderMonitor: Monitors both StartupFolders (StartupFolder,StartupFolder_COMMON)
- Initialzes vbs Subsystem Registry Key on Start to 0 and tracks it with RegNotifyChangeKeyValue
- main: Grouping everything together, managing Threads etc
