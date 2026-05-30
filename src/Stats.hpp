#pragma once

#include <cstdint>
#include <filesystem>

struct PetStats {
    float hunger    = 80.0f;
    float happiness = 80.0f;
    float energy    = 80.0f;
};

namespace StatsStore {
    // Returns true if a save file was loaded. secondsSinceSave is set to
    // 0 if no file existed; otherwise to the wall-clock seconds elapsed
    // since the last save so callers can decay stats forward.
    bool Load(PetStats& outStats, double& outSecondsSinceSave);

    void Save(const PetStats& stats);

    std::filesystem::path Path();
}
