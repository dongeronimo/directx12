#include "pch.h"
#include "image_load.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_MSC_SECURE_CRT
std::shared_ptr<common::ImageData> common::ImageData::LoadFromFile(const std::string path)
{
    int texChannels;
    int w; int h;
    stbi_uc* bytes = stbi_load(path.c_str(),
        &w, &h, &texChannels, STBI_rgb_alpha);
    uint64_t size = w * h * 4;
    std::vector<uint8_t> pixels(size);
    memcpy(pixels.data(), bytes, size);
    stbi_image_free(bytes);
    return std::make_shared<common::ImageData>(w, h, pixels, size, path);
}

common::ImageData::ImageData(int w, int h, std::vector<uint8_t> data, uint64_t size, std::string name)
    :Width(w), Height(h), Data(data), Size(size), Name(name)
{
}
