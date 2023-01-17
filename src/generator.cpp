#include "generator.hpp"

#include <list>
#include <iostream>
#include <unordered_set>

#include <ctre.hpp>

#include "variable_substitutor.hpp"

namespace arti {

    generator::generator(generator_template &&template_v)
        : m_Template(std::move(template_v))
        , m_Vars(template_v.m_DefaultVars) { 
    }

    generator::generator(const generator_template &template_v)
        : m_Template(template_v) {
    }

    tl::expected<void, std::string> generator::loadVars(const opt::variables_map &params) {
        for (const auto &[k, v] : m_Template.m_DefaultVars) {
            m_Vars[k] = v;
        }

        if (! params.contains("name")) {
            return tl::unexpected<std::string>{ "The 'name' parameter is required" };
        }

        m_Vars["name"] = params.at("name").as<std::string>();

        if (params.contains("define")) {
            auto additionalVars = params.at("define").as<std::vector<std::string>>();

            for (const auto &var : additionalVars) {
                auto match = ctre::match<"(?<name>[a-zA-Z][a-zA-Z0-9_]*)(=(?<value>.*))?">(var);

                if (match) {
                    m_Vars[match.get<"name">().str()] = match.get<"value">().str();
                }
                else {
                    return tl::unexpected<std::string>{ fmt::format("Invalid variable definition '{}'", var) };
                } 
            }
        }

        return processVars();
    }

    tl::expected<void, std::string> generator::processVars() {
        std::map<std::string, std::unordered_set<std::string>> graph;
        
        for (const auto [k, v] : m_Vars) {
            auto match = ctre::match<"[{]{2}([ ]*)?(?<varname>[a-zA-Z][a-zA-Z0-9_]*)([ ]*)?[}]{2}">(v);

            if (match) {
                graph[k].insert(match.get<"varname">().str());
            }
        }

        const auto cycleDetection = [&]() -> bool {
            std::unordered_map<std::string, bool> visited;

            return false;
        };

        const auto topologicalSort = [&]() -> std::list<std::string> {
            std::unordered_map<std::string, bool> visited;
            std::list<std::string> order;

            auto dfs = [&] (const auto &recurse, const std::string &node) -> void {
                visited[node] = true;

                for (const auto &neigh : graph[node]) {
                    if (! visited[neigh]) {
                        recurse(recurse, neigh);
                    }
                }

                order.push_back(node);
            };

            for (const auto &[k, _] : graph) {
                if (! visited[k])
                    dfs(dfs, k);
            }

            return order;
        };

        auto order = topologicalSort();

        for (const auto &process : order) {
            auto match = ctre::match<"[{]{2}([ ]*)?(?<varname>[a-zA-Z][a-zA-Z0-9_]*)([ ]*)?[}]{2}">(m_Vars[process]);

            if (match) {
                m_Vars[process] = m_Vars[match.get<"varname">().str()];
            }
        }

        return {};
    }

    tl::expected<void, std::string> generator::run() const {
        if (m_Template.m_Type == decltype(m_Template)::types::Unknown) {
            return tl::unexpected<std::string>{ "Unexpected template type received" };
        }

        enum class GenerateFileError {
            UnableToOpenTemplate,
            AlreadyExisting,
            UnableToCreate,
            Unknown
        };

        const auto generateFile = [&](fs::path templateFile, fs::path newFile) -> tl::expected<void, GenerateFileError> {
            using vars = variable_substitutor;
 
            std::ifstream templateFileStream;
            std::ofstream newFileStream;

            try {
                templateFileStream.open(templateFile);
            }
            catch(...) {
                return tl::unexpected{ GenerateFileError::UnableToOpenTemplate };
            }

            if (! templateFileStream.is_open()) {
                return tl::unexpected{ GenerateFileError::UnableToOpenTemplate };
            }

            if (fs::exists(newFile)) {
                return tl::unexpected{ GenerateFileError::AlreadyExisting };
            }

            try {
                fs::copy(templateFile, newFile);

                newFileStream.open(newFile);
            }
            catch(...) {
                return tl::unexpected{ GenerateFileError::UnableToCreate };
            }

            std::string line;

            while (std::getline(templateFileStream, line)) {
                newFileStream << vars::run(line, m_Vars) << '\n';
            }

            return {};
        };

        if (m_Template.m_Type == decltype(m_Template)::types::File) {
            using vars = variable_substitutor;

            const auto templateFile = m_Template.m_Location / m_Template.m_TemplateRoot;
            const auto newFile = fs::current_path() / vars::run(m_Template.m_TemplateRoot, m_Vars);
            
            if (! fs::exists(templateFile)) {
                return tl::unexpected<std::string>{ "The template file provided does not exist" };
            }

            if (auto ex = generateFile(templateFile, newFile); ! ex) {
                auto errorCode = ex.error();

                switch (errorCode) {
                    case decltype(errorCode)::AlreadyExisting:
                        return tl::unexpected<std::string>{ fmt::format("The file '{}' already exists", newFile.filename().string()) };
                        break;
                    case decltype(errorCode)::UnableToCreate:
                        return tl::unexpected<std::string>{ fmt::format("Couldn't create the file '{}'", newFile.filename().string()) };
                        break;
                    case decltype(errorCode)::UnableToOpenTemplate:
                        return tl::unexpected<std::string>{ "Couldn't open the template file provided" };
                        break;
                    case decltype(errorCode)::Unknown:
                        return tl::unexpected<std::string>{ "Unknown error ocurred :(" };
                        break;
                }
            }

            return {};
        }
        
        if (m_Template.m_Type == decltype(m_Template)::types::Folder) {
            const auto basePath = m_Template.m_Location;
            const auto baseTemplatePath = basePath / m_Template.m_TemplateRoot;
            const auto baseNewPath = fs::current_path();

            if (! fs::exists(baseTemplatePath)) {
                return tl::unexpected<std::string>{ "The template folder does not exist" };
            }

            if (
                auto baseNewPathS = variable_substitutor::run(baseNewPath / m_Template.m_TemplateRoot, m_Vars); 
                ! baseNewPathS.empty()
               ) {
               
                if (fs::exists(baseNewPathS)) {
                    return tl::unexpected<std::string>{ fmt::format("The folder '{}' already exists", baseNewPathS) };
                }
                
                try {
                    fs::create_directory(baseNewPathS);
                }
                catch(...) {
                    return tl::unexpected<std::string>{ fmt::format("Unable to create folder '{}'", baseNewPathS) };
                }
            }
            else {
                return tl::unexpected<std::string>{ fmt::format("The folder '{}' already exists", baseNewPathS) };
            }

            for (const auto &entry : fs::recursive_directory_iterator(baseTemplatePath)) {
                const auto tPath = entry.path();
                const auto tName = tPath.filename().string();

                const auto newPath = variable_substitutor::run(
                    fmt::format("{}/{}", 
                        tPath.parent_path().string().replace(
                            0, basePath.string().length(), baseNewPath.string()
                        ), 
                        tName
                    ),
                    m_Vars
                );

                if (fs::is_directory(tPath)) {
                    if (fs::exists(newPath)) {
                        fmt::print("The directory '{}' already exists, omitting its creation\n", newPath);
                        continue;
                    }

                    try {
                        fs::create_directory(newPath);
                    }
                    catch(...) {
                        return tl::unexpected<std::string>{ fmt::format("Error, Couldn't create the directory '{}'", newPath) };
                    }
                }
                else if (fs::is_regular_file(tPath)) {
                    if (auto ex = generateFile(tPath, newPath); ! ex) {
                        auto errorCode = ex.error();

                        switch (errorCode) {
                            case decltype(errorCode)::AlreadyExisting:
                                fmt::print("The file '{}' already exists, omitting its creation\n", newPath);
                                continue;
                                break;
                            case decltype(errorCode)::UnableToCreate:
                                // TODO: Maybe clean?
                                return tl::unexpected<std::string>{ fmt::format("Couldn't create the file '{}'", newPath) };
                                break;
                            case decltype(errorCode)::UnableToOpenTemplate:
                                break;
                            case decltype(errorCode)::Unknown:
                                return tl::unexpected<std::string>{ "Unknown error ocurred :(" };
                                break;
                        }
                    }
                }
                else {
                    return tl::unexpected<std::string>{ "Unrecognized or invalid file type provided on template" };
                }
            }
        }

        return {};
    }

}
