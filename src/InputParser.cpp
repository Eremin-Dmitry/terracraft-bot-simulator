#include "InputParser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace Terracraft
{
    namespace
    {
        constexpr int s_minRoomCount = 1;
        constexpr int s_maxRoomCount = 255;
        constexpr int s_minResourceAmount = 0;
        constexpr int s_maxResourceAmount = 255;
        constexpr int s_minFoodAmount = 2;
        constexpr int s_maxFoodAmount = 255;
    }

    ParseResult InputParser::ParseFile(const std::string& inputFilePath) const
    {
        std::ifstream inputFile(inputFilePath);
        if (!inputFile.is_open())
        {
            return { std::nullopt, inputFilePath };
        }

        std::vector<std::string> lines;
        std::string currentLine;
        while (std::getline(inputFile, currentLine))
        {
            if (!currentLine.empty() && currentLine.back() == '\r')
            {
                currentLine.pop_back();
            }

            if (!currentLine.empty())
            {
                lines.push_back(currentLine);
            }
        }

        if (lines.empty())
        {
            return { std::nullopt, "" };
        }

        int lastRoomNumber = 0;
        if (!TryParseInteger(lines[0], lastRoomNumber) ||
            lastRoomNumber < s_minRoomCount ||
            lastRoomNumber > s_maxRoomCount)
        {
            return { std::nullopt, lines[0] };
        }

        const std::size_t expectedLineCount = static_cast<std::size_t>(lastRoomNumber + 3);
        if (lines.size() != expectedLineCount)
        {
            return { std::nullopt, lines.size() > 1 ? lines.back() : lines[0] };
        }

        Dungeon dungeon(lastRoomNumber);
        std::unordered_set<int> describedRooms;

        for (int lineIndex = 1; lineIndex <= lastRoomNumber + 1; ++lineIndex)
        {
            const std::string& line = lines[static_cast<std::size_t>(lineIndex)];
            const std::vector<std::string> tokens = SplitBySpaces(line);

            if (tokens.size() != 6)
            {
                return { std::nullopt, line };
            }

            int roomNumber = 0;
            if (!TryParseInteger(tokens[0], roomNumber) || !dungeon.HasRoom(roomNumber))
            {
                return { std::nullopt, line };
            }

            if (describedRooms.count(roomNumber) != 0)
            {
                return { std::nullopt, line };
            }
            describedRooms.insert(roomNumber);

            Room& room = dungeon.GetRoom(roomNumber);
            room.m_adjacentRooms.clear();

            const std::vector<std::string> adjacentRoomTokens = SplitByComma(tokens[1]);
            if (adjacentRoomTokens.empty())
            {
                return { std::nullopt, line };
            }

            for (const std::string& adjacentRoomText : adjacentRoomTokens)
            {
                int adjacentRoomNumber = 0;
                if (!TryParseInteger(adjacentRoomText, adjacentRoomNumber) ||
                    !dungeon.HasRoom(adjacentRoomNumber) ||
                    adjacentRoomNumber == roomNumber)
                {
                    return { std::nullopt, line };
                }

                room.m_adjacentRooms.insert(adjacentRoomNumber);
            }

            for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
            {
                int resourceAmount = 0;
                if (!TryParseInteger(tokens[static_cast<std::size_t>(resourceIndex + 2)], resourceAmount) ||
                    !IsResourceAmountValid(resourceAmount))
                {
                    return { std::nullopt, line };
                }

                room.m_resources[static_cast<std::size_t>(resourceIndex)] = resourceAmount;
            }
        }

        for (const Room& room : dungeon.GetRooms())
        {
            for (int adjacentRoomNumber : room.m_adjacentRooms)
            {
                const Room& adjacentRoom = dungeon.GetRoom(adjacentRoomNumber);
                if (adjacentRoom.m_adjacentRooms.count(room.m_roomNumber) == 0)
                {
                    return { std::nullopt, lines[static_cast<std::size_t>(room.m_roomNumber + 1)] };
                }
            }
        }

        const std::string& settingsLine = lines.back();
        const std::vector<std::string> settingsTokens = SplitBySpaces(settingsLine);
        if (settingsTokens.size() != 2)
        {
            return { std::nullopt, settingsLine };
        }

        SimulationSettings settings;
        if (!TryParseInteger(settingsTokens[0], settings.m_initialFood) ||
            settings.m_initialFood < s_minFoodAmount ||
            settings.m_initialFood > s_maxFoodAmount ||
            !TryParseResourceType(settingsTokens[1], settings.m_targetResource))
        {
            return { std::nullopt, settingsLine };
        }

        return { ParsedInput{ std::move(dungeon), settings }, "" };
    }

    bool InputParser::TryParseInteger(const std::string& text, int& value)
    {
        if (text.empty())
        {
            return false;
        }

        int parsedValue = 0;
        for (char symbol : text)
        {
            if (!std::isdigit(static_cast<unsigned char>(symbol)))
            {
                return false;
            }

            parsedValue = parsedValue * 10 + (symbol - '0');
            if (parsedValue > 100000)
            {
                return false;
            }
        }

        value = parsedValue;
        return true;
    }

    bool InputParser::TryParseResourceType(const std::string& text, ResourceType& resourceType)
    {
        const auto& names = GetResourceNames();
        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            if (text == names[static_cast<std::size_t>(resourceIndex)])
            {
                resourceType = static_cast<ResourceType>(resourceIndex);
                return true;
            }
        }

        return false;
    }

    bool InputParser::IsResourceAmountValid(int amount)
    {
        return amount >= s_minResourceAmount && amount <= s_maxResourceAmount;
    }

    std::vector<std::string> InputParser::SplitBySpaces(const std::string& line)
    {
        std::istringstream lineStream(line);
        std::vector<std::string> tokens;
        std::string token;

        while (lineStream >> token)
        {
            tokens.push_back(token);
        }

        return tokens;
    }

    std::vector<std::string> InputParser::SplitByComma(const std::string& text)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream textStream(text);

        while (std::getline(textStream, token, ','))
        {
            if (token.empty())
            {
                return {};
            }

            tokens.push_back(token);
        }

        return tokens;
    }
}
