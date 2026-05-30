#include "Animation.hpp"

void AnimationPlayer::Play(const Animation* anim) {
    m_Anim = anim;
    m_Elapsed = 0.0f;
    m_FrameIdx = 0;
    m_Finished = false;
}

void AnimationPlayer::Update(float dt) {
    if (!m_Anim || m_Anim->frames.empty() || m_Finished) return;

    m_Elapsed += dt;
    while (m_Elapsed >= m_Anim->frameDuration) {
        m_Elapsed -= m_Anim->frameDuration;
        ++m_FrameIdx;
        if (m_FrameIdx >= static_cast<int>(m_Anim->frames.size())) {
            if (m_Anim->loop) {
                m_FrameIdx = 0;
            } else {
                m_FrameIdx = static_cast<int>(m_Anim->frames.size()) - 1;
                m_Finished = true;
                break;
            }
        }
    }
}

int AnimationPlayer::CurrentFrame() const {
    if (!m_Anim || m_Anim->frames.empty()) return 0;
    return m_Anim->frames[m_FrameIdx];
}
