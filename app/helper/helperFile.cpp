#include "helperFile.h"

using namespace vkengine::Log;

namespace vkengine {
    namespace helper {

        cString extractFilename(const cString& spvFilename)
        {
            if (spvFilename.length() < 4 || spvFilename.substr(spvFilename.length() - 4) != ".spv") {
                EXIT_TO_LOGGER("Shader file does not have .spv extension: %s", spvFilename);
            }

            // 경로와 마지막 .spv 제거 ex: path/triangle.vert.spv -> triangle.vert
            size_t lastSlash = spvFilename.find_last_of("/\\");
            size_t start = (lastSlash == cString::npos) ? 0 : lastSlash + 1;
            size_t end = spvFilename.length();
            size_t lastDot = spvFilename.find_last_of('.');
            if (lastDot != cString::npos && lastDot > start)
                end = lastDot;

            return spvFilename.substr(start, end - start);
        }

        std::vector<cChar> readFile(const cString& filename)
        {
            // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // 파일을 열 수 없는 경우 예외를 발생시킵니다.
            if (!file.is_open()) {
                EXIT_TO_LOGGER("failed to open file!");
            }

            size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
            std::vector<cChar> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
            file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
            file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
            file.close();                           // 파일을 닫습니다.

            return buffer;
        }

        std::vector<cChar> readSPVFile(const cString& filename)
        {
            // 파일 확장자가 .spv인지 확인합니다.
            if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".spv") {
                EXIT_TO_LOGGER("Shader file does not have .spv extension: %s", filename.c_str());

            }

            // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            // 파일을 열 수 없는 경우 예외를 발생시킵니다.
            if (!file.is_open()) {
                EXIT_TO_LOGGER("failed to open file!");
            }

            // Get file size and validate it's a valid SPIR-V file
            size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
            if (fileSize == 0 || fileSize % 4 != 0) {
                EXIT_TO_LOGGER("Invalid SPIR-V file size: %zu bytes", fileSize);
            }

            std::vector<cChar> buffer(fileSize);     // 파일 포인터를 파일의 시작으로 이동합니다.
            file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
            file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
            file.close();                           // 파일을 닫습니다.

            return buffer;
        }

        bool fileExists(const cString& filename)
        {
            std::ifstream f(filename.c_str());
            return !f.fail();
        }

    }
}