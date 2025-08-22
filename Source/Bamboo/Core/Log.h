// Bamboo/Core/Log.h
 #pragma once

#include "../Bamboo/Core/Ref.h"
#include <spdlog/spdlog.h>

namespace Bamboo
{

class Log {
	public:
		static void Init();
		static Ref < spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() {return s_ClientLogger;}
	
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};


} // namespace Bamboo

#define BAMBOO_CORE_TRACE(...)    ::Bamboo::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define BAMBOO_CORE_INFO(...)     ::Bamboo::Log::GetCoreLogger()->info(__VA_ARGS__)
#define BAMBOO_CORE_WARN(...)     ::Bamboo::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define BAMBOO_CORE_ERROR(...)    ::Bamboo::Log::GetCoreLogger()->error(__VA_ARGS__)
#define BAMBOO_CORE_CRITICAL(...) ::Bamboo::Log::GetCoreLogger()->critical(__VA_ARGS__)