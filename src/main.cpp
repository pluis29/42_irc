#include "server.hpp"

void handle_sigint(int sig);

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Please provide the correct command-line arguments: "
                     "./ircserv <port> <password>"
                  << std::endl;
        return EXIT_FAILURE;
    }
    signal(SIGINT, handle_sigint);
    try {
        Server server(std::atoi(argv[1]), argv[2]);
        server.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return EXIT_SUCCESS;
}

void handle_sigint(int sig) {
    (void)sig;
    throw std::runtime_error("");
}
