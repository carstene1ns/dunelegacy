/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <CursorManager.h>
#include <FileClasses/GFXManager.h>
#include <globals.h>
#include <Game.h>
#include <ObjectManager.h>
#include <units/UnitBase.h>
#include <structures/StructureBase.h>
#include <structures/Palace.h>

CursorManager::CursorManager() : 
    normalCursor(nullptr),
    moveCursor(nullptr),
    attackCursor(nullptr),
    captureCursor(nullptr),
    carryallDropCursor(nullptr),
    initialized(false) {
}

CursorManager::~CursorManager() {
    cleanup();
}

void CursorManager::initialize() {
    if (initialized) {
        return;
    }

    // Create hardware cursors from existing UI graphics
    SDL_Surface* normalSurface = pGFXManager->getUIGraphicSurface(UI_CursorNormal);
    SDL_Surface* moveSurface = pGFXManager->getUIGraphicSurface(UI_CursorMove_Zoomlevel0);
    SDL_Surface* attackSurface = pGFXManager->getUIGraphicSurface(UI_CursorAttack_Zoomlevel0);
    SDL_Surface* captureSurface = pGFXManager->getUIGraphicSurface(UI_CursorCapture_Zoomlevel0);
    SDL_Surface* carryallDropSurface = pGFXManager->getUIGraphicSurface(UI_CursorCarryallDrop_Zoomlevel0);

    // Create hardware cursors with center hot spots
    if (normalSurface) {
        normalCursor = SDL_CreateColorCursor(normalSurface, normalSurface->w / 2, normalSurface->h / 2);
    }
    
    if (moveSurface) {
        moveCursor = SDL_CreateColorCursor(moveSurface, moveSurface->w / 2, moveSurface->h / 2);
    }
    
    if (attackSurface) {
        attackCursor = SDL_CreateColorCursor(attackSurface, attackSurface->w / 2, attackSurface->h / 2);
    }
    
    if (captureSurface) {
        captureCursor = SDL_CreateColorCursor(captureSurface, captureSurface->w / 2, captureSurface->h / 2);
    }
    
    if (carryallDropSurface) {
        carryallDropCursor = SDL_CreateColorCursor(carryallDropSurface, carryallDropSurface->w / 2, carryallDropSurface->h / 2);
    }

    // Set default cursor
    if (normalCursor) {
        SDL_SetCursor(normalCursor);
    }

    initialized = true;
}

void CursorManager::cleanup() {
    if (!initialized) {
        return;
    }

    if (normalCursor) {
        SDL_FreeCursor(normalCursor);
        normalCursor = nullptr;
    }
    
    if (moveCursor) {
        SDL_FreeCursor(moveCursor);
        moveCursor = nullptr;
    }
    
    if (attackCursor) {
        SDL_FreeCursor(attackCursor);
        attackCursor = nullptr;
    }
    
    if (captureCursor) {
        SDL_FreeCursor(captureCursor);
        captureCursor = nullptr;
    }
    
    if (carryallDropCursor) {
        SDL_FreeCursor(carryallDropCursor);
        carryallDropCursor = nullptr;
    }

    initialized = false;
}

void CursorManager::setCursorMode(int mode) {
    if (!initialized) {
        return;
    }

    SDL_Cursor* cursorToSet = normalCursor; // Default fallback

    switch (mode) {
        case Game::CursorMode_Normal:
        case Game::CursorMode_Placing:
            cursorToSet = normalCursor;
            break;
        case Game::CursorMode_Move:
            cursorToSet = moveCursor ? moveCursor : normalCursor;
            break;
        case Game::CursorMode_Attack:
            cursorToSet = attackCursor ? attackCursor : normalCursor;
            break;
        case Game::CursorMode_Capture:
            cursorToSet = captureCursor ? captureCursor : normalCursor;
            break;
        case Game::CursorMode_CarryallDrop:
            cursorToSet = carryallDropCursor ? carryallDropCursor : normalCursor;
            break;
        default:
            cursorToSet = normalCursor;
            break;
    }

    if (cursorToSet) {
        SDL_SetCursor(cursorToSet);
    }
}

bool CursorManager::canSetCursorMode(int mode, const std::vector<Uint32>& selectedObjects) {
    if (selectedObjects.empty()) {
        return mode == Game::CursorMode_Normal || mode == Game::CursorMode_Placing;
    }

    for (Uint32 objectID : selectedObjects) {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if (!pObject) {
            continue;
        }

        switch (mode) {
            case Game::CursorMode_Move:
                if (pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
                    return true;
                }
                break;
            case Game::CursorMode_Attack:
                if (pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable() && pObject->canAttack()) {
                    return true;
                } else if ((pObject->getItemID() == Structure_Palace) && 
                          ((pObject->getOwner()->getHouseID() == HOUSE_HARKONNEN) || (pObject->getOwner()->getHouseID() == HOUSE_SARDAUKAR))) {
                    Palace* pPalace = static_cast<Palace*>(pObject);
                    if (pPalace->isSpecialWeaponReady()) {
                        return true;
                    }
                }
                break;
            case Game::CursorMode_Capture:
                if (pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable() && pObject->canAttack() && pObject->isInfantry()) {
                    return true;
                }
                break;
            case Game::CursorMode_CarryallDrop:
                if (pObject->isAUnit() && (pObject->getOwner() == pLocalHouse) && pObject->isRespondable()) {
                    return true;
                }
                break;
            case Game::CursorMode_Normal:
            case Game::CursorMode_Placing:
                return true;
        }
    }

    return false;
}
