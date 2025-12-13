#ifndef INCLUDE_VK_UNIFORM_BUFFER_2_H_
#define INCLUDE_VK_UNIFORM_BUFFER_2_H_

#include "VKContext.h"
#include "VKbuffer2.h"

namespace vkengine {

    template <typename Type>
    class VKUniformBuffer2 {

    public:
        VKUniformBuffer2(VKcontext& ctx, Type& data) : buffer(ctx), data(data) {
            static_assert(std::is_trivially_copyable_v<Type>,
                "UniformBuffer data type must be trivially copyable\n");
            // 안내: vector 같이 동적 메모리를 사용하는 컨테이너가 가 포함되어 있으면
            //      memcpy()로 간단히 복사할 수 없습니다.

            buffer.createUniformBuffer(sizeof(Type), &this->data);
        }

        VKUniformBuffer2(VKUniformBuffer2& other) noexcept 
            : buffer(std::move(other.buffer)), data(other.data)
        {}

        ~VKUniformBuffer2() = default;

        void updateData()
        {
            buffer.updateData(&this->data, sizeof(this->data), 0);
        }

        Type& Data() {
            return this->data;
        }

        const VKBaseBuffer2& Buffer() const {
            return this->buffer;
        }

        VKResourceBinding& ResourceBinding()
        {
            return this->buffer.ResourceBinding();
        }

    private:
        Type& data;
        VKBaseBuffer2 buffer;
    };

}

#endif // !
