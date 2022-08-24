#include "AtlasUIPCH.h"
#include "RmlConfig.h"

#include "AssetRegistry.h"
#include "RmlFileInterface.h"
#include "RmlRawDataAsset.h"
#include "RmlRenderInterface.h"
#include "RmlSystemInterface.h"
#include "RmlUiCss.h"
#include "RmlUiFont.h"
#include "RmlUIPage.h"
#include "AtlasResource/ResourceLoader.h"

#include <RmlUi/Core.h>
#include <RmlUi/Debugger/Debugger.h>

#include "AtlasTrace/Logging.h"

namespace
{
    std::unique_ptr<Rml::FileInterface> g_fileInterface;
    std::unique_ptr<Rml::SystemInterface> g_systemInterface;
    std::unique_ptr<Rml::RenderInterface> g_renderInterface;

    atlas::resource::AssetPtr<atlas::ui::rml::asset_types::RmlUiFont> g_mainFont;
    atlas::resource::AssetPtr<atlas::ui::rml::asset_types::RmlUiFont> g_backupFont;

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiPageLoader(const atlas::resource::FileData& data)
    {
        std::string page = reinterpret_cast<const char*>(data.m_pData.get());
        return std::make_shared<atlas::ui::rml::asset_types::RmlUiPage>(data.m_FilePath, page);
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiCssLoader(const atlas::resource::FileData& data)
    {
        std::string page = reinterpret_cast<const char*>(data.m_pData.get());
        return std::make_shared<atlas::ui::rml::asset_types::RmlUiCss>(data.m_FilePath, page);
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rmlUiFontLoader(atlas::resource::FileData& data)
    {
        return std::make_shared<atlas::ui::rml::asset_types::RmlUiFont>(data.m_FilePath, std::move(data.m_pData), data.m_Size);
    }

    atlas::resource::AssetPtr<atlas::resource::ResourceAsset> rawRmlDataLoader(atlas::resource::FileData& data)
    {
        return std::make_shared<atlas::ui::rml::asset_types::RmlRawDataAsset>(data.m_FilePath, std::move(data.m_pData), data.m_Size);
    }
}

bool atlas::ui::rml::config::initialiseRmlUi(bgfx::ViewId uiViewId)
{
    resource::ResourceLoader::RegisterBundle<resources::registry::CoreBundle>();

    resource::ResourceLoader::RegisterTypeHandler<asset_types::RmlRawDataAsset>(rawRmlDataLoader);
    resource::ResourceLoader::RegisterTypeHandler<asset_types::RmlUiPage>(rmlUiPageLoader);
    resource::ResourceLoader::RegisterTypeHandler<asset_types::RmlUiCss>(rmlUiCssLoader);
    resource::ResourceLoader::RegisterTypeHandler<asset_types::RmlUiFont>(rmlUiFontLoader);

    g_fileInterface = std::make_unique<RmlFileInterface>();
    g_systemInterface = std::make_unique<RmlSystemInterface>();
    g_renderInterface = std::make_unique<RmlRenderInterface>(uiViewId);

    SetFileInterface(g_fileInterface.get());
    SetSystemInterface(g_systemInterface.get());
    SetRenderInterface(g_renderInterface.get());

    Rml::Initialise();

    Rml::Debugger::SetVisible(true);

    g_mainFont = resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, asset_types::RmlUiFont>(
        resources::registry::core_bundle::fonts::c_LatoLatin_Regular);
    g_backupFont = resource::ResourceLoader::LoadAsset<resources::registry::CoreBundle, asset_types::RmlUiFont>(
        resources::registry::core_bundle::fonts::c_NotoEmoji_Regular);

    if (!g_mainFont || !Rml::LoadFontFace(
        g_mainFont->GetData(),
        static_cast<int>(g_mainFont->GetDataSize()),
        "LatoLatin",
        Rml::Style::FontStyle::Normal,
        Rml::Style::FontWeight::Normal))
    {
        AT_ERROR(AtlasUI, "Failed to load main font. AssetBundleId: {}:{}",
            resources::registry::CoreBundle::GetStringId(),
            resources::registry::core_bundle::fonts::c_LatoLatin_Regular.m_Value);
        return false;
    }

    if (!g_backupFont || !Rml::LoadFontFace(
       g_backupFont->GetData(),
       static_cast<int>(g_backupFont->GetDataSize()),
       "NotoEmoji",
       Rml::Style::FontStyle::Normal,
       Rml::Style::FontWeight::Normal))
    {
        AT_ERROR(AtlasUI, "Failed to load backup font. AssetBundleId: {}:{}",
            resources::registry::CoreBundle::GetStringId(),
            resources::registry::core_bundle::fonts::c_NotoEmoji_Regular.m_Value);
        return false;
    }

    AT_INFO(AtlasUI, "RmlUi initialised");
    return true;
}
