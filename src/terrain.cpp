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
#include "blur.h"
#include "glad/gl.h"

#include <stb_image.h>
#include <stb_image_write.h>

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
        } else if (nrChannels == 4) {
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

void
Terrain::GenerateGaussianImageFromHardEdgeImage(const std::string &hardImagePath,
                                                const std::string &gaussianImageOutputPath,
                                                float sigma)
{

    stbi_set_flip_vertically_on_load(false);

    int width, height;

    unsigned char *image_data{nullptr};
    int channels = 0;
    image_data = stbi_load(hardImagePath.c_str(), &width, &height, &channels, 0);

    if (image_data == nullptr) {
        std::cerr << "Failed loading image: " << hardImagePath << "!" << std::endl;
        return;
    }

    if (channels < 3) {
        std::cerr << "Input images must be RGB images." << std::endl;
    }

    // copy data
    int size = width * height;

    // output channels r,g,b
    float *newb = new float[size];
    float *newg = new float[size];
    float *newr = new float[size];

    // input channels r,g,b
    float *oldb = new float[size];
    float *oldg = new float[size];
    float *oldr = new float[size];

    // channels copy r,g,b
    for (int i = 0; i < size; ++i) {
        oldb[i] = image_data[channels * i + 0] / 255.f;
        oldg[i] = image_data[channels * i + 1] / 255.f;
        oldr[i] = image_data[channels * i + 2] / 255.f;
    }

    // input channels r,g,b
    float *origb = new float[size];
    float *origg = new float[size];
    float *origr = new float[size];

    // channels copy r,g,b
    for (int i = 0; i < size; ++i) {
        origb[i] = image_data[channels * i + 0] / 255.f;
        origg[i] = image_data[channels * i + 1] / 255.f;
        origr[i] = image_data[channels * i + 2] / 255.f;
    }

    Blur::fast_gaussian_blur(oldb, newb, width, height, sigma);
    Blur::fast_gaussian_blur(oldg, newg, width, height, sigma);
    Blur::fast_gaussian_blur(oldr, newr, width, height, sigma);

    // channels copy r,g,b
    for (int i = 0; i < size; ++i) {
        image_data[channels * i + 0] = (unsigned char)std::min(255.f, std::max(0.f, 255.f * newb[i]));
        image_data[channels * i + 1] = (unsigned char)std::min(255.f, std::max(0.f, 255.f * newg[i]));
        image_data[channels * i + 2] = (unsigned char)std::min(255.f, std::max(0.f, 255.f * newr[i]));

        // Add original occlusion map on top of blurred map
        image_data[channels * i + 0] = (unsigned char)std::min(255.f, std::max(0.f, image_data[channels * i + 0] + 255.f * origb[i]));
        image_data[channels * i + 1] = (unsigned char)std::min(255.f, std::max(0.f, image_data[channels * i + 1] + 255.f * origg[i]));
        image_data[channels * i + 2] = (unsigned char)std::min(255.f, std::max(0.f, image_data[channels * i + 2] + 255.f * origr[i]));
    }

    // save
    std::string file(gaussianImageOutputPath);
    std::string ext = file.substr(file.size() - 3);
    if (ext == "bmp")
        stbi_write_bmp(gaussianImageOutputPath.c_str(), width, height, channels, image_data);
    else if (ext == "jpg")
        stbi_write_jpg(gaussianImageOutputPath.c_str(), width, height, channels, image_data, 90);
    else {
        if (ext != "png") {
            std::cerr << "format '" << ext << "' not supported writing default .png" << std::endl;
            file = file.substr(0, file.size() - 4) + std::string(".png");
        }
        stbi_write_png(file.c_str(), width, height, channels, image_data, channels * width);
    }
    stbi_image_free(image_data);

    // clean memory
    delete[] newr;
    delete[] newb;
    delete[] newg;
    delete[] oldr;
    delete[] oldb;
    delete[] oldg;
    delete[] origb;
    delete[] origg;
    delete[] origr;
}
