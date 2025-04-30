#ifndef SPATIALGRID_H
#define SPATIALGRID_H

#include <DataTypes.h>
#include <vector>
#include <list>

class ObjectBase;

class SpatialGrid {
private:
    std::vector<std::vector<std::list<ObjectBase*>>> grid;
    int cellSize;
    int gridWidth;
    int gridHeight;

    bool isValidCell(int x, int y) const noexcept {
        return x >= 0 && x < gridWidth && y >= 0 && y < gridHeight;
    }

public:
    SpatialGrid(int mapWidth, int mapHeight, int cellSize);
    
    void updatePosition(ObjectBase* obj, const Coord& oldPos, const Coord& newPos) noexcept;
    void removeObject(ObjectBase* obj, const Coord& pos) noexcept;
    const std::list<ObjectBase*>& getObjectsInCell(const Coord& pos) const noexcept;
};

#endif // SPATIALGRID_H 