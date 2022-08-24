#pragma once
#include <string_view>

namespace atlas::scene
{
    class EcsManager;

    class SystemBase
    {
    public:
        virtual ~SystemBase() = default;

        virtual std::string_view GetName() const = 0;

        virtual void Initialise(EcsManager&)
        {
        }

        virtual void Update(EcsManager&) = 0;

        virtual void Render(EcsManager&)
        {
        }
    };
}
