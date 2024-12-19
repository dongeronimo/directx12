//TODO: create the components for transform hierarchy
//TODO: load the bones as 
#include "pch.h"
#include "../common/swapchain.h"
#include "../Common/offscreen_rtv.h"
#include "dx_context.h"
#include "../Common/mesh_load.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "entities.h"
#include "entt/entt.hpp"
#include "skinned_mesh.h"
#include <iostream>
using Microsoft::WRL::ComPtr;
using namespace skinning;
constexpr int W = 800;
constexpr int H = 600;

std::vector<std::shared_ptr<common::Mesh>> gMeshes;

//void LoadCapoeira(skinning::DxContext& context, entt::registry& registry);
int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"colored_triangle_t", L"Colored Triangle", W, H);
	window.Show();

	std::unique_ptr<skinning::DxContext> context = std::make_unique<skinning::DxContext>();
	//Swapchain is to show on screen.
	common::Swapchain swapchain(window.Hwnd(), W, H, *context);
	//the scene will not be rendered to the screen, but to this offscreen render target view.
	common::OffscreenRTV offscreenRTV(W, H, *context, DXGI_FORMAT_R8G8B8A8_UNORM);
	//TODO: offscreen scene render pass
	//TODO: presentation render pass
	//TODO: create the world

	entt::registry worldRegistry;
	//load the scene
	const aiScene* scene = skinning::io::LoadScene("assets/capoeira.glb");
	//find the armature root node
	aiNode* armatureRoot = skinning::io::FindArmatureRoot(scene->mRootNode, scene);
	//TODO: load bones
	std::vector<entt::entity> entitiesWithBones; //all these entities will have bone components and all but the root will have boneHierarchy
	skinning::io::LoadBoneHierarchy(armatureRoot, scene, entitiesWithBones, entt::null, worldRegistry);
	for (entt::entity& e : entitiesWithBones)
	{
		skinning::Bone& bone = worldRegistry.get<skinning::Bone>(e);
		std::cout << "entity = " << static_cast<uint32_t>(e) << std::endl;
		std::cout << "bone=" << bone.name << ", id=" << bone.id << std::endl;
		if (worldRegistry.any_of<skinning::BoneHierarchy>(e))
		{
			skinning::BoneHierarchy& hierarchy = worldRegistry.get<skinning::BoneHierarchy>(e);
			std::cout << "parent=" << static_cast<uint32_t>(hierarchy.parent) << std::endl;
		}
		else {
			std::cout << "is root" << std::endl;
		}
	}
	//TODO: load mesh - the meshes will lack bone weights they'll have to be set later
	std::vector<std::shared_ptr<skinning::io::MeshData>> meshes = skinning::io::LoadMeshes(scene);
	//TODO: set bone weights
	for (auto mesh : meshes)
	{
		//for each mesh vertex V evaluate each bone B if the bone has 
	}



	//std::unordered_map<std::string, entt::entity> boneMap;
	//skinning::io::LoadBone(armatureRoot, worldRegistry, entt::null, scene, boneMap);
	////TODO: load mesh - the meshes will lack bone weights they'll have to be set later
	//std::vector<std::shared_ptr<skinning::io::MeshData>> meshes = skinning::io::LoadMeshes(scene);
	//std::vector<std::string> boneNames;
	//// Extract keys
	//for (const auto& pair : boneMap) {
	//	boneNames.push_back(pair.first);
	//}
	////TODO: distribute bone influence and weights to the vertexes
	//skinning::io::ApplyWeightsToVertexes(meshes, boneNames, scene);
	//TODO: load animations
	//TODO: create the vertex buffer and index buffer
	//...
	//done, delete the resources created by the load process
	delete scene;
	//TODO: finish creating the game object
	//TODO: camera buffer
	//TODO:model matrix buffer, holds the model matrix for each game object
	//TODO:bones matrix buffer, holds the bones matrices for each game object
	//TODO:instance id buffer, holds the instance ids
	window.mOnIdle = [](){
		//TODO: advance animation based on delta time and update the bones matrix
		//TODO: for each game object update it's model matrix data
	};

	window.mOnResize = [](int w, int h) {
	};

	window.MainLoop();
}
/// <summary>
/// For each node create bone data and hierarchy
/// </summary>
/// <param name="node"></param>
/// <param name="registry"></param>
/// <param name="parentEntity"></param>
void ProcessNode(aiNode* node, entt::registry& registry, entt::entity parentEntity, 
	const aiScene* scene, std::unordered_map<std::string, entt::entity>& boneMap);


void LoadAnimations(const aiScene* scene, entt::registry& registry,
	const std::unordered_map<std::string, entt::entity>& boneMap);
/// <summary>
/// For now the name is wrong since it'll only load the skinned mesh, not other assets
//TODO: move skinned mesh loading somewhere else
/// </summary>
/// <param name="context"></param>
/// <param name="registry"></param>
void LoadCapoeira(skinning::DxContext& context, entt::registry& registry)
{
	//load the scene
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("assets/capoeira.glb", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights);
	//load the bones
	aiNode* armatureRoot = skinning::io::FindArmatureRoot(scene->mRootNode, scene);
	assert(armatureRoot);
	std::unordered_map<std::string, entt::entity> boneMap;
	ProcessNode(armatureRoot, registry, entt::null, scene, boneMap);
	LoadAnimations(scene, registry, boneMap);
}

void ProcessNode(aiNode* node, entt::registry& registry, entt::entity parentEntity, 
	const aiScene* scene, std::unordered_map<std::string, entt::entity>& boneMap)
{
	using namespace skinning;
	// Create a new entity for this node
	entt::entity currentEntity = registry.create();
	// Load the local transformation matrix from the Assimp node
	aiMatrix4x4 transform = node->mTransformation.Transpose();
	// Convert Assimp's matrix to DirectX::XMMATRIX
	DirectX::XMMATRIX localTransform = DirectX::XMMatrixIdentity();
	localTransform = DirectX::XMMATRIX(
		transform.a1, transform.b1, transform.c1, transform.d1,
		transform.a2, transform.b2, transform.c2, transform.d2,
		transform.a3, transform.b3, transform.c3, transform.d3,
		transform.a4, transform.b4, transform.c4, transform.d4
	);
	// Add Bone data (set initial local transform)
	Bone boneComponent;
	boneComponent.name = node->mName.C_Str();
	
	std::string nodeName = node->mName.C_Str();
	boneMap[nodeName] = currentEntity;

	boneComponent.offsetMatrix = DirectX::XMMatrixIdentity(); // Offset will come from aiMesh::mBones later
	boneComponent.localPosition = { 0.0f, 0.0f, 0.0f };
	boneComponent.localScale = { 1.0f, 1.0f, 1.0f };
	boneComponent.localRotation = DirectX::XMQuaternionIdentity();
	// Decompose the local transform into position, rotation, and scale
	DirectX::XMVECTOR scale, rotation, translation;
	DirectX::XMMatrixDecompose(&scale, &rotation, &translation, localTransform);
	boneComponent.localPosition = {
		DirectX::XMVectorGetX(translation),
		DirectX::XMVectorGetY(translation),
		DirectX::XMVectorGetZ(translation)
	};
	boneComponent.localScale = {
		DirectX::XMVectorGetX(scale),
		DirectX::XMVectorGetY(scale),
		DirectX::XMVectorGetZ(scale)
	};
	boneComponent.localRotation = rotation;
	registry.emplace<Bone>(currentEntity, boneComponent);
	// Add Hierarchy data
	BoneHierarchy hierarchyComponent = { parentEntity };
	registry.emplace<BoneHierarchy>(currentEntity, hierarchyComponent);
	// Process child nodes recursively
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessNode(node->mChildren[i], registry, currentEntity, scene, boneMap);
	}
}
aiNode* FindArmatureRoot(aiNode* rootNode, const aiScene* scene) {
	// Traverse the node hierarchy to find the armature root
	for (unsigned int i = 0; i < rootNode->mNumChildren; i++) {
		aiNode* child = rootNode->mChildren[i];

		// Check if this child references any bones in the meshes
		for (unsigned int j = 0; j < scene->mNumMeshes; j++) {
			aiMesh* mesh = scene->mMeshes[j];
			for (unsigned int k = 0; k < mesh->mNumBones; k++) {
				if (std::string(mesh->mBones[k]->mName.C_Str()) == std::string(child->mName.C_Str())) {
					return child; // Found the armature root
				}
			}
		}

		// Recursively check children
		aiNode* result = FindArmatureRoot(child, scene);
		if (result) {
			return result;
		}
	}
	return nullptr; // No armature root found
}

// Utility function to load all animations and assign them to entities
void LoadAnimations(const aiScene* scene, entt::registry& registry,
	const std::unordered_map<std::string, entt::entity>& boneMap) {
	using namespace skinning;
	if (!scene->HasAnimations()) {
		return; // No animations in the scene
	}

	// Iterate over all animations in the file
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
		const aiAnimation* aiAnim = scene->mAnimations[i];

		// Process each bone's animation channel
		for (unsigned int j = 0; j < aiAnim->mNumChannels; ++j) {
			const aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
			std::string boneName = aiNodeAnim->mNodeName.C_Str();

			// Find the corresponding bone entity in the ECS
			auto it = boneMap.find(boneName);
			if (it == boneMap.end()) {
				// Bone not found in the ECS registry (skip it)
				continue;
			}

			entt::entity boneEntity = it->second;

			// Retrieve or create the Animation component for this bone
			auto& animation = registry.emplace_or_replace<skinning::Animation>(boneEntity);

			// Set global animation properties
			animation.duration = static_cast<float>(aiAnim->mDuration);
			animation.ticksPerSecond = static_cast<float>(aiAnim->mTicksPerSecond != 0.0 ? aiAnim->mTicksPerSecond : 25.0);

			// Process position keyframes
			for (unsigned int k = 0; k < aiNodeAnim->mNumPositionKeys; ++k) {
				const aiVectorKey& key = aiNodeAnim->mPositionKeys[k];
				animation.keyframes.push_back({
					static_cast<float>(key.mTime),
					DirectX::XMFLOAT3(key.mValue.x, key.mValue.y, key.mValue.z), // Position
					DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),                         // Default scale
					DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)                  // Default rotation
					});
			}

			// Process rotation keyframes
			for (unsigned int k = 0; k < aiNodeAnim->mNumRotationKeys; ++k) {
				const aiQuatKey& key = aiNodeAnim->mRotationKeys[k];
				DirectX::XMFLOAT4 rotation(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);

				// Match keyframe time to an existing keyframe or add a new one
				auto it = std::find_if(animation.keyframes.begin(), animation.keyframes.end(),
					[&key](const Keyframe& kf) {
						return std::abs(kf.time - static_cast<float>(key.mTime)) < 0.001f;
					});
				if (it != animation.keyframes.end()) {
					it->rotation = rotation; // Update existing keyframe
				}
				else {
					animation.keyframes.push_back({
						static_cast<float>(key.mTime),
						DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), // Default position
						DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), // Default scale
						rotation                             // Rotation
						});
				}
			}

			// Process scaling keyframes
			for (unsigned int k = 0; k < aiNodeAnim->mNumScalingKeys; ++k) {
				const aiVectorKey& key = aiNodeAnim->mScalingKeys[k];
				DirectX::XMFLOAT3 scale(key.mValue.x, key.mValue.y, key.mValue.z);

				// Match keyframe time to an existing keyframe or add a new one
				auto it = std::find_if(animation.keyframes.begin(), animation.keyframes.end(),
					[&key](const Keyframe& kf) {
						return std::abs(kf.time - static_cast<float>(key.mTime)) < 0.001f;
					});
				if (it != animation.keyframes.end()) {
					it->scale = scale; // Update existing keyframe
				}
				else {
					animation.keyframes.push_back({
						static_cast<float>(key.mTime),
						DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), // Default position
						scale,                               // Scale
						DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) // Default rotation
						});
				}
			}

			// Sort keyframes by time to ensure smooth interpolation
			std::sort(animation.keyframes.begin(), animation.keyframes.end(),
				[](const Keyframe& a, const Keyframe& b) {
					return a.time < b.time;
				});
		}
	}
}