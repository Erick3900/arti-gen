#pragma once

#include <boost/program_options.hpp>

#include "utils/error.hpp"

#include "generator_template.hpp"

namespace opt = boost::program_options;

namespace arti {

    class generator {
      public:
        using variables_map = std::unordered_map<std::string, std::string>;

        generator() = delete;

        generator(generator_template &&template_v);
        generator(const generator_template &template_v);

        ~generator() = default;

        generator(generator &&) = default;
        generator(const generator &) = default;

        generator &operator=(generator &&) = default;
        generator &operator=(const generator &) = default;

        tl::expected<void, std::string> loadVars(const opt::variables_map &params);
        tl::expected<void, std::string> run() const;

      private:
        tl::expected<void, std::string> processVars();

        generator_template m_Template;
        variables_map m_Vars;
    };

}
