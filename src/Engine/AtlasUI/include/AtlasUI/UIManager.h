#pragma once

namespace atlas::ui
{
    class UIPage;
}

namespace atlas::ui::ui_manager
{
    UIPage* pushPage(UIPage* page);

    template<typename TPage, typename ... TArgs>
    UIPage* pushPage(TArgs&&... args)
    {
        return pushPage(new TPage(std::forward<TArgs>(args)...));
    }

    void removePage(UIPage* page);

    void update();
}
