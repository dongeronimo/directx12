#include "pch.h"
#include "mesh_load.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

const aiScene* LoadScene(Assimp::Importer& importer, const std::string& path);

DirectX::XMFLOAT3 aiVec3ToDirectXVector(aiVector3D& vec)
{
    DirectX::XMFLOAT3 v(vec.x, vec.y, vec.z);
    return v;
}
DirectX::XMFLOAT2 removeZ(DirectX::XMFLOAT3 vec)
{
    DirectX::XMFLOAT2 v(vec.x, vec.y);
    return v;
}


std::vector<common::MeshData> common::LoadMeshes(const std::string& filename)
{
    Assimp::Importer importer;
    const aiScene* scene = LoadScene(importer, filename);
    std::vector<common::MeshData> result(scene->mNumMeshes);
    for (int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh* currMesh = scene->mMeshes[m];
        common::MeshData& md = result[m];
        //i assume that all vertexes have normals and uv
        md.normals.resize(currMesh->mNumVertices);
        md.vertices.resize(currMesh->mNumVertices);
        md.uv.resize(currMesh->mNumVertices);
        for (uint32_t i = 0; i < currMesh->mNumVertices; i++) {
            md.vertices[i] = aiVec3ToDirectXVector(currMesh->mVertices[i]);
            md.normals[i] = aiVec3ToDirectXVector(currMesh->mNormals[i]);
            md.uv[i] = removeZ(aiVec3ToDirectXVector(currMesh->mTextureCoords[0][i]));
        }
        std::vector<uint16_t> indexData;
        for (unsigned int j = 0; j < currMesh->mNumFaces; j++) {
            aiFace face = currMesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                indexData.push_back(face.mIndices[k]);
            }
        }
        md.name = std::string(currMesh->mName.C_Str());
        md.indices = indexData;
        assert(md.indices.size() > 0);
        assert(md.vertices.size() > 0);
    }
    return result;
}

const aiScene* LoadScene(Assimp::Importer& importer, const std::string& path) {
    const aiScene* scene = importer.ReadFile(path.c_str(),
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);
    const char* err = importer.GetErrorString();
    if (!scene) {
        throw std::runtime_error(importer.GetErrorString());
    }
    else {
        return scene;
    }
}
