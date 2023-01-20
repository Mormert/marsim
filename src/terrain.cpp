// MIT License

// Copyright (c) 2023 Johan Lind, Ermias Tewolde

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "terrain.h"
#include "glad/gl.h"

#include <stb_image.h>

#include <iostream>

Terrain::Terrain(const std::string &gaussianImagePath)
{

    stbi_set_flip_vertically_on_load(false);

    unsigned char *image_data{nullptr};
    int channels = 0;
    image_data = stbi_load(gaussianImagePath.c_str(), &width, &height, &channels, 1);

    if (image_data == nullptr) {
        std::cerr << "Failed loading image!" << std::endl;
        return;
    }

    map.resize(width * height);

    std::copy(image_data, image_data + width * height, map.data());

    stbi_image_free(image_data);

    generateTexture(gaussianImagePath);
}

unsigned char
Terrain::getHeight(int x, int y)
{
    if (x < 0 || x >= width) {
        return 0;
    }

    if (y < 0 || y >= height) {
        return 0;
    }

    return map[y * width + x];
}

void
Terrain::generateTexture(const std::string &gaussianImagePath)
{

    glGenTextures(1, &terrainTextureID);
    int nrChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(gaussianImagePath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if (nrChannels == 1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            format = GL_RED;
        } else if (nrChannels == 3) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            format = GL_RGB;
        }
        else if (nrChannels == 4) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, terrainTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        std::cout << "Texture failed to load at path: " << gaussianImagePath << std::endl;
    }

    stbi_image_free(data);
}
unsigned int
Terrain::getTextureID()
{
    return terrainTextureID;
}
Terrain::~Terrain()
{
    if (terrainTextureID) {
        glDeleteTextures(1, &terrainTextureID);
    }
}
int
Terrain::getTextureHeight()
{
    return height;
}
int
Terrain::getTextureWidth()
{
    return width;
}
