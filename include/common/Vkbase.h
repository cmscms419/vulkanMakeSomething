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
        static cUint64_t GetTypeID() {
            static const cChar* typeName = typeid(T).name();
            static const cUint64_t typeId = static_cast<cUint64_t>(hash(typeName));
            return typeId;
        }

        // Type ID 등록 및 이름 매핑
        static void RegisterType(cUint64_t id, const cString& name) {
            typeMap()[id] = name;
        }

        // unique ID 생성 및 이름 매핑
        static void RegisterUniqueID(cUint64_t id, const cString& name) {
            uniqueMap()[id] = name;
        }


        static cString GetTypeNameByID(cUint64_t id) {
            auto it = typeMap().find(id);
            if (it != typeMap().end()) return it->second;
            return {};
        }

        // 템플릿 기반 타입 이름 반환
        template<typename T>
        static constexpr cString GetTypeName() {
            return cString(typeid(T).name());
        }

        // 커스텀 이름으로 ID 생성
        static constexpr cUint64_t GenerateID(const cChar* name) {
            return static_cast<cUint64_t>(hash(name));
        }

    private:
        static std::unordered_map<cUint64_t, cString>& typeMap() {
            static std::unordered_map<cUint64_t, cString> map;
            return map;
        }
        static std::unordered_map<cUint64_t, cString>& uniqueMap() {
            static std::unordered_map<cUint64_t, cString> map;
            return map;
        }
    };

    class VKid {
    public:
        virtual ~VKid() = default;
#if 0
        virtual const cUint64_t GetTypeID() const {
            return HASH_ID(this->name); // Returns the hash of the type name
        };
        virtual const char* GetTypeName() const {
            return this->name; // Returns the type name of the object
        };
        virtual const cUint64_t GetID() const {
            return HASH_ID(this->name); // Returns the hash of the name
        };
        virtual const char* GetName() const {
            return this->name; // Returns the name of the object
        };
#else 
        virtual const cUint64_t GetTypeID() const = 0; // Returns the type ID of the object
        virtual const char* GetTypeName() const = 0; // Returns the type name of the object
        virtual const cUint64_t GetID() const = 0; // Returns the ID of the object
        virtual const char* GetName() const = 0; // Returns the name of the object
        virtual cString ToString() const = 0;
        virtual VKid* Clone() const = 0;
#endif
    protected:
        static constexpr cUint64_t* type = nullptr; // Base class name for type identification
    };

    class VKbaseID : public VKid {
    public:
        VKbaseID(const cString& type, const cString& name) : name(name) {
            TypeIDManager::RegisterType(GetTypeID(), GetTypeName());
        }

        virtual const cUint64_t GetTypeID() const override {
            return TypeIDManager::GetTypeID<VKbaseID>();
        }

        virtual const char* GetTypeName() const override {
            return TypeIDManager::GetTypeName<VKbaseID>().c_str();
        }

        virtual const cUint64_t GetID() const override {
            return *type;
        }

        virtual const char* GetName() const override {
            return this->name.c_str();
        }

        virtual cString ToString() const override {
            return "VKbaseID: " + this->name;
        }

        virtual VKid* Clone() const override {
            return new VKbaseID(*this);
        }

    protected:
        cString type;
        cString name;
    };
}

#endif