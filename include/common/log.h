#ifndef _INCLUDE_LOG_H_
#define _INCLUDE_LOG_H_

#include "base_types.h"
#include "macros.h"

#include <fstream>
#include <iostream>
#include <cassert>

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

            // 다른 복사 생성자와 대입 연산자를 삭제하여 싱글톤 패턴을 보장
            Logger();
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
