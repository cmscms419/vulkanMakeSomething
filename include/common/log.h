#ifndef _INCLUDE_LOG_H_
#define _INCLUDE_LOG_H_

#include "base_types.h"
#include "macros.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <cassert>
#include <string>
#include <chrono>
#include <iomanip>
#include <stdio.h>


namespace vkengine {
    namespace Log {

        class Logger {
        public:
            ~Logger();

            static Logger& getInstance();

            static void printLog(cString message);

            private:
            static std::unique_ptr<Logger> instance;

            std::ofstream logFile;
            size_t messagesProcessed;

            Logger() : messagesProcessed(0) {
                logFile.open("engine_log.txt", std::ios::out | std::ios::app);
               
                if (!logFile.is_open()) {
                    _PRINT_TO_CONSOLE_("Error: Unable to open log file.\n");
                }
            }

            // 다른 복사 생성자와 대입 연산자를 삭제하여 싱글톤 패턴을 보장
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            void cleanup();
        };

        template <typename... Args>
        void PRINT_TO_LOGGER(const cString& fmt, Args&&... args)
        {
            char buffer[2048];
            snprintf(buffer, sizeof(buffer), fmt.c_str(), args...);
            Logger::printLog(static_cast<cString>(buffer));
        }

        template <typename... Args>
        void EXIT_TO_LOGGER(const std::string& fmt, Args&&... args)
        {
            char buffer[2048];
            snprintf(buffer, sizeof(buffer), fmt.c_str(), args...);
            Logger::printLog(static_cast<cString>(buffer));
            assert(false);
            exit(EXIT_FAILURE);
        }

    }
}

#endif // !_INCLUDE_LOG_H_
