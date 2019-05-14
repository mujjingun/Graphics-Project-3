#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H

namespace ou {

class ECSEngine;

class EntitySystem {
public:
    EntitySystem() = default;
    virtual ~EntitySystem() = default;
    virtual void update(ECSEngine& engine, float deltaTime) = 0;
    virtual void afterUpdate(ECSEngine&) {}
};
}

#endif // ENTITYSYSTEM_H
