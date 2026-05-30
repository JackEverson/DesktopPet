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

    // How long the transient states last before returning to idle.
    constexpr float kEatingDurationSec = 1.6f;
    constexpr float kHappyDurationSec  = 1.4f;

    // Save cadence
    constexpr float kAutoSaveIntervalSec = 30.0f;

    // Resource paths (relative to working dir; CMake copies res/ next to exe)
    inline const char* kSpriteSheetPath = "res/pet_sprites.png";
}
