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

    private:
        void initializePhysics();

        std::unique_ptr<CalculatePhysicsSimulation> physics;
        cFloat sphereScale = 0.2f; // main.cpp의 Sphere ModelConfig.setScale()과 동일한 값이어야 함

        std::vector<InstanceData> sphereInstanceData;     // Sphere 인스턴스 데이터
        cUint32_t instanceCount = 10000;                   // Sphere count

    };
}

#endif // PHYSICAL_SIMULATION_H_