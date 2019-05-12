#include "rendersystem.h"
#include "graphics/framebuffer.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>

static const int NUMBER_OF_LIGHT_SUPPORTED = 4;
static const int N_TIGER_FRAMES = 12;

RenderSystem::RenderSystem()
    : m_simpleShader("Shaders/simple.vert", "Shaders/simple.frag")
    , m_phongShader("Shaders/Phong_Tx.vert", "Shaders/Phong_Tx.frag")
{
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    m_viewMatrix = glm::lookAt(4.0f / 6.0f * glm::vec3(500.0f, 600.0f, 500.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    init_lights_and_material();
    init_flags();

    prepare_tiger();
}

void RenderSystem::init_lights_and_material()
{
    m_phongShader.setUniform(glm::vec4(0.115f, 0.115f, 0.115f, 1.0f), "u_global_ambient_color");

    for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; ++i) {
        m_phongShader.setUniform(0, "u_light[%d].light_on", i);
        m_phongShader.setUniform(glm::vec4(0, 0, 1, 0), "u_light[%d].position", i);
        m_phongShader.setUniform(glm::vec4(0, 0, 0, 1), "u_light[%d].ambient_color", i);
        if (i == 0) {
            m_phongShader.setUniform(glm::vec4(1, 1, 1, 1), "u_light[%d].diffuse_color", i);
            m_phongShader.setUniform(glm::vec4(1, 1, 1, 1), "u_light[%d].specular_color", i);
        } else {
            m_phongShader.setUniform(glm::vec4(0, 0, 0, 1), "u_light[%d].diffuse_color", i);
            m_phongShader.setUniform(glm::vec4(0, 0, 0, 1), "u_light[%d].specular_color", i);
        }
        m_phongShader.setUniform(glm::vec3(0, 0, -1), "u_light[%d].spot_direction", i);
        m_phongShader.setUniform(0.0f, "u_light[%d].spot_exponent", i);
        m_phongShader.setUniform(180.0f, "u_light[%d].spot_cutoff_angle", i);
        m_phongShader.setUniform(glm::vec4(1, 0, 0, 0), "u_light[%d].light_attenuation_factors", i);
    }

    m_phongShader.setUniform(glm::vec4(0.2f, 0.2f, 0.2f, 1.0f), "u_material.ambient_color");
    m_phongShader.setUniform(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f), "u_material.diffuse_color");
    m_phongShader.setUniform(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), "u_material.specular_color");
    m_phongShader.setUniform(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), "u_material.emissive_color");
    m_phongShader.setUniform(0.0f, "u_material.specular_exponent"); // [0.0, 128.0]
}

void RenderSystem::init_flags()
{
    m_flag_tiger_animation = 1;
    m_flag_polygon_fill = 1;
    m_flag_texture_mapping = 1;
    m_flag_fog = 0;

    m_phongShader.setUniform(m_flag_fog, "u_flag_fog");
    m_phongShader.setUniform(m_flag_texture_mapping, "u_flag_texture_mapping");
}

template <typename AttrType>
static std::vector<AttrType> read_geometry(const char* filename)
{
    std::cout << "Reading geometry from the geometry file " << filename << "...\n";

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open the object file " << filename << " ...\n";
        throw std::runtime_error("Cannot open object file.");
    }

    int n_triangles;
    file.read(reinterpret_cast<char*>(&n_triangles), sizeof(int));

    std::vector<AttrType> object(n_triangles * 3);
    file.read(reinterpret_cast<char*>(object.data()), sizeof(AttrType) * object.size());

    std::cout << "Read " << n_triangles << " primitives successfully.\n\n";

    return object;
}

void RenderSystem::prepare_tiger()
{
    m_tiger_n_triangles.resize(N_TIGER_FRAMES);
    m_tiger_vertex_offset.resize(N_TIGER_FRAMES);

    struct TigerAttr {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    std::vector<TigerAttr> buf;

    for (int i = 0; i < N_TIGER_FRAMES; i++) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/tiger/Tiger_"
                 << i / 10 << i % 10 << "_triangles_vnt.geom";

        auto frame = read_geometry<TigerAttr>(filename.str().c_str());

        m_tiger_n_triangles[i] = frame.size();

        // append frame data
        std::copy(frame.begin(), frame.end(), std::back_inserter(buf));

        if (i == 0) {
            m_tiger_vertex_offset[i] = 0;
        } else {
            m_tiger_vertex_offset[i] = m_tiger_vertex_offset[i - 1] + 3 * m_tiger_n_triangles[i - 1];
        }
    }

    m_tigerVbo.setData(buf, GL_STATIC_DRAW);

    auto binding = m_tigerVao.getBinding(0);
    binding.bindVertexBuffer(m_tigerVbo, 0, sizeof(TigerAttr));

    auto posAttr = m_tigerVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(TigerAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_tigerVao.enableVertexAttrib(0);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(TigerAttr, normal));

    auto uvAttr = m_tigerVao.enableVertexAttrib(0);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(TigerAttr, uv));

    m_tigerMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_tigerMaterial.diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
    m_tigerMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_tigerMaterial.specularExponent = 51.2f;
    m_tigerMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_tigerTexture = ou::Texture(GL_TEXTURE_2D);
    m_tigerTexture.loadFromFile("Data/dynamic_objects/tiger/tiger_tex2.jpg");

    m_tigerTexture.generateMipmap();

    m_tigerTexture.setMagFilter(GL_NEAREST);
    m_tigerTexture.setMinFilter(GL_NEAREST);

    m_tigerTexture.setWrapS(GL_REPEAT);
    m_tigerTexture.setWrapT(GL_REPEAT);
}

void RenderSystem::update(ou::ECSEngine& engine, float deltaTime)
{
    // Clear framebuffer
    float clearColor[] = { 0, 0, 0, 0 };
    ou::FrameBuffer::defaultBuffer().clear(GL_COLOR, 0, clearColor);

    float clearDepth[] = { 1 };
    ou::FrameBuffer::defaultBuffer().clear(GL_DEPTH, 0, clearDepth);
}
