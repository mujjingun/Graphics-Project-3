#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>

namespace ou {

class Texture {
	GLuint m_id;

public:
	Texture();
	Texture(GLenum target);
	~Texture();

	Texture(Texture const&) = delete;
	Texture& operator=(Texture const&) = delete;

	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	GLuint id() const;

	void setWrapS(GLint param);
	void setWrapT(GLint param);
	void setMinFilter(GLint param);
	void setMagFilter(GLint param);

	void allocateMultisample2D(GLsizei samples, GLenum internalformat,
		GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	void allocateMultisample3D(GLsizei samples, GLenum internalformat,
		GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

	void allocateStorage2D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height);
	void uploadTexture2D(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const void* pixels);

	void allocateStoarge3D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth);
	void uploadTexture3D(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
		GLenum format, GLenum type, const void* pixels);

	void use(GLenum target);
	void useAsTexture(GLuint unit);
	void useAsImage(GLuint unit, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);

	void saveToImage(GLenum format, GLenum type, GLsizei width, GLsizei height, GLsizei depth, const char* filename) const;

    void loadFromFile(const char* filename);

    void generateMipmap();
};
}

#endif // TEXTURE_H
