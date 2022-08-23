#include "Cook.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <variant>
#include <Windows.h>

#include "Arguments.h"
#include "Asset.h"
#include "AssetHandler.h"
#include "AssetTree.h"
#include "ShaderAssetHandler.h"
#include "AtlasTrace/Logging.h"


void cookAssets(const std::vector<std::string>& types, const AssetTree::TreeNode& node, const std::filesystem::path& dataRoot, const std::filesystem::path& outputRoot)
{
    for(auto& group : node.m_ChildNodes)
    {
        cookAssets(types, *group, dataRoot, outputRoot);
    }

    for(const auto& asset : node.m_Assets)
    {
        assert(asset.m_pAssociatedHandler);

        if (!types.empty())
        {
            if (std::ranges::find(types, asset.m_Type) == types.end())
            {
                continue;
            }
        }

        std::filesystem::path outputPath = outputRoot / asset.m_pAssociatedHandler->GetAssetRelativeOutputPath(asset);
        if (!exists(outputPath.parent_path()) && !create_directories(outputPath.parent_path()))
        {
            AT_ERROR(Cook, "Failed to create directory {}", outputPath.parent_path().string());
            continue;
        }

        auto assets = asset.m_pAssociatedHandler->Cook(asset);
        if (std::holds_alternative<ErrorString>(assets))
        {
            AT_ERROR(Cook, "Failed to build asset {}\n{}", asset.m_SourceAssetPath.string(), std::get<1>(assets));
            continue;
        }

        for(auto& writableAsset : std::get<0>(assets))
        {
            std::ofstream file;
            auto outputFile = outputRoot / writableAsset.m_OutputPath;
            file.open(outputFile, std::ios::binary | std::ios::out);
            if (file.fail())
            {
                AT_ERROR(Cook, "Unable to open path for writing {}. {}", writableAsset.m_OutputPath.string(), GetLastError());
                continue;
            }

            file.write(reinterpret_cast<const char*>(writableAsset.m_pData.get()), writableAsset.m_DataSize);
        }

    }
}
ExitCode asset_builder::actions::cook(const Arguments& args)
{
    if (!args.m_OutputFile.m_bIsSet)
    {
        return ExitCode::MissingRequiredArgument;
    }

    namespace fs = std::filesystem;

    if (!args.m_ShaderIncludes.m_Value.empty())
    {
        ShaderAssetHandler::SetCommonIncludes(args.m_ShaderIncludes.m_Value);
    }

    const fs::path dataRoot = args.m_DataRoot.m_Value;
    const fs::path outputRoot = args.m_OutputFile.m_Value;
    const AssetTree tree = AssetTree::CreateFromFileStructure(args.m_DataRoot.m_Value, args.m_Platform.m_Value);
    for(auto& group : tree.GetRoot().m_ChildNodes)
    {
        cookAssets(args.m_Types.m_Value, *group, dataRoot, outputRoot);
    }

    return ExitCode::Success;
}
