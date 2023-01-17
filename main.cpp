#include "internal/config.hpp"

#include "options_parser.hpp"
#include "generator_template.hpp"
#include "generator.hpp"

void printVersion();
tl::expected<arti::generator_template, std::string> loadTemplate(const opt::variables_map &vars);

#include <iostream>
#include "variable_substitutor.hpp"

int main(int argc, char* argv[]) {
    auto parserEx = [&] {
        arti::options_parser options;

        return options.parse(argc, argv);
    }();

    if (!parserEx) {
        auto [errorCode, errorInfo] = std::move(parserEx).error();

        switch (errorCode) {
            case decltype(errorCode)::Empty:
                fmt::print("{}, see usage with '--help' or '-h'\n", errorInfo);
                break;
            case decltype(errorCode)::Help:
                fmt::print("{}\n", errorInfo);
                break;
            case decltype(errorCode)::Invalid:
                fmt::print("Invalid params provided: '{}'\n", errorInfo);
                break;
        }

        return 1;
    }

    auto options = std::move(parserEx).value();

    if (options.contains("version")) {
        printVersion();
        return 1;
    }

    // TODO: Implement TUI

    if (options.contains("interactive")) {
        fmt::print("Interactive TUI not implemented yet!\n");
        return 1;
    }

    auto loadTemplateEx = loadTemplate(options);

    if (!loadTemplateEx) {
        fmt::print("Error loading the template: {}\n", loadTemplateEx.error());

        return 1;
    }

    auto template_v = std::move(loadTemplateEx).value();

    arti::generator gen{ std::move(template_v) };

    if (auto ex = gen.loadVars(options); ! ex) {
        fmt::print("{}\n", ex.error());
        return 1;
    }

    if (auto ex = gen.run(); ! ex) {
        fmt::print("{}\n", ex.error());
        return 1;
    }
}

void printVersion() {
    using namespace arti;

    fmt::print(
        "{} version {}\n\n"
        "       Author:  {}\n"
        "Author Github: {}\n",
        config::project_name,
        config::project_version,
        config::project_author,
        config::project_author_github
    );
}

tl::expected<arti::generator_template, std::string> loadTemplate(const opt::variables_map &vars) {
    using expected_t = tl::expected<arti::generator_template, std::string>;
    using error_t = expected_t::unexpected_type;

    if (! vars.contains("template")) {
        return error_t{ "Template parameter is required" };
    }

    auto templateParam = vars.at("template").as<std::string>();

    auto templateRet = [&]() -> expected_t {
        auto templateEx = arti::generator_template::loadFromConfig(templateParam);

        if (templateEx) {
            return std::move(templateEx).value();
        }

        auto [errorCode, errorInfo] = std::move(templateEx).error();

        return error_t{ errorInfo };
    }();

    return std::move(templateRet);
}
