#pragma once
#include "pch.h"
namespace common
{
    inline float RandomNumber(float _min, float _max)
    {
        // Seed with a real random value, if available
        static std::random_device rd;
        static std::mt19937 gen(rd());  // Standard mersenne_twister_engine
        std::uniform_real_distribution<> realDist(_min, _max);
        double randomDouble = realDist(gen);
        return static_cast<float>(randomDouble);
    }

    inline DirectX::XMFLOAT3 RandomNormalizedVector()
    {
        DirectX::XMFLOAT3 v0(
            RandomNumber(-1.0f, 1.0f),
            RandomNumber(-1.0f, 1.0f),
            RandomNumber(-1.0f, 1.0f)
        );
        DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&v0);
        v1 = DirectX::XMVector3Normalize(v1);
        DirectX::XMStoreFloat3(&v0, v1);
        return v0;
    }
}