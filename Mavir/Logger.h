#pragma once
#include <filesystem>
#include "ext/plog/Log.h"
#include "ext/plog/Appenders/RollingFileAppender.h"
#include "ext/plog/Formatters/TxtFormatter.h"
#include "ext/plog/Init.h"

class Logger
{
	public:
		inline static bool g_loggerInitialized = false; // Prevents double Initialization from the Logger by initializing a default and saving the current state

		/* @brief Checks if Logger is initialized, creates a Logpath, rotates Logs if they reach a specific Size and initializes the Logger
		*  @param Path to the main Executable to make sure the log is in the same Path
		*/
		static void InitializeLogger(std::filesystem::path g_exeDirectory);
	private:
		
};

