#include "generator_template.hpp"

#include <utility>

#include <fmt/format.h>
#include <fmt/chrono.h>

#include "internal/config.hpp"

namespace arti {

    generator_template::expected_t generator_template::loadFromPath(fs::path templatePath) {
        return expected_t::unexpected_type{ { errors::ParseError, "Not implemented yet" } };
    }

    generator_template::expected_t generator_template::loadFromConfig(std::string_view name) {
        static const toml::table s_UserConfiog = [] {
            try {
                return toml::parse_file(fmt::format("{}/config.toml", arti::config::config_path));
            }
            catch(std::exception &e) {
                fmt::print("Couldn't load user config from '{}'"
                           ", make sure the config files exists and has the right perms\n"
                           "Reported error: {}\n",
                           arti::config::config_path,
                           e.what());

                return toml::table{ };
            }
            catch(...) {
                fmt::print("Couldn't load user config from '{}'"
                           ", make sure the config files exists and has the right perms\n",
                           arti::config::config_path);

                return toml::table{ };
            }
        }();

        if (s_UserConfiog.empty()) {
            return expected_t::unexpected_type{ { errors::ConfigNotFound, "User config not found" } };
        }

        if (! s_UserConfiog.contains(name)) {
            return expected_t::unexpected_type{ { errors::NotFound, fmt::format("Template '{}' not found on user config", name) } };
        }

        auto templateConfig = *s_UserConfiog.get(name)->as_table();

        auto templateType = [&] {
            auto typeStr = templateConfig.get("type")->value_or<std::string>("unknown");

            std::transform(
                typeStr.cbegin(),
                typeStr.cend(),
                typeStr.begin(),
                [](char c) {
                    return std::tolower(c);
                }
            );

            if (typeStr == "file") {
                return types::File;
            }
            else if (typeStr == "folder") {
                return types::Folder;
            }

            return types::Unknown;
        }();

        if (templateType == types::Unknown) {
            return expected_t::unexpected_type{ { errors::ParseError, fmt::format("The template '{}' has an unknown 'type' value", name) } };
        }

        if (! templateConfig.contains("folder")) {
            return expected_t::unexpected_type{ { errors::ParseError, fmt::format("The template '{}' doesn't have the 'name' property", name) } };
        }


        if (! templateConfig.contains("root")) {
            return expected_t::unexpected_type{ { errors::ParseError, fmt::format("The template '{}' doesn't have the 'root' property", name) } };
        }

        bool nameParamOptional = [&] {
            auto fromOpt = templateConfig.get_as<bool>("name_param_optional");
            if (fromOpt) {
                return fromOpt->value_or(false);
            }

            return false;
        }();
        std::string templateName = templateConfig.get_as<std::string>("name")->value_or("");
        std::string templateRoot = templateConfig.get_as<std::string>("root")->value_or("");
        fs::path templatePath{ fmt::format("{}/{}", arti::config::config_path, templateConfig.get_as<std::string>("folder")->value_or("")) };


        generator_template temp{
            templateType,
            nameParamOptional,
            templatePath,
            templateName,
            templateRoot
        };

        temp.loadDefaultVars();

        return std::move(temp);
    }

    void generator_template::loadDefaultVars() {
        m_DefaultVars["now"] = fmt::format("{:%A %B %d, %Y - %I:%M:%S%p}", fmt::localtime(std::time(nullptr)));
        m_DefaultVars["today"] = fmt::format("{:%A %B %d, %Y}", fmt::localtime(std::time(nullptr)));
        m_DefaultVars["full_cwd"] = fs::current_path().string();
        m_DefaultVars["cwd"] = fs::current_path().filename().string();

        const auto varsPath = m_Location / "vars.toml";

        if (! fs::exists(varsPath)) {
            return;
        }

        // TODO: Implement 'dynamic' variables, dependency graph?

        auto vars = toml::parse_file(varsPath.string());

        for (const auto &[key, value] : vars) {
            const std::string k{ key.str() };

            value.visit([&](auto &&v) {
                if constexpr (toml::is_string<decltype(v)>) {
                    m_DefaultVars[k] = v.template value_or<std::string>("");
                }
                else if constexpr (toml::is_integer<decltype(v)>) {
                    m_DefaultVars[k] = std::to_string(v.template value_or<int64_t>(0));
                }
                else if constexpr (toml::is_floating_point<decltype(v)>) {
                    m_DefaultVars[k] = std::to_string(v.template value_or<double>(0.0));
                }
            });
        }
    }

    std::string_view generator_template::getName() const {
        return m_Name;
    }

    const fs::path &generator_template::getRootPath() const {
        return m_Location;
    }

    generator_template::generator_template(types type, bool nameParamOptional, fs::path path,std::string name, std::string root)
        : m_Type(type)
        , m_NameParamOptional(nameParamOptional)
        , m_Location(std::move(path))
        , m_Name(std::move(name))
        , m_TemplateRoot(std::move(root)) {
    }

}
