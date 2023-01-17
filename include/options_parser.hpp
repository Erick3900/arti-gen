#pragma once

#include <boost/program_options.hpp>

#include "utils/error.hpp"

namespace opt = boost::program_options;

namespace arti {

    class options_parser {
      public:
        enum class errors {
            Invalid,
            Empty,
            Help
        };

        using expected_t = arti::expected<opt::variables_map, errors>;

        options_parser();
        ~options_parser() = default;

        expected_t parse(int argc, char* argv[]);
        std::string help() const;

      private:
        opt::options_description m_Options;
    };

}