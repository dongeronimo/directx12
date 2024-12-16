#include "pch.h"
#include "../Common/window.h"
#include "dx_context.h"
#include "offscreen_rtv.h"
#include "swapchain.h"
#include "offscreen_render_pass.h"
#include "presentation_render_pass.h"
#include "entities.h"
#include "root_signature_service.h"
#include "transforms_pipeline.h"
#include "camera.h"
#include "../Common/mesh.h"
#include "model_matrix.h"
#include "presentation_pipeline.h"
#include "../Common/game_timer.h"
#include "../Common/concatenate.h"
#include "../Common/mathutils.h"
#include "instanced_transform_pipeline.h"
#include "instance_index.h"
#include "../Common/data_buffer.h"
using Microsoft::WRL::ComPtr;

constexpr int W = 1024;
constexpr int H = 768;
std::vector<std::shared_ptr<common::Mesh>> gMeshes;

entt::registry gRegistry;
common::GameTimer gTimer;
void LoadAssets(rtt::DxContext& context)
{
	auto cube = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "cube.glb");
	auto monkey = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "monkey.glb");
	auto sphere = common::io::LoadMesh(context.Device(),
		context.CommandQueue(), "sphere.glb");

	for (auto x : cube)
		gMeshes.push_back(x);
	for (auto x : monkey)
		gMeshes.push_back(x);
	for (auto x : sphere)
		gMeshes.push_back(x);
}

int main()
{
	//////Create the window//////
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"colored_triangle_t", L"Colored Triangle", W, H);
	window.Show();
	//////Create directx//////
	//create the context that has, among other things, the device
	std::shared_ptr<rtt::DxContext> context = std::make_shared<rtt::DxContext>();
	//the scene won't be rendered to the swapchain directly. It'll be rendered to an
	// offscreen target and then that target will show the result in a quad
	//the offscreenRTV will hold the rendering result
	std::shared_ptr<rtt::OffscreenRTV> offscreenRTV = std::make_shared<rtt::OffscreenRTV>(W, H, *context);
	//the swap chain 
	std::shared_ptr<rtt::Swapchain> swapchain = std::make_shared<rtt::Swapchain>(
		window.Hwnd(), W, H, *context);
	//asset load - we need a command queue to load the assets because the meshes are sent to the vertex buffers in the gpu.
	LoadAssets(*context);
	//create the offscreen render pass
	std::shared_ptr<rtt::OffscreenRenderPass> offscreenRP = std::make_shared<rtt::OffscreenRenderPass>();
	std::shared_ptr<rtt::PresentationRenderPass> presentationRP = std::make_shared<rtt::PresentationRenderPass>();

	///////Create the world///////
	const auto monkey = gRegistry.create();
	gRegistry.emplace<rtt::entities::GameObject>(monkey, L"Monkey");
	gRegistry.emplace<rtt::entities::Monkey>(monkey, 1u);
	gRegistry.emplace<rtt::entities::Transform>(monkey,
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f ),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f ),
		DirectX::XMQuaternionIdentity());
	gRegistry.emplace<rtt::entities::DeltaTransform>(monkey,
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), //position variation
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), //scale variation
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), //axis of rotation
		45.0f //angular speed of rotation
	);
	for (auto i = 0; i < 4000; i++)
	{
		const auto gameObject = gRegistry.create();
		auto name = Concatenate(L"GameObject", i);
		gRegistry.emplace<rtt::entities::GameObject>(gameObject, name);
		gRegistry.emplace<rtt::entities::Cube>(gameObject, 0u);
		int x = i / 64;
		int z = i % 64;
		float dist = 1.0f;
		float offset = dist * 64.0f / 2.0f;
		gRegistry.emplace<rtt::entities::Transform>(gameObject,
			DirectX::XMFLOAT3(dist * x - offset , -5.f, dist * z - offset),
			DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
			DirectX::XMQuaternionIdentity());
		DirectX::XMFLOAT3 axis(
			common::RandomNumber(-1.0f, 1.0f),
			common::RandomNumber(-1.0f, 1.0f),
			common::RandomNumber(-1.0f, 1.0f)
		);
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&axis);
		v1 = DirectX::XMVector3Normalize(v1);
		DirectX::XMStoreFloat3(&axis, v1);
		float speed = common::RandomNumber(-90.0f, 90.0f);
		gRegistry.emplace<rtt::entities::DeltaTransform>(gameObject,
			DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
			v1,
			speed
		);
	}
	//create root signature
	ComPtr<ID3D12RootSignature> transformsRootSignature = rtt::RootSignatureForShaderTransforms(context->Device());
	ComPtr<ID3D12RootSignature> presentationRootSignature = rtt::RootSignatureForShaderPresentation(context->Device());
	//create pipelines
	rtt::TransformsPipeline transformsPipeline(
		L"transforms_vertex_shader.cso",
		L"transforms_pixel_shader.cso",
		transformsRootSignature,
		context->Device(),
		context->SampleCount(),
		context->QualityLevels());
	std::shared_ptr<rtt::PresentationPipeline> presentationPipeline = std::make_shared<rtt::PresentationPipeline>(
		context->Device(), context->CommandQueue(), presentationRootSignature, 
		context->SampleCount(),
		context->QualityLevels());
	std::shared_ptr<rtt::InstancedTransformPipeline> instancedPipeline = std::make_shared<rtt::InstancedTransformPipeline>(
		L"instanced_transform_vertex_shader.cso",
		L"instanced_transform_pixel_shader.cso",
		context->Device(),
		context->SampleCount(),
		context->QualityLevels()
	);
	//create camera
	rtt::Camera camera(*context);
	camera.SetPerspective(60.0f, ((float)W / (float)H), 0.01f, 100.f);
	camera.LookAt({ 6.0f, 10.0f, 14.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	//create the model view buffer
	std::shared_ptr<rtt::ModelMatrix> modelMatrixForMonkeys = std::make_shared<rtt::ModelMatrix>(*context);
	std::shared_ptr<rtt::InstanceData<int, 4096>> monkeyInstanceIndexes = std::make_shared<rtt::InstanceData<int, 4096>>(context->Device(), context->CommandQueue());
	//instance index buffer for the cubes
	std::shared_ptr<rtt::InstanceData<int, 4096>> cubeInstanceIndexes = std::make_shared<rtt::InstanceData<int, 4096>>(context->Device(), context->CommandQueue());
	std::shared_ptr<rtt::ModelMatrix>  modelMatrixForCubes = std::make_shared<rtt::ModelMatrix>(*context);
	
	common::DataBuffer<DirectX::XMFLOAT4X4, 1024> teste(context->Device(),
		context->CommandQueue());
	//////Main loop//////
	static float r = 0;
	window.mOnIdle = [&context, &swapchain,&offscreenRTV, &offscreenRP, 
		&presentationRP, &modelMatrixForMonkeys,&monkeyInstanceIndexes, &camera, &transformsRootSignature,
		&transformsPipeline, &presentationRootSignature, &presentationPipeline, &instancedPipeline,
		&modelMatrixForCubes, &cubeInstanceIndexes]()
	{
		// Fill out the Viewport
		D3D12_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(W);
		viewport.Height = static_cast<float>(H);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Fill out a scissor rect
		D3D12_RECT scissorRect;
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = static_cast<float>(W);
		scissorRect.bottom = static_cast<float>(H);
			
		/////// Update Logic ///////
		gTimer.Tick();
		auto gameUpdateView = gRegistry.view<
			const rtt::entities::DeltaTransform,
			rtt::entities::Transform>();
		gameUpdateView.each([](
			const rtt::entities::DeltaTransform deltaTransform, 
			rtt::entities::Transform& transform
			) {
			using namespace DirectX;
			//position
			XMVECTOR pos = XMLoadFloat3(&transform.position);
			XMVECTOR posVelocity = XMLoadFloat3(&deltaTransform.dPosition);
			XMVECTOR dPos = XMVectorScale(posVelocity, gTimer.DeltaTime());
			pos = XMVectorAdd(pos, dPos);
			DirectX::XMStoreFloat3(&transform.position, pos);
			//scale
			XMVECTOR scale = XMLoadFloat3(&transform.scale);
			XMVECTOR scaleVelocity = XMLoadFloat3(&deltaTransform.dScale);
			XMVECTOR dScale = XMVectorScale(scaleVelocity, gTimer.DeltaTime());
			scale = XMVectorAdd(scale, dScale);
			DirectX::XMStoreFloat3(&transform.scale, scale);
			//rotation
			float rotationSpeed = XMConvertToRadians(deltaTransform.rotationSpeed); 
			float scaledAngle = rotationSpeed * gTimer.DeltaTime();
			XMVECTOR rotationVariation = XMQuaternionRotationAxis(deltaTransform.rotationAxis ,
				scaledAngle);
			XMVECTOR currentQuaternion = transform.rotation;
			currentQuaternion = XMQuaternionMultiply(currentQuaternion, rotationVariation);
			currentQuaternion = XMQuaternionNormalize(currentQuaternion);
			transform.rotation = currentQuaternion;
		});
		/////// Draw Scene ///////
		context->WaitPreviousFrame();
		context->ResetCommandList();
		//activate offscreen render pass
		offscreenRP->Begin(
			context->CommandList(),
			offscreenRTV->RenderTargetTexture(),
			offscreenRTV->RenderTargetView(),
			offscreenRTV->DepthStencilView(),
			{0,0,0,1}
		);
		//set viewport and scissors
		context->CommandList()->RSSetViewports(1, &viewport);
		context->CommandList()->RSSetScissorRects(1, &scissorRect);

		//Send cube data to GPU, we need to send the matrices and the indices.
		auto cubeDrawDataView = gRegistry.view<const rtt::entities::Transform, const rtt::entities::Cube>();
		modelMatrixForCubes->BeginStore();
		cubeInstanceIndexes->BeginStore(context->CommandList());
		int cubeIdx = 0;
		cubeDrawDataView.each([&modelMatrixForCubes, &cubeInstanceIndexes, &cubeIdx](const rtt::entities::Transform t, const rtt::entities::Cube) 
		{
			modelMatrixForCubes->Store(t);
			cubeInstanceIndexes->Store(cubeIdx);
			cubeIdx++;
		});
		modelMatrixForCubes->EndStore(context->CommandList());
		cubeInstanceIndexes->EndStore(context->CommandList());
		//bind root signature
		context->CommandList()->SetGraphicsRootSignature(instancedPipeline->RootSignature().Get());
		////root param 1 = view projection buffer - all objects will use the same camera
		camera.StoreInBuffer();
		context->CommandList()->SetGraphicsRootConstantBufferView(1,
			camera.GetConstantBuffer()->GetGPUVirtualAddress());
		//bind the pipeline 
		context->CommandList()->SetPipelineState(instancedPipeline->Pipeline().Get());
		context->CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//bind the cube buffers
		////root param 0 = model matrix buffer - it's different between object classes (cube, monkey, etc)
		std::vector<ID3D12DescriptorHeap*> cubeDescriptorHeaps = {modelMatrixForCubes->DescriptorHeap().Get() };
		context->CommandList()->SetDescriptorHeaps(cubeDescriptorHeaps.size(), cubeDescriptorHeaps.data());
		context->CommandList()->SetGraphicsRootDescriptorTable(0,
			modelMatrixForCubes->DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		////vertex input 0 = vertex buffer
		auto cubeVBV = gMeshes[0]->VertexBufferView();
		context->CommandList()->IASetVertexBuffers(0, 1, &cubeVBV);
		////vertex input 1 = instance buffer
		auto instanceBV = cubeInstanceIndexes->InstanceBufferView();
		context->CommandList()->IASetVertexBuffers(1, 1, &instanceBV);
		////index list
		auto cubeIBV = gMeshes[0]->IndexBufferView();
		context->CommandList()->IASetIndexBuffer(&cubeIBV);
		//draw the cube instances
		context->CommandList()->DrawIndexedInstanced(gMeshes[0]->NumberOfIndices(), cubeIdx, 0, 0, 0);
		//TODO: Send monkey data to GPU, we need to send the matrices and the indices.
		auto monkeyDrawDataView = gRegistry.view<const rtt::entities::Transform, const rtt::entities::Monkey>();
		modelMatrixForMonkeys->BeginStore();
		monkeyInstanceIndexes->BeginStore(context->CommandList());
		int monkeyIndex = 0;
		monkeyDrawDataView.each([&modelMatrixForMonkeys, &monkeyInstanceIndexes, &monkeyIndex](const rtt::entities::Transform t, const rtt::entities::Monkey m) {
			modelMatrixForMonkeys->Store(t);
			monkeyInstanceIndexes->Store(monkeyIndex);
			monkeyIndex++;
		});
		modelMatrixForMonkeys->EndStore(context->CommandList());
		monkeyInstanceIndexes->EndStore(context->CommandList());
		//TODO: write monkey data
		////root param 0 
		std::vector<ID3D12DescriptorHeap*> monkeyDescriptorHeaps = { modelMatrixForMonkeys->DescriptorHeap().Get() };
		context->CommandList()->SetDescriptorHeaps(monkeyDescriptorHeaps.size(), monkeyDescriptorHeaps.data());
		context->CommandList()->SetGraphicsRootDescriptorTable(0,modelMatrixForMonkeys->DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
		////vertex input 0 = vertex buffer
		auto monkeyCubeVBV = gMeshes[1]->VertexBufferView();
		context->CommandList()->IASetVertexBuffers(0, 1, &monkeyCubeVBV);
		////vertex input 1 = instance buffer
		auto monkeyInstanceBV = monkeyInstanceIndexes->InstanceBufferView();
		context->CommandList()->IASetVertexBuffers(1, 1, &monkeyInstanceBV);
		////index list
		auto monkeyIBV = gMeshes[1]->IndexBufferView();
		context->CommandList()->IASetIndexBuffer(&monkeyIBV);
		//TODO: draw monkey
		context->CommandList()->DrawIndexedInstanced(gMeshes[1]->NumberOfIndices(), cubeIdx, 0, 0, 0);
		
		//end the offscreen render pass
		offscreenRP->End(context->CommandList(),
			offscreenRTV->RenderTargetTexture());
		//activate final result render pass
		presentationRP->Begin(context->CommandList(), *swapchain);
		//bind root signature
		context->BindRootSignatureForPresentation(
			presentationRootSignature,
			presentationPipeline->SamplerHeap(),
			offscreenRTV->SrvHeap());
		//bind pipeline

		presentationPipeline->Bind(context->CommandList(), viewport, scissorRect);
		//draw quad
		presentationPipeline->Draw(context->CommandList());
		presentationRP->End(context->CommandList(), *swapchain);
		context->Present(swapchain->SwapChain());
	};
	//////On Resize handle//////
	window.mOnResize = [](int newW, int newH) {};
	//////On Create handle/////
	window.mOnCreate = []() {
		gTimer.Reset();
		gTimer.Start();
	};
	//////Fire main loop///////
	window.MainLoop();
	gMeshes.clear();
	return 0;
}
