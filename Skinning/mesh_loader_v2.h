#pragma once
#include "pch.h"
#include <tiny_gltf.h>
namespace skinning
{
    /// <summary>
    /// describes the vertex layout.
    /// </summary>
    struct Vertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texCoord;
        std::vector<uint8_t> jointIDs; // Bone IDs
        std::vector<float> weights;    // Weights
    };
    //TODO: Deprecated
    class SkinnedMeshPrefab
    {
    public:
        const std::map<std::string, std::vector<skinning::Vertex>> meshes;
        const std::unordered_set<int> armatureNodesIds;
        const std::unordered_map<int, std::unordered_set<int>> hierarchy;
        SkinnedMeshPrefab(std::map<std::string, std::vector<skinning::Vertex>> _meshes,
            std::unordered_set<int> _armatureNodesIds,
            std::unordered_map<int, std::unordered_set<int>> _hierarchy)
            :meshes(_meshes), armatureNodesIds(_armatureNodesIds), hierarchy(_hierarchy) {}

    };
    /// <summary>
    /// A mesh component. Holds the vertex and index buffers
    /// </summary>
    struct Mesh
    {
        std::string name;
        Microsoft::WRL::ComPtr<ID3D12Resource> mVertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
        Microsoft::WRL::ComPtr<ID3D12Resource> mIndexBuffer;
        D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
        const int mNumberOfIndices;
    };
    /// <summary>
    /// Marks the entity as a prefab. Prefabs are identified by name,
    /// the name must be unique (but at the moment there's no mechanism
    /// to validate uniqueness).
    /// TODO: create assertion to validade if the name is alredy in use
    /// </summary>
    struct Prefab
    {
        std::string name;
    };
    /// <summary>
    /// Describes a bone joint. Maybe can be used as a generic transform object?
    /// </summary>
    struct BoneJoint
    {
        DirectX::XMVECTOR position;
        DirectX::XMVECTOR scale;
        DirectX::XMVECTOR rotation;
    };
    /// <summary>
    /// I need the T-Pose matrix because the final world position of a vertex in a 
    /// skin takes that into account
    /// </summary>
    struct TPoseMatrix
    {
        DirectX::XMMATRIX offsetMatrix;
    };
    /// <summary>
    /// Holds the parent.
    /// </summary>
    struct Parent
    {
        entt::entity parent = entt::null;
    };
    /// <summary>
    /// Holds a list of children
    /// </summary>
    struct Children
    {
        std::vector<entt::entity> children;
    };
    /// <summary>
    /// The id of the bone, used by the vertex buffer (jointID)
    /// </summary>
    struct BoneId
    {
        int nodeID;
    };

}
namespace skinning::io
{
    /// <summary>
    /// Appends the necessary prefixes to the file name.
    /// </summary>
    /// <param name="file"></param>
    /// <returns></returns>
    std::string AssembleFilePath(std::string file);
    /// <summary>
    /// Loads the skin from the file and creates entities from it.
    /// It'll create the bone hierarchy entities and the meshes entities. 
    /// All entities will have the Prefab component. Prefab.Name must be unique
    /// or you'll screw up the queries.
    /// The parent node entities of the hierarchy will have no Parent component. 
    /// The leaf will node have no Children component. All the others will have both.
    /// All bone joints will have BoneJoint and TPose components.
    /// Meshes will have only mesh and prefab components.
    /// </summary>
    /// <param name="file"></param>
    /// <param name="registry"></param>
    /// <param name="assetId"></param>
    /// <param name="device"></param>
    /// <param name="commandQueue"></param>
    void LoadSkinnedMeshAsset(const std::string& file,
        entt::registry& registry,
        const std::string& assetId,
        Microsoft::WRL::ComPtr<ID3D12Device> device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue);
    //TODO: deprecated
    std::shared_ptr< SkinnedMeshPrefab> LoadFromFile(std::string file, entt::registry& registry);
    /// <summary>
    /// To instantia9te9669
    /// </summary>
    /// <param name="prefabName"></param>
    /// <param name="gameObjectName"></param>
    void InstantiatePrefab(std::string prefabName, std::string gameObjectName);
}

