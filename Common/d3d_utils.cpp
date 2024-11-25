#include "pch.h"
#include "d3d_utils.h"
using Microsoft::WRL::ComPtr;
void myd3d::RunCommands(
    ID3D12Device* device,
    ID3D12CommandQueue* commandQueue,
    std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>)> callback)
{
    assert(device != nullptr);
    assert(commandQueue != nullptr);
    //create the fence that'll wait for the execution
    ComPtr<ID3D12Fence> _fence;
    const UINT64 fenceCompletitionValue = 1; //this value indicates that the execution is complete 
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));//fence value begins at 0, a value smaller then completitionValue
    //creates the allocator
    ComPtr<ID3D12CommandAllocator> cmdAllocator;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&cmdAllocator));
    cmdAllocator->Reset();
    //creates the list that relies on the allocator
    ComPtr<ID3D12GraphicsCommandList> cmdList;
    hr = device->CreateCommandList(0,//default gpu 
        D3D12_COMMAND_LIST_TYPE_DIRECT, //type of commands - direct means that the commands can be executed by the gpu
        cmdAllocator.Get(),
        NULL, IID_PPV_ARGS(&cmdList));
    cmdList->Close();
    cmdAllocator->Reset();
    cmdList->Reset(cmdAllocator.Get(), nullptr);
    callback(cmdList);
    cmdList->Close();
    ID3D12CommandList* ppCommandLists[] = { cmdList.Get() };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);
    commandQueue->Signal(_fence.Get(), fenceCompletitionValue);//increases the fence to the value that indicates that the process is done
    //wait until the fence value goes from 0 to 1
    HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent != nullptr);
    _fence->SetEventOnCompletion(fenceCompletitionValue, fenceEvent);
    WaitForSingleObject(fenceEvent, INFINITE);
}
