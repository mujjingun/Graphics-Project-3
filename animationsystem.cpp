#include "animationsystem.h"
#include "components.h"
#include "ecs/ecsengine.h"
#include "ecs/entity.h"

#include <iostream>

AnimationSystem::AnimationSystem()
    : m_elapsedTime(0)
{
}

void AnimationSystem::update(ou::ECSEngine& engine, float deltaTime)
{
    m_elapsedTime += deltaTime;

    float period = 1.0f;
    for (ou::Entity& ent : engine.iterate<Tiger>()) {
        Tiger& tiger = ent.get<Tiger>();
        tiger.currFrame = glm::fract(m_elapsedTime / period) * 12;
        tiger.rotationAngle += deltaTime * glm::radians(90.0);
    }
}
