#include "skinned_mesh.h"
DirectX::XMFLOAT3 _aiVec3ToDirectXVector(aiVector3D& vec)
{
	DirectX::XMFLOAT3 v(vec.x, vec.y, vec.z);
	return v;
}
DirectX::XMFLOAT2 _removeZ(DirectX::XMFLOAT3 vec)
{
	DirectX::XMFLOAT2 v(vec.x, vec.y);
	return v;
}
namespace skinning::io
{
	const aiScene* LoadScene(const std::string& filename)
	{
		Assimp::Importer importer;
		importer.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights);
		const aiScene* scene = importer.GetOrphanedScene();
		assert(scene);
		return scene;
	}

	aiNode* FindArmatureRoot(aiNode* rootNode, const aiScene* scene)
	{
		// Traverse the node hierarchy to find the armature root
		for (unsigned int i = 0; i < rootNode->mNumChildren; i++) {
			aiNode* child = rootNode->mChildren[i];
			// Check if this child references any bones in the meshes
			for (unsigned int j = 0; j < scene->mNumMeshes; j++) {
				aiMesh* mesh = scene->mMeshes[j];
				for (unsigned int k = 0; k < mesh->mNumBones; k++) {
					if (std::string(mesh->mBones[k]->mName.C_Str()) == 
						std::string(child->mName.C_Str())) {
						return child; // Found the armature root
					}
				}
			}
			// Recursively check children
			aiNode* result = skinning::io::FindArmatureRoot(child, scene);
			if (result) {
				return result;
			}
		}
		return nullptr; // No armature root found
	}

	/// <summary>
	/// is this node really a bone? If it is it'll exist in a list of bones of a mesh in the scene
	/// </summary>
	/// <param name="node"></param>
	/// <param name="scene"></param>
	/// <returns></returns>
	bool IsNodeReallyBone(const aiNode* node, const aiScene* scene)
	{
		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* m = scene->mMeshes[i];
			for (auto j = 0; j < m->mNumBones; j++)
			{
				const std::string meshBoneName = m->mBones[j]->mName.C_Str();
				const std::string myBoneName = node->mName.C_Str();
				if (meshBoneName == myBoneName)
					return true;
			}
		}
		return false;
	}
	/// <summary>
	/// Find the aiBone with the given name.
	/// </summary>
	/// <param name="name"></param>
	/// <param name="scene"></param>
	/// <returns></returns>
	aiBone* GetBone(const std::string& myBoneName, const aiScene* scene)
	{
		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* m = scene->mMeshes[i];
			for (auto j = 0; j < m->mNumBones; j++)
			{
				const std::string meshBoneName = m->mBones[j]->mName.C_Str();
				if (meshBoneName == myBoneName)
					return m->mBones[j];
			}
		}
		return nullptr;
	}
	/// <summary>
	/// load the offset matrix to a directxmath matrix. The offset matrix is the T-Pose matrix.
	/// </summary>
	/// <param name="name"></param>
	/// <param name="scene"></param>
	/// <returns></returns>
	DirectX::XMMATRIX CreateOffsetMatrix(std::string name, const aiScene* scene)
	{
		aiBone* aiBoneData = GetBone(name, scene);
		aiMatrix4x4 aiOffsetMatrix = aiBoneData->mOffsetMatrix.Transpose();
		DirectX::XMMATRIX transform = DirectX::XMMATRIX(
			aiOffsetMatrix.a1, aiOffsetMatrix.b1, aiOffsetMatrix.c1, aiOffsetMatrix.d1,
			aiOffsetMatrix.a2, aiOffsetMatrix.b2, aiOffsetMatrix.c2, aiOffsetMatrix.d2,
			aiOffsetMatrix.a3, aiOffsetMatrix.b3, aiOffsetMatrix.c3, aiOffsetMatrix.d3,
			aiOffsetMatrix.a4, aiOffsetMatrix.b4, aiOffsetMatrix.c4, aiOffsetMatrix.d4
		);
		return transform;
	}
	void DecomposeLocalTransform(Bone& boneComponent, const DirectX::XMMATRIX& localTransform)
	{
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
	}
	void LoadBoneHierarchy(aiNode* node, 
		const aiScene* scene, 
		std::vector<entt::entity>& boneEntitiesList,
		entt::entity parentEntity,
		entt::registry& registry)
	{
		//is this really a bone?
		if (!IsNodeReallyBone(node, scene))
		{
			return; //it isn't, halt the process
		}
		const std::string name = node->mName.C_Str();
		aiMatrix4x4 transform = node->mTransformation.Transpose();
		DirectX::XMMATRIX localTransform = DirectX::XMMatrixIdentity();
		localTransform = DirectX::XMMATRIX(
			transform.a1, transform.a2, transform.a3, transform.a4,
			transform.b1, transform.b2, transform.b3, transform.b4,
			transform.c1, transform.c2, transform.c3, transform.c4,
			transform.d1, transform.d2, transform.d3, transform.d4
		);
		Bone boneComponent;
		boneComponent.id = boneEntitiesList.size();
		boneComponent.name = name;
		boneComponent.offsetMatrix = CreateOffsetMatrix(name, scene);
		DecomposeLocalTransform(boneComponent, localTransform);
		entt::entity currentEntity = registry.create();
		registry.emplace<Bone>(currentEntity, boneComponent);
		if (parentEntity != entt::null)
		{
			BoneHierarchy hierarchyComponent = { parentEntity };
			registry.emplace<BoneHierarchy>(currentEntity, hierarchyComponent);
		}
		boneEntitiesList.push_back(currentEntity);
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			LoadBoneHierarchy(node->mChildren[i], scene,  boneEntitiesList, currentEntity,registry);
		}
	}


//	void LoadBone(aiNode* node, entt::registry& registry, entt::entity parentEntity, const aiScene* scene, 
//		std::unordered_map<std::string, entt::entity>& boneMap, int id)
//	{
//		//is this really a bone?
//		if (!IsNodeReallyBone(node, scene))
//		{
//			return; //it isn't, halt the process
//		}
//		const std::string name = node->mName.C_Str();
//		// Create a new entity for this node and get its transform
//		entt::entity currentEntity = registry.create();
//		aiMatrix4x4 transform = node->mTransformation.Transpose();
//		DirectX::XMMATRIX localTransform = DirectX::XMMatrixIdentity();
//		localTransform = DirectX::XMMATRIX(
//			transform.a1, transform.a2, transform.a3, transform.a4,
//			transform.b1, transform.b2, transform.b3, transform.b4,
//			transform.c1, transform.c2, transform.c3, transform.c4,
//			transform.d1, transform.d2, transform.d3, transform.d4
//		);
//		//create the bone component
//		Bone boneComponent;
//		boneComponent.id = id;
//		id++;
//		boneComponent.name = name;
//		//get the offset matrix
//		boneComponent.offsetMatrix = CreateOffsetMatrix(name, scene);
//		//get the local transform as decomposed values
//		DecomposeLocalTransform(boneComponent, localTransform);
//		aiBone* aiBoneData = GetBone(name, scene);
//		//the bone is ready, let us add it to the entity and map the hierarchy too
//		registry.emplace<Bone>(currentEntity, boneComponent);
//		BoneHierarchy hierarchyComponent = { parentEntity };
//		registry.emplace<BoneHierarchy>(currentEntity, hierarchyComponent);
//		//add the entity, that has a bone component, to the bone map
//		boneMap.insert({ name, currentEntity });
//		//go to the next bone
//		for (unsigned int i = 0; i < node->mNumChildren; i++) {
//			LoadBone(node->mChildren[i], registry, currentEntity, scene, boneMap);
//		}
//	}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Helper function to process a single mesh
	std::shared_ptr<MeshData> ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
		std::shared_ptr<MeshData> meshData;
		meshData->name = mesh->mName.C_Str();

		// Initialize vertex data
		meshData->vertices.resize(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			auto& vertex = meshData->vertices[i];
			vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
			if (mesh->mTextureCoords[0]) { // Check for UV coordinates
				vertex.uv = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			}
			else {
				vertex.uv = { 0.0f, 0.0f };
			}
			for (int j = 0; j < 4; ++j) {
				vertex.boneWeights[j] = { 0, 0.0f }; // Initialize bone weights
			}
		}
		// Copy indices
		meshData->indices.reserve(mesh->mNumFaces * 3); // Assuming all faces are triangles
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j) {
				meshData->indices.push_back(static_cast<uint16_t>(face.mIndices[j]));
			}
		}

		return meshData;
	}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
	std::vector<std::shared_ptr<MeshData>> LoadMeshes(const aiScene* scene)
	{
		std::vector<std::shared_ptr<MeshData>> result;
		for (auto i = 0; i < scene->mNumMeshes; i++)
		{
			result.push_back(ProcessMesh(scene->mMeshes[i], scene));
		}
		return result;
	}
}