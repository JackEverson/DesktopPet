#pragma once

namespace Config {
    // Sprite (sushi.png) is a single static frame.
    constexpr int kSpriteWidth  = 96;
    constexpr int kSpriteHeight = 78;

    // The window is a touch larger than the sprite so per-state motion
    // (bob, hop) has room to play without clipping.
    constexpr int kBobMargin     = 8;
    constexpr int kWindowWidth   = kSpriteWidth;
    constexpr int kWindowHeight  = kSpriteHeight + kBobMargin * 2;

    // Render cadence
    constexpr int kTargetFps = 30;

    // Stat decay per second (visible over a few minutes)
    constexpr float kHungerDecayPerSec    = 0.10f;
    constexpr float kHappinessDecayPerSec = 0.06f;
    constexpr float kEnergyDecayPerSec    = 0.04f;

    // Restoration amounts
    constexpr float kFeedRestore        = 35.0f;
    constexpr float kPetRestore         = 25.0f;
    constexpr float kSleepRestorePerSec = 4.0f;

    // Auto-state thresholds
    constexpr float kAutoSleepEnergyThreshold = 15.0f;
    constexpr float kAutoWakeEnergyThreshold  = 80.0f;

    // Eating animation: 3 bites, each 4s (2s regular → 2s eat sprite → third disappears).
    constexpr int   kEatingBiteCount   = 3;
    constexpr float kEatingDurationSec = 6.0f;  // kEatingBiteCount × 2s per bite
    constexpr float kBiteEatPhaseSec   = 1.0f;  // eat sprite shows in the last 1s of each bite

    // How long the transient states last before returning to idle.
    constexpr float kHappyDurationSec  = 1.4f;

    // Save cadence
    constexpr float kAutoSaveIntervalSec = 30.0f;

    // Resource paths (relative to working dir; CMake copies res/ next to exe)
    inline const char* kSpriteSheetPath      = "res/pet_sprites.png";
    inline const char* kSleepSpriteSheetPath = "res/pet_sprites_sleepy.png";
    inline const char* kEatSpriteSheetPath   = "res/pet_sprites_eat.png";
    inline const char* kFoodSpriteSheetPath  = "res/pet_food.png";
}
