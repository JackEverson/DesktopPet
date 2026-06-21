#pragma once

#include "Animation.hpp"
#include "Stats.hpp"

enum class PetState {
    Idle,
    Eating,
    Happy,
    Sleeping,
};

struct PetPose {
    int xOffset; // px relative to the sprite's neutral position
    int yOffset; // px relative to the sprite's neutral position
};

class Pet {
public:
    Pet();

    void Update(float dt);

    // User actions
    void Feed();
    void PetIt();
    void PutToSleep();

    // Persistence
    void RestoreFromSave(const PetStats& loaded, double secondsOffline);
    PetStats SnapshotStats() const { return m_Stats; }

    int  CurrentSpriteFrame() const { return m_Player.CurrentFrame(); }
    PetPose CurrentPose() const;
    PetState State() const { return m_State; }
    float PhaseTime() const { return m_PhaseTime; }
    const PetStats& Stats() const { return m_Stats; }

private:
    void EnterState(PetState s);
    void DecayStats(float dt);

    PetStats m_Stats;
    PetState m_State = PetState::Idle;
    AnimationPlayer m_Player;
    float m_PhaseTime = 0.0f;

    Animation m_AnimIdle;
    Animation m_AnimEating;
    Animation m_AnimHappy;
    Animation m_AnimSleeping;
};
