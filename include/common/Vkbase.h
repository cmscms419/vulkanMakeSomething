#ifndef INCLUDE_VK_BASE_H
#define INCLUDE_VK_BASE_H

#include "common.h"

namespace vkengine {
    static constexpr cUint64_t hash(const cChar* str, cUint64_t hash = 5381) {
        while (*str) {
            hash = ((hash << 5) + hash) + *str; // hash * 33 + c
            str++;
        }
        return static_cast<cUint64_t>(hash);
    }

    // 타입별 ID 관리자 클래스
    class TypeIDManager {
    public:
        // 템플릿 기반 타입 ID 생성
        template<typename T>
        static cUint64_t getTypeID() {
            static const cChar* typeName = typeid(T).name();
            static const cUint64_t typeId = static_cast<cUint64_t>(hash(typeName));
            return typeId;
        }

        // Type ID 등록 및 이름 매핑
        static void registerType(cUint64_t id, const cString& name) {
            getTypeMap()[id] = name;
        }

        // unique ID 생성 및 이름 매핑
        static void registerUniqueID(cUint64_t id, const cString& name) {
            getUniqueMap()[id] = name;
        }


        static cString getTypeNameByID(cUint64_t id) {
            auto it = getTypeMap().find(id);
            if (it != getTypeMap().end()) return it->second;
            return {};
        }

        // 템플릿 기반 타입 이름 반환
        template<typename T>
        static constexpr cString getTypeName() {
            return cString(typeid(T).name());
        }

        // 커스텀 이름으로 ID 생성
        static constexpr cUint64_t generateID(const cChar* name) {
            return static_cast<cUint64_t>(hash(name));
        }

    private:
        static std::unordered_map<cUint64_t, cString>& getTypeMap() {
            static std::unordered_map<cUint64_t, cString> map;
            return map;
        }
        static std::unordered_map<cUint64_t, cString>& getUniqueMap() {
            static std::unordered_map<cUint64_t, cString> map;
            return map;
        }
    };

    class ID {
    public:
        virtual ~ID() = default;
#if 0
        virtual const cUint64_t getTypeID() const {
            return HASH_ID(this->name); // Returns the hash of the type name
        };
        virtual const char* getTypeName() const {
            return this->name; // Returns the type name of the object
        };
        virtual const cUint64_t getID() const {
            return HASH_ID(this->name); // Returns the hash of the name
        };
        virtual const char* getName() const {
            return this->name; // Returns the name of the object
        };
#else 
        virtual const cUint64_t getTypeID() const = 0; // Returns the type ID of the object
        virtual const char* getTypeName() const = 0; // Returns the type name of the object
        virtual const cUint64_t getID() const = 0; // Returns the ID of the object
        virtual const char* getName() const = 0; // Returns the name of the object
        virtual cString toString() const = 0;
        virtual ID* clone() const = 0;
#endif
    protected:
        static constexpr cUint64_t* type = nullptr; // Base class name for type identification
    };

    class BaseID : public ID {
    public:
        BaseID(const cString& type, const cString& name) : name(name) {
            TypeIDManager::registerType(getTypeID(), getTypeName());
        }

        virtual const cUint64_t getTypeID() const override {
            return TypeIDManager::getTypeID<BaseID>();
        }

        virtual const char* getTypeName() const override {
            return TypeIDManager::getTypeName<BaseID>().c_str();
        }

        virtual const cUint64_t getID() const override {
            return *type;
        }

        virtual const char* getName() const override {
            return this->name.c_str();
        }

        virtual cString toString() const override {
            return "BaseID: " + this->name;
        }

        virtual ID* clone() const override {
            return new BaseID(*this);
        }

    protected:
        cString type;
        cString name;
    };
}

#endif
