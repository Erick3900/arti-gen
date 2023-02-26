#pragma once

#include <string>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include <toml.hpp>

#include <fmt/format.h>

#include <boost/program_options.hpp>

#include "utils/error.hpp"

namespace fs = std::filesystem;
namespace opt = boost::program_options;

namespace arti {

    class generator;

    class generator_template {
      friend class generator;

      public:
        enum class types {
            File,
            Folder,
            Unknown
        };

        enum class errors {
            NotFound,
            ConfigNotFound,
            ParseError
        };

        using variables_map = std::unordered_map<std::string, std::string>;
        using expected_t = arti::expected<generator_template, errors>;

        static expected_t loadFromPath(fs::path templatePath);
        static expected_t loadFromConfig(std::string_view name);

        generator_template() = delete;
        ~generator_template() = default;

        generator_template(generator_template &&) = default;
        generator_template(const generator_template &) = default;

        generator_template &operator=(generator_template &&) = default;
        generator_template &operator=(const generator_template &) = default;

        void loadDefaultVars();

        std::string_view getName() const;
        const fs::path &getRootPath() const;

        template <typename T>
        requires std::is_constructible_v<std::string, T>
        std::string_view operator[](const T &key) const {
            if constexpr (std::is_same_v<std::string, T>) {
                return m_DefaultVars.at(key);
            }
            else {
                return m_DefaultVars.at(std::string{ key });
            }
        }

        template <typename T>
        requires std::is_constructible_v<std::string, T>
        std::string_view at(const T &key) const {
            if constexpr (std::is_same_v<std::string, T>) {
                return m_DefaultVars.at(key);
            }
            else {
                return m_DefaultVars.at(std::string{ key });
            }
        }

        std::string toString() const {
            std::stringstream ss;

            ss << "Template name: " << m_Name << std::endl;
            ss << "Template type: " << [&] {
                if (m_Type == types::Folder) {
                    return "Folder";
                }
                else if (m_Type == types::File) {
                    return "File";
                }

                return "Unknown";
            }() << std::endl;
            ss << "Location: " << m_Location.string() << std::endl;
            ss << "Root: " << m_TemplateRoot << std::endl;
            ss << "Variables: " << std::endl;

            for (const auto &[k, v] : m_DefaultVars) {
                ss << "* " << k << ": " << v << std::endl;
            }

            return ss.str();
        }

      private:
        generator_template(types type, bool nameParamOptional, fs::path path, std::string name, std::string root);

        types m_Type;
        bool m_NameParamOptional;
        fs::path m_Location;
        std::string m_Name;
        std::string m_TemplateRoot;
        variables_map m_DefaultVars;
    };

}
