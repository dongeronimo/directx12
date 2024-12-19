#pragma once
#include "pch.h"

namespace skinning
{
    /// <summary>
    /// A bone from the character. 
    /// It may be possible to generalize this bone component to a generic Transform component but for now
    /// i'll use just the bones case
    /// </summary>
    struct Bone {
        int id;
        std::string name;         // Bone name
        DirectX::XMMATRIX offsetMatrix;
        DirectX::XMFLOAT3 localPosition;
        DirectX::XMFLOAT3 localScale;
        DirectX::XMVECTOR localRotation;
        // Helper to calculate the local transformation matrix
        DirectX::XMMATRIX GetLocalTransform() const {
            using namespace DirectX;
            return XMMatrixScaling(localScale.x, localScale.y, localScale.z) *
                XMMatrixRotationQuaternion(localRotation) *
                XMMatrixTranslation(localPosition.x, localPosition.y, localPosition.z);
        }
    };
    /// <summary>
    /// A hierarchy of bones. It always go from parent to children.
    /// It may be possible to generalize this to a transform hierarchy but for now i'll use just the bone case
    /// </summary>
    struct BoneHierarchy {
        entt::entity parent;      // Parent bone entity
    };
    /// <summary>
    /// Keyframe of an animation. I don't think you should use it as a component, but as data of the animation
    /// component
    /// </summary>
    struct Keyframe
    {
        float time;                      // Time of the keyframe (in seconds)
        DirectX::XMFLOAT3 position;      // Position at this keyframe
        DirectX::XMFLOAT3 scale;         // Scale at this keyframe
        DirectX::XMFLOAT4 rotation;      // Rotation (quaternion) at this keyframe
    };
    /// <summary>
    /// Animation component. Animation is per-bone, so the entity should have a Bone, BoneHierarchy and Animation
    /// </summary>
    struct Animation {
        float duration;  // Total duration of the animation in ticks
        float ticksPerSecond;  // Playback speed in ticks per second (default 25 if undefined)
        std::vector<Keyframe> keyframes; // List of keyframes for this bone
        float currentTime;               // Current position in the animation (in seconds)
        bool looping;                    // Whether the animation should loop
        // Constructor
        Animation(bool loop = true) : currentTime(0.0f), looping(loop) {}
        // Reset the animation state
        void Reset() {
            currentTime = 0.0f;
        }
        // Update animation time (call this per frame)
        void Update(float deltaTime) {
            currentTime += deltaTime;
            if (looping && !keyframes.empty()) {
                float duration = keyframes.back().time; // Last keyframe's time
                if (currentTime > duration) {
                    currentTime = fmod(currentTime, duration);
                }
            }
        }
        // Get the interpolated transform at the current time
        DirectX::XMMATRIX GetTransform() const {
            if (keyframes.empty()) {
                return DirectX::XMMatrixIdentity(); // No keyframes, return identity
            }
            // Find the two keyframes surrounding the current time
            const Keyframe* prevKeyframe = nullptr;
            const Keyframe* nextKeyframe = nullptr;
            for (size_t i = 0; i < keyframes.size(); ++i) {
                if (keyframes[i].time >= currentTime) {
                    nextKeyframe = &keyframes[i];
                    if (i > 0) {
                        prevKeyframe = &keyframes[i - 1];
                    }
                    else {
                        prevKeyframe = &keyframes[0];
                    }
                    break;
                }
            }
            if (!prevKeyframe || !nextKeyframe) {
                return DirectX::XMMatrixIdentity(); // Edge case: no valid keyframes
            }
            // Interpolation factor (time between the two keyframes)
            float t = (currentTime - prevKeyframe->time) /
                (nextKeyframe->time - prevKeyframe->time);
            // Interpolate position, scale, and rotation
            DirectX::XMVECTOR pos = DirectX::XMVectorLerp(
                DirectX::XMLoadFloat3(&prevKeyframe->position),
                DirectX::XMLoadFloat3(&nextKeyframe->position), t);
            DirectX::XMVECTOR scale = DirectX::XMVectorLerp(
                DirectX::XMLoadFloat3(&prevKeyframe->scale),
                DirectX::XMLoadFloat3(&nextKeyframe->scale), t);
            DirectX::XMVECTOR rot = DirectX::XMQuaternionSlerp(
                DirectX::XMLoadFloat4(&prevKeyframe->rotation),
                DirectX::XMLoadFloat4(&nextKeyframe->rotation), t);
            // Construct the transform matrix
            return DirectX::XMMatrixScalingFromVector(scale) *
                DirectX::XMMatrixRotationQuaternion(rot) *
                DirectX::XMMatrixTranslationFromVector(pos);
        }
    };

    // Utility function to update a Bone component from an Animation component
    static void UpdateBoneFromAnimation(entt::registry& registry, entt::entity entity) {
        // Ensure the entity has both Animation and Bone components
        if (!registry.all_of<Animation, Bone>(entity)) {
            return;
        }

        auto& animation = registry.get<Animation>(entity);
        auto& bone = registry.get<Bone>(entity);

        // Get the interpolated transform from the Animation component
        DirectX::XMMATRIX transform = animation.GetTransform();

        // Decompose the matrix into position, scale, and rotation
        DirectX::XMVECTOR scale, rotation, position;
        DirectX::XMMatrixDecompose(&scale, &rotation, &position, transform);

        // Update the Bone component
        bone.offsetMatrix = transform; // Store the full matrix
        DirectX::XMStoreFloat3(&bone.localPosition, position);   // Update position
        DirectX::XMStoreFloat3(&bone.localScale, scale);         // Update scale
        bone.localRotation = rotation;                          // Update rotation as XMVECTOR
    }
}