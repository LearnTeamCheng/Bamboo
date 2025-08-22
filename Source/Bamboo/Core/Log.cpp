#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Bamboo
{

    Ref<spdlog::logger> Log::s_CoreLogger;
    Ref<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {

        s_CoreLogger = spdlog::stderr_color_mt("BAMBOO");
        s_CoreLogger->set_pattern("[%T] %^%n: %v%$");
        s_CoreLogger->set_level(spdlog::level::trace);
    }
}

