#include "TextureUtils.hxx"

#include "../../vendor/stb_image.h"
#include "../../vendor/glad.h"

unsigned int GL::Utils::loadTexture2D(const std::string &texturePath)
{
  unsigned int textureDescriptor;

  glGenTextures(1, &textureDescriptor);
  glBindTexture(GL_TEXTURE_2D, textureDescriptor);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int textureWidth, textureHeight, nrChannels;

  unsigned char *textureData = stbi_load(
    texturePath.c_str(),
    &textureWidth,
    &textureHeight,
    &nrChannels,
    0);

  if (textureData) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    // TODO: logging
  }

  stbi_image_free(textureData);
  return textureDescriptor;
}

unsigned int GL::Utils::releaseTexture2D(unsigned int *textureDescriptor)
{
  glDeleteTextures(1, textureDescriptor);
  glBindTexture(GL_TEXTURE_2D, 0);
}
