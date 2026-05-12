#pragma once

#include "Resource.hpp"

#include <set>
#include <vector>

namespace Terracraft
{
    struct Room
    {
        int m_roomNumber = 0;
        std::set<int> m_adjacentRooms;
        ResourceArray m_resources = { 0, 0, 0, 0 };
        ResourceCollectedArray m_collectedResources = { false, false, false, false };
        bool m_hasFreeCollection = true;
        bool m_wasVisited = false;
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
