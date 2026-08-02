#ifndef PHYSICAL_SIMULATION_H_
#define PHYSICAL_SIMULATION_H_

#include "Application.h"
#include "CalculatePhysicsSimulation.h"

#include <memory>

namespace vkengine
{
    class PhysicalSimulation : public Application
    {
    public:
        PhysicalSimulation(cString root_path);
        PhysicalSimulation(const ApplicationConfig &config, cString root_path);
        ~PhysicalSimulation();

        virtual void update() override;
        virtual void run() override;
        virtual void updateGui();

    private:
        void initializePhysics();
        void createInstanceBuffers(cUint32_t maxInstanceCount, cUint32_t modelIndex);
        void updateInstanceData(cUint32_t currentInstanceCount);


        std::unique_ptr<CalculatePhysicsSimulation> physics;
        std::vector<InstanceData> sphereInstanceData; // Sphere 인스턴스 데이터

        cUint32_t maxInstanceCount = 10000;           // 최대객수
        cFloat speedCamera = 15.0f;                   // Camera 이동 속도
    };
}

#endif // PHYSICAL_SIMULATION_H_