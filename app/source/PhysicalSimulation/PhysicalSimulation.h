#ifndef PHYSICAL_SIMULATION_H_
#define PHYSICAL_SIMULATION_H_

#include "Application.h"

namespace vkengine
{
    class PhysicalSimulation : public Application
    {
        public:
        PhysicalSimulation(cString root_path);
        PhysicalSimulation(const ApplicationConfig& config, cString root_path);
        ~PhysicalSimulation();

        virtual void update() override;
        private:
        
    };
}

#endif // PHYSICAL_SIMULATION_H_