#include "animationsystem.h"
#include "components.h"
#include "ecs/ecsengine.h"
#include "ecs/entity.h"
#include "input.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <iostream>
#include <random>

AnimationSystem::AnimationSystem()
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
    for (ou::Entity& ent : engine.iterate<Tiger>()) {
        auto& tiger = ent.get<Tiger>();
        auto& hitbox = ent.get<Hitbox>();
        tiger.currFrame = glm::fract(tiger.elapsedTime / 0.2f) * 12;
        tiger.elapsedTime += deltaTime;
        tiger.lastPos = hitbox.pos;

        float angle = tiger.elapsedTime * glm::radians(90.0f);
        float magn = 200.0f * (1 + glm::sin(angle * 5.0f) * 0.1f);
        hitbox.pos = glm::rotate(glm::mat4(1.0), -angle, glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::vec4(magn, 0.0f, 0.0f, 1.0f);
    }

    for (ou::Entity& ent : engine.iterate<Wolf>()) {
        Wolf& wolf = ent.get<Wolf>();
        wolf.currFrame = glm::fract(wolf.elapsedTime / 0.5f) * 17;
        wolf.elapsedTime += deltaTime;
    }

    float carSpeed = 300.0f;
    for (ou::Entity& ent : engine.iterate<Car>()) {
        Car& car = ent.get<Car>();
        Hitbox& hitbox = ent.get<Hitbox>();

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
            car.dests[1] = car.dests[0] - glm::normalize(diff) * 400.0f;
            car.dests[2] = pickCoord() * 500.f;

            car.interval = bezierLength(car.dests[0], car.dests[1], car.dests[2]) / carSpeed;
        }

        float t = car.elapsedTime / car.interval;
        glm::vec2 lastPos = car.pos;
        glm::vec2 lastRearPos = lastPos - car.dir * 200.0f;
        car.pos = bezier(car.dests[0], car.dests[1], car.dests[2], t);

        float speed = glm::length(car.pos - lastPos) / deltaTime;
        if (speed > 0) {
            glm::vec2 dir = glm::normalize(car.pos - lastPos);
            car.dir = dir;
            car.angle = std::atan2(dir.x, dir.y);
        }

        car.wheelAngle += deltaTime * speed * glm::radians(1.0f);

        float radius = bezierCurvature(car.dests[0], car.dests[1], car.dests[2], t);

        const float wheelDistance = 40.0f;
        car.wheelRot = glm::asin(wheelDistance / (2.0f * glm::abs(radius))) * glm::sign(radius);
        car.wheelRot = glm::clamp(car.wheelRot * 15.0f, glm::radians(-90.0f), glm::radians(90.0f));

        glm::vec2 rearPos = car.pos - car.dir * 200.0f;
        glm::vec2 rearDir = rearPos - lastRearPos;
        car.rearRot = std::atan2(rearDir.x, rearDir.y) - car.angle;

        hitbox.pos = glm::vec3(car.pos.x, 0, car.pos.y);
    }

    Input& input = engine.getOne<Input>();
    SceneState const& scene = engine.getOne<SceneState>();

    auto unproject = [&](glm::vec3& point) {
        if (!input.isMouseInScreen()) {
            return false;
        }

        glm::ivec2 clickPos = input.mousePos();

        glm::vec2 normPos = glm::vec2(clickPos) / glm::vec2(scene.windowSize) * 2.0f - 1.0f;

        float aspectRatio = static_cast<float>(scene.windowSize.x) / scene.windowSize.y;
        glm::mat4 proj = glm::perspective(glm::radians(scene.primary.fov), aspectRatio, 20.0f, 20000.0f);

        Camera const& cam = scene.primary;
        glm::mat4 view = glm::lookAt(cam.eyePos, cam.eyePos + cam.lookDir, cam.upDir);

        glm::mat4 invVP = glm::inverse(proj * view);
        glm::vec4 screenPos = glm::vec4(normPos.x, -normPos.y, 1.0f, 1.0f);
        glm::vec4 worldPos = invVP * screenPos;

        glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

        float distance;
        if (glm::intersectRayPlane(cam.eyePos, dir, glm::vec3(0), glm::vec3(0, 1, 0), distance)) {
            point = cam.eyePos + dir * distance;
            return true;
        };

        return false;
    };

    glm::vec3 mouseUnprojPos{};
    bool mouseOnFloor = unproject(mouseUnprojPos);
    if (mouseOnFloor && !scene.secondCamOn && input.isKeyPressed('z') && input.isMouseClicked()) {
        Hitbox hitbox;
        hitbox.pos = mouseUnprojPos;
        engine.addEntity(ou::Entity{ Teapot{}, hitbox });
    }

    bool jump = false;
    if (input.isKeyPressed('u')) {
        input.keyUp('u');
        jump = true;
    }

    for (ou::Entity& ent : engine.iterate<Teapot>()) {
        auto& teapot = ent.get<Teapot>();
        auto& hitbox = ent.get<Hitbox>();

        if (input.isKeyPressed('j')) {
            teapot.angle += glm::radians(360.0f) * deltaTime;
        }

        teapot.vel += teapot.acc * deltaTime;
        hitbox.pos += teapot.vel * deltaTime;

        float bounce = 0.7f;
        float friction = 0.7f;

        if (hitbox.pos.y < 0) {
            hitbox.pos.y = 0;
            teapot.vel.y *= -bounce;
            teapot.vel.x *= friction;
            teapot.vel.z *= friction;
        }

        if (hitbox.pos.y > 500.0f) {
            hitbox.pos.y = 500.0f;
            teapot.vel.y *= -bounce;
        }

        if (hitbox.pos.x < -500.0f || hitbox.pos.x > 500.0f) {
            hitbox.pos.x = glm::clamp(hitbox.pos.x, -500.0f, 500.0f);
            teapot.vel.x *= -bounce;
        }
        if (hitbox.pos.z < -500.0f || hitbox.pos.z > 500.0f) {
            hitbox.pos.z = glm::clamp(hitbox.pos.z, -500.0f, 500.0f);
            teapot.vel.z *= -bounce;
        }

        if (jump) {
            std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
            teapot.vel += glm::vec3(dist(engine.rand()), 1000.0f, dist(engine.rand()));
        }
    }

    for (ou::Entity& ent : engine.iterate<Spider>()) {
        auto& spider = ent.get<Spider>();
        auto& hitbox = ent.get<Hitbox>();

        if (mouseOnFloor) {
            glm::vec3 diff = mouseUnprojPos - hitbox.pos;
            glm::vec3 dir = glm::normalize(diff);
            spider.angle = std::atan2(dir.x, dir.z);
            if (glm::length(diff) > 100.0f) {
                hitbox.pos += dir * 100.0f * deltaTime;
                hitbox.pos = glm::clamp(hitbox.pos, -500.0f, 500.0f);
                hitbox.pos.y = 0;
                spider.currFrame = glm::fract(spider.elapsedTime / 0.5f) * 16;
                spider.elapsedTime += deltaTime;
            }
        }
    }

    // collision detection
    for (ou::Entity& ent : engine.iterate<Hitbox>()) {
        auto& a = ent.get<Hitbox>();

        for (ou::Entity& ent : engine.iterate<Hitbox>()) {
            auto& b = ent.get<Hitbox>();

            if (&a == &b) {
                continue;
            }

            auto diff = a.pos - b.pos;
            auto length = glm::length(diff);
            if (length < a.size + b.size) {
                diff = glm::normalize(diff) * (length - (a.size + b.size));
                auto norm = 1.f / (a.weight + b.weight);
                a.pos -= diff * b.weight * norm;
                b.pos += diff * a.weight * norm;
            }
        }
    }
}
