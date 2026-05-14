#pragma once

#include "Resource.hpp"

#include <set>
#include <vector>

namespace Terracraft
{
    struct Room
    {
        int roomNumber = 0;
        std::set<int> adjacentRooms;
        ResourceArray resources = { 0, 0, 0, 0 };
        ResourceCollectedArray collectedResources = { false, false, false, false };
        bool hasFreeCollection = true;
        bool wasVisited = false;
    };

    class Dungeon
    {
    public:
        explicit Dungeon(int roomCount);

        int GetLastRoomNumber() const;
        bool HasRoom(int roomNumber) const;
        Room& GetRoom(int roomNumber);
        const Room& GetRoom(int roomNumber) const;
        const std::vector<Room>& GetRooms() const;

    private:
        std::vector<Room> m_rooms;
    };
}
