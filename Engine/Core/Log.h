// Bamboo/Core/Log.h
#pragma once

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include "../Platform/Platform.h"


namespace Bamboo
{
	// 日志等级定义
	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

	class Log
	{
	public:
		// 三个基础入口
		template<typename... Args>
		static void Info(Args&&... args)
		{
			LogMessage(LogLevel::Info, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Warning(Args&&... args)
		{
			LogMessage(LogLevel::Warning, std::forward<Args>(args)...);
		}

		template<typename... Args>
		static void Error(Args&&... args)
		{
			LogMessage(LogLevel::Error, std::forward<Args>(args)...);
		}

		// 日志输出文件设置
		static void SetOutputFile(const std::string& filename)
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			s_logFile.open(filename, std::ios::out | std::ios::app);
		}

		// 是否开启写入文件
		static void EnableFileOutput(bool enable)
		{
			s_outputToFile = enable;
		}

	private:
		template<typename... Args>
		static void LogMessage(LogLevel level, Args&&... args)
		{
			std::ostringstream oss;
			((oss << args << ','), ...);

			std::string prefix;
			WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
			switch (level)
			{
			case LogLevel::Info:
				prefix = "[Info] ";
				color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case LogLevel::Warning:
				prefix = "[Warning] ";
				color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case LogLevel::Error:
				prefix = "[Error] ";
				color = FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			}

			std::string message = prefix + oss.str();

#ifdef _WIN32
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
			WORD saved_attributes;
			GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
			saved_attributes = consoleInfo.wAttributes;
			SetConsoleTextAttribute(hConsole, color);
			std::cout << message << std::endl;
			SetConsoleTextAttribute(hConsole, saved_attributes);
#else
			std::cout << message << std::endl;
#endif

			// 文件输出
			if (s_outputToFile && s_logFile.is_open())
			{
				std::lock_guard<std::mutex> lock(s_mutex);
				s_logFile << message << std::endl;
			}
		}

	private:
		static inline std::ofstream s_logFile;
		static inline bool s_outputToFile = false;
		static inline std::mutex s_mutex;
	};

	// 宏封装简写
#define B_LOGI(...) Bamboo::Log::Info(__VA_ARGS__)
#define B_LOGW(...) Bamboo::Log::Warning(__VA_ARGS__)
#define B_LOGE(...) Bamboo::Log::Error(__VA_ARGS__)

} // namespace Bamboo
