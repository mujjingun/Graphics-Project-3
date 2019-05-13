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

private:
    ou::Shader m_simpleShader;
    ou::Shader m_phongShader;

    bool m_flag_tiger_animation;
    bool m_flag_polygon_fill;
    bool m_flag_texture_mapping;
    bool m_flag_fog;

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

    ou::VertexBuffer m_carWheelVbo;
    ou::VertexArray m_carWheelVao;
    int m_carWheelNVertices;

    ou::VertexBuffer m_carNutVbo;
    ou::VertexArray m_carNutVao;
    int m_carNutNVertices;

    ou::VertexBuffer m_cowVbo;
    ou::VertexArray m_cowVao;
    int m_cowNVertices;

    ou::VertexBuffer m_teapotVbo;
    ou::VertexArray m_teapotVao;
    int m_teapotNVertices;
};

#endif // RENDERSYSTEM_H
