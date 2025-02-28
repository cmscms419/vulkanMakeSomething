#ifndef INCLUDE_IMGUI_H_
#define INCLUDE_IMGUI_H_

#include "../_common.h"

namespace vkengine {

    namespace gui {
        class vkGUI {
        public:
            vkGUI();
            ~vkGUI();
            void init();
            void initResources(VkRenderPass renderPass, VkQueue copyQueue, const std::string& shadersPath);
            void update();
        private:
            
        };
    }
}

#endif // !
