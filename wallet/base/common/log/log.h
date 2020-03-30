#pragma once
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"


#define SZAPPNAME  "wallet"
#define RET_OK 0

extern std::string  __pathname__;
extern  std::shared_ptr<spdlog::logger> fileLog;
extern  std::shared_ptr<spdlog::logger> console;

//初始化日志功能（级别）
#define LOG_INIT(level) fileLog->flush_on(level)
#define LOG_UNINT() 

#define LOG_INFO(format, ...)     fileLog->info(format, ##__VA_ARGS__);console->info(format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)    fileLog->debug(format, ##__VA_ARGS__);console->debug(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)    fileLog->warn(format, ##__VA_ARGS__);console->warn(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)    fileLog->error(format, ##__VA_ARGS__);console->error(format, ##__VA_ARGS__)
