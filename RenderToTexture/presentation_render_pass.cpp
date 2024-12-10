#include "presentation_render_pass.h"
#include "swapchain.h"
/// <summary>
/// Set the state of the render target and bind the render target
/// </summary>
void rtt::PresentationRenderPass::Begin(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Swapchain& swapchain)
{
    swapchain.UpdateCurrentBackbuffer();
    CD3DX12_RESOURCE_BARRIER fromPresentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(
        swapchain.SwapChainBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT, ///Present is the state that the render target view has to be to be able to present the image
        D3D12_RESOURCE_STATE_RENDER_TARGET//Render target is the state that the RTV has to be to be used by the output merger state.
    );
    commandList->ResourceBarrier(1,
        &fromPresentToRenderTarget);
    D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle = swapchain.CurrentBackbufferView();
    // get a handle to the depth/stencil buffer
    D3D12_CPU_DESCRIPTOR_HANDLE _dsvHandle = swapchain.DepthStencilView();
    // set the render target for the output merger stage (the output of the pipeline)
    commandList->OMSetRenderTargets(1, &_rtvHandle, FALSE, &_dsvHandle);
    //clear render target
    std::array<float, 4> color = { 0.0f, 1.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(_rtvHandle, color.data(), 0, nullptr);
    commandList->ClearDepthStencilView(_dsvHandle, D3D12_CLEAR_FLAG_DEPTH,
        1.0f, 0, 0, nullptr);
}

void rtt::PresentationRenderPass::End(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
    Swapchain& swapchain)
{
    CD3DX12_RESOURCE_BARRIER fromRenderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        swapchain.SwapChainBuffer().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, ///Present is the state that the render target view has to be to be able to present the image
        D3D12_RESOURCE_STATE_PRESENT//Render target is the state that the RTV has to be to be used by the output merger state.
    );
    commandList->ResourceBarrier(1,
        &fromRenderTargetToPresent);
}
