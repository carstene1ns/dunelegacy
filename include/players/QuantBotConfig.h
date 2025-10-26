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

#ifndef QUANTBOTCONFIG_H
#define QUANTBOTCONFIG_H

#include <DataTypes.h>
#include <string>

/**
 * Configuration structure for QuantBot AI behavior.
 * All settings can be customized via "QuantBot Config.ini" in the user config directory.
 */
struct QuantBotConfig {
    
    // === Difficulty-specific settings ===
    struct DifficultySettings {
        // Attack behavior
        bool attackEnabled;                     // Can this difficulty attack at all?
        bool ornithopterAttackEnabled;          // Can ornithopters attack?
        int ornithopterAttackThreshold;         // Minimum ornithopters needed to attack
        
        // Military limits
        float militaryValueMultiplier;          // Multiplier for initial military value (Campaign)
        int militaryValueLimitCustomSmallMap;   // Military value limit for small maps (Custom)
        int militaryValueLimitCustomMediumMap;  // Military value limit for medium maps (Custom)
        int militaryValueLimitCustomLargeMap;   // Military value limit for large maps (Custom)
        
        // Harvester limits
        int harvesterLimitPerRefineryMultiplier; // Harvesters per refinery
        int harvesterLimitCustomSmallMap;       // Harvester limit for small maps (Custom)
        int harvesterLimitCustomMediumMap;      // Harvester limit for medium maps (Custom)
        int harvesterLimitCustomLargeMap;       // Harvester limit for large maps (Custom)
    };
    
    DifficultySettings defend;   // Very Easy (Defend only)
    DifficultySettings easy;
    DifficultySettings medium;
    DifficultySettings hard;
    DifficultySettings brutal;
    
    // === Unit composition ratios (house-specific) ===
    // These are used when totalDamage < 3000 (early game)
    // Same ratios across all difficulties - ornithopter spam controlled by attack flags
    struct UnitRatios {
        float tank;
        float siegeTank;
        float launcher;
        float special;      // Devastator/Deviator/Sonic Tank
        float ornithopter;
    };
    
    struct HouseRatios {
        UnitRatios atreides;
        UnitRatios harkonnen;
        UnitRatios ordos;
        UnitRatios fremen;
        UnitRatios sardaukar;
        UnitRatios mercenary;  // Default for other houses
    };
    
    // Single set of ratios used across all difficulties
    HouseRatios unitRatios;
    
    // === General AI behavior ===
    int attackTimerMs;              // Time between attack checks (milliseconds)
    float attackThresholdPercent;   // Don't attack until military reaches this % of limit (0.0-1.0)
    int minMoneyForProduction;      // Minimum money needed to queue production
    
    // Constructor with default values
    QuantBotConfig();
    
    // Load from INI file (creates default file if not exists)
    bool load(const std::string& filepath);
    
    // Save to INI file
    bool save(const std::string& filepath) const;
    
    // Get difficulty settings by difficulty enum
    const DifficultySettings& getSettings(int difficulty) const;
    DifficultySettings& getSettings(int difficulty);
    
    // Get unit ratios by house ID (same ratios for all difficulties)
    const UnitRatios& getRatios(int houseID) const;
    
    // Logging and verification
    void logSettings() const;
    std::string getConfigHash() const;  // For multiplayer consistency check
};

// Global instance (initialized on first use)
QuantBotConfig& getQuantBotConfig();

// Get config file paths
std::string getQuantBotConfigFilepath();
std::string getObjectDataFilepath();

// Get config file hashes for multiplayer verification
std::string getObjectDataHash();

#endif // QUANTBOTCONFIG_H

