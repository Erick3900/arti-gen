#include "options_parser.hpp"

namespace arti {

    options_parser::options_parser()
        : m_Options{ "Artichoke Template Generator:\n\nUsage" } {
        auto optionsDef = m_Options.add_options();

        optionsDef("version,v", "Prints the program version information");
        optionsDef("interactive,i", "Runs program on CLI interactive mode");
        optionsDef("template,t", opt::value<std::string>(), "Specifies the template to use");
        optionsDef("define,d", opt::value<std::vector<std::string>>()->multitoken(), "Variable definition for template substitution");
        optionsDef("name,n", opt::value<std::string>(), "Specifies the name of the project or file to be generated");
        optionsDef("help,h", "Prints this help message");
    }

    options_parser::expected_t options_parser::parse(int argc, char **argv) {
        opt::variables_map vars;

        try {
            opt::store(opt::parse_command_line(argc, argv, m_Options), vars);

            if (vars.empty()) {
                return expected_t::unexpected_type{ { errors::Empty, "No params provided" } };
            }

            if (vars.contains("help")) {
                return expected_t::unexpected_type{ { errors::Help, help() } };
            }

            opt::notify(vars);
        }
        catch(std::exception &err) {
            return expected_t::unexpected_type{ { errors::Invalid, err.what() } };
        }

        return std::move(vars);
    }

    std::string options_parser::help() const {
        return (std::stringstream{} << m_Options).str();
    }

}