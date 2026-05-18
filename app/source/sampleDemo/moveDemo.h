#ifndef RESUME_DEMO_H_
#define RESUME_DEMO_H_

#include "Application.h"

namespace vkengine
{
    class DemoApplication : public Application
    {
    public:
        DemoApplication(cString root_path);
        ~DemoApplication();

        virtual void update() override;
    private:
        cFloat FindYGround(const cVec3& position);
        void resolveWallCollisions(cVec3& position);

        cVec3 velocity;
        cBool isGrounded;
        cBool isMoving;
        cBool isSpeedUp;
        cFloat currentFacingYaw;
    };
}

#endif // RESUME_DEMO_H_
