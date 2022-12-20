#pragma once
#include <format>

#if defined(WIN32)
#include <Windows.h>
#endif

namespace atlas::trace::logging
{
    enum class LogLevel
    {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    inline std::string_view getLogLevelAsString(LogLevel logLevel)
    {
        switch(logLevel)
        {
            case LogLevel::Trace: return "Trace";
            case LogLevel::Debug: return "Debug";
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            case LogLevel::Critical: return "Critical";
        }

        return "Unknown";
    }

    template <typename... T>
    void doLog(const std::string_view channel, const LogLevel logLevel, const std::string_view file, const int line, const std::string_view fmt, T&&... args)
    {
        const auto trimmedFileName = strrchr(std::string(file).c_str(), '\\') ? strrchr(std::string(file).c_str(), '\\') + 1 : file;
        constexpr std::string_view c_format = "[{}][{}][{}:{}] {}\n";

        const std::string templatedString = std::format(c_format, getLogLevelAsString(logLevel), channel, trimmedFileName, line, fmt);
        std::string outStr = std::vformat(templatedString, std::make_format_args(args...));

        printf(outStr.c_str());
#if defined(WIN32)
        OutputDebugStringA(outStr.c_str());
#endif
    }
}
