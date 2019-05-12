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
    void init_lights_and_material();
    void init_flags();
    void prepare_tiger();

private:
    ou::Shader m_simpleShader;
    ou::Shader m_phongShader;

    glm::mat4 m_viewMatrix;

    bool m_flag_tiger_animation;
    bool m_flag_polygon_fill;
    bool m_flag_texture_mapping;
    bool m_flag_fog;

    std::vector<int> m_tiger_n_triangles;
    std::vector<int> m_tiger_vertex_offset;
    ou::VertexBuffer m_tigerVbo;
    ou::VertexArray m_tigerVao;
    ou::Texture m_tigerTexture;
    PhongMaterial m_tigerMaterial;
};

#endif // RENDERSYSTEM_H
