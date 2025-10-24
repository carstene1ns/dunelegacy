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

#include <players/QuantBotConfig.h>
#include <FileClasses/INIFile.h>
#include <misc/fnkdat.h>
#include <misc/FileSystem.h>
#include <data.h>
#include <globals.h>

// Constructor with default values
QuantBotConfig::QuantBotConfig() {
    
    // === DEFEND DIFFICULTY (Very Easy) ===
    defend.attackEnabled = false;                           // Never attacks
    defend.ornithopterAttackEnabled = false;                // No ornithopter attacks
    defend.ornithopterAttackThreshold = 999;                // Effectively disabled
    defend.militaryValueMultiplier = 1.8f;                  // Campaign: 180% of initial
    defend.militaryValueLimitCustomSmallMap = 4000;         // Custom small
    defend.militaryValueLimitCustomMediumMap = 8000;        // Custom medium
    defend.militaryValueLimitCustomLargeMap = 12000;        // Custom large
    defend.harvesterLimitPerRefineryMultiplier = 2;         // 2 harvesters per refinery
    defend.harvesterLimitCustomSmallMap = 2;
    defend.harvesterLimitCustomMediumMap = 3;
    defend.harvesterLimitCustomLargeMap = 4;
    
    // === EASY DIFFICULTY ===
    easy.attackEnabled = true;                              // Can attack
    easy.ornithopterAttackEnabled = false;                  // NO ornithopter attacks
    easy.ornithopterAttackThreshold = 999;                  // Disabled
    easy.militaryValueMultiplier = 1.0f;                    // Campaign: 100% of initial
    easy.militaryValueLimitCustomSmallMap = 6000;
    easy.militaryValueLimitCustomMediumMap = 12000;
    easy.militaryValueLimitCustomLargeMap = 18000;
    easy.harvesterLimitPerRefineryMultiplier = 1;           // 1 harvester per refinery
    easy.harvesterLimitCustomSmallMap = 2;
    easy.harvesterLimitCustomMediumMap = 3;
    easy.harvesterLimitCustomLargeMap = 4;
    
    // === MEDIUM DIFFICULTY ===
    medium.attackEnabled = true;
    medium.ornithopterAttackEnabled = false;                // NO ornithopter attacks
    medium.ornithopterAttackThreshold = 999;                // Disabled
    medium.militaryValueMultiplier = 1.5f;                  // Campaign: 150% of initial
    medium.militaryValueLimitCustomSmallMap = 8000;
    medium.militaryValueLimitCustomMediumMap = 16000;
    medium.militaryValueLimitCustomLargeMap = 24000;
    medium.harvesterLimitPerRefineryMultiplier = 2;         // 2 harvesters per refinery
    medium.harvesterLimitCustomSmallMap = 3;
    medium.harvesterLimitCustomMediumMap = 4;
    medium.harvesterLimitCustomLargeMap = 5;
    
    // === HARD DIFFICULTY ===
    hard.attackEnabled = true;
    hard.ornithopterAttackEnabled = true;
    hard.ornithopterAttackThreshold = 4;                    // Needs 4+ ornithopters
    hard.militaryValueMultiplier = 2.0f;                    // Campaign: 200% of initial
    hard.militaryValueLimitCustomSmallMap = 10000;
    hard.militaryValueLimitCustomMediumMap = 20000;
    hard.militaryValueLimitCustomLargeMap = 30000;
    hard.harvesterLimitPerRefineryMultiplier = 2;
    hard.harvesterLimitCustomSmallMap = 4;
    hard.harvesterLimitCustomMediumMap = 5;
    hard.harvesterLimitCustomLargeMap = 6;
    
    // === BRUTAL DIFFICULTY ===
    brutal.attackEnabled = true;
    brutal.ornithopterAttackEnabled = true;
    brutal.ornithopterAttackThreshold = 4;
    brutal.militaryValueMultiplier = 3.0f;                  // Campaign: 300% of initial
    brutal.militaryValueLimitCustomSmallMap = 15000;
    brutal.militaryValueLimitCustomMediumMap = 30000;
    brutal.militaryValueLimitCustomLargeMap = 45000;
    brutal.harvesterLimitPerRefineryMultiplier = 3;         // 3 harvesters per refinery
    brutal.harvesterLimitCustomSmallMap = 5;
    brutal.harvesterLimitCustomMediumMap = 6;
    brutal.harvesterLimitCustomLargeMap = 8;
    
    // === UNIT COMPOSITION RATIOS ===
    // Set ratios for each difficulty level, with progressive ornithopter reduction on lower difficulties
    
    // --- DEFEND DIFFICULTY (Very Easy) ---
    // No ornithopters, focus on ground units
    unitRatiosDefend.atreides.tank = 0.05f;
    unitRatiosDefend.atreides.siegeTank = 0.05f;
    unitRatiosDefend.atreides.launcher = 0.25f;
    unitRatiosDefend.atreides.special = 0.65f;        // Sonic tank
    unitRatiosDefend.atreides.ornithopter = 0.00f;    // None for Very Easy
    
    unitRatiosDefend.harkonnen.tank = 0.15f;
    unitRatiosDefend.harkonnen.siegeTank = 0.15f;
    unitRatiosDefend.harkonnen.launcher = 0.70f;
    unitRatiosDefend.harkonnen.special = 0.00f;       // Devastator (spread to other units)
    unitRatiosDefend.harkonnen.ornithopter = 0.00f;   // Can't build
    
    unitRatiosDefend.ordos.tank = 0.35f;
    unitRatiosDefend.ordos.siegeTank = 0.35f;
    unitRatiosDefend.ordos.launcher = 0.00f;          // Can't build
    unitRatiosDefend.ordos.special = 0.30f;           // Deviator
    unitRatiosDefend.ordos.ornithopter = 0.00f;       // None for Very Easy
    
    unitRatiosDefend.fremen.tank = 0.70f;
    unitRatiosDefend.fremen.siegeTank = 0.10f;
    unitRatiosDefend.fremen.launcher = 0.20f;
    unitRatiosDefend.fremen.special = 0.00f;
    unitRatiosDefend.fremen.ornithopter = 0.00f;      // None for Very Easy
    
    unitRatiosDefend.sardaukar.tank = 0.10f;
    unitRatiosDefend.sardaukar.siegeTank = 0.45f;
    unitRatiosDefend.sardaukar.launcher = 0.45f;
    unitRatiosDefend.sardaukar.special = 0.00f;
    unitRatiosDefend.sardaukar.ornithopter = 0.00f;   // None for Very Easy
    
    unitRatiosDefend.mercenary.tank = 0.35f;
    unitRatiosDefend.mercenary.siegeTank = 0.35f;
    unitRatiosDefend.mercenary.launcher = 0.25f;
    unitRatiosDefend.mercenary.special = 0.05f;
    unitRatiosDefend.mercenary.ornithopter = 0.00f;   // None for Very Easy
    
    // --- EASY DIFFICULTY ---
    // Minimal ornithopters (5% max)
    unitRatiosEasy.atreides.tank = 0.05f;
    unitRatiosEasy.atreides.siegeTank = 0.00f;
    unitRatiosEasy.atreides.launcher = 0.25f;
    unitRatiosEasy.atreides.special = 0.65f;          // Sonic tank
    unitRatiosEasy.atreides.ornithopter = 0.05f;      // Minimal
    
    unitRatiosEasy.harkonnen.tank = 0.10f;
    unitRatiosEasy.harkonnen.siegeTank = 0.10f;
    unitRatiosEasy.harkonnen.launcher = 0.70f;
    unitRatiosEasy.harkonnen.special = 0.10f;         // Devastator
    unitRatiosEasy.harkonnen.ornithopter = 0.00f;     // Can't build
    
    unitRatiosEasy.ordos.tank = 0.30f;
    unitRatiosEasy.ordos.siegeTank = 0.30f;
    unitRatiosEasy.ordos.launcher = 0.00f;            // Can't build
    unitRatiosEasy.ordos.special = 0.35f;             // Deviator
    unitRatiosEasy.ordos.ornithopter = 0.05f;         // Minimal
    
    unitRatiosEasy.fremen.tank = 0.70f;
    unitRatiosEasy.fremen.siegeTank = 0.05f;
    unitRatiosEasy.fremen.launcher = 0.20f;
    unitRatiosEasy.fremen.special = 0.00f;
    unitRatiosEasy.fremen.ornithopter = 0.05f;        // Minimal
    
    unitRatiosEasy.sardaukar.tank = 0.05f;
    unitRatiosEasy.sardaukar.siegeTank = 0.45f;
    unitRatiosEasy.sardaukar.launcher = 0.45f;
    unitRatiosEasy.sardaukar.special = 0.00f;
    unitRatiosEasy.sardaukar.ornithopter = 0.05f;     // Minimal
    
    unitRatiosEasy.mercenary.tank = 0.35f;
    unitRatiosEasy.mercenary.siegeTank = 0.35f;
    unitRatiosEasy.mercenary.launcher = 0.25f;
    unitRatiosEasy.mercenary.special = 0.00f;
    unitRatiosEasy.mercenary.ornithopter = 0.05f;     // Minimal
    
    // --- MEDIUM DIFFICULTY ---
    // Reduced ornithopters (10% max)
    unitRatiosMedium.atreides.tank = 0.00f;
    unitRatiosMedium.atreides.siegeTank = 0.00f;
    unitRatiosMedium.atreides.launcher = 0.25f;
    unitRatiosMedium.atreides.special = 0.65f;        // Sonic tank
    unitRatiosMedium.atreides.ornithopter = 0.10f;    // Reduced
    
    unitRatiosMedium.harkonnen.tank = 0.10f;
    unitRatiosMedium.harkonnen.siegeTank = 0.10f;
    unitRatiosMedium.harkonnen.launcher = 0.70f;
    unitRatiosMedium.harkonnen.special = 0.10f;       // Devastator
    unitRatiosMedium.harkonnen.ornithopter = 0.00f;   // Can't build
    
    unitRatiosMedium.ordos.tank = 0.30f;
    unitRatiosMedium.ordos.siegeTank = 0.30f;
    unitRatiosMedium.ordos.launcher = 0.00f;          // Can't build
    unitRatiosMedium.ordos.special = 0.30f;           // Deviator
    unitRatiosMedium.ordos.ornithopter = 0.10f;       // Reduced
    
    unitRatiosMedium.fremen.tank = 0.65f;
    unitRatiosMedium.fremen.siegeTank = 0.10f;
    unitRatiosMedium.fremen.launcher = 0.15f;
    unitRatiosMedium.fremen.special = 0.00f;
    unitRatiosMedium.fremen.ornithopter = 0.10f;      // Reduced
    
    unitRatiosMedium.sardaukar.tank = 0.05f;
    unitRatiosMedium.sardaukar.siegeTank = 0.45f;
    unitRatiosMedium.sardaukar.launcher = 0.40f;
    unitRatiosMedium.sardaukar.special = 0.00f;
    unitRatiosMedium.sardaukar.ornithopter = 0.10f;   // Reduced
    
    unitRatiosMedium.mercenary.tank = 0.35f;
    unitRatiosMedium.mercenary.siegeTank = 0.35f;
    unitRatiosMedium.mercenary.launcher = 0.20f;
    unitRatiosMedium.mercenary.special = 0.00f;
    unitRatiosMedium.mercenary.ornithopter = 0.10f;   // Reduced
    
    // --- HARD DIFFICULTY ---
    // Original balanced ratios
    unitRatiosHard.atreides.tank = 0.00f;
    unitRatiosHard.atreides.siegeTank = 0.00f;
    unitRatiosHard.atreides.launcher = 0.20f;
    unitRatiosHard.atreides.special = 0.65f;          // Sonic tank
    unitRatiosHard.atreides.ornithopter = 0.15f;      // Original
    
    unitRatiosHard.harkonnen.tank = 0.10f;
    unitRatiosHard.harkonnen.siegeTank = 0.10f;
    unitRatiosHard.harkonnen.launcher = 0.70f;
    unitRatiosHard.harkonnen.special = 0.10f;         // Devastator
    unitRatiosHard.harkonnen.ornithopter = 0.00f;     // Can't build
    
    unitRatiosHard.ordos.tank = 0.25f;
    unitRatiosHard.ordos.siegeTank = 0.25f;
    unitRatiosHard.ordos.launcher = 0.00f;            // Can't build
    unitRatiosHard.ordos.special = 0.25f;             // Deviator
    unitRatiosHard.ordos.ornithopter = 0.25f;         // Original
    
    unitRatiosHard.fremen.tank = 0.65f;
    unitRatiosHard.fremen.siegeTank = 0.05f;
    unitRatiosHard.fremen.launcher = 0.20f;
    unitRatiosHard.fremen.special = 0.00f;
    unitRatiosHard.fremen.ornithopter = 0.10f;        // Original
    
    unitRatiosHard.sardaukar.tank = 0.05f;
    unitRatiosHard.sardaukar.siegeTank = 0.40f;
    unitRatiosHard.sardaukar.launcher = 0.45f;
    unitRatiosHard.sardaukar.special = 0.00f;
    unitRatiosHard.sardaukar.ornithopter = 0.10f;     // Original
    
    unitRatiosHard.mercenary.tank = 0.30f;
    unitRatiosHard.mercenary.siegeTank = 0.30f;
    unitRatiosHard.mercenary.launcher = 0.30f;
    unitRatiosHard.mercenary.special = 0.05f;
    unitRatiosHard.mercenary.ornithopter = 0.10f;     // Original
    
    // --- BRUTAL DIFFICULTY ---
    // Same as Hard (original ratios)
    unitRatiosBrutal = unitRatiosHard;
    
    // === GENERAL AI BEHAVIOR ===
    attackTimerMs = 15000;                      // 15 seconds between attacks
    attackThresholdPercent = 0.30f;             // Attack when military >= 30% of limit
    minMoneyForProduction = 500;                // Minimum money to produce units
}

// Helper function to save difficulty settings to INI
static void saveDifficultySettings(INIFile& iniFile, const std::string& section, const std::string& prefix, 
                                   const QuantBotConfig::DifficultySettings& settings) {
    iniFile.setBoolValue(section, prefix + "_AttackEnabled", settings.attackEnabled);
    iniFile.setBoolValue(section, prefix + "_OrnithopterAttackEnabled", settings.ornithopterAttackEnabled);
    iniFile.setIntValue(section, prefix + "_OrnithopterAttackThreshold", settings.ornithopterAttackThreshold);
    iniFile.setDoubleValue(section, prefix + "_MilitaryValueMultiplier", settings.militaryValueMultiplier);
    iniFile.setIntValue(section, prefix + "_MilitaryValueLimitSmallMap", settings.militaryValueLimitCustomSmallMap);
    iniFile.setIntValue(section, prefix + "_MilitaryValueLimitMediumMap", settings.militaryValueLimitCustomMediumMap);
    iniFile.setIntValue(section, prefix + "_MilitaryValueLimitLargeMap", settings.militaryValueLimitCustomLargeMap);
    iniFile.setIntValue(section, prefix + "_HarvesterLimitMultiplier", settings.harvesterLimitPerRefineryMultiplier);
    iniFile.setIntValue(section, prefix + "_HarvesterLimitSmallMap", settings.harvesterLimitCustomSmallMap);
    iniFile.setIntValue(section, prefix + "_HarvesterLimitMediumMap", settings.harvesterLimitCustomMediumMap);
    iniFile.setIntValue(section, prefix + "_HarvesterLimitLargeMap", settings.harvesterLimitCustomLargeMap);
}

// Helper function to load difficulty settings from INI
static void loadDifficultySettings(const INIFile& iniFile, const std::string& section, 
                                   const std::string& prefix, QuantBotConfig::DifficultySettings& settings) {
    settings.attackEnabled = iniFile.getBoolValue(section, prefix + "_AttackEnabled", settings.attackEnabled);
    settings.ornithopterAttackEnabled = iniFile.getBoolValue(section, prefix + "_OrnithopterAttackEnabled", settings.ornithopterAttackEnabled);
    settings.ornithopterAttackThreshold = iniFile.getIntValue(section, prefix + "_OrnithopterAttackThreshold", settings.ornithopterAttackThreshold);
    settings.militaryValueMultiplier = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_MilitaryValueMultiplier", settings.militaryValueMultiplier));
    settings.militaryValueLimitCustomSmallMap = iniFile.getIntValue(section, prefix + "_MilitaryValueLimitSmallMap", settings.militaryValueLimitCustomSmallMap);
    settings.militaryValueLimitCustomMediumMap = iniFile.getIntValue(section, prefix + "_MilitaryValueLimitMediumMap", settings.militaryValueLimitCustomMediumMap);
    settings.militaryValueLimitCustomLargeMap = iniFile.getIntValue(section, prefix + "_MilitaryValueLimitLargeMap", settings.militaryValueLimitCustomLargeMap);
    settings.harvesterLimitPerRefineryMultiplier = iniFile.getIntValue(section, prefix + "_HarvesterLimitMultiplier", settings.harvesterLimitPerRefineryMultiplier);
    settings.harvesterLimitCustomSmallMap = iniFile.getIntValue(section, prefix + "_HarvesterLimitSmallMap", settings.harvesterLimitCustomSmallMap);
    settings.harvesterLimitCustomMediumMap = iniFile.getIntValue(section, prefix + "_HarvesterLimitMediumMap", settings.harvesterLimitCustomMediumMap);
    settings.harvesterLimitCustomLargeMap = iniFile.getIntValue(section, prefix + "_HarvesterLimitLargeMap", settings.harvesterLimitCustomLargeMap);
}

// Helper function to save unit ratios to INI
static void saveUnitRatios(INIFile& iniFile, const std::string& section, const std::string& prefix, 
                          const QuantBotConfig::UnitRatios& ratios) {
    iniFile.setDoubleValue(section, prefix + "_Tank", ratios.tank);
    iniFile.setDoubleValue(section, prefix + "_SiegeTank", ratios.siegeTank);
    iniFile.setDoubleValue(section, prefix + "_Launcher", ratios.launcher);
    iniFile.setDoubleValue(section, prefix + "_Special", ratios.special);
    iniFile.setDoubleValue(section, prefix + "_Ornithopter", ratios.ornithopter);
}

// Helper function to load unit ratios from INI
static void loadUnitRatios(const INIFile& iniFile, const std::string& section, 
                          const std::string& prefix, QuantBotConfig::UnitRatios& ratios) {
    ratios.tank = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_Tank", ratios.tank));
    ratios.siegeTank = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_SiegeTank", ratios.siegeTank));
    ratios.launcher = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_Launcher", ratios.launcher));
    ratios.special = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_Special", ratios.special));
    ratios.ornithopter = static_cast<float>(iniFile.getDoubleValue(section, prefix + "_Ornithopter", ratios.ornithopter));
}

bool QuantBotConfig::save(const std::string& filepath) const {
    try {
        INIFile iniFile(true, "QuantBot AI Configuration");
        
        // === DIFFICULTY SETTINGS ===
        saveDifficultySettings(iniFile, "Difficulty Settings", "Defend", defend);
        saveDifficultySettings(iniFile, "Difficulty Settings", "Easy", easy);
        saveDifficultySettings(iniFile, "Difficulty Settings", "Medium", medium);
        saveDifficultySettings(iniFile, "Difficulty Settings", "Hard", hard);
        saveDifficultySettings(iniFile, "Difficulty Settings", "Brutal", brutal);
        
        // === UNIT RATIOS (per difficulty) ===
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Atreides", unitRatiosDefend.atreides);
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Harkonnen", unitRatiosDefend.harkonnen);
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Ordos", unitRatiosDefend.ordos);
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Fremen", unitRatiosDefend.fremen);
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Sardaukar", unitRatiosDefend.sardaukar);
        saveUnitRatios(iniFile, "Unit Ratios Defend", "Mercenary", unitRatiosDefend.mercenary);
        
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Atreides", unitRatiosEasy.atreides);
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Harkonnen", unitRatiosEasy.harkonnen);
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Ordos", unitRatiosEasy.ordos);
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Fremen", unitRatiosEasy.fremen);
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Sardaukar", unitRatiosEasy.sardaukar);
        saveUnitRatios(iniFile, "Unit Ratios Easy", "Mercenary", unitRatiosEasy.mercenary);
        
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Atreides", unitRatiosMedium.atreides);
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Harkonnen", unitRatiosMedium.harkonnen);
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Ordos", unitRatiosMedium.ordos);
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Fremen", unitRatiosMedium.fremen);
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Sardaukar", unitRatiosMedium.sardaukar);
        saveUnitRatios(iniFile, "Unit Ratios Medium", "Mercenary", unitRatiosMedium.mercenary);
        
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Atreides", unitRatiosHard.atreides);
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Harkonnen", unitRatiosHard.harkonnen);
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Ordos", unitRatiosHard.ordos);
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Fremen", unitRatiosHard.fremen);
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Sardaukar", unitRatiosHard.sardaukar);
        saveUnitRatios(iniFile, "Unit Ratios Hard", "Mercenary", unitRatiosHard.mercenary);
        
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Atreides", unitRatiosBrutal.atreides);
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Harkonnen", unitRatiosBrutal.harkonnen);
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Ordos", unitRatiosBrutal.ordos);
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Fremen", unitRatiosBrutal.fremen);
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Sardaukar", unitRatiosBrutal.sardaukar);
        saveUnitRatios(iniFile, "Unit Ratios Brutal", "Mercenary", unitRatiosBrutal.mercenary);
        
        // === GENERAL BEHAVIOR ===
        iniFile.setIntValue("General Behavior", "AttackTimerMs", attackTimerMs);
        iniFile.setDoubleValue("General Behavior", "AttackThresholdPercent", attackThresholdPercent);
        iniFile.setIntValue("General Behavior", "MinMoneyForProduction", minMoneyForProduction);
        
        // Save to file
        if (!iniFile.saveChangesTo(filepath)) {
            SDL_Log("Warning: Failed to save QuantBot config to %s", filepath.c_str());
            return false;
        }
        
        SDL_Log("QuantBot config saved to: %s", filepath.c_str());
        return true;
        
    } catch (std::exception& e) {
        SDL_Log("Error saving QuantBot config: %s", e.what());
        return false;
    }
}

bool QuantBotConfig::load(const std::string& filepath) {
    try {
        // Check if file exists in user directory
        if (!existsFile(filepath)) {
            SDL_Log("QuantBot config not found in user directory: %s", filepath.c_str());
            
            // Try to copy template from install directory
            try {
                auto templateFile = pFileManager->openFile("QuantBot Config.ini");
                if (templateFile) {
                    SDL_Log("Copying QuantBot Config template to user directory...");
                    
                    // Read template file
                    INIFile templateINI(templateFile.get());
                    
                    // Save to user directory
                    if (templateINI.saveChangesTo(filepath)) {
                        SDL_Log("QuantBot config template copied successfully");
                    } else {
                        SDL_Log("Warning: Failed to copy QuantBot config template, creating defaults");
                        return save(filepath);  // Fallback: create default file programmatically
                    }
                } else {
                    SDL_Log("Warning: Template file not found, creating defaults programmatically");
                    return save(filepath);  // Fallback: create default file
                }
            } catch (std::exception& e) {
                SDL_Log("Error copying QuantBot config template: %s", e.what());
                SDL_Log("Creating default configuration programmatically");
                return save(filepath);  // Fallback: create default file
            }
        }
        
        INIFile iniFile(filepath);
        
        // === LOAD DIFFICULTY SETTINGS ===
        loadDifficultySettings(iniFile, "Difficulty Settings", "Defend", defend);
        loadDifficultySettings(iniFile, "Difficulty Settings", "Easy", easy);
        loadDifficultySettings(iniFile, "Difficulty Settings", "Medium", medium);
        loadDifficultySettings(iniFile, "Difficulty Settings", "Hard", hard);
        loadDifficultySettings(iniFile, "Difficulty Settings", "Brutal", brutal);
        
        // === LOAD UNIT RATIOS (per difficulty) ===
        // Defend difficulty
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Atreides", unitRatiosDefend.atreides);
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Harkonnen", unitRatiosDefend.harkonnen);
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Ordos", unitRatiosDefend.ordos);
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Fremen", unitRatiosDefend.fremen);
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Sardaukar", unitRatiosDefend.sardaukar);
        loadUnitRatios(iniFile, "Unit Ratios Defend", "Mercenary", unitRatiosDefend.mercenary);
        
        // Easy difficulty
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Atreides", unitRatiosEasy.atreides);
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Harkonnen", unitRatiosEasy.harkonnen);
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Ordos", unitRatiosEasy.ordos);
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Fremen", unitRatiosEasy.fremen);
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Sardaukar", unitRatiosEasy.sardaukar);
        loadUnitRatios(iniFile, "Unit Ratios Easy", "Mercenary", unitRatiosEasy.mercenary);
        
        // Medium difficulty
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Atreides", unitRatiosMedium.atreides);
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Harkonnen", unitRatiosMedium.harkonnen);
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Ordos", unitRatiosMedium.ordos);
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Fremen", unitRatiosMedium.fremen);
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Sardaukar", unitRatiosMedium.sardaukar);
        loadUnitRatios(iniFile, "Unit Ratios Medium", "Mercenary", unitRatiosMedium.mercenary);
        
        // Hard difficulty
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Atreides", unitRatiosHard.atreides);
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Harkonnen", unitRatiosHard.harkonnen);
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Ordos", unitRatiosHard.ordos);
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Fremen", unitRatiosHard.fremen);
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Sardaukar", unitRatiosHard.sardaukar);
        loadUnitRatios(iniFile, "Unit Ratios Hard", "Mercenary", unitRatiosHard.mercenary);
        
        // Brutal difficulty
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Atreides", unitRatiosBrutal.atreides);
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Harkonnen", unitRatiosBrutal.harkonnen);
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Ordos", unitRatiosBrutal.ordos);
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Fremen", unitRatiosBrutal.fremen);
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Sardaukar", unitRatiosBrutal.sardaukar);
        loadUnitRatios(iniFile, "Unit Ratios Brutal", "Mercenary", unitRatiosBrutal.mercenary);
        
        // === LOAD GENERAL BEHAVIOR ===
        attackTimerMs = iniFile.getIntValue("General Behavior", "AttackTimerMs", attackTimerMs);
        attackThresholdPercent = static_cast<float>(iniFile.getDoubleValue("General Behavior", "AttackThresholdPercent", attackThresholdPercent));
        minMoneyForProduction = iniFile.getIntValue("General Behavior", "MinMoneyForProduction", minMoneyForProduction);
        
        SDL_Log("QuantBot config loaded from: %s", filepath.c_str());
        return true;
        
    } catch (std::exception& e) {
        SDL_Log("Error loading QuantBot config: %s", e.what());
        SDL_Log("Using default QuantBot configuration");
        return false;
    }
}

const QuantBotConfig::DifficultySettings& QuantBotConfig::getSettings(int difficulty) const {
    switch (difficulty) {
        case 0: return easy;
        case 1: return medium;
        case 2: return hard;
        case 3: return brutal;
        case 4: return defend;
        default: return medium;
    }
}

QuantBotConfig::DifficultySettings& QuantBotConfig::getSettings(int difficulty) {
    switch (difficulty) {
        case 0: return easy;
        case 1: return medium;
        case 2: return hard;
        case 3: return brutal;
        case 4: return defend;
        default: return medium;
    }
}

const QuantBotConfig::UnitRatios& QuantBotConfig::getRatios(int houseID, int difficulty) const {
    // Select the appropriate difficulty set
    const HouseRatios* ratios = nullptr;
    switch (difficulty) {
        case 0: ratios = &unitRatiosEasy; break;
        case 1: ratios = &unitRatiosMedium; break;
        case 2: ratios = &unitRatiosHard; break;
        case 3: ratios = &unitRatiosBrutal; break;
        case 4: ratios = &unitRatiosDefend; break;
        default: ratios = &unitRatiosMedium; break;
    }
    
    // Select the appropriate house from that difficulty set
    switch (houseID) {
        case HOUSE_ATREIDES: return ratios->atreides;
        case HOUSE_HARKONNEN: return ratios->harkonnen;
        case HOUSE_ORDOS: return ratios->ordos;
        case HOUSE_FREMEN: return ratios->fremen;
        case HOUSE_SARDAUKAR: return ratios->sardaukar;
        case HOUSE_MERCENARY: return ratios->mercenary;
        default: return ratios->mercenary;
    }
}

// Global instance
static QuantBotConfig* g_quantBotConfig = nullptr;

QuantBotConfig& getQuantBotConfig() {
    if (!g_quantBotConfig) {
        g_quantBotConfig = new QuantBotConfig();
        
        // Load from file
        std::string configPath = getQuantBotConfigFilepath();
        g_quantBotConfig->load(configPath);
        
        // Log configuration for debugging
        g_quantBotConfig->logSettings();
    }
    return *g_quantBotConfig;
}

std::string getQuantBotConfigFilepath() {
    // Config file is in config subdirectory of game directory
    return getDuneLegacyDataDir() + "/config/QuantBot Config.ini";
}

void QuantBotConfig::logSettings() const {
    SDL_Log("==================== QUANTBOT CONFIGURATION ====================");
    SDL_Log("Config file: %s", getQuantBotConfigFilepath().c_str());
    SDL_Log("");
    
    SDL_Log("=== DIFFICULTY SETTINGS ===");
    SDL_Log("DEFEND:  Attack=%d OrnAttack=%d OrnThresh=%d MilMult=%.2f HarvMult=%d",
        defend.attackEnabled, defend.ornithopterAttackEnabled, defend.ornithopterAttackThreshold,
        defend.militaryValueMultiplier, defend.harvesterLimitPerRefineryMultiplier);
    SDL_Log("EASY:    Attack=%d OrnAttack=%d OrnThresh=%d MilMult=%.2f HarvMult=%d",
        easy.attackEnabled, easy.ornithopterAttackEnabled, easy.ornithopterAttackThreshold,
        easy.militaryValueMultiplier, easy.harvesterLimitPerRefineryMultiplier);
    SDL_Log("MEDIUM:  Attack=%d OrnAttack=%d OrnThresh=%d MilMult=%.2f HarvMult=%d",
        medium.attackEnabled, medium.ornithopterAttackEnabled, medium.ornithopterAttackThreshold,
        medium.militaryValueMultiplier, medium.harvesterLimitPerRefineryMultiplier);
    SDL_Log("HARD:    Attack=%d OrnAttack=%d OrnThresh=%d MilMult=%.2f HarvMult=%d",
        hard.attackEnabled, hard.ornithopterAttackEnabled, hard.ornithopterAttackThreshold,
        hard.militaryValueMultiplier, hard.harvesterLimitPerRefineryMultiplier);
    SDL_Log("BRUTAL:  Attack=%d OrnAttack=%d OrnThresh=%d MilMult=%.2f HarvMult=%d",
        brutal.attackEnabled, brutal.ornithopterAttackEnabled, brutal.ornithopterAttackThreshold,
        brutal.militaryValueMultiplier, brutal.harvesterLimitPerRefineryMultiplier);
    SDL_Log("");
    
    SDL_Log("=== GENERAL BEHAVIOR ===");
    SDL_Log("AttackTimerMs: %d", attackTimerMs);
    SDL_Log("AttackThresholdPercent: %.2f", attackThresholdPercent);
    SDL_Log("MinMoneyForProduction: %d", minMoneyForProduction);
    SDL_Log("");
    
    SDL_Log("=== UNIT RATIOS (showing Easy difficulty sample) ===");
    SDL_Log("Atreides:  Tank=%.2f Siege=%.2f Launcher=%.2f Special=%.2f Orni=%.2f",
        unitRatiosEasy.atreides.tank, unitRatiosEasy.atreides.siegeTank, 
        unitRatiosEasy.atreides.launcher, unitRatiosEasy.atreides.special, unitRatiosEasy.atreides.ornithopter);
    SDL_Log("Harkonnen: Tank=%.2f Siege=%.2f Launcher=%.2f Special=%.2f Orni=%.2f",
        unitRatiosEasy.harkonnen.tank, unitRatiosEasy.harkonnen.siegeTank,
        unitRatiosEasy.harkonnen.launcher, unitRatiosEasy.harkonnen.special, unitRatiosEasy.harkonnen.ornithopter);
    SDL_Log("Ordos:     Tank=%.2f Siege=%.2f Launcher=%.2f Special=%.2f Orni=%.2f",
        unitRatiosEasy.ordos.tank, unitRatiosEasy.ordos.siegeTank,
        unitRatiosEasy.ordos.launcher, unitRatiosEasy.ordos.special, unitRatiosEasy.ordos.ornithopter);
    SDL_Log("===============================================================");
}

std::string QuantBotConfig::getConfigHash() const {
    // Create a string representation of all config values for multiplayer verification
    std::string configStr;
    
    // Difficulty settings
    auto addDiffSettings = [&](const char* name, const DifficultySettings& s) {
        configStr += name;
        configStr += std::to_string(s.attackEnabled);
        configStr += std::to_string(s.ornithopterAttackEnabled);
        configStr += std::to_string(s.ornithopterAttackThreshold);
        configStr += std::to_string(s.militaryValueMultiplier);
        configStr += std::to_string(s.harvesterLimitPerRefineryMultiplier);
        configStr += std::to_string(s.militaryValueLimitCustomSmallMap);
        configStr += std::to_string(s.militaryValueLimitCustomMediumMap);
        configStr += std::to_string(s.militaryValueLimitCustomLargeMap);
        configStr += std::to_string(s.harvesterLimitCustomSmallMap);
        configStr += std::to_string(s.harvesterLimitCustomMediumMap);
        configStr += std::to_string(s.harvesterLimitCustomLargeMap);
    };
    
    addDiffSettings("defend", defend);
    addDiffSettings("easy", easy);
    addDiffSettings("medium", medium);
    addDiffSettings("hard", hard);
    addDiffSettings("brutal", brutal);
    
    // Unit ratios - add all difficulties and houses
    auto addRatios = [&](const char* name, const UnitRatios& r) {
        configStr += name;
        configStr += std::to_string(r.tank);
        configStr += std::to_string(r.siegeTank);
        configStr += std::to_string(r.launcher);
        configStr += std::to_string(r.special);
        configStr += std::to_string(r.ornithopter);
    };
    
    addRatios("DefendAtr", unitRatiosDefend.atreides);
    addRatios("DefendHar", unitRatiosDefend.harkonnen);
    addRatios("DefendOrd", unitRatiosDefend.ordos);
    addRatios("EasyAtr", unitRatiosEasy.atreides);
    addRatios("EasyHar", unitRatiosEasy.harkonnen);
    addRatios("EasyOrd", unitRatiosEasy.ordos);
    addRatios("MediumAtr", unitRatiosMedium.atreides);
    addRatios("MediumHar", unitRatiosMedium.harkonnen);
    addRatios("MediumOrd", unitRatiosMedium.ordos);
    // Add remaining houses and difficulties...
    
    // General behavior
    configStr += std::to_string(attackTimerMs);
    configStr += std::to_string(attackThresholdPercent);
    configStr += std::to_string(minMoneyForProduction);
    
    // Return a simple hash (first 16 chars for readability)
    std::hash<std::string> hasher;
    size_t hashValue = hasher(configStr);
    char hashStr[32];
    snprintf(hashStr, sizeof(hashStr), "%016zx", hashValue);
    return std::string(hashStr);
}

