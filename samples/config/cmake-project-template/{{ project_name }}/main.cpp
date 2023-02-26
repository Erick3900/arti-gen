#include <iostream>

#include <internal/config.hpp>

namespace {{ project_namespace }} {
    static void greet() {
        std::cout << "Hello, " << {{ project_namespace }}::config::project_name << "!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    {{ project_namespace }}::greet();
}
