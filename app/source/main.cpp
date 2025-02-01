
#include "Application.h"

int main(int argc, char* argv[]) {

    std::unique_ptr<vkutil::Application> app;
    app = std::make_unique<vkutil::Application>();

    //switch (atoi(argv[1]))
    //{
    //default:
    //    break;
    //}

    app->init();
    app->run();
    app->cleanup();

    return EXIT_SUCCESS;
}