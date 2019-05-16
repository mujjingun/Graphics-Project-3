#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "ecs/entitysystem.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/vertexarray.h"
#include "graphics/vertexbuffer.h"

#include <functional>
#include <glm/glm.hpp>
#include <vector>

struct PhongMaterial {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float specularExponent;
    glm::vec4 emissive;

    void setMaterial(ou::Shader& shader) const;
};

struct VNTAttr {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class ObjectModel {
    std::vector<int> m_nVertices;
    std::vector<int> m_vertexOffset;
    ou::VertexBuffer m_vbo;
    ou::VertexArray m_vao;

public:
    using cb_type = std::function<std::vector<VNTAttr>(int)>;
    ObjectModel(cb_type getGeometry, int nFrames = 1);

    void render(ou::Shader& shader, glm::mat4 modelViewMat, glm::mat4 projMat, int frame = 0) const;
};

class RenderSystem : public ou::EntitySystem {
public:
    RenderSystem();

    // EntitySystem interface
public:
    void update(ou::ECSEngine& engine, float deltaTime) override;

private:
    void initLightsAndMaterial();
    void initFlags();
    void prepareAxes();
    void prepareFloor();
    void prepareTiger();
    void prepareCar();
    void prepareIronman();
    void prepareWolf();
    void prepareSpider();

    void render(ou::ECSEngine& engine, glm::mat4 viewMatrix, float fov);

private:
    ou::Shader m_simpleShader;
    ou::Shader m_phongShader;

    ou::VertexBuffer m_axesVbo;
    ou::VertexArray m_axesVao;

    ObjectModel m_floor;
    ou::Texture m_floorTexture;
    PhongMaterial m_floorMaterial;

    ObjectModel m_tiger;
    PhongMaterial m_tigerMaterial;
    ou::Texture m_tigerTexture;

    ObjectModel m_wolf;
    PhongMaterial m_wolfMaterial;

    ObjectModel m_spider;
    PhongMaterial m_spiderMaterial;

    ObjectModel m_carBody;
    PhongMaterial m_carBodyMaterial;

    ObjectModel m_carWheel;
    PhongMaterial m_carWheelMaterial;

    ObjectModel m_carNut;
    PhongMaterial m_carNutMaterial;

    ObjectModel m_cow;
    PhongMaterial m_cowMaterial;

    ObjectModel m_teapot;
    PhongMaterial m_teapotMaterial;

    ObjectModel m_ironman;
    PhongMaterial m_ironmanMaterial;
};

#endif // RENDERSYSTEM_H
