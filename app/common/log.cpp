#include "log.h"


namespace vkengine
{
    namespace Log
    {
        std::unique_ptr<Logger> Logger::instance = nullptr;

        Logger::Logger() : messagesProcessed(0)
        {
            logFile.open("engine_log.txt", std::ios::out | std::ios::app);

            if (!logFile.is_open())
            {
                _PRINT_TO_CONSOLE_("Error: Unable to open log file.\n");
            }
        }

        Logger::~Logger()
        {
            this->cleanup();
        }

        void Logger::cleanup()
        {
            if (logFile.is_open())
            {
                logFile.flush();
                logFile.close();
            }
        }

        void Logger::printLog(cString message)
        {
            Logger &logger = getInstance();

            _PRINT_TO_CONSOLE_("%s", message.c_str());

            if (logger.logFile.is_open())
            {
                logger.logFile << message.c_str();
                logger.logFile.flush();
                logger.messagesProcessed++;
            }
            else
            {
                _PRINT_TO_CONSOLE_("Error: Log file is not open.\n");
                _PRINT_TO_CONSOLE_("message lost: %s", message.c_str());
            }
        }
        
        Logger &Logger::getInstance()
        {
            if (!instance)
            {
                instance = std::unique_ptr<Logger>(new Logger());
            }
            return *instance;
        }
    }
}