#pragma once

#include <array>
#include <string>

namespace Terracraft
{
    enum class ResourceType
    {
        Iron = 0,
        Gold = 1,
        Gems = 2,
        Exp = 3
    };

    constexpr int ResourceCount = 4;

    using ResourceArray = std::array<int, ResourceCount>;
    using ResourceCollectedArray = std::array<bool, ResourceCount>;

    inline const std::array<std::string, ResourceCount>& GetResourceNames()
    {
        static const std::array<std::string, ResourceCount> s_resourceNames =
        {
            "iron",
            "gold",
            "gems",
            "exp"
        };

        return s_resourceNames;
    }

    inline ResourceArray GetBaseResourceValues()
    {
        return { 7, 11, 23, 1 };
    }

    inline int ToIndex(ResourceType resourceType)
    {
        return static_cast<int>(resourceType);
    }
}
