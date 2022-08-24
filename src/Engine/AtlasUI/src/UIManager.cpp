#include "AtlasUIPCH.h"
#include "UIManager.h"

#include "UIPage.h"

namespace
{
    std::vector<std::unique_ptr<atlas::ui::UIPage>> g_pages;
}

atlas::ui::UIPage* atlas::ui::ui_manager::pushPage(UIPage* page)
{
    page->Initialise();
    g_pages.push_back(std::unique_ptr<UIPage>(page));
    return page;
}

void atlas::ui::ui_manager::removePage(UIPage* page)
{
    auto [begin, end] =
        std::ranges::remove_if(
            g_pages,
            [page](const std::unique_ptr<UIPage>& p) { return p.get() == page; });
    g_pages.erase(begin, end);
}

void atlas::ui::ui_manager::update()
{
    for(const auto& page : g_pages)
    {
        page->Update();
    }
}
