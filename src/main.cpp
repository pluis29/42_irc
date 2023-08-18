#include "server.hpp"

void handle_sigint(int sig) {
    (void)sig;
    throw std::runtime_error("");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Please provide the correct command-line arguments: "
                     "./ircserv <port> <password>"
                  << std::endl;
        return EXIT_FAILURE;
    }
    signal(SIGINT, handle_sigint);
    try {
        Server server(HOST, argv[1], argv[2]);
        server.init_pool();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
