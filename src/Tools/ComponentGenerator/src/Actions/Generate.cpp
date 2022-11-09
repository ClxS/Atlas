#include "Generate.h"
#include "Arguments.h"

#include <filesystem>
#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <ranges>

#include "tinyxml2.h"
#include "AtlasTrace/Logging.h"

namespace
{
    namespace fs = std::filesystem;

    struct DefinedType
    {
        std::string m_Tag;
        std::string m_FullName;
        std::vector<std::string> m_Dependencies;
    };

    const std::vector<std::string> c_metadataTags
    {
        "ComponentVisibility"
    };

    // TODO: Add support for argument matching
    // TODO: Add support for EditorOnly + other modes
    const std::vector<DefinedType> c_knownTypes
    {
        {"Matrix4f", "Eigen::Matrix4f", { "Eigen/Core" }},
        {"Vector2f", "Eigen::Vector2f", { "Eigen/Core" }},
        {"Vector3f", "Eigen::Vector3f", { "Eigen/Core" }},
        {"Quaternionf", "Eigen::Quaternionf", { "Eigen/Geometry" }},
        {"Angle", "atlas::maths_helpers::Angle", { "AtlasCore/MathsHelpers.h" }},
        {"Colour32", "atlas::core::Colour32", { "AtlasCore/Colour.h" }},
        {"EntityId", "atlas::scene::EntityId", { "AtlasScene/ECS/Entity.h" }},
        {"int", "int", {  }},
        {"float", "float", {  }},
        {"double", "double", {  }},
        {"bool", "bool", {  }},
        {"byte", "uint8_t", { "cstdint" }},
        {"FixedString<32>", "std::array<char, 32>", { "array" }},
        {"ModelAsset", "atlas::resource::AssetPtr<atlas::render::ModelAsset>", { "AtlasResource/AssetPtr.h", "AtlasRender/AssetTypes/ModelAsset.h" }},
    };

    struct ComponentMetadataEntry
    {
        std::string m_Key;
        std::string m_Value;
    };

    struct ComponentField
    {
        std::string m_Name;
        std::string m_Tag;
        std::string m_Type;
        std::optional<std::string> m_DefaultValue;
    };

    struct Component
    {
        std::string m_Name;
        std::optional<std::string> m_Inherits;
        std::optional<std::vector<std::string>> m_RequiredComponents;
        std::optional<std::string> m_SubNamespace;

        std::vector<ComponentMetadataEntry> m_Metadata;
        std::vector<ComponentField> m_Fields;
        std::set<fs::path> m_Dependencies;
    };

    std::vector<std::filesystem::path> locateFiles(const std::filesystem::path& root)
    {
        std::vector<fs::path> outputFiles{};
        std::queue<std::filesystem::path> pathsToCheck{};
        pathsToCheck.emplace(root);

        while (!pathsToCheck.empty())
        {
            fs::path currentPath = pathsToCheck.front();
            pathsToCheck.pop();

            for (const auto& entry : fs::directory_iterator(currentPath))
            {
                if (entry.is_directory())
                {
                    pathsToCheck.emplace(entry);
                    continue;
                }

                fs::path path = entry;

                const bool isXml = path.has_extension() && path.extension() == ".xml";
                if (isXml)
                {
                    outputFiles.emplace_back(path);
                }
            }
        }

        return outputFiles;
    }

    std::optional<Component> readComponent(tinyxml2::XMLElement* xmlNode)
    {
        if (!xmlNode)
        {
            return {};
        }

        Component component{};
        component.m_Name = xmlNode->Value();

        const auto requiresAttribute = xmlNode->FindAttribute("Requires");
        if (requiresAttribute)
        {
            component.m_RequiredComponents = std::vector<std::string>{};

            std::string strValue = std::string(requiresAttribute->Value());
            for(const auto& import : std::views::split(strValue, ';'))
            {
                component.m_RequiredComponents->emplace_back(import);
            }
        }

        const auto inheritsComponents = xmlNode->FindAttribute("Inherits");
        if (inheritsComponents)
        {
            component.m_Inherits = inheritsComponents->Value();
        }

        for(auto& metadataTag : c_metadataTags)
        {
            const auto attribute = xmlNode->FindAttribute(metadataTag.c_str());
            if (!attribute)
            {
                continue;
            }

            component.m_Metadata.emplace_back(metadataTag, std::string{attribute->Value()});
        }

        tinyxml2::XMLElement* element = xmlNode->FirstChildElement();
        while (element)
        {
            ComponentField field{};
            field.m_Name = element->Value();
            field.m_Tag = element->Attribute("Type");

            const auto defaultAttribute = element->FindAttribute("Default");
            if (defaultAttribute)
            {
                field.m_DefaultValue = defaultAttribute->Value();
            }

            component.m_Fields.emplace_back(field);
            element = element->NextSiblingElement();
        }

        return component;
    }

    std::vector<Component> collectComponents(const fs::path& file)
    {
        tinyxml2::XMLDocument doc;
        auto status = doc.LoadFile(file.string().c_str());
        if (status != tinyxml2::XML_SUCCESS)
        {
            AT_ERROR(ComponentGenerator, "Failed to load file: {}. Error: {}, {}, {}, Line: {}",
                     file.string(),
                     static_cast<int>(status),
                     doc.ErrorName(),
                     doc.ErrorStr(),
                     doc.ErrorLineNum());
            return {};
        }

        const auto root = doc.RootElement();
        if (!root && std::strcmp(root->Name(), "Components") != 0)
        {
            AT_ERROR(ComponentGenerator, "Failed to load file: {}. Error: {}",
                     file.string(),
                     "Expected a root element named 'Components'");
            return {};
        }

        std::optional<std::string> subNamespace = {};
        const auto subNamespaceAttribute = root->FindAttribute("SubNamespace");
        if (subNamespaceAttribute)
        {
            subNamespace = subNamespaceAttribute->Value();
        }

        std::vector<Component> outComponents;
        auto componentNode = root->FirstChildElement();
        while (componentNode != nullptr)
        {
            auto component = readComponent(componentNode);
            if (!component.has_value())
            {
                AT_ERROR(ComponentGenerator, "Failed to read component node");
            }
            else
            {
                component->m_SubNamespace = subNamespace;
                outComponents.emplace_back(component.value());
            }

            componentNode = componentNode->NextSiblingElement();
        }

        return outComponents;
    }

    std::string getComponentFinalName(const Component& component, const Arguments& args, bool includeNamespace)
    {
        std::string componentNamespace =
            includeNamespace
            ? std::format("{}{}::",
                    args.m_Namespace.m_Value,
                    component.m_SubNamespace.has_value()
                        ? std::format("::{}", component.m_SubNamespace.value())
                        : "")
            : "";

        return std::format("{}{}Component",
            componentNamespace,
            component.m_Name);
    }

    std::string getComponentFinalName(const std::string_view& componentName, const std::vector<Component>& components, const Arguments& args, bool includeNamespace)
    {
        const auto result = std::find_if(components.begin(), components.end(), [&](const Component& component)
        {
            return component.m_Name == componentName;
        });

        if (result == components.end())
        {
            return "UNKNOWN";
        }

        return getComponentFinalName(*result, args, includeNamespace);
    }

    fs::path getComponentRelativeOutputPath(const Component& component, const Arguments& args)
    {
        return fs::path(args.m_ProjectName.m_Value) / "Components"/ (getComponentFinalName(component, args, false) + ".h");
    }

    void collectDependencies(
        Component& component,
        const std::vector<Component>& allComponents,
        const Arguments& args)
    {
        for(auto& field : component.m_Fields)
        {
            auto item = std::ranges::find_if(c_knownTypes, [field](const DefinedType& type)
            {
                return type.m_Tag == field.m_Tag;
            });

            if (item != c_knownTypes.end())
            {
                field.m_Type = item->m_FullName;
                for(const auto& dependency : item->m_Dependencies)
                {
                    component.m_Dependencies.emplace(dependency);
                }
            }
            else
            {
                AT_ERROR(ComponentGenerator, "Unknown type {} on Component {}", field.m_Type, component.m_Name);
                field.m_Type = field.m_Tag;
            }
        }

        if (component.m_Inherits.has_value())
        {
            const auto item = std::ranges::find_if(allComponents, [component](const Component& type)
            {
                return type.m_Name == component.m_Inherits.value();
            });

            if (item != allComponents.end())
            {
                component.m_Dependencies.emplace(getComponentRelativeOutputPath(*item, args));
            }
            else
            {
                AT_ERROR(ComponentGenerator, "Unknown base type {} on Component {}", component.m_Inherits.value(), component.m_Name);
            }
        }
    }

    void writeComponent(const Component& component, const std::vector<Component>& components, const Arguments& args)
    {
        std::stringstream outFile;
        outFile << "// THIS FILE IS AUTOGENERATED AND SHOULD NOT BE DIRECTLY MODIFIED.\n\n";
        outFile << "#pragma once\n";
        outFile << "\n";
        for(const auto& dep : component.m_Dependencies)
        {
            auto depStr = dep.string();
            std::ranges::replace(depStr, '\\', '/');
            outFile << std::format("#include \"{}\"\n", depStr);
        }
        outFile << "\n";
        outFile <<
            std::format("namespace {}{}\n",
                args.m_Namespace.m_Value,
                component.m_SubNamespace.has_value()
                    ? std::format("::{}", component.m_SubNamespace.value())
                    : "");
        outFile << "{\n";
        outFile << std::format("\tstruct {}{}\n",
            getComponentFinalName(component, args, false),
            component.m_Inherits.has_value() ? std::format(" : public {}", getComponentFinalName(component.m_Inherits.value(), components, args, true)) : "");
        outFile << "\t{\n";

        for(const auto& field : component.m_Fields)
        {
            outFile << std::format("\t\t{} m_{}{{{}}};\n",
                field.m_Type,
                field.m_Name,
                field.m_DefaultValue.value_or(""));
        }

        outFile << "\t};\n";
        outFile << "}\n";

        const auto outputPath = args.m_ComponentRoot.m_Value / "generated" / getComponentRelativeOutputPath(component, args);
        fs::create_directories(outputPath.parent_path());

        std::ofstream out(outputPath);
        out << outFile.str();
        out.close();
    }

    void writeRegistrationScript(const std::vector<Component>& components, const Arguments& args)
    {
        // Write Header
        {
            std::stringstream outFile;
            outFile << "// THIS FILE IS AUTOGENERATED AND SHOULD NOT BE DIRECTLY MODIFIED.\n\n";
            outFile << "#pragma once\n\n";
            outFile << std::format("namespace {}\n", args.m_Namespace.m_Value);
            outFile << "{\n";
            outFile << "\tvoid registerComponents();\n";
            outFile << "}\n";

            const auto outputPath = args.m_ComponentRoot.m_Value / "generated" / "ComponentRegistration.h";
            fs::create_directories(outputPath.parent_path());
            std::ofstream out(outputPath);
            out << outFile.str();
            out.close();
        }

        // Write Source
        {
            std::stringstream outFile;
            outFile << "// THIS FILE IS AUTOGENERATED AND SHOULD NOT BE DIRECTLY MODIFIED.\n\n";

            outFile << std::format("#include \"{}PCH.h\"\n", args.m_ProjectName.m_Value);
            outFile << std::format("#include \"ComponentRegistration.h\"\n");
            outFile << std::format("#include \"AtlasScene/ECS/Components/ComponentRegistry.h\"\n");

            for(const auto& component : components)
            {
                outFile << std::format("#include \"{}.h\"\n", getComponentFinalName(component, args, false));
            }
            outFile << "\n";
            outFile << std::format("void {}::registerComponents()\n", args.m_Namespace.m_Value);
            outFile << "{\n";
            for(const auto& component : components)
            {
                std::string fullComponentName = getComponentFinalName(component, args, true);

                std::stringstream metadata;
                for(size_t metadataIdx = 0; metadataIdx < component.m_Metadata.size(); metadataIdx++)
                {
                    if (metadataIdx > 0)
                    {
                        metadata << ", ";
                    }

                    auto& metadataEntry = component.m_Metadata[metadataIdx];
                    metadata << std::format(R"({{ "{}", "{}" }})", metadataEntry.m_Key, metadataEntry.m_Value);
                }

                std::stringstream requiredComponents;
                if (component.m_RequiredComponents.has_value())
                {
                    for(size_t requirementIdx = 0; requirementIdx < component.m_RequiredComponents->size(); requirementIdx++)
                    {
                        if (requirementIdx > 0)
                        {
                            requiredComponents << ", ";
                        }

                        requiredComponents << std::format(
                            "atlas::scene::ComponentRegistry::GetComponentId<{}>()",
                            getComponentFinalName(component.m_RequiredComponents->at(requirementIdx), components, args, true));
                    }
                }

                std::stringstream fieldRegistrations;
                int fieldCount = static_cast<int>(component.m_Fields.size());
                for(int fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
                {
                    if (fieldIndex > 0)
                    {
                        fieldRegistrations << ",\n";
                    }

                    fieldRegistrations << std::format(
                        "\t\t\t{{ \"{}\", \"{}\", {}, {}, atlas::scene::ComponentFieldInfoId {{ {} }} }}",
                        component.m_Fields[fieldIndex].m_Name,
                        component.m_Fields[fieldIndex].m_Tag,
                        std::format("offsetof({}, m_{})", fullComponentName, component.m_Fields[fieldIndex].m_Name),
                        std::format("sizeof({})", component.m_Fields[fieldIndex].m_Type),
                        fieldIndex);
                }

                std::stringstream componentRegistration;
                componentRegistration << std::format(
                    "{{\n\t\t{},\n\t\t\"{}\",\n\t\t{{ {} }},\n\t\t{{ {} }},\n\t\t{{\n{} }} }}",
                    std::format(
                        "atlas::scene::ComponentRegistry::GetComponentId<{}>()",
                        fullComponentName),
                    component.m_Name,
                    metadata.str(),
                    requiredComponents.str(),
                    fieldRegistrations.str());

                outFile << std::format(
                    "\tatlas::scene::ComponentRegistry::RegisterComponent<{}>({});\n",
                    fullComponentName,
                    componentRegistration.str());
            }

            outFile << "}\n";

            const auto outputPath = args.m_ComponentRoot.m_Value / "generated" / "ComponentRegistration.cpp";
            fs::create_directories(outputPath.parent_path());
            std::ofstream out(outputPath);
            out << outFile.str();
            out.close();
        }
    }
}

ExitCode component_generator::actions::generate(const Arguments& args)
{
    std::vector<Component> components{};
    for (const fs::path& path : locateFiles(args.m_ComponentRoot.m_Value))
    {
        AT_INFO(ComponentGenerator, "Processing {}", path.string());
        auto fileComponents = collectComponents(path);
        components.insert(components.end(), fileComponents.begin(), fileComponents.end());
    }

    for(auto& component : components)
    {
        AT_INFO(ComponentGenerator, "Processing component: {}", component.m_Name);
        collectDependencies(component, components, args);
        writeComponent(component, components, args);
    }

    writeRegistrationScript(components, args);

    return ExitCode::Success;
}
