#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include "ecs/entitysystem.h"
#include "graphics/shader.h"
#include "graphics/vertexbuffer.h"
#include "graphics/vertexarray.h"
#include "graphics/texture.h"

#include <glm/glm.hpp>
#include <vector>

struct PhongMaterial {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float specularExponent;
    glm::vec4 emissive;

    void setMaterial(ou::Shader &shader) const;
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

    ou::VertexBuffer m_floorVbo;
    ou::VertexArray m_floorVao;
    ou::Texture m_floorTexture;
    PhongMaterial m_floorMaterial;

    std::vector<int> m_tigerNVertices;
    std::vector<int> m_tigerVertexOffset;
    ou::VertexBuffer m_tigerVbo;
    ou::VertexArray m_tigerVao;
    ou::Texture m_tigerTexture;
    PhongMaterial m_tigerMaterial;

    ou::VertexBuffer m_carBodyVbo;
    ou::VertexArray m_carBodyVao;
    int m_carBodyNVertices;
    PhongMaterial m_carBodyMaterial;

    ou::VertexBuffer m_carWheelVbo;
    ou::VertexArray m_carWheelVao;
    int m_carWheelNVertices;
    PhongMaterial m_carWheelMaterial;

    ou::VertexBuffer m_carNutVbo;
    ou::VertexArray m_carNutVao;
    int m_carNutNVertices;
    PhongMaterial m_carNutMaterial;

    ou::VertexBuffer m_cowVbo;
    ou::VertexArray m_cowVao;
    int m_cowNVertices;
    PhongMaterial m_cowMaterial;

    ou::VertexBuffer m_teapotVbo;
    ou::VertexArray m_teapotVao;
    int m_teapotNVertices;
    PhongMaterial m_teapotMaterial;

    ou::VertexBuffer m_ironmanVbo;
    ou::VertexArray m_ironmanVao;
    int m_ironmanNVertices;
    PhongMaterial m_ironmanMaterial;

    std::vector<int> m_wolfNVertices;
    std::vector<int> m_wolfVertexOffset;
    ou::VertexBuffer m_wolfVbo;
    ou::VertexArray m_wolfVao;
    PhongMaterial m_wolfMaterial;

    std::vector<int> m_spiderNVertices;
    std::vector<int> m_spiderVertexOffset;
    ou::VertexBuffer m_spiderVbo;
    ou::VertexArray m_spiderVao;
    PhongMaterial m_spiderMaterial;
};

#endif // RENDERSYSTEM_H
