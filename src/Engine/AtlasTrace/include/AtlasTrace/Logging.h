#pragma once

#include "TypedLogging.h"

#define AT_LOG(CHANNEL, LOG_LEVEL, ...) atlas::trace::logging::doLog(#CHANNEL, LOG_LEVEL, __FILE__, __LINE__, __VA_ARGS__)

#define AT_CRITICAL(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Critical, __VA_ARGS__)
#define AT_ERROR(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Error, __VA_ARGS__)
#define AT_WARN(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Warning, __VA_ARGS__)
#define AT_INFO(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Info, __VA_ARGS__)
#define AT_DEBUG(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Debug, __VA_ARGS__)
#define AT_TRACE(CHANNEL, ...) AT_LOG(CHANNEL, atlas::trace::logging::LogLevel::Trace, __VA_ARGS__)
