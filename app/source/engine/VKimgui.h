#ifndef INCLUDE_IMGUI_H_
#define INCLUDE_IMGUI_H_

#include "../_common.h"

#include "./imconfig.h"
#include "./imgui_internal.h"
#include "./imgui_impl_glfw.h"
#include "./imgui_impl_vulkan.h"
#include "./imstb_rectpack.h"
#include "./imstb_textedit.h"
#include "./imstb_truetype.h"

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
