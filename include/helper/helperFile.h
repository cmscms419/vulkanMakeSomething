#ifndef INCLUDE_HELPER_FILE_H_
#define INCLUDE_HELPER_FILE_H_

#include <set>
#include <fstream>

#include "common.h"
#include "struct.h"
#include "log.h"


namespace vkengine {
    namespace helper {

        cString extractFilename(const cString& spvFilename);

        // 파일을 읽어오는 함수
        std::vector<cChar> readFile(const std::string& filename);

        // spv 파일을 읽어오는 함수
        std::vector<cChar> readSPVFile(const std::string& filename);

        bool fileExists(const std::string& filename);

    }
}

#endif // !INCLUDE_HELPER_FILE_H_
