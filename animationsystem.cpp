#include "animationsystem.h"
#include "components.h"
#include "ecs/ecsengine.h"
#include "ecs/entity.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>

AnimationSystem::AnimationSystem()
    : m_elapsedTime(0)
{
}

static glm::vec2 bezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float t)
{
    float ct = 1 - t;
    glm::vec2 p = ct * ct * p0;
    p += 2.0f * t * ct * p1;
    p += t * t * p2;

    return p;
}

static float bezierLength(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2)
{
    glm::vec2 p = p0;
    float len = 0;
    for (float t = 0; t < 1; t += 1.f / 50) {
        glm::vec2 tp = bezier(p0, p1, p2, t);
        len += glm::distance(p, tp);
        p = tp;
    }
    return len;
}

static float bezierCurvature(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, float t)
{
    glm::vec2 ct = p1 - p0;
    glm::vec2 bt = 2.0f * (1 - t) * ct + 2.0f * t * (p2 - p1);
    glm::vec2 dt = p2 - p0;

    float dot = ct.x * -dt.y + ct.y * dt.x;

    return glm::length(bt) * glm::sign(dot);
}

void AnimationSystem::update(ou::ECSEngine& engine, float deltaTime)
{
    m_elapsedTime += deltaTime;

    float period = 0.2f;
    for (ou::Entity& ent : engine.iterate<Tiger>()) {
        Tiger& tiger = ent.get<Tiger>();
        tiger.currFrame = glm::fract(m_elapsedTime / period) * 12;
        tiger.elapsedTime += deltaTime;
        tiger.lastPos = tiger.pos;

        float angle = tiger.elapsedTime * glm::radians(90.0f);
        float magn = 200.0f * (1 + glm::sin(angle * 5.0f) * 0.1f);
        tiger.pos = glm::rotate(glm::mat4(1.0), -angle, glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::vec4(magn, 0.0f, 0.0f, 1.0f);
    }

    float carSpeed = 300.0f;
    for (ou::Entity& ent : engine.iterate<Car>()) {
        Car& car = ent.get<Car>();

        car.elapsedTime += deltaTime;

        // pick a new place to move to
        auto pickCoord = [&] {
            std::uniform_real_distribution<float> dist(-1, 1);
            glm::vec2 dest = glm::vec2(dist(engine.rand()), dist(engine.rand()));
            return dest;
        };

        while (car.dests.size() < 3) {
            car.dests.resize(3);
            car.dests[0] = pickCoord() * 500.f;
            car.dests[2] = pickCoord() * 500.f;
            auto dir = car.dests[2] - car.dests[0];
            car.dests[1] = (car.dests[0] + car.dests[2]) * 0.5f + glm::vec2(dir.y, dir.x);

            car.interval = bezierLength(car.dests[0], car.dests[1], car.dests[2]) / carSpeed;
        }

        if (car.elapsedTime >= car.interval) {
            car.elapsedTime -= car.interval;

            glm::vec2 diff = car.dests[1] - car.dests[2];
            car.dests[0] = car.dests[2];
            car.dests[1] = car.dests[0] - diff;
            car.dests[2] = pickCoord() * 500.f;

            car.interval = bezierLength(car.dests[0], car.dests[1], car.dests[2]) / carSpeed;
        }

        float t = car.elapsedTime / car.interval;
        glm::vec2 lastPos = car.pos;
        car.pos = bezier(car.dests[0], car.dests[1], car.dests[2], t);

        float speed = glm::length(car.pos - lastPos) / deltaTime;
        if (speed > 0) {
            glm::vec2 dir = glm::normalize(car.pos - lastPos);
            car.angle = std::atan2(dir.x, dir.y);
        }

        car.wheelAngle += deltaTime * speed * glm::radians(1.0f);

        float radius = bezierCurvature(car.dests[0], car.dests[1], car.dests[2], t);

        const float wheelDistance = 40.0f;
        car.wheelRot = glm::asin(wheelDistance / (2.0f * glm::abs(radius))) * glm::sign(radius);
        car.wheelRot *= 10.0f;
    }
}
