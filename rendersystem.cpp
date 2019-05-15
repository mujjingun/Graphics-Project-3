#include "rendersystem.h"
#include "components.h"
#include "ecs/ecsengine.h"
#include "ecs/entity.h"
#include "graphics/framebuffer.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

static const int NUMBER_OF_LIGHT_SUPPORTED = 4;
static const int N_TIGER_FRAMES = 12;
static const int N_WOLF_FRAMES = 17;
static const int N_SPIDER_FRAMES = 16;

RenderSystem::RenderSystem()
    : m_simpleShader("Shaders/simple.vert", "Shaders/simple.frag")
    , m_phongShader("Shaders/Phong_Tx.vert", "Shaders/Phong_Tx.frag")
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    initLightsAndMaterial();
    initFlags();

    prepareAxes();
    prepareFloor();
    prepareTiger();
    prepareCar();
    prepareIronman();
    prepareWolf();
    prepareSpider();
}

void RenderSystem::initLightsAndMaterial()
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

void RenderSystem::initFlags()
{
    m_phongShader.setUniform(0, "u_flag_fog");
    m_phongShader.setUniform(1, "u_flag_texture_mapping");
}

void RenderSystem::prepareAxes()
{
    const glm::vec3 axes_vertices[6] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
    };

    m_axesVbo.setData(axes_vertices, GL_STATIC_DRAW);

    auto binding = m_axesVao.getBinding(0);
    binding.bindVertexBuffer(m_axesVbo, 0, sizeof(glm::vec3));

    auto posAttr = m_axesVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, 0);
    posAttr.setBinding(binding);
}

struct VNTAttr {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

static std::vector<VNTAttr> read_geometry(const char* filename)
{
    std::cout << "Reading geometry from the geometry file " << filename << "...\n";

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open the object file " << filename << " ...\n";
        throw std::runtime_error("Cannot open object file.");
    }

    int n_triangles;
    file.read(reinterpret_cast<char*>(&n_triangles), sizeof(int));

    std::vector<VNTAttr> object(n_triangles * 3);
    file.read(reinterpret_cast<char*>(object.data()), sizeof(VNTAttr) * object.size());

    std::cout << "Read " << n_triangles << " primitives successfully.\n\n";

    return object;
}

void RenderSystem::prepareFloor()
{
    // vertices enumerated counterclockwise
    const VNTAttr rectangleVertices[6] = {
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
        { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
        { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
        { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
    };

    m_floorVbo.setData(rectangleVertices, GL_STATIC_DRAW);

    auto binding = m_floorVao.getBinding(0);
    binding.bindVertexBuffer(m_floorVbo, 0, sizeof(VNTAttr));

    auto posAttr = m_floorVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_floorVao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_floorVao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

    m_floorMaterial.ambient = glm::vec4(0.0f, 0.05f, 0.0f, 1.0f);
    m_floorMaterial.diffuse = glm::vec4(0.2f, 0.5f, 0.2f, 1.0f);
    m_floorMaterial.specular = glm::vec4(0.24f, 0.5f, 0.24f, 1.0f);
    m_floorMaterial.specularExponent = 2.5f;
    m_floorMaterial.emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    m_floorTexture = ou::Texture(GL_TEXTURE_2D);
    m_floorTexture.loadFromFile("Data/static_objects/checker_tex.jpg");

    m_floorTexture.generateMipmap();

    m_floorTexture.setMagFilter(GL_LINEAR);
    m_floorTexture.setMinFilter(GL_LINEAR);

    m_floorTexture.setWrapS(GL_REPEAT);
    m_floorTexture.setWrapT(GL_REPEAT);
}

void RenderSystem::prepareTiger()
{
    m_tigerNVertices.resize(N_TIGER_FRAMES);
    m_tigerVertexOffset.resize(N_TIGER_FRAMES);

    std::vector<VNTAttr> buf;

    for (int i = 0; i < N_TIGER_FRAMES; i++) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/tiger/Tiger_"
                 << std::setfill('0') << std::setw(2)
                 << i << "_triangles_vnt.geom";

        auto frame = read_geometry(filename.str().c_str());

        m_tigerNVertices[i] = frame.size();

        // append frame data
        std::copy(frame.begin(), frame.end(), std::back_inserter(buf));

        if (i == 0) {
            m_tigerVertexOffset[i] = 0;
        } else {
            m_tigerVertexOffset[i] = m_tigerVertexOffset[i - 1] + m_tigerNVertices[i - 1];
        }
    }

    m_tigerVbo.setData(buf, GL_STATIC_DRAW);

    auto binding = m_tigerVao.getBinding(0);
    binding.bindVertexBuffer(m_tigerVbo, 0, sizeof(VNTAttr));

    auto posAttr = m_tigerVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_tigerVao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_tigerVao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

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

static std::vector<VNTAttr> read_geometry_text(const char* filename)
{
    std::cout << "Reading geometry from the geometry file " << filename << "...\n";

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open the object file " << filename << " ...\n";
        throw std::runtime_error("Cannot open object file.");
    }

    int n_triangles;
    file >> n_triangles;

    std::vector<VNTAttr> object(n_triangles * 3);
    for (int i = 0; i < n_triangles; ++i) {
        for (int j = 0; j < 3; ++j) {
            VNTAttr& attr = object[i * 3 + j];
            file >> attr.pos.x >> attr.pos.y >> attr.pos.z;
        }
        glm::vec3 normal = glm::normalize(glm::cross(
            object[i * 3 + 1].pos - object[i * 3].pos,
            object[i * 3 + 2].pos - object[i * 3].pos));
        object[i * 3].normal = object[i * 3 + 1].normal = object[i * 3 + 2].normal = normal;
    }

    std::cout << "Read " << n_triangles << " primitives successfully.\n\n";

    return object;
}

static int make_geometry_text(ou::VertexArray& vao, ou::VertexBuffer& vbo, const char* filename)
{
    auto vertices = read_geometry_text(filename);

    vbo.setData(vertices, GL_STATIC_DRAW);

    auto binding = vao.getBinding(0);
    binding.bindVertexBuffer(vbo, 0, sizeof(VNTAttr));

    auto posAttr = vao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, 0);
    posAttr.setBinding(binding);

    auto normalAttr = vao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = vao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

    return vertices.size();
}

void RenderSystem::prepareCar()
{
    m_carBodyNVertices = make_geometry_text(m_carBodyVao, m_carBodyVbo, "Data/car_body_triangles_v.txt");

    m_carBodyMaterial.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_carBodyMaterial.diffuse = glm::vec4(0.498f, 1.000f, 0.831f, 1.0f);
    m_carBodyMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carBodyMaterial.specularExponent = 91.2f;
    m_carBodyMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_carWheelNVertices = make_geometry_text(m_carWheelVao, m_carWheelVbo, "Data/car_wheel_triangles_v.txt");

    m_carWheelMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_carWheelMaterial.diffuse = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_carWheelMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carWheelMaterial.specularExponent = 91.2f;
    m_carWheelMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_carNutNVertices = make_geometry_text(m_carNutVao, m_carNutVbo, "Data/car_nut_triangles_v.txt");

    m_carNutMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_carNutMaterial.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_carNutMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carNutMaterial.specularExponent = 91.2f;
    m_carNutMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_cowNVertices = make_geometry_text(m_cowVao, m_cowVbo, "Data/cow_triangles_v.txt");

    m_cowMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_cowMaterial.diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
    m_cowMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_cowMaterial.specularExponent = 51.2f;
    m_cowMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_teapotNVertices = make_geometry_text(m_teapotVao, m_teapotVbo, "Data/teapot_triangles_v.txt");

    m_teapotMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_teapotMaterial.diffuse = glm::vec4(0.3f, 1.0f, 1.0f, 1.0f);
    m_teapotMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_teapotMaterial.specularExponent = 91.2f;
    m_teapotMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareIronman()
{
    auto vertices = read_geometry("Data/static_objects/ironman_vnt.geom");
    m_ironmanNVertices = vertices.size();

    m_ironmanVbo.setData(vertices, GL_STATIC_DRAW);

    auto binding = m_ironmanVao.getBinding(0);
    binding.bindVertexBuffer(m_ironmanVbo, 0, sizeof(VNTAttr));

    auto posAttr = m_ironmanVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_ironmanVao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_ironmanVao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

    m_ironmanMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_ironmanMaterial.diffuse = glm::vec4(0.85164f, 0.10648f, 0.12648f, 1.0f);
    m_ironmanMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_ironmanMaterial.specularExponent = 91.2f;
    m_ironmanMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareWolf()
{
    m_wolfNVertices.resize(N_WOLF_FRAMES);
    m_wolfVertexOffset.resize(N_WOLF_FRAMES);

    std::vector<VNTAttr> buf;

    for (int i = 0; i < N_WOLF_FRAMES; i++) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/wolf/wolf_"
                 << std::setfill('0') << std::setw(2)
                 << i << "_vnt.geom";

        auto frame = read_geometry(filename.str().c_str());

        m_wolfNVertices[i] = frame.size();

        // append frame data
        std::copy(frame.begin(), frame.end(), std::back_inserter(buf));

        if (i == 0) {
            m_wolfVertexOffset[i] = 0;
        } else {
            m_wolfVertexOffset[i] = m_wolfVertexOffset[i - 1] + m_wolfNVertices[i - 1];
        }
    }

    m_wolfVbo.setData(buf, GL_STATIC_DRAW);

    auto binding = m_wolfVao.getBinding(0);
    binding.bindVertexBuffer(m_wolfVbo, 0, sizeof(VNTAttr));

    auto posAttr = m_wolfVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_wolfVao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_wolfVao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

    m_wolfMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_wolfMaterial.diffuse = glm::vec4(0.85164f, 0.85164f, 0.85164f, 1.0f);
    m_wolfMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_wolfMaterial.specularExponent = 20.2f;
    m_wolfMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareSpider()
{
    m_spiderNVertices.resize(N_SPIDER_FRAMES);
    m_spiderVertexOffset.resize(N_SPIDER_FRAMES);

    std::vector<VNTAttr> buf;

    for (int i = 0; i < N_SPIDER_FRAMES; i++) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/spider/spider_vnt_"
                 << std::setfill('0') << std::setw(2)
                 << i << ".geom";

        auto frame = read_geometry(filename.str().c_str());

        m_spiderNVertices[i] = frame.size();

        // append frame data
        std::copy(frame.begin(), frame.end(), std::back_inserter(buf));

        if (i == 0) {
            m_spiderVertexOffset[i] = 0;
        } else {
            m_spiderVertexOffset[i] = m_spiderVertexOffset[i - 1] + m_spiderNVertices[i - 1];
        }
    }

    m_spiderVbo.setData(buf, GL_STATIC_DRAW);

    auto binding = m_spiderVao.getBinding(0);
    binding.bindVertexBuffer(m_spiderVbo, 0, sizeof(VNTAttr));

    auto posAttr = m_spiderVao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_spiderVao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_spiderVao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);

    m_spiderMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_spiderMaterial.diffuse = glm::vec4(0.0f, 0.2f, 0.8f, 1.0f);
    m_spiderMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_spiderMaterial.specularExponent = 20.2f;
    m_spiderMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void PhongMaterial::setMaterial(ou::Shader& shader) const
{
    shader.setUniform(ambient, "u_material.ambient_color");
    shader.setUniform(diffuse, "u_material.diffuse_color");
    shader.setUniform(emissive, "u_material.emissive_color");
    shader.setUniform(specular, "u_material.specular_color");
    shader.setUniform(specularExponent, "u_material.specular_exponent");
}

void RenderSystem::render(ou::ECSEngine& engine, glm::mat4 viewMatrix, float fov)
{
    SceneState const& scene = engine.getOne<SceneState>();
    float aspectRatio = static_cast<float>(scene.windowSize.x) / scene.windowSize.y;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 20.0f, 20000.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set initial light uniforms
    m_phongShader.setUniform(true, "u_light[0].light_on");
    m_phongShader.setUniform(glm::vec4(0.0f, 100.0f, 0.0f, 1.0f), "u_light[0].position");
    m_phongShader.setUniform(glm::vec4(0.13f, 0.13f, 0.13f, 1.0f), "u_light[0].ambient_color");
    m_phongShader.setUniform(glm::vec4(0.5f, 0.5f, 0.5f, 1.5f), "u_light[0].diffuse_color");
    m_phongShader.setUniform(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f), "u_light[0].specular_color");

    m_phongShader.setUniform(true, "u_light[1].light_on");
    m_phongShader.setUniform(viewMatrix * glm::vec4(-200.0f, 500.0f, -200.0f, 1.0f), "u_light[1].position");
    m_phongShader.setUniform(glm::vec4(0.152f, 0.152f, 0.152f, 1.0f), "u_light[1].ambient_color");
    m_phongShader.setUniform(glm::vec4(0.572f, 0.572f, 0.572f, 1.0f), "u_light[1].diffuse_color");
    m_phongShader.setUniform(glm::vec4(0.772f, 0.772f, 0.772f, 1.0f), "u_light[1].specular_color");
    m_phongShader.setUniform(glm::mat3(viewMatrix) * glm::vec3(0.0f, -1.0f, 0.0f), "u_light[1].spot_direction");
    m_phongShader.setUniform(20.0f, "u_light[1].spot_cutoff_angle");
    m_phongShader.setUniform(8.0f, "u_light[1].spot_exponent");

    // draw the floor
    {
        glm::mat4 modelViewMatrix, modelViewProjectionMatrix;
        glm::mat3 modelViewMatrixInvTrans;

        modelViewMatrix = glm::translate(viewMatrix, glm::vec3(-500.0f, 0.0f, 500.0f));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));

        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        m_phongShader.setUniform(0, "u_base_texture");
        m_phongShader.setUniform(1, "u_flag_texture_mapping");

        m_floorMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CCW);
        m_phongShader.use();
        glActiveTexture(GL_TEXTURE0);
        m_floorTexture.use(GL_TEXTURE_2D);
        m_floorVao.use();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // draw tigers
    for (ou::Entity const& ent : engine.iterate<Tiger>()) {
        Tiger const& tiger = ent.get<Tiger>();

        glm::vec3 dir = glm::normalize(tiger.pos - tiger.lastPos);

        glm::mat4 modelViewMatrix, modelViewProjectionMatrix;
        glm::mat3 modelViewMatrixInvTrans;

        modelViewMatrix = glm::translate(viewMatrix, tiger.pos);
        modelViewMatrix = glm::rotate(modelViewMatrix, std::atan2(dir.x, dir.z), glm::vec3(0, 1, 0));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(0.5f));
        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));

        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        m_phongShader.setUniform(0, "u_base_texture");
        m_phongShader.setUniform(1, "u_flag_texture_mapping");

        m_tigerMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        glActiveTexture(GL_TEXTURE0);
        m_tigerTexture.use(GL_TEXTURE_2D);
        m_tigerVao.use();
        glDrawArrays(GL_TRIANGLES, m_tigerVertexOffset[tiger.currFrame], m_tigerNVertices[tiger.currFrame]);
    }

    // draw wolf
    for (ou::Entity const& ent : engine.iterate<Wolf>()) {
        Wolf const& wolf = ent.get<Wolf>();

        glm::mat4 modelViewMatrix, modelViewProjectionMatrix;
        glm::mat3 modelViewMatrixInvTrans;

        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(200.0f));

        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));

        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_wolfMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_wolfVao.use();
        glDrawArrays(GL_TRIANGLES, m_wolfVertexOffset[wolf.currFrame], m_wolfNVertices[wolf.currFrame]);
    }

    // draw spider
    for (ou::Entity const& ent : engine.iterate<Spider>()) {
        Spider const& spider = ent.get<Spider>();

        glm::mat4 modelViewMatrix, modelViewProjectionMatrix;
        glm::mat3 modelViewMatrixInvTrans;

        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(80.0f, 0, 0));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(80.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(180.0f), glm::vec3(0, 0, 1));

        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));

        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_spiderMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_spiderVao.use();
        glDrawArrays(GL_TRIANGLES, m_spiderVertexOffset[spider.currFrame], m_spiderNVertices[spider.currFrame]);
    }

    // draw ironman
    {
        glm::mat4 modelViewMatrix, modelViewProjectionMatrix;
        glm::mat3 modelViewMatrixInvTrans;

        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(0.0f, 50.0f, -120.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(30.0f), glm::vec3(1, 0, 0));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(50.0f));

        modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));

        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_ironmanMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_ironmanVao.use();
        glDrawArrays(GL_TRIANGLES, 0, m_ironmanNVertices);
    }

    // draw car
    for (ou::Entity const& ent : engine.iterate<Car>()) {
        Car const& car = ent.get<Car>();

        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(car.pos.x, 0.0f, car.pos.y));
        modelMatrix = glm::rotate(modelMatrix, car.angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 4.89f, -4.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));
        glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        glFrontFace(GL_CCW);
        m_phongShader.use();

        // draw car body
        m_carBodyVao.use();
        m_carBodyMaterial.setMaterial(m_phongShader);
        glDrawArrays(GL_TRIANGLES, 0, m_carBodyNVertices);

        // draw wheels
        auto drawWheelAndNut = [&](glm::mat4 const& modelMat, float offset) {
            glm::mat4 modelViewMatrix = viewMatrix * modelMat;
            glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));
            glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
            m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
            m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
            m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");
            m_carWheelMaterial.setMaterial(m_phongShader);

            m_carWheelVao.use();
            glDrawArrays(GL_TRIANGLES, 0, m_carWheelNVertices);

            for (int i = 0; i < 5; ++i) {
                glm::mat4 nutMat = glm::rotate(modelMat, glm::radians(72.0f * i), glm::vec3(0, 0, 1));
                nutMat = glm::translate(nutMat, glm::vec3(1.7f - 0.5f, 0.0f, offset));

                glm::mat4 modelViewMatrix = viewMatrix * nutMat;
                glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));
                glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
                m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
                m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
                m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");
                m_carNutMaterial.setMaterial(m_phongShader);

                m_carNutVao.use();
                glDrawArrays(GL_TRIANGLES, 0, m_carNutNVertices);
            }
        };

        glm::mat4 wheelModelMatrix;

        // draw wheel 0
        wheelModelMatrix = glm::translate(modelMatrix, glm::vec3(-3.9f, -3.5f, 4.5f));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelRot, glm::vec3(0, 1, 0));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelAngle, glm::vec3(0, 0, 1));
        drawWheelAndNut(wheelModelMatrix, 1.0f);

        // draw wheel 1
        wheelModelMatrix = glm::translate(modelMatrix, glm::vec3(3.9f, -3.5f, 4.5f));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.rearRot, glm::vec3(0, 1, 0));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelAngle, glm::vec3(0, 0, 1));
        drawWheelAndNut(wheelModelMatrix, 1.0f);

        // draw wheel 2
        wheelModelMatrix = glm::translate(modelMatrix, glm::vec3(-3.9f, -3.5f, -4.5f));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelRot, glm::vec3(0, 1, 0));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelAngle, glm::vec3(0, 0, 1));
        drawWheelAndNut(wheelModelMatrix, -1.0f);

        // draw wheel 3
        wheelModelMatrix = glm::translate(modelMatrix, glm::vec3(3.9f, -3.5f, -4.5f));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.rearRot, glm::vec3(0, 1, 0));
        wheelModelMatrix = glm::rotate(wheelModelMatrix, car.wheelAngle, glm::vec3(0, 0, 1));
        drawWheelAndNut(wheelModelMatrix, -1.0f);
    }

    // draw teapot
    for (ou::Entity const& ent : engine.iterate<Teapot>()) {
        Teapot const& teapot = ent.get<Teapot>();

        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, teapot.pos);
        modelMatrix = glm::rotate(modelMatrix, teapot.angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1.6f, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));

        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));
        glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");
        m_teapotMaterial.setMaterial(m_phongShader);

        m_phongShader.use();
        m_teapotVao.use();
        glDrawArrays(GL_TRIANGLES, 0, m_teapotNVertices);
    }

    // draw cow
    {
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::inverse(glm::lookAt(scene.second.eyePos,
            scene.second.eyePos + scene.second.lookDir, scene.second.upDir));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, -0.22f, 0));

        glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
        glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMatrix));
        glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
        m_phongShader.setUniform(modelViewMatrix, "u_ModelViewMatrix");
        m_phongShader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
        m_phongShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");
        m_cowMaterial.setMaterial(m_phongShader);

        m_phongShader.use();
        m_cowVao.use();
        glDrawArrays(GL_TRIANGLES, 0, m_cowNVertices);
    }

    // draw axes
    {
        glm::mat4 modelViewProjectionMatrix;

        modelViewProjectionMatrix = projectionMatrix * viewMatrix;
        modelViewProjectionMatrix = glm::scale(modelViewProjectionMatrix, glm::vec3(50.0f));
        m_simpleShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        glFrontFace(GL_CCW);
        glLineWidth(2.0f);
        m_simpleShader.use();
        m_axesVao.use();
        m_simpleShader.setUniform(glm::vec3(1, 0, 0), "u_primitive_color");
        glDrawArrays(GL_LINES, 0, 2);
        m_simpleShader.setUniform(glm::vec3(0, 1, 0), "u_primitive_color");
        glDrawArrays(GL_LINES, 2, 2);
        m_simpleShader.setUniform(glm::vec3(0, 0, 1), "u_primitive_color");
        glDrawArrays(GL_LINES, 4, 2);
    }
}

void RenderSystem::update(ou::ECSEngine& engine, float)
{
    SceneState const& scene = engine.getOne<SceneState>();

    {
        Camera const& cam = scene.primary;
        glm::mat4 viewMatrix = glm::lookAt(cam.eyePos, cam.eyePos + cam.lookDir, cam.upDir);

        glViewport(0, 0, scene.windowSize.x, scene.windowSize.y);
        glScissor(0, 0, scene.windowSize.x, scene.windowSize.y);
        render(engine, viewMatrix, scene.primary.fov);
    }

    if (scene.secondCamOn) {
        Camera const& cam = scene.second;
        glm::mat4 viewMatrix = glm::lookAt(cam.eyePos, cam.eyePos + cam.lookDir, cam.upDir);

        glViewport(0, 0, scene.windowSize.x / 2, scene.windowSize.y / 2);
        glScissor(0, 0, scene.windowSize.x / 2, scene.windowSize.y / 2);
        render(engine, viewMatrix, scene.second.fov);
    }

    if (scene.carViewportOn) {
        glViewport(scene.windowSize.x / 2, 0, scene.windowSize.x / 2, scene.windowSize.y / 2);
        glScissor(scene.windowSize.x / 2, 0, scene.windowSize.x / 2, scene.windowSize.y / 2);

        Car const& car = engine.getOneEnt<CarCam>().get<Car>();

        glm::mat3 rot = glm::rotate(glm::mat4(1.0f), car.angle, glm::vec3(0, 1, 0));

        glm::vec3 pos = glm::vec3(car.pos.x, 80.0f, car.pos.y) + rot * glm::vec3(0, 0, 68.0f);
        glm::vec3 lookDir = rot * glm::vec3(0, 0, 1);

        glm::mat4 viewMatrix = glm::lookAt(pos - lookDir * 80.0f, pos + lookDir, glm::vec3(0, 1, 0));

        render(engine, viewMatrix, 45.0f);
    }

    if (scene.tigerViewportOn) {
        glViewport(scene.windowSize.x / 2, scene.windowSize.y / 2, scene.windowSize.x / 2, scene.windowSize.y / 2);
        glScissor(scene.windowSize.x / 2, scene.windowSize.y / 2, scene.windowSize.x / 2, scene.windowSize.y / 2);

        Tiger const& tiger = engine.getOneEnt<TigerCam>().get<Tiger>();

        glm::vec3 dir = glm::normalize(tiger.pos - tiger.lastPos);
        float angle = std::atan2(dir.x, dir.z);

        glm::mat3 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        glm::vec3 pos = tiger.pos + rot * glm::vec3(0, 60.0f, 50.0f);
        glm::vec3 lookDir = rot * glm::vec3(0, -0.1f, 1);

        glm::mat4 viewMatrix = glm::lookAt(pos, pos + lookDir, glm::vec3(0, 1, 0));

        render(engine, viewMatrix, 90.0f);
    }
}
