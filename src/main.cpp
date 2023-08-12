#include "server.hpp"

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        Server server(HOST, argv[1], argv[2]);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
