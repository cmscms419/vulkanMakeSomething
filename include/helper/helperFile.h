#ifndef INCLUDE_HELPER_FILE_H_
#define INCLUDE_HELPER_FILE_H_

#include <vector>
#include <filesystem>

#include "common.h"

namespace vkengine
{
    namespace helper
    {
        namespace file
        {

            cString extractFilename(const cString &spvFilename);

            // --- filesystem 유틸리티 ---

            // "model.glb" → "model_cache.bin" 경로 반환
            cString getCachePath(const cString &modelFilename);

            // "/a/b/model.glb" → "/a/b"
            cString getParentDirectory(const cString &filePath);

            // "/a/b/model.glb" → "model"  (확장자 제외)
            cString getStem(const cString &filePath);

            // "/a/b/model.glb" → "model.glb"  (파일명만)
            cString getFilenameOnly(const cString &filePath);

            // "dummy/../textures/foo.png" → "textures/foo.png"
            cString normalizePath(const cString &filePath);

            // 디렉토리 재귀 생성 (없으면 생성)
            cBool createDirectories(const cString &dirPath);

            // 캐시가 원본보다 최신인지 확인
            cBool isCacheNewer(const cString &modelPath, const cString &cachePath);

            // 파일을 읽어오는 함수
            std::vector<cChar> readFile(const cString &filename);

            // spv 파일을 읽어오는 함수
            std::vector<cChar> readSPVFile(const cString &filename);

            cBool fileExists(const cString &filename);

            cBool writeString(std::ofstream &stream, const cString &str);

            cBool readString(std::ifstream &stream, cString &str);

            // Helper methods for binary I/O
            // Template implementations (must be in header for C++14)
            template <typename T>
            bool writeValue(std::ofstream &stream, const T &value)
            {
                stream.write(reinterpret_cast<const char *>(&value), sizeof(T));
                return stream.good();
            }

            template <typename T>
            bool readValue(std::ifstream &stream, T &value)
            {
                stream.read(reinterpret_cast<char *>(&value), sizeof(T));
                return stream.good();
            }

            template <typename T>
            bool writeVector(std::ofstream &stream, const std::vector<T> &vec)
            {
                uint32_t size = static_cast<uint32_t>(vec.size());
                if (!writeValue(stream, size))
                    return false;

                if (size > 0)
                {
                    stream.write(reinterpret_cast<const char *>(vec.data()), size * sizeof(T));
                    return stream.good();
                }
                return true;
            }

            template <typename T>
            bool readVector(std::ifstream &stream, std::vector<T> &vec)
            {
                uint32_t size;
                if (!readValue(stream, size))
                    return false;

                vec.resize(size);
                if (size > 0)
                {
                    stream.read(reinterpret_cast<char *>(vec.data()), size * sizeof(T));
                    return stream.good();
                }
                return true;
            }
        
        }
    }
}

#endif // !INCLUDE_HELPER_FILE_H_
