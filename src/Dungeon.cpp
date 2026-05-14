#include "Dungeon.hpp"

#include <stdexcept>

namespace Terracraft
{
    Dungeon::Dungeon(int roomCount)
        : m_rooms(static_cast<std::size_t>(roomCount + 1))
    {
        for (int roomNumber = 0; roomNumber <= roomCount; ++roomNumber)
        {
            m_rooms[static_cast<std::size_t>(roomNumber)].roomNumber = roomNumber;
        }
    }

    int Dungeon::GetLastRoomNumber() const
    {
        return static_cast<int>(m_rooms.size()) - 1;
    }

    bool Dungeon::HasRoom(int roomNumber) const
    {
        return roomNumber >= 0 && roomNumber <= GetLastRoomNumber();
    }

    Room& Dungeon::GetRoom(int roomNumber)
    {
        if (!HasRoom(roomNumber))
        {
            throw std::out_of_range("Room number is out of range");
        }

        return m_rooms[static_cast<std::size_t>(roomNumber)];
    }

    const Room& Dungeon::GetRoom(int roomNumber) const
    {
        if (!HasRoom(roomNumber))
        {
            throw std::out_of_range("Room number is out of range");
        }

        return m_rooms[static_cast<std::size_t>(roomNumber)];
    }

    const std::vector<Room>& Dungeon::GetRooms() const
    {
        return m_rooms;
    }
}
