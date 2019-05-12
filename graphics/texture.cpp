#include "texture.h"

#include <algorithm>
#include <iostream>
#include <vector>

#include <FreeImage/FreeImage.h>

namespace ou {

GLuint Texture::id() const
{
    return m_id;
}

void Texture::setWrapS(GLint param)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, param);
}

void Texture::setWrapT(GLint param)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, param);
}

void Texture::setMinFilter(GLint param)
{
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, param);
}

void Texture::setMagFilter(GLint param)
{
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, param);
}

void Texture::allocateMultisample2D(GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    // No DSA equivalent.
    GLint original;
    glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &original);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat, width, height, fixedsamplelocations);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, original);
}

void Texture::allocateMultisample3D(GLsizei samples, GLenum internalformat,
    GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    // No DSA equivalent.
    GLint original;
    glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, &original);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, m_id);
    glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, internalformat, width, height, depth, fixedsamplelocations);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, original);
}

void Texture::allocateStorage2D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
    glTextureStorage2D(m_id, levels, internalFormat, width, height);
}

void Texture::uploadTexture2D(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
    GLenum format, GLenum type, const void* pixels)
{
    glTextureSubImage2D(m_id, level, xoffset, yoffset, width, height, format, type, pixels);
}

void Texture::allocateStoarge3D(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth)
{
    glTextureStorage3D(m_id, levels, internalFormat, width, height, depth);
}

void Texture::uploadTexture3D(GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
    GLenum format, GLenum type, const void* pixels)
{
    glTextureSubImage3D(m_id, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void Texture::use(GLenum target)
{
    glBindTexture(target, m_id);
}

void Texture::useAsTexture(GLuint unit)
{
    glBindTextureUnit(unit, m_id);
}

void Texture::useAsImage(GLuint unit, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    glBindImageTexture(unit, m_id, level, layered, layer, access, format);
}

void Texture::loadFromFile(const char* filename)
{
    int width, height;
    GLvoid* data;

    FREE_IMAGE_FORMAT tx_file_format = FreeImage_GetFileType(filename, 0);

    // assume everything is fine with reading texture from file: no error checking
    FIBITMAP* tx_pixmap = FreeImage_Load(tx_file_format, filename);
    int tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

    std::cout << " * A " << tx_bits_per_pixel
              << "-bit texture was read from " << filename << ".\n";

    FIBITMAP* tx_pixmap_32;
    if (tx_bits_per_pixel == 32)
        tx_pixmap_32 = tx_pixmap;
    else {
        std::cout << " * Converting texture from " << tx_bits_per_pixel
                  << " bits to 32 bits...\n";
        tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
    }

    width = FreeImage_GetWidth(tx_pixmap_32);
    height = FreeImage_GetHeight(tx_pixmap_32);
    data = FreeImage_GetBits(tx_pixmap_32);

    allocateStorage2D(1, GL_RGBA8, width, height);
    uploadTexture2D(0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);

    std::cout << " * Loaded " << width << "x" << height
              << " RGBA texture into graphics memory.\n\n";

    FreeImage_Unload(tx_pixmap_32);
    if (tx_bits_per_pixel != 32) {
        FreeImage_Unload(tx_pixmap);
    }
}

void Texture::generateMipmap()
{
    glGenerateTextureMipmap(m_id);
}

Texture::Texture()
    : m_id(0)
{
}

Texture::Texture(GLenum target)
{
    glCreateTextures(target, 1, &m_id);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0))
{
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    glDeleteTextures(1, &m_id);
    m_id = std::exchange(other.m_id, 0);
    return *this;
}
}
