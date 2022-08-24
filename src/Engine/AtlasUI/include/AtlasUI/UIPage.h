#pragma once

namespace atlas::ui
{
    class UIPage
    {
    public:
        UIPage(UIPage&) = delete;
        UIPage(UIPage&&) = delete;
        UIPage& operator=(UIPage&) = delete;
        UIPage& operator=(UIPage&&) = delete;

        UIPage() = default;
        virtual ~UIPage() = default;

    public:
        virtual void Initialise() = 0;
        virtual void Update() = 0;
    };
}
