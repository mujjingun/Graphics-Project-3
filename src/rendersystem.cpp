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

void PhongMaterial::setMaterial(ou::Shader& shader) const
{
    shader.setUniform(ambient, "u_material.ambient_color");
    shader.setUniform(diffuse, "u_material.diffuse_color");
    shader.setUniform(emissive, "u_material.emissive_color");
    shader.setUniform(specular, "u_material.specular_color");
    shader.setUniform(specularExponent, "u_material.specular_exponent");
}

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

ObjectModel::ObjectModel(cb_type getGeometry, int nFrames)
    : m_nVertices(nFrames)
    , m_vertexOffset(nFrames)
{
    std::vector<VNTAttr> buf;

    for (int i = 0; i < nFrames; i++) {
        auto frame = getGeometry(i);

        m_nVertices[i] = frame.size();

        // append frame data
        std::copy(frame.begin(), frame.end(), std::back_inserter(buf));

        if (i == 0) {
            m_vertexOffset[i] = 0;
        } else {
            m_vertexOffset[i] = m_vertexOffset[i - 1] + m_nVertices[i - 1];
        }
    }

    m_vbo.setData(buf, GL_STATIC_DRAW);

    auto binding = m_vao.getBinding(0);
    binding.bindVertexBuffer(m_vbo, 0, sizeof(VNTAttr));

    auto posAttr = m_vao.enableVertexAttrib(0);
    posAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, pos));
    posAttr.setBinding(binding);

    auto normalAttr = m_vao.enableVertexAttrib(1);
    normalAttr.setFormat(3, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, normal));
    normalAttr.setBinding(binding);

    auto uvAttr = m_vao.enableVertexAttrib(2);
    uvAttr.setFormat(2, GL_FLOAT, GL_FALSE, offsetof(VNTAttr, uv));
    uvAttr.setBinding(binding);
}

void ObjectModel::render(ou::Shader& shader, glm::mat4 modelViewMat, glm::mat4 projMat, int frame) const
{
    glm::mat3 modelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(modelViewMat));
    glm::mat4 modelViewProjectionMatrix = projMat * modelViewMat;
    shader.setUniform(modelViewMat, "u_ModelViewMatrix");
    shader.setUniform(modelViewMatrixInvTrans, "u_ModelViewMatrixInvTrans");
    shader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

    m_vao.use();
    glDrawArrays(GL_TRIANGLES, m_vertexOffset[frame], m_nVertices[frame]);
}

RenderSystem::RenderSystem()
    : m_simpleShader("Shaders/simple.vert", "Shaders/simple.frag")
    , m_phongShader("Shaders/Phong_Tx.vert", "Shaders/Phong_Tx.frag")
    , m_floor([](int) {
        return std::vector<VNTAttr>{
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            { { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
            { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
            { { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
            { { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
        };
    })
    , m_tiger([](int i) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/tiger/Tiger_"
                 << std::setfill('0') << std::setw(2)
                 << i << "_triangles_vnt.geom";

        return read_geometry(filename.str().c_str());
    },
          N_TIGER_FRAMES)
    , m_wolf([](int i) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/wolf/wolf_"
                 << std::setfill('0') << std::setw(2)
                 << i << "_vnt.geom";

        return read_geometry(filename.str().c_str());
    },
          N_WOLF_FRAMES)
    , m_spider([](int i) {
        std::ostringstream filename;
        filename << "Data/dynamic_objects/spider/spider_vnt_"
                 << std::setfill('0') << std::setw(2)
                 << i << ".geom";

        return read_geometry(filename.str().c_str());
    },
          N_SPIDER_FRAMES)
    , m_carBody([](int) {
        return read_geometry_text("Data/car_body_triangles_v.txt");
    })
    , m_carWheel([](int) {
        return read_geometry_text("Data/car_wheel_triangles_v.txt");
    })
    , m_carNut([](int) {
        return read_geometry_text("Data/car_nut_triangles_v.txt");
    })
    , m_cow([](int) {
        return read_geometry_text("Data/cow_triangles_v.txt");
    })
    , m_teapot([](int) {
        return read_geometry_text("Data/teapot_triangles_v.txt");
    })
    , m_ironman([](int) {
        return read_geometry("Data/static_objects/ironman_vnt.geom");
    })
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

void RenderSystem::prepareFloor()
{
    m_floorMaterial.ambient = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    m_floorMaterial.diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_floorMaterial.specular = glm::vec4(0.24f, 0.24f, 0.24f, 1.0f);
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

void RenderSystem::prepareCar()
{
    m_carBodyMaterial.ambient = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    m_carBodyMaterial.diffuse = glm::vec4(0.498f, 1.000f, 0.831f, 1.0f);
    m_carBodyMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carBodyMaterial.specularExponent = 91.2f;
    m_carBodyMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_carWheelMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.9745f, 1.0f);
    m_carWheelMaterial.diffuse = glm::vec4(0.1f, 0.1f, 0.9f, 1.0f);
    m_carWheelMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carWheelMaterial.specularExponent = 91.2f;
    m_carWheelMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_carNutMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_carNutMaterial.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m_carNutMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_carNutMaterial.specularExponent = 91.2f;
    m_carNutMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_cowMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_cowMaterial.diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
    m_cowMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_cowMaterial.specularExponent = 51.2f;
    m_cowMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);

    m_teapotMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_teapotMaterial.diffuse = glm::vec4(0.3f, 1.0f, 1.0f, 1.0f);
    m_teapotMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_teapotMaterial.specularExponent = 91.2f;
    m_teapotMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareIronman()
{
    m_ironmanMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_ironmanMaterial.diffuse = glm::vec4(0.85164f, 0.10648f, 0.12648f, 1.0f);
    m_ironmanMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_ironmanMaterial.specularExponent = 91.2f;
    m_ironmanMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareWolf()
{
    m_wolfMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_wolfMaterial.diffuse = glm::vec4(0.85164f, 0.85164f, 0.85164f, 1.0f);
    m_wolfMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_wolfMaterial.specularExponent = 20.2f;
    m_wolfMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::prepareSpider()
{
    m_spiderMaterial.ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
    m_spiderMaterial.diffuse = glm::vec4(0.0f, 0.2f, 0.8f, 1.0f);
    m_spiderMaterial.specular = glm::vec4(0.728281f, 0.655802f, 0.466065f, 1.0f);
    m_spiderMaterial.specularExponent = 20.2f;
    m_spiderMaterial.emissive = glm::vec4(0.1f, 0.1f, 0.0f, 1.0f);
}

void RenderSystem::render(ou::ECSEngine& engine, glm::mat4 viewMatrix, float fov)
{
    SceneState const& scene = engine.getOne<SceneState>();
    float aspectRatio = static_cast<float>(scene.windowSize.x) / scene.windowSize.y;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 20.0f, 20000.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene.wireframeOn) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

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
        glm::mat4 modelViewMatrix;
        modelViewMatrix = glm::translate(viewMatrix, glm::vec3(-500.0f, 0.0f, 500.0f));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(1000.0f, 1000.0f, 1000.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        m_phongShader.setUniform(0, "u_base_texture");
        m_phongShader.setUniform(1, "u_flag_texture_mapping");

        m_floorMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CCW);
        m_phongShader.use();
        glActiveTexture(GL_TEXTURE0);
        m_floorTexture.use(GL_TEXTURE_2D);
        m_floor.render(m_phongShader, modelViewMatrix, projectionMatrix);
    }

    // draw tigers
    for (ou::Entity const& ent : engine.iterate<Tiger>()) {
        auto const& tiger = ent.get<Tiger>();
        auto const& hitbox = ent.get<Hitbox>();

        glm::vec3 dir = glm::normalize(hitbox.pos - tiger.lastPos);

        glm::mat4 modelViewMatrix;
        modelViewMatrix = glm::translate(viewMatrix, hitbox.pos);
        modelViewMatrix = glm::rotate(modelViewMatrix, std::atan2(dir.x, dir.z), glm::vec3(0, 1, 0));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(0.5f));

        m_phongShader.setUniform(0, "u_base_texture");
        m_phongShader.setUniform(1, "u_flag_texture_mapping");

        m_tigerMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        glActiveTexture(GL_TEXTURE0);
        m_tigerTexture.use(GL_TEXTURE_2D);
        m_tiger.render(m_phongShader, modelViewMatrix, projectionMatrix, tiger.currFrame);
    }

    // draw wolf
    for (ou::Entity const& ent : engine.iterate<Wolf>()) {
        Wolf const& wolf = ent.get<Wolf>();

        glm::mat4 modelViewMatrix;
        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(-45.f), glm::vec3(0, 1, 0));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(200.0f));

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_wolfMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_wolf.render(m_phongShader, modelViewMatrix, projectionMatrix, wolf.currFrame);
    }

    // draw spider
    for (ou::Entity const& ent : engine.iterate<Spider>()) {
        auto const& spider = ent.get<Spider>();
        auto const& hitbox = ent.get<Hitbox>();

        glm::mat4 modelViewMatrix;
        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::translate(modelViewMatrix, hitbox.pos);
        modelViewMatrix = glm::rotate(modelViewMatrix, spider.angle, glm::vec3(0, 1, 0));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(80.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(180.0f), glm::vec3(0, 0, 1));

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_spiderMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_spider.render(m_phongShader, modelViewMatrix, projectionMatrix, spider.currFrame);
    }

    // draw ironman
    {
        glm::mat4 modelViewMatrix;
        modelViewMatrix = viewMatrix;
        modelViewMatrix = glm::translate(modelViewMatrix, glm::vec3(0.0f, 50.0f, -120.0f));
        modelViewMatrix = glm::rotate(modelViewMatrix, glm::radians(30.0f), glm::vec3(1, 0, 0));
        modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(50.0f));

        m_phongShader.setUniform(0, "u_flag_texture_mapping");

        m_ironmanMaterial.setMaterial(m_phongShader);

        glFrontFace(GL_CW);
        m_phongShader.use();
        m_ironman.render(m_phongShader, modelViewMatrix, projectionMatrix);
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

        glFrontFace(GL_CCW);
        m_phongShader.use();

        // draw car body
        m_carBodyMaterial.setMaterial(m_phongShader);
        m_carBody.render(m_phongShader, viewMatrix * modelMatrix, projectionMatrix);

        // draw wheels
        auto drawWheelAndNut = [&](glm::mat4 const& modelMat, float offset) {

            m_carWheelMaterial.setMaterial(m_phongShader);
            m_carWheel.render(m_phongShader, viewMatrix * modelMat, projectionMatrix);

            for (int i = 0; i < 5; ++i) {
                glm::mat4 nutMat = glm::rotate(modelMat, glm::radians(72.0f * i), glm::vec3(0, 0, 1));
                nutMat = glm::translate(nutMat, glm::vec3(1.7f - 0.5f, 0.0f, offset));

                m_carNutMaterial.setMaterial(m_phongShader);

                m_carNut.render(m_phongShader, viewMatrix * nutMat, projectionMatrix);
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
        auto const& teapot = ent.get<Teapot>();
        auto const& hitbox = ent.get<Hitbox>();

        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::translate(modelMatrix, hitbox.pos);
        modelMatrix = glm::rotate(modelMatrix, teapot.angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1.6f, 0));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));

        m_teapotMaterial.setMaterial(m_phongShader);

        m_phongShader.use();
        m_teapot.render(m_phongShader, viewMatrix * modelMatrix, projectionMatrix);
    }

    // draw cow
    {
        glm::mat4 modelMatrix(1.0f);
        modelMatrix = glm::inverse(glm::lookAt(scene.second.eyePos,
            scene.second.eyePos + scene.second.lookDir, scene.second.upDir));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, -0.22f, 0));

        m_cowMaterial.setMaterial(m_phongShader);

        m_phongShader.use();
        m_cow.render(m_phongShader, viewMatrix * modelMatrix, projectionMatrix);
    }

    // draw axes
    {
        glm::mat4 modelViewProjectionMatrix;

        modelViewProjectionMatrix = projectionMatrix * viewMatrix;
        modelViewProjectionMatrix = glm::scale(modelViewProjectionMatrix, glm::vec3(50.0f));
        m_simpleShader.setUniform(modelViewProjectionMatrix, "u_ModelViewProjectionMatrix");

        glFrontFace(GL_CCW);
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

    // Primary camera
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

        glm::mat3 rot = glm::mat3(glm::rotate(glm::mat4(1.0f), car.angle, glm::vec3(0, 1, 0)));

        glm::vec3 pos = glm::vec3(car.pos.x, 80.0f, car.pos.y) + rot * glm::vec3(0, 0, 68.0f);
        glm::vec3 lookDir = rot * glm::vec3(0, 0, 1);

        glm::mat4 viewMatrix = glm::lookAt(pos - lookDir * 80.0f, pos + lookDir, glm::vec3(0, 1, 0));

        render(engine, viewMatrix, 45.0f);
    }

    if (scene.tigerViewportOn) {
        glViewport(scene.windowSize.x / 2, scene.windowSize.y / 2, scene.windowSize.x / 2, scene.windowSize.y / 2);
        glScissor(scene.windowSize.x / 2, scene.windowSize.y / 2, scene.windowSize.x / 2, scene.windowSize.y / 2);

        auto const& tigerEnt = engine.getOneEnt<TigerCam>();
        auto const& tiger = tigerEnt.get<Tiger>();
        auto const& hitbox = tigerEnt.get<Hitbox>();

        glm::vec3 dir = glm::normalize(hitbox.pos - tiger.lastPos);
        float angle = std::atan2(dir.x, dir.z);

        glm::mat3 rot = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0)));
        glm::vec3 pos = hitbox.pos + rot * glm::vec3(0, 60.0f, 50.0f);
        glm::vec3 lookDir = rot * glm::vec3(0, -0.1f, 1);

        glm::mat4 viewMatrix = glm::lookAt(pos, pos + lookDir, glm::vec3(0, 1, 0));

        render(engine, viewMatrix, 90.0f);
    }
}
