#include "helperFile.h"
#include "log.h"
#include <fstream>
#include <filesystem>

using namespace vkengine::Log;

namespace vkengine
{
    namespace helper
    {
        namespace file
        {
            // --- filesystem 유틸리티 구현 ---

            // "model.glb" → "model_cache.bin" 경로 반환
            cString getCachePath(const cString &modelFilename)
            {
                std::filesystem::path modelPath(modelFilename);
                cString cacheFilename = modelPath.stem().string() + "_cache.bin";
                return (modelPath.parent_path() / cacheFilename).string();
            }

            // "/a/b/model.glb" → "/a/b"
            cString getParentDirectory(const cString &filePath)
            {
                return std::filesystem::path(filePath).parent_path().string();
            }

            // "/a/b/model.glb" → "model"  (확장자 제외)
            cString getStem(const cString &filePath)
            {
                return std::filesystem::path(filePath).stem().string();
            }

            // "/a/b/model.glb" → "model.glb"  (파일명만)
            cString getFilenameOnly(const cString &filePath)
            {
                return std::filesystem::path(filePath).filename().string();
            }

            // "dummy/../textures/foo.png" → "textures/foo.png"
            // "..\textures\foo.png" 같은 상대경로를 정규화하는 데 사용
            cString normalizePath(const cString &filePath)
            {
                std::filesystem::path fullPath("dummy/" + filePath);
                return fullPath.lexically_normal().string();
            }

            // 디렉토리 재귀 생성 (없으면 생성)
            cBool createDirectories(const cString &dirPath)
            {
                std::error_code ec;
                std::filesystem::create_directories(dirPath, ec);
                return !ec;
            }

            // 캐시가 원본보다 최신인지 확인
            cBool isCacheNewer(const cString &modelPath, const cString &cachePath)
            {
                std::error_code ec;
                if (!std::filesystem::exists(cachePath, ec) || ec)
                    return false;
                auto modelTime = std::filesystem::last_write_time(modelPath, ec);
                if (ec) return false;
                auto cacheTime = std::filesystem::last_write_time(cachePath, ec);
                if (ec) return false;
                return cacheTime > modelTime;
            }

            cString extractFilename(const cString &spvFilename)
            {
                if (spvFilename.length() < 4 || spvFilename.substr(spvFilename.length() - 4) != ".spv")
                {
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

            std::vector<cChar> readFile(const cString &filename)
            {
                // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
                std::ifstream file(filename, std::ios::ate | std::ios::binary);

                // 파일을 열 수 없는 경우 예외를 발생시킵니다.
                if (!file.is_open())
                {
                    EXIT_TO_LOGGER("failed to open file!");
                }

                size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
                std::vector<cChar> buffer(fileSize);    // 파일 포인터를 파일의 시작으로 이동합니다.
                file.seekg(0);                          // 파일 포인터를 파일의 시작으로 이동합니다.
                file.read(buffer.data(), fileSize);     // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
                file.close();                           // 파일을 닫습니다.

                return buffer;
            }

            std::vector<cChar> readSPVFile(const cString &filename)
            {
                // 파일 확장자가 .spv인지 확인합니다.
                if (filename.length() < 4 || filename.substr(filename.length() - 4) != ".spv")
                {
                    EXIT_TO_LOGGER("Shader file does not have .spv extension: %s", filename.c_str());
                }

                // 파일 끝으로 이동하여 파일 크기를 가져옵니다.
                std::ifstream file(filename, std::ios::ate | std::ios::binary);

                // 파일을 열 수 없는 경우 예외를 발생시킵니다.
                if (!file.is_open())
                {
                    EXIT_TO_LOGGER("failed to open file!");
                }

                // Get file size and validate it's a valid SPIR-V file
                size_t fileSize = (size_t)file.tellg(); // 파일 크기를 이용하여 버퍼를 할당합니다.
                if (fileSize == 0 || fileSize % 4 != 0)
                {
                    EXIT_TO_LOGGER("Invalid SPIR-V file size: %zu bytes", fileSize);
                }

                std::vector<cChar> buffer(fileSize); // 파일 포인터를 파일의 시작으로 이동합니다.
                file.seekg(0);                       // 파일 포인터를 파일의 시작으로 이동합니다.
                file.read(buffer.data(), fileSize);  // 파일 내용을 버퍼에 읽어옵니다. -> 파일을 fileSize 크기만큼 한번에 읽어온다.
                file.close();                        // 파일을 닫습니다.

                return buffer;
            }

            bool fileExists(const cString &filename)
            {
                std::ifstream f(filename.c_str());
                return !f.fail();
            }

            cBool writeString(std::ofstream &stream, const cString &str)
            {
                uint32_t length = static_cast<uint32_t>(str.length());
                if (!writeValue(stream, length))
                    return false;

                if (length > 0)
                {
                    stream.write(str.c_str(), length);
                    return stream.good();
                }
                return true;
            }

            cBool readString(std::ifstream &stream, cString &str)
            {
                uint32_t length = 0;
                if (!readValue(stream, length))
                    return false;

                if (length > 0)
                {
                    str.resize(length);
                    stream.read(&str[0], length);
                    return stream.good();
                }
                str.clear();
                return true;
            }
        }
    }
}