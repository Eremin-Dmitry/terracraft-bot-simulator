#include "AliceBot.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <queue>
#include <sstream>

namespace Terracraft
{
    namespace
    {
        constexpr int s_startRoomNumber = 0;
    }

    AliceBot::AliceBot(Dungeon dungeon, SimulationSettings settings)
        : m_dungeon(std::move(dungeon))
        , m_settings(settings)
        , m_remainingFood(settings.initialFood)
    {
    }

    std::vector<std::string> AliceBot::RunSimulation()
    {
        VisitRoom(s_startRoomNumber);

        const int explorationFoodLimit = m_settings.initialFood / 2;
        int spentFoodForExploration = 0;

        while (spentFoodForExploration < explorationFoodLimit && HasUnvisitedRooms())
        {
            const int nextRoomNumber = FindNextExplorationRoom();
            if (nextRoomNumber < 0)
            {
                break;
            }

            std::vector<int> path = BuildPathToRoom(nextRoomNumber, true);
            if (path.empty() || static_cast<int>(path.size()) > explorationFoodLimit - spentFoodForExploration)
            {
                break;
            }

            for (int roomNumber : path)
            {
                MoveToRoom(roomNumber, false);
                ++spentFoodForExploration;
                TryCollectBestResourceInCurrentRoom();
            }
        }

        std::vector<int> returnPath = BuildReturnPathToStart();
        for (std::size_t stepIndex = 0; stepIndex < returnPath.size(); ++stepIndex)
        {
            const bool isFinalReturnMove = returnPath[stepIndex] == s_startRoomNumber;
            MoveToRoom(returnPath[stepIndex], isFinalReturnMove);

            if (!isFinalReturnMove)
            {
                SpendExtraFoodOnCurrentRoom();
            }
        }

        WriteResult();
        return m_actions;
    }

    void AliceBot::VisitRoom(int roomNumber)
    {
        m_currentRoomNumber = roomNumber;
        m_dungeon.GetRoom(roomNumber).wasVisited = true;
    }

    void AliceBot::MoveToRoom(int roomNumber, bool isFinalReturnMove)
    {
        --m_remainingFood;
        VisitRoom(roomNumber);

        std::ostringstream actionStream;
        actionStream << "go " << roomNumber;
        m_actions.push_back(actionStream.str());

        if (!isFinalReturnMove)
        {
            WriteCurrentRoomState();
        }
    }

    void AliceBot::TryCollectBestResourceInCurrentRoom()
    {
        Room& room = m_dungeon.GetRoom(m_currentRoomNumber);
        if (!RoomHasAvailableResource(room))
        {
            return;
        }

        const ResourceType bestResource = FindBestAvailableResource(room);
        CollectResource(bestResource);
    }

    void AliceBot::SpendExtraFoodOnCurrentRoom()
    {
        Room& room = m_dungeon.GetRoom(m_currentRoomNumber);

        while (m_remainingFood > GetReturnDistanceToStart() && RoomHasAvailableResource(room))
        {
            std::vector<ResourceType> availableResources = GetAvailableResourcesByValue(room);
            if (availableResources.empty())
            {
                return;
            }

            CollectResource(availableResources.front());
        }
    }

    void AliceBot::CollectResource(ResourceType resourceType)
    {
        Room& room = m_dungeon.GetRoom(m_currentRoomNumber);
        const int resourceIndex = ToIndex(resourceType);

        if (!room.hasFreeCollection)
        {
            --m_remainingFood;
        }

        room.hasFreeCollection = false;
        room.collectedResources[static_cast<std::size_t>(resourceIndex)] = true;
        m_collectedResources[static_cast<std::size_t>(resourceIndex)] += room.resources[static_cast<std::size_t>(resourceIndex)];

        std::ostringstream actionStream;
        actionStream << "collect " << GetResourceNames()[static_cast<std::size_t>(resourceIndex)];
        m_actions.push_back(actionStream.str());

        WriteCurrentRoomState();
    }

    void AliceBot::WriteCurrentRoomState()
    {
        const Room& room = m_dungeon.GetRoom(m_currentRoomNumber);
        std::ostringstream stateStream;
        stateStream << "state " << room.roomNumber;

        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            stateStream << ' ';
            if (room.collectedResources[static_cast<std::size_t>(resourceIndex)])
            {
                stateStream << '_';
            }
            else
            {
                stateStream << room.resources[static_cast<std::size_t>(resourceIndex)];
            }
        }

        m_actions.push_back(stateStream.str());
    }

    void AliceBot::WriteResult()
    {
        ResourceArray resourceValues = GetBaseResourceValues();
        resourceValues[static_cast<std::size_t>(ToIndex(m_settings.targetResource))] *= 2;

        int totalValue = 0;
        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            totalValue += m_collectedResources[static_cast<std::size_t>(resourceIndex)] *
                resourceValues[static_cast<std::size_t>(resourceIndex)];
        }

        std::ostringstream resultStream;
        resultStream << "result";
        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            resultStream << ' ' << m_collectedResources[static_cast<std::size_t>(resourceIndex)];
        }
        resultStream << ' ' << totalValue;

        m_actions.push_back(resultStream.str());
    }

    bool AliceBot::HasUnvisitedRooms() const
    {
        bool hasUnvisitedRoom = false;
        for (const Room& room : m_dungeon.GetRooms())
        {
            if (!room.wasVisited)
            {
                hasUnvisitedRoom = true;
                break;
            }
        }

        return hasUnvisitedRoom;
    }

    int AliceBot::FindNextExplorationRoom() const
    {
        const Room& currentRoom = m_dungeon.GetRoom(m_currentRoomNumber);
        for (int adjacentRoomNumber : currentRoom.adjacentRooms)
        {
            if (!m_dungeon.GetRoom(adjacentRoomNumber).wasVisited)
            {
                return adjacentRoomNumber;
            }
        }

        std::vector<int> path = BuildPathToNearestUnvisitedRoom();
        if (path.empty())
        {
            return -1;
        }

        return path.back();
    }

    std::vector<int> AliceBot::BuildPathToNearestUnvisitedRoom() const
    {
        std::queue<int> roomQueue;
        std::vector<int> previousRoom(static_cast<std::size_t>(m_dungeon.GetLastRoomNumber() + 1), -1);
        std::vector<bool> wasSeen(static_cast<std::size_t>(m_dungeon.GetLastRoomNumber() + 1), false);

        roomQueue.push(m_currentRoomNumber);
        wasSeen[static_cast<std::size_t>(m_currentRoomNumber)] = true;

        int bestTargetRoom = -1;
        while (!roomQueue.empty() && bestTargetRoom < 0)
        {
            const std::size_t levelSize = roomQueue.size();
            std::vector<int> candidatesOnLevel;

            for (std::size_t index = 0; index < levelSize; ++index)
            {
                const int roomNumber = roomQueue.front();
                roomQueue.pop();

                const Room& room = m_dungeon.GetRoom(roomNumber);
                for (int adjacentRoomNumber : room.adjacentRooms)
                {
                    if (wasSeen[static_cast<std::size_t>(adjacentRoomNumber)])
                    {
                        continue;
                    }

                    if (!m_dungeon.GetRoom(adjacentRoomNumber).wasVisited)
                    {
                        candidatesOnLevel.push_back(adjacentRoomNumber);
                        previousRoom[static_cast<std::size_t>(adjacentRoomNumber)] = roomNumber;
                        wasSeen[static_cast<std::size_t>(adjacentRoomNumber)] = true;
                    }
                    else
                    {
                        previousRoom[static_cast<std::size_t>(adjacentRoomNumber)] = roomNumber;
                        wasSeen[static_cast<std::size_t>(adjacentRoomNumber)] = true;
                        roomQueue.push(adjacentRoomNumber);
                    }
                }
            }

            if (!candidatesOnLevel.empty())
            {
                bestTargetRoom = *std::min_element(candidatesOnLevel.begin(), candidatesOnLevel.end());
            }
        }

        if (bestTargetRoom < 0)
        {
            return {};
        }

        std::vector<int> reversedPath;
        for (int roomNumber = bestTargetRoom; roomNumber != m_currentRoomNumber; roomNumber = previousRoom[static_cast<std::size_t>(roomNumber)])
        {
            reversedPath.push_back(roomNumber);
        }

        std::reverse(reversedPath.begin(), reversedPath.end());
        return reversedPath;
    }

    std::vector<int> AliceBot::BuildReturnPathToStart() const
    {
        std::queue<int> roomQueue;
        std::vector<int> distance(static_cast<std::size_t>(m_dungeon.GetLastRoomNumber() + 1), -1);

        roomQueue.push(s_startRoomNumber);
        distance[static_cast<std::size_t>(s_startRoomNumber)] = 0;

        while (!roomQueue.empty())
        {
            const int roomNumber = roomQueue.front();
            roomQueue.pop();

            const Room& room = m_dungeon.GetRoom(roomNumber);
            for (int adjacentRoomNumber : room.adjacentRooms)
            {
                if (!m_dungeon.GetRoom(adjacentRoomNumber).wasVisited)
                {
                    continue;
                }

                if (distance[static_cast<std::size_t>(adjacentRoomNumber)] == -1)
                {
                    distance[static_cast<std::size_t>(adjacentRoomNumber)] = distance[static_cast<std::size_t>(roomNumber)] + 1;
                    roomQueue.push(adjacentRoomNumber);
                }
            }
        }

        std::vector<int> path;
        int roomNumber = m_currentRoomNumber;
        while (roomNumber != s_startRoomNumber)
        {
            const Room& room = m_dungeon.GetRoom(roomNumber);
            int nextRoomNumber = -1;

            for (int adjacentRoomNumber : room.adjacentRooms)
            {
                if (!m_dungeon.GetRoom(adjacentRoomNumber).wasVisited)
                {
                    continue;
                }

                if (distance[static_cast<std::size_t>(adjacentRoomNumber)] == distance[static_cast<std::size_t>(roomNumber)] - 1)
                {
                    nextRoomNumber = adjacentRoomNumber;
                    break;
                }
            }

            if (nextRoomNumber < 0)
            {
                return {};
            }

            path.push_back(nextRoomNumber);
            roomNumber = nextRoomNumber;
        }

        return path;
    }

    std::vector<int> AliceBot::BuildPathToRoom(int targetRoomNumber, bool onlyVisitedIntermediateRooms) const
    {
        if (targetRoomNumber == m_currentRoomNumber)
        {
            return {};
        }

        std::queue<int> roomQueue;
        std::vector<int> previousRoom(static_cast<std::size_t>(m_dungeon.GetLastRoomNumber() + 1), -1);
        std::vector<bool> wasSeen(static_cast<std::size_t>(m_dungeon.GetLastRoomNumber() + 1), false);

        roomQueue.push(m_currentRoomNumber);
        wasSeen[static_cast<std::size_t>(m_currentRoomNumber)] = true;

        while (!roomQueue.empty())
        {
            const int roomNumber = roomQueue.front();
            roomQueue.pop();

            const Room& room = m_dungeon.GetRoom(roomNumber);
for (int adjacentRoomNumber : room.adjacentRooms)
                {
                    if (wasSeen[static_cast<std::size_t>(adjacentRoomNumber)])
                    {
                        continue;
                    }

                    const bool canEnterRoom = adjacentRoomNumber == targetRoomNumber ||
                        !onlyVisitedIntermediateRooms ||
                        m_dungeon.GetRoom(adjacentRoomNumber).wasVisited;

                if (!canEnterRoom)
                {
                    continue;
                }

                previousRoom[static_cast<std::size_t>(adjacentRoomNumber)] = roomNumber;
                wasSeen[static_cast<std::size_t>(adjacentRoomNumber)] = true;

                if (adjacentRoomNumber == targetRoomNumber)
                {
                    std::vector<int> reversedPath;
                    for (int pathRoom = targetRoomNumber; pathRoom != m_currentRoomNumber; pathRoom = previousRoom[static_cast<std::size_t>(pathRoom)])
                    {
                        reversedPath.push_back(pathRoom);
                    }

                    std::reverse(reversedPath.begin(), reversedPath.end());
                    return reversedPath;
                }

                roomQueue.push(adjacentRoomNumber);
            }
        }

        return {};
    }

    ResourceType AliceBot::FindBestAvailableResource(const Room& room) const
    {
        return GetAvailableResourcesByValue(room).front();
    }

    std::vector<ResourceType> AliceBot::GetAvailableResourcesByValue(const Room& room) const
    {
        std::vector<ResourceType> availableResources;
        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            if (room.resources[static_cast<std::size_t>(resourceIndex)] > 0 &&
                !room.collectedResources[static_cast<std::size_t>(resourceIndex)])
            {
                availableResources.push_back(static_cast<ResourceType>(resourceIndex));
            }
        }

        std::sort(availableResources.begin(), availableResources.end(), [this](ResourceType left, ResourceType right)
        {
            const int leftValue = GetResourceValue(left);
            const int rightValue = GetResourceValue(right);
            if (leftValue != rightValue)
            {
                return leftValue > rightValue;
            }

            return ToIndex(left) < ToIndex(right);
        });

        return availableResources;
    }

    int AliceBot::GetResourceValue(ResourceType resourceType) const
    {
        ResourceArray resourceValues = GetBaseResourceValues();
        resourceValues[static_cast<std::size_t>(ToIndex(m_settings.targetResource))] *= 2;
        return resourceValues[static_cast<std::size_t>(ToIndex(resourceType))];
    }

    int AliceBot::GetReturnDistanceToStart() const
    {
        return static_cast<int>(BuildReturnPathToStart().size());
    }

    bool AliceBot::RoomHasAvailableResource(const Room& room) const
    {
        bool hasAvailableResource = false;
        for (int resourceIndex = 0; resourceIndex < ResourceCount; ++resourceIndex)
        {
            if (room.resources[static_cast<std::size_t>(resourceIndex)] > 0 &&
                !room.collectedResources[static_cast<std::size_t>(resourceIndex)])
            {
                hasAvailableResource = true;
                break;
            }
        }

        return hasAvailableResource;
    }
}
