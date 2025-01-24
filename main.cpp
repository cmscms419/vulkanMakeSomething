
#include "../../include/source/Application.h"

using namespace vkutil;

int main(int argc, char* argv[]) {

    Application app;
    app.init();
    
    app.run();

    app.cleanup();

    return EXIT_SUCCESS;

}