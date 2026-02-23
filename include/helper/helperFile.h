#ifndef INCLUDE_HELPER_FILE_H_
#define INCLUDE_HELPER_FILE_H_

#include <set>
#include <fstream>

#include "common.h"
#include "log.h"

namespace vkengine
{
    namespace helper
    {
        namespace file
        {

            cString extractFilename(const cString &spvFilename);

            // 파일을 읽어오는 함수
            std::vector<cChar> readFile(const cString &filename);

            // spv 파일을 읽어오는 함수
            std::vector<cChar> readSPVFile(const cString &filename);

            bool fileExists(const cString &filename);
        }
    }
}

#endif // !INCLUDE_HELPER_FILE_H_
