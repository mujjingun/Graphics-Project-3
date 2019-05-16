#ifndef ANIMATIONSYSTEM_H
#define ANIMATIONSYSTEM_H

#include "ecs/entitysystem.h"

class AnimationSystem : public ou::EntitySystem
{
public:
    AnimationSystem();

    // EntitySystem interface
public:
    void update(ou::ECSEngine &engine, float deltaTime) override;
};

#endif // ANIMATIONSYSTEM_H
