#ifndef _INCLUDE_LOG_H_
#define _INCLUDE_LOG_H_

#include "common.h"
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
            ~Logger() {
                if (logFile.is_open()) {
                    logFile.close();
                    logFile.flush();
                    logFile.close();
                }
            }

            static Logger& getInstance()
            {
                if (!instance) {
                    instance = std::unique_ptr<Logger>(new Logger());
                }
                return *instance;
            }

            static void printLog(cString message)
            {
                Logger& logger = getInstance();

                _PRINT_TO_CONSOLE_("%s", message.c_str());

                if (logger.logFile.is_open()) {
                    logger.logFile << message.c_str();
                    logger.logFile.flush();
                    logger.messagesProcessed++;
                }
                else {
                    _PRINT_TO_CONSOLE_("Error: Log file is not open.\n");
                    _PRINT_TO_CONSOLE_("message lost: %s", message.c_str());
                }
            }

            template <typename... Args>
            std::string formatToString(const Args&... args) {
                std::ostringstream oss;
                (oss << ... << args); // fold expression (C++17)
                return oss.str();
            }

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

            template <typename T>
            void fromatToStream(std::ostringstream oss, T&& arg) {
                oss << std::forward<T>(arg);
            }
            
            template <typename T, typename... Args>
            void formatToStream(std::ostringstream& oss, T&& first, Args&&... args)
            {
                oss << std::forward<T>(arg);

                if (sizeof...(args) > 0) {
                    formatToStream(oss, std::forward<Args>(args)...);
                }
            }

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
