#include "Logger.h"

void Logger::InitializeLogger(std::filesystem::path g_exeDirectory)
{
	if (g_loggerInitialized)
		return;

	auto logPath = g_exeDirectory / "mavir.log"; // Saves log in the same directory as the running .exe

	static plog::RollingFileAppender<plog::TxtFormatter>
		fileAppender(logPath.string().c_str(), 1024 * 1024, 3); // Rotates Files if they Reach ~1mb


	plog::init(plog::info, &fileAppender);
	PLOG_INFO << "[+] Mavir Antivirus started!";
	PLOG_INFO << "[+] EXE-Path: " << g_exeDirectory;
	g_loggerInitialized = true; 
}