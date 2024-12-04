#pragma once
#include "pch.h"
#include "../Common/d3d_utils.h"
//using Microsoft::WRL::ComPtr;
namespace common
{
    class Pipeline;

    class Context
    {
    private:
#if defined(_DEBUG)
        Microsoft::WRL::ComPtr<ID3D12Debug> debugLayer;
        Microsoft::WRL::ComPtr<ID3D12DebugDevice> debugDevice;
#endif
        // direct3d device
        Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;;
        //contais the command lists.
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
        // swapchain used to switch between render targets
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain = nullptr;
        // current render target view we are on
        int frameIndex = INT_MAX;

        std::shared_ptr<common::RenderTargetViewData> rtvData = nullptr;
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainRenderTargets;
        //Represents the allocations of storage for graphics processing unit (GPU) commands, one per frame
        std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocator;
        // a command list we can record commands into, then execute them to render the frame. We need one
        // per cpu thread. Since our app will be single threaded we create just one.
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
        // fences for each frame. We need to wait on these fences until the gpu finishes it's job
        std::vector<Microsoft::WRL::ComPtr<ID3D12Fence>> fence; //TODO refactor
        // this value is incremented each frame. each fence will have its own value
        std::vector<uint64_t> fenceValue;
        //// a handle to an event when our fence is unlocked by the gpu
        //HANDLE fenceEvent;
    public:
        Context(int w, int h, HWND hwnd);
        Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(const std::wstring& name);
        Microsoft::WRL::ComPtr<ID3D12Device> GetDevice()const {
            return device;
        }
        void CreateVertexBufferFromData(std::vector<Vertex>& _vertices,
            Microsoft::WRL::ComPtr<ID3D12Resource>& _vertexBuffer,
            D3D12_VERTEX_BUFFER_VIEW& _vertexBufferView,
            const std::wstring& name);
        void WaitForPreviousFrame();
        void ResetCurrentCommandList();
        void TransitionCurrentRenderTarget(D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
        void SetCurrentOutputMergerTarget();
        void ClearRenderTargetView(std::array<float, 4> rgba);
        void BindRootSignature(Microsoft::WRL::ComPtr<ID3D12RootSignature> rs);
        void BindPipeline(std::shared_ptr<Pipeline> pipe);
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList()const {
            return commandList;
        }
        void Present();
    };
    /// <summary>
    /// Initializes direct3d. Must be called after we create the window.
    /// </summary>
    /// <param name="w"></param>
    /// <param name="h"></param>
    /// <param name="hwnd"></param>
    /// <returns></returns>
    //bool InitD3D(int w, int h, HWND hwnd);
    void CleanupD3D();
    void WaitForPreviousFrame();
    //void Render();
    //void UpdatePipeline();
}
