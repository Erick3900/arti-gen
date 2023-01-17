#pragma once

#include <string_view>
#include <unordered_map>

namespace arti {
    
    class variable_substitutor {
      public:
        using variables_map = std::unordered_map<std::string, std::string>;

        variable_substitutor() = delete;
        ~variable_substitutor() = delete;

        variable_substitutor(variable_substitutor &&) = delete;
        variable_substitutor(const variable_substitutor &) = delete;

        variable_substitutor &operator=(variable_substitutor &&) = delete;
        variable_substitutor &operator=(const variable_substitutor &) = delete;

        static std::string run(const std::string &line, const variables_map &vars);
    };

}
