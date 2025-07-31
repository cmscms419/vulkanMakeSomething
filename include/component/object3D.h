#ifndef INCLUDE_OBJECT3D_H_
#define INCLUDE_OBJECT3D_H_ 

#include "common.h"
#include "struct.h"
#include "macros.h"
#include "vkmath.h"

#include "VKbuffer.h"
#include "VKtexture.h"
#include "helper.h"
#include "VKDescriptor.h"
#include "Camera.h"

namespace vkengine {

    class VKBaseObject {
    public:
        // 기본 생성자 추가
        VKBaseObject() = default;
        VKBaseObject(VKdeviceHandler* device) : 
            physicalDevice(device->physicalDevice), 
            logicaldevice(device->logicaldevice), 
            commandPool(device->commandPool), 
            graphicsVKQueue(device->graphicsVKQueue) 
        {};
    protected:
        
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };  // 물리 디바이스 -> GPU Physical Handle
        VkDevice logicaldevice{ VK_NULL_HANDLE };           // 논리 디바이스 -> GPU Logical Handle
        VkCommandPool commandPool{ VK_NULL_HANDLE };        // 커맨드 풀 -> 커맨드 버퍼를 생성하는 데 사용
        VkQueue graphicsVKQueue{ VK_NULL_HANDLE };          // 그래픽스 큐 -> 그래픽스 명령을 처리하는 큐
    };

    namespace object {

        class Object3d : public VKBaseObject 
        {
        public:
            Object3d(VKdeviceHandler* device) : VKBaseObject(device) {};
            ~Object3d() {};
            void createVertexBuffer(std::vector<Vertex>& vertices = std::vector<Vertex>());
            void createIndexBuffer(std::vector<cUint16_t>& indices = std::vector<cUint16_t>());
            void createModelViewProjBuffers();
            void createTexture(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB)
            {
                this->texture->createTextureImage();
                this->texture->createTextureImageView(format);
                this->texture->createTextureSampler();
            };

            virtual void draw(VkCommandBuffer commandBuffer, bool drawIndex = true);
            virtual void cleanup() {
                this->vertexBuffer.cleanup();
                this->indexBuffer.cleanup();
                this->vertices.clear();
                this->indices.clear();
                if (this->texture) {
                    this->texture->cleanup();
                    if (this->texture->resourcePNG) {
                        delete this->texture->resourcePNG; // Replace direct destructor call with delete  
                    }
                }

                for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
                {
                    modelviewprojUniformBuffer[i].cleanup();
                }
            };
            
            VertexBuffer* getVertexBuffer() { return &this->vertexBuffer; }
            IndexBuffer* getIndexBuffer() { return &this->indexBuffer; }
            UniformBuffer* getModelViewProjUniformBuffer(cUint16_t currentImage) { return &this->modelviewprojUniformBuffer[currentImage]; }
            cString getName() { return this->name; }
            VkTextureBase* getTexture() { return this->texture; } // 텍스처 객체 반환
            std::vector<Vertex>* getVertices() { return &this->vertices; }
            std::vector<cUint16_t>* getIndices() { return &this->indices; }
            cVec3 getPosition() { return this->position; }
            cQuat getRotation() { return this->rotation; }
            cVec3 getScale() { return this->scale; }
            cMat4 getMatrix() { return this->matrix; }

            void setName(cString name) { this->name = name; }
            void setVertices(std::vector<Vertex>& vertices) { this->vertices = vertices; }
            void setIndices(std::vector<cUint16_t>& indices) { this->indices = indices; }
            void setPosition(cVec3 position) { this->position = position; }
            void setRotation(cQuat rotation) { this->rotation = rotation; }
            void setScale(cVec3 scale) { this->scale = scale; }
            void setMatrix(cMat4 matrix) { this->matrix = matrix; }
            void setTexturePNG(TextureResourcePNG* resourcePNG) { this->texture->setResourcePNG(resourcePNG); }
            void setTextureKTX(TextureResourceKTX* resourceKTX) { this->texture->setResourceKTX(resourceKTX); } // KTX 텍스처 설정
            void CaluateRotation(cQuat rotation) { this->rotation *= rotation; }
            void RotationAngle(float angle, cVec3 axis);
            void updateMatrix();

        protected:

            VertexBuffer vertexBuffer{};
            IndexBuffer indexBuffer{};
            UniformBuffer modelviewprojUniformBuffer[MAX_FRAMES_IN_FLIGHT]{}; // swapChain image 개수만큼 uniform buffer 생성
            VkTextureBase* texture = nullptr; // 텍스처 객체
            cString name = "";

            std::vector<Vertex> vertices; // 버텍스 데이터
            std::vector<cUint16_t> indices; // 인덱스 데이터

            cVec3 position = { 0.0f, 0.0f, 0.0f };
            cQuat rotation = cQuat(1.0f, 0.0f, 0.0f, 0.0f);
            cVec3 scale = { 1.0f, 1.0f, 1.0f };
            cMat4 matrix = cMat4(1.0f);

        };

        class TextureArrayObject3D : public Object3d
        {
        public:
            TextureArrayObject3D(VKdeviceHandler* device) : Object3d(device) {
                this->texture = new Vk2DTextureArray();
                this->texture->initializeDeviceHandles
                (device->physicalDevice, device->logicaldevice, device->commandPool, device->graphicsVKQueue);
            };
            virtual void cleanup() {
                Object3d::cleanup();
            };
        private:
        };

        class SkyBox : public Object3d
        {
        public:

            SkyBox(VKdeviceHandler* device) : Object3d(device) {
                this->texture = new VKcubeMap();
                this->texture->initializeDeviceHandles
                (device->physicalDevice, device->logicaldevice, device->commandPool, device->graphicsVKQueue);
            };

            void updateUniformBuffer(uint32_t currentImage, Camera* camera);
            void createTexture2(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB) {
                this->texture->createTextureImgae2(format);
                this->texture->createTextureImageView(format);
                this->texture->createTextureSampler();
            };

            virtual void cleanup() {
                Object3d::cleanup();
            };
        private:
        };

        class ModelObject : public Object3d
        {
        public:
            ModelObject(VKdeviceHandler* device) : Object3d(device) {
                this->texture = new Vk2DTexture();
                this->texture->initializeDeviceHandles
                (device->physicalDevice, device->logicaldevice, device->commandPool, device->graphicsVKQueue);
            };

            void updateUniformBuffer(uint32_t currentImage, cMat4 world, Camera* camera);

            virtual void cleanup() {
                Object3d::cleanup();
            };

        private:
        };


        // Particle은 Object를 상속받지 않습니다.
        // VertexBuffer와 IndexBuffer를 사용하지 않기 때문입니다.
        // 어떻게 할지 명확하게 정의되지 않음, 하지만 사용해야 하기에 VKBaseObject를 상속받습니다.
        class Particle : VKBaseObject {
        public:
            Particle(VKdeviceHandler* device) : VKBaseObject(device) {};
            void setParticles(std::vector<Particle>& particles) { this->objects = particles; };
            void addParticle(const Particle& particle) { this->objects.push_back(particle); };
            size_t getParticleCount() const { return this->objects.size(); };
            
            PaterialBuffer InputPartucleBuffer; // 입력 파티클 버퍼
            PaterialBuffer OutputPartucleBuffer; // 출력 파티클 버퍼

        private:
            std::vector<Particle> objects;
        };

    }
}



#endif // !INCLUDE_OBJECT3D_H_
