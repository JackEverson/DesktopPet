#pragma once

#include <vector>

struct Animation {
    std::vector<int> frames;   // indices into the sprite sheet
    float frameDuration;       // seconds per frame
    bool loop;
};

class AnimationPlayer {
public:
    void Play(const Animation* anim);
    void Update(float dt);
    int  CurrentFrame() const;
    bool Finished() const { return m_Finished; }

private:
    const Animation* m_Anim = nullptr;
    float m_Elapsed   = 0.0f;
    int   m_FrameIdx  = 0;
    bool  m_Finished  = false;
};
