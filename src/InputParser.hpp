#pragma once

#include "Dungeon.hpp"

#include <optional>
#include <string>

namespace Terracraft
{
    struct SimulationSettings
    {
        int m_initialFood = 0;
        ResourceType m_targetResource = ResourceType::Iron;
    };

    struct ParsedInput
    {
        Dungeon m_dungeon;
        SimulationSettings m_settings;
    };

    struct ParseResult
    {
        std::optional<ParsedInput> m_input;
        std::string m_invalidLine;
    };

    class InputParser
    {
    public:
        ParseResult ParseFile(const std::string& inputFilePath) const;

    private:
        static bool TryParseInteger(const std::string& text, int& value);
        static bool TryParseResourceType(const std::string& text, ResourceType& resourceType);
        static bool IsResourceAmountValid(int amount);
        static std::vector<std::string> SplitBySpaces(const std::string& line);
        static std::vector<std::string> SplitByComma(const std::string& text);
    };
}
