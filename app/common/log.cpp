#include "log.h"

namespace vkengine
{
    namespace Log
    {
        std::unique_ptr<Logger> Logger::instance = nullptr;
    }
}