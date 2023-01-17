#include "variable_substitutor.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include <ctre.hpp>

#include <fmt/format.h>

namespace arti {

    template<typename T>
    class GetType;

    std::string variable_substitutor::run(const std::string &line, const variables_map &vars) {     
        const auto substituteVariable = [&](const std::string &varName) -> std::string {
            std::unordered_set<std::string_view> alreadyVisited;

            if (vars.contains(varName)) {
                return vars.at(varName);
            }

            return "";
        };

        auto ret = ctre::range<"[{]{2}([ ]*)?(?<varname>[a-zA-Z][a-zA-Z0-9_]*)([ ]*)?[}]{2}">(line);

        std::stringstream ss;

        auto lastIter = line.begin();

        for (const auto &v : ret) {
            ss << std::string_view{ lastIter, v.begin() };
            ss << substituteVariable(v.get<"varname">().to_string());
 
            lastIter = v.end();
        }

        ss << std::string_view{ lastIter, line.end() };

        return ss.str();
    }

}
