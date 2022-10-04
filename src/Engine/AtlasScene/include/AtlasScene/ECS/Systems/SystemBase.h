#pragma once
#include <string_view>

namespace atlas::scene
{
    class EcsManager;

    class SystemBase
    {
    public:
        virtual ~SystemBase() = default;

        [[nodiscard]] virtual std::string_view GetName() const = 0;

        virtual void Initialise(EcsManager&)
        {
        }

        virtual void Update(EcsManager&)
        {
        }

        virtual void Render(EcsManager&)
        {
        }
    };
}
