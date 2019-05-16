#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <unordered_map>

#ifdef __GNUC__
#define PRINTF_LIKE __attribute__((__format__(__printf__, 3, 4)))
#else
#define PRINTF_LIKE
#endif

namespace ou {

class Shader {
    GLuint m_id;
    std::unordered_map<std::string, GLint> m_uniforms;

public:
    Shader();
    Shader(const char* vertex_file_path, const char* fragment_file_path);
    Shader(const char* comp_file_path);
    ~Shader();

    Shader(Shader const&) = delete;
    Shader& operator=(Shader const&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    PRINTF_LIKE void setUniform(int value, const char* fmt, ...);
    PRINTF_LIKE void setUniform(float value, const char* fmt, ...);
    PRINTF_LIKE void setUniform(glm::vec2 const& vec, const char* fmt, ...);
    PRINTF_LIKE void setUniform(glm::vec3 const& vec, const char* fmt, ...);
    PRINTF_LIKE void setUniform(glm::vec4 const& vec, const char* fmt, ...);
    PRINTF_LIKE void setUniform(glm::mat3 const& mat, const char* fmt, ...);
    PRINTF_LIKE void setUniform(glm::mat4 const& mat, const char* fmt, ...);

    void setUniform(GLint location, int value);
    void setUniform(GLint location, float value);
    void setUniform(GLint location, glm::vec2 const& vec);
    void setUniform(GLint location, glm::vec3 const& vec);
    void setUniform(GLint location, glm::vec4 const& vec);
    void setUniform(GLint location, glm::mat3 const& mat);
    void setUniform(GLint location, glm::mat4 const& mat);

    void use() const;

    GLuint id() const;
};
}

#endif // UTILS_H
