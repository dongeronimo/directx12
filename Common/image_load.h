#pragma once
#include "pch.h"
namespace common
{
	class ImageData
	{
	public:
		static std::shared_ptr<ImageData> LoadFromFile(const std::string path);
		const int Width;
		const int Height;
		const std::vector<uint8_t> Data;
		const uint64_t Size;
		const std::string Name;
	private:
		ImageData(int w, int h, std::vector<uint8_t> data, uint64_t size, std::string Name);
		
	};
}

