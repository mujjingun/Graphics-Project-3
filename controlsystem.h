#ifndef CONTROLSYSTEM_H
#define CONTROLSYSTEM_H

#include "ecs/entitysystem.h"

class ControlSystem : public ou::EntitySystem {
public:
    ControlSystem();

    // EntitySystem interface
public:
    void update(ou::ECSEngine &engine, float deltaTime) override;
    void afterUpdate(ou::ECSEngine &engine) override;
};

#endif // CONTROLSYSTEM_H
