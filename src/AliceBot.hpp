#pragma once

#include "Dungeon.hpp"
#include "InputParser.hpp"

#include <string>
#include <vector>

namespace Terracraft
{
    class AliceBot
    {
    public:
        AliceBot(Dungeon dungeon, SimulationSettings settings);

        std::vector<std::string> RunSimulation();

    private:
        void VisitRoom(int roomNumber);
        void MoveToRoom(int roomNumber, bool isFinalReturnMove);
        void TryCollectBestResourceInCurrentRoom();
        void SpendExtraFoodOnCurrentRoom();
        void CollectResource(ResourceType resourceType);
        void WriteCurrentRoomState();
        void WriteResult();

        bool HasUnvisitedRooms() const;
        int FindNextExplorationRoom() const;
        std::vector<int> BuildPathToNearestUnvisitedRoom() const;
        std::vector<int> BuildReturnPathToStart() const;
        std::vector<int> BuildPathToRoom(int targetRoomNumber, bool onlyVisitedIntermediateRooms) const;
        ResourceType FindBestAvailableResource(const Room& room) const;
        std::vector<ResourceType> GetAvailableResourcesByValue(const Room& room) const;
        int GetResourceValue(ResourceType resourceType) const;
        int GetReturnDistanceToStart() const;
        bool RoomHasAvailableResource(const Room& room) const;

        Dungeon m_dungeon;
        SimulationSettings m_settings;
        std::vector<std::string> m_actions;
        ResourceArray m_collectedResources = { 0, 0, 0, 0 };
        int m_currentRoomNumber = 0;
        int m_remainingFood = 0;
    };
}
