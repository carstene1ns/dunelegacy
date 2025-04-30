#include <SpatialGrid.h>
#include <ObjectBase.h>
#include <globals.h>

SpatialGrid::SpatialGrid(int mapWidth, int mapHeight, int cellSize)
    : cellSize(cellSize) {
    if (mapWidth <= 0 || mapHeight <= 0 || cellSize <= 0) {
        throw std::invalid_argument("Invalid dimensions for SpatialGrid");
    }

    gridWidth = (mapWidth + cellSize - 1) / cellSize;
    gridHeight = (mapHeight + cellSize - 1) / cellSize;

    try {
        grid.resize(gridHeight, std::vector<std::list<ObjectBase*>>(gridWidth));
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Failed to allocate memory for spatial grid");
    }
}

void SpatialGrid::updatePosition(ObjectBase* obj, const Coord& oldPos, const Coord& newPos) noexcept {
    if (!obj) {
        SDL_Log("Warning: Attempted to update position of null object");
        return;
    }

    // Remove from old cell if valid
    if (oldPos.isValid()) {
        int oldGridX = oldPos.x / cellSize;
        int oldGridY = oldPos.y / cellSize;
        if (isValidCell(oldGridX, oldGridY)) {
            auto& oldCell = grid[oldGridY][oldGridX];
            auto it = std::find(oldCell.begin(), oldCell.end(), obj);
            if (it != oldCell.end()) {
                oldCell.erase(it);
            } else {
                SDL_Log("Warning: Object not found in expected grid cell at (%d,%d)", oldGridX, oldGridY);
            }
        }
    }

    // Add to new cell if valid
    if (newPos.isValid()) {
        int newGridX = newPos.x / cellSize;
        int newGridY = newPos.y / cellSize;
        if (isValidCell(newGridX, newGridY)) {
            auto& newCell = grid[newGridY][newGridX];
            if (std::find(newCell.begin(), newCell.end(), obj) == newCell.end()) {
                newCell.push_back(obj);
            } else {
                SDL_Log("Warning: Object already exists in grid cell at (%d,%d)", newGridX, newGridY);
            }
        }
    }
}

void SpatialGrid::removeObject(ObjectBase* obj, const Coord& pos) noexcept {
    if (!obj) {
        SDL_Log("Warning: Attempted to remove null object");
        return;
    }

    if (!pos.isValid()) {
        SDL_Log("Warning: Attempted to remove object with invalid position");
        return;
    }

    int gridX = pos.x / cellSize;
    int gridY = pos.y / cellSize;
    
    if (isValidCell(gridX, gridY)) {
        auto& cell = grid[gridY][gridX];
        auto it = std::find(cell.begin(), cell.end(), obj);
        if (it != cell.end()) {
            cell.erase(it);
        } else {
            SDL_Log("Warning: Object not found in grid cell at (%d,%d)", gridX, gridY);
        }
    }
}

const std::list<ObjectBase*>& SpatialGrid::getObjectsInCell(const Coord& pos) const noexcept {
    static const std::list<ObjectBase*> emptyList;
    
    if (!pos.isValid()) {
        return emptyList;
    }
    
    int gridX = pos.x / cellSize;
    int gridY = pos.y / cellSize;
    
    if (!isValidCell(gridX, gridY)) {
        return emptyList;
    }
    
    return grid[gridY][gridX];
} 