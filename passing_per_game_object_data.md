## intro
For the purpouses of this tutorial a game object is something that has a position and rotation 
and a mesh (unlike unity's game objects that can be meshless). The idea here is to pass the model
matrix to the shader. It could be used to pass other data that vary per-object like material properties 
too. 

This tutorial assumes that you know how to initialize dx12 and run commands in the command queue.

## the shader

transforms_vertex_shader.hlsl:
```//inputs
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};
//outputs
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};
//describes the model matrix
struct ModelMatrixStruct
{
    float4x4 mat;
};
//where i store the model matrices
StructuredBuffer<ModelMatrixStruct> ModelMatrices : register(t0);
//a root constant (aka push constant), it's the index at ModelMatrices 
cbuffer RootConstants : register(b0)
{
    uint matrixIndex; // Root constant passed by CPU
}
//holds the view projection matrix
cbuffer ConstantBuffer : register(b1)
{
    float4x4 viewProjectionMatrix;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4x4 modelMatrix = ModelMatrices[matrixIndex].mat;
    float4x4 mvpMatrix = mul(modelMatrix, viewProjectionMatrix);
    output.pos = mul(float4(input.pos, 1.0f), mvpMatrix);
    output.color = float4(input.uv, 1.0f, 1.0f);
    return output;
}
```
The model matrix, our per-object data, is represented by the ModelMatrixStruct and stored in a Structured
Buffer of ModelMatrixStruct type. ModelMatrices will have all game objects' matrices so we need a way to 
get the correct matrix for each object. 

In older shader language versions (up to shader language 9 iirc) we had a shader built-in variable that 
gave us the instance id. In the most recent shader languages we no longer have this variable. To replace 
it we use a root constant (aka Push Constant).

The root constant in our shader is the variable RootConstants, and it has an uint_32 for the id. 

## root signature
```
        //the table of root signature parameters
        std::array<CD3DX12_ROOT_PARAMETER, 3> rootParams;
        //1) ModelMatrices 
        CD3DX12_DESCRIPTOR_RANGE srvRange(
            D3D12_DESCRIPTOR_RANGE_TYPE_SRV, //it's a shader resource view 
            1, 
            0); //register t0
        rootParams[0].InitAsDescriptorTable(1, &srvRange);
        //2) RootConstants
        rootParams[1].InitAsConstants(1, //inits one dword (that'll carry the instanceId)
            0);//register b0
        //ConstantBuffer (view/projection data)
        rootParams[2].InitAsConstantBufferView(1); //at register b1

        //create root signature
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(rootParams.size(),//number of parameters
            rootParams.data(),//parameter list
            0,//number of static samplers
            nullptr,//static samplers list
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT //flags
        );
        //We need this serialization step
        ID3DBlob* signature;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signature, nullptr);
        assert(hr == S_OK);
        ComPtr<ID3D12RootSignature> rootSignature = nullptr;
        hr = device->CreateRootSignature(0,
            signature->GetBufferPointer(), //the serialized data is used here
            signature->GetBufferSize(), //the serialized data is used here
            IID_PPV_ARGS(&rootSignature));
```
Three parameters:
0) model matrix as a shader resource view at register T0.
1) one dword as root constant as register B0.
2) the constant buffer for view/projection matrix at register B1.
Mind that the order in the parameter array matter and that it doesn't have to be the same order that they
appear in the shader, the link between the root signature paramters and the shader uniforms is by their registers.

## creating the buffer for the shader resource view
I'll use 2 buffers: one visible only the gpu that'll be used by the shader and a staging buffer that'll receive data
from the cpu and copy it to the gpu-only buffer during the command list execution.
```
std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> structuredBuffer;
//Shader Resource View heap
std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> srvHeap;
//staging buffer resource
std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> uploadBuffer;
std::vector<void*> mappedData;
```
Why are they vectors? Because it's a ring buffer, one for each frame, because their contents vary frame-by-frame and
the CPU may be building the next frame while the GPU is executing the previous frame. In that case i need that all data 
that varies per frame be in different objects so that one frame does not interfere with the other.

structuredBuffer and srvHeap are the gpu data, structured buffer holds the data, srvHeap holds the descriptors.
uploadBuffer holds the data that the cpu can write to, while mappedData holds the pointers to these memories.

Creating the objects:

1) the gpu objects:
```
   CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
   CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
       bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
   structuredBuffer.resize(FRAMEBUFFER_COUNT);
   srvHeap.resize(FRAMEBUFFER_COUNT);
   for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
   {
       HRESULT r = ctx.GetDevice()->CreateCommittedResource(
           &heapProperties,
           D3D12_HEAP_FLAG_NONE,
           &bufferDesc,
           D3D12_RESOURCE_STATE_COMMON, // Initial state
           nullptr,
           IID_PPV_ARGS(&structuredBuffer[i]));
       //the heap to hold the views that we'll need.
       D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
       srvHeapDesc.NumDescriptors = 1; // Just one SRV for now
       srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
       srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
       ctx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap[i]));


       //now that i have the heap to hold the view and the resource i create the Shader Resource View
       //that connects the resource to the view
       D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
       srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
       srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
       srvDesc.Buffer.FirstElement = 0;
       srvDesc.Buffer.NumElements = numMatrices; // Number of matrices
       srvDesc.Buffer.StructureByteStride = matrixSize;
       srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
       srvDesc.Format = DXGI_FORMAT_UNKNOWN;

       ctx.GetDevice()->CreateShaderResourceView(structuredBuffer[i].Get(), &srvDesc,
           srvHeap[i]->GetCPUDescriptorHandleForHeapStart());
   }
```
The resource:
- a D3D12_HEAP_TYPE_DEFAULT has the most GPU bandwidth but can't be acessed by the CPU. That means that we can't memcpy data to it.
- D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS is necessary because we can acesss the elements of ModelMatrices in a random manner. 

The view heap:
- D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV is because it'll be used by a Shader Resource View.
- D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE means that the shader can access it. We need that.

The shader resource view properties:
- D3D12_SRV_DIMENSION_BUFFER means that the resource is a buffer.
- D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING is an 1:1 mapping.
- DXGI_FORMAT_UNKNOWN because this isn't a texture so it has no format.

2) the staging buffer
```
   CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
   CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
   uploadBuffer.resize(FRAMEBUFFER_COUNT);
   mappedData.resize(FRAMEBUFFER_COUNT);
   for (auto i = 0; i < FRAMEBUFFER_COUNT; i++)
   {
       ctx.GetDevice()->CreateCommittedResource(
           &uploadHeapProps,
           D3D12_HEAP_FLAG_NONE,
           &uploadBufferDesc,
           D3D12_RESOURCE_STATE_GENERIC_READ,
           nullptr,
           IID_PPV_ARGS(&uploadBuffer[i]));
       auto n0 = Concatenate(L"ModelMatrixStagingBuffer", i);
       uploadBuffer[i]->SetName(n0.c_str());
       uploadBuffer[i]->Map(0, nullptr, &mappedData[i]);
   }
```  

## Root Constant limitation
- The root signature can't hold many values as root constants. It can hold up to 64 dwords (64 * 4 bytes = 256 bytes).
- Mind that most of the times some of these dword will alredy be in use, inline descriptors and descriptor tables eat space in the root descriptor.
