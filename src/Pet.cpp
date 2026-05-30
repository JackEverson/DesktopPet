#include "Pet.hpp"
#include "Config.hpp"

#include <algorithm>
#include <cmath>

namespace {
    inline float Clamp01_100(float v) { return std::clamp(v, 0.0f, 100.0f); }
}

Pet::Pet() {
    // Sushi is a single-frame sprite; state is conveyed through motion
    // (see CurrentPose) rather than frame swaps. The Animation timer is
    // repurposed to expire transient states (Eating, Happy) back to Idle.
    m_AnimIdle.frames = { 0 };
    m_AnimIdle.frameDuration = 1.0f;
    m_AnimIdle.loop = true;

    m_AnimEating.frames = { 0 };
    m_AnimEating.frameDuration = Config::kEatingDurationSec;
    m_AnimEating.loop = false;

    m_AnimHappy.frames = { 0 };
    m_AnimHappy.frameDuration = Config::kHappyDurationSec;
    m_AnimHappy.loop = false;

    m_AnimSleeping.frames = { 0 };
    m_AnimSleeping.frameDuration = 1.0f;
    m_AnimSleeping.loop = true;

    EnterState(PetState::Idle);
}

void Pet::EnterState(PetState s) {
    m_State = s;
    m_PhaseTime = 0.0f;
    switch (s) {
        case PetState::Idle:     m_Player.Play(&m_AnimIdle);     break;
        case PetState::Eating:   m_Player.Play(&m_AnimEating);   break;
        case PetState::Happy:    m_Player.Play(&m_AnimHappy);    break;
        case PetState::Sleeping: m_Player.Play(&m_AnimSleeping); break;
    }
}

void Pet::DecayStats(float dt) {
    m_Stats.hunger    = Clamp01_100(m_Stats.hunger    - Config::kHungerDecayPerSec    * dt);
    m_Stats.happiness = Clamp01_100(m_Stats.happiness - Config::kHappinessDecayPerSec * dt);
    m_Stats.energy    = Clamp01_100(m_Stats.energy    - Config::kEnergyDecayPerSec    * dt);
}

void Pet::Update(float dt) {
    DecayStats(dt);
    m_PhaseTime += dt;

    if (m_State == PetState::Sleeping) {
        m_Stats.energy = Clamp01_100(m_Stats.energy + Config::kSleepRestorePerSec * dt);
        if (m_Stats.energy >= Config::kAutoWakeEnergyThreshold) {
            EnterState(PetState::Idle);
        }
    }

    m_Player.Update(dt);

    if (m_Player.Finished() && (m_State == PetState::Eating || m_State == PetState::Happy)) {
        EnterState(PetState::Idle);
    }

    if (m_State == PetState::Idle && m_Stats.energy <= Config::kAutoSleepEnergyThreshold) {
        EnterState(PetState::Sleeping);
    }
}

PetPose Pet::CurrentPose() const {
    const float t = m_PhaseTime;
    switch (m_State) {
        case PetState::Idle: {
            // Gentle vertical bob, ~2 px amplitude, slow.
            const int y = static_cast<int>(std::round(std::sin(t * 2.2f) * 2.0f));
            return { 0, y };
        }
        case PetState::Eating: {
            // Fast horizontal wiggle – "chewing".
            const int x = static_cast<int>(std::round(std::sin(t * 18.0f) * 2.0f));
            return { x, 0 };
        }
        case PetState::Happy: {
            // Bouncy upward hop – abs(sin) so it always lifts off the ground.
            const float lift = std::fabs(std::sin(t * 7.5f));
            const int y = -static_cast<int>(std::round(lift * 6.0f));
            return { 0, y };
        }
        case PetState::Sleeping: {
            // Slow breathing – tiny vertical drift.
            const int y = static_cast<int>(std::round(std::sin(t * 1.0f) * 1.0f));
            return { 0, y };
        }
    }
    return { 0, 0 };
}

void Pet::Feed() {
    m_Stats.hunger = Clamp01_100(m_Stats.hunger + Config::kFeedRestore);
    EnterState(PetState::Eating);
}

void Pet::PetIt() {
    m_Stats.happiness = Clamp01_100(m_Stats.happiness + Config::kPetRestore);
    EnterState(PetState::Happy);
}

void Pet::PutToSleep() {
    EnterState(PetState::Sleeping);
}

void Pet::RestoreFromSave(const PetStats& loaded, double secondsOffline) {
    m_Stats = loaded;
    constexpr double kMaxOfflineSeconds = 12.0 * 60.0 * 60.0;
    const float dt = static_cast<float>(std::min(secondsOffline, kMaxOfflineSeconds));
    DecayStats(dt);
}
