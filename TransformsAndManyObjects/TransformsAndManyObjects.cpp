#include "pch.h"
#include "../Common/window.h"
#include "../Common/mesh.h"
#include "../Common/mesh_load.h"
#include "direct3d_context.h"
#include "Pipeline.h"
#include "view_projection.h"
#include "transform.h"
#include "model_matrix.h"

using Microsoft::WRL::ComPtr;

constexpr int W = 800;
constexpr int H = 600;

class GameObject :public transforms::Transform
{
	public:
	int mesh;
};
/// <summary>
/// Storage for the meshes. Do not repeat IDs.
/// </summary>
std::unordered_map<int, std::shared_ptr<common::Mesh>> gMeshTable;
/// <summary>
/// Load the meshes into gMeshTable. It expects that the context has alredy been created.
/// </summary>
/// <param name="ctx"></param>
void LoadMeshes(transforms::Context& ctx);


std::vector<std::shared_ptr<GameObject>> gGameObjects;
std::unique_ptr<transforms::ModelMatrix> mModelMatrix;

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	common::Window window(hInstance, L"transforms_t", L"Transforms");
	window.Show();
	std::unique_ptr<transforms::Context> ctx = std::make_unique<transforms::Context>(W, H, window.Hwnd());

	LoadMeshes(*ctx);

	std::unique_ptr<transforms::RootSignatureService> rootSignatureService = std::make_unique<transforms::RootSignatureService>();
	const std::wstring myRootSignatureName = L"MyRootSignature";

	transforms::ViewProjection viewProjection(*ctx);
	viewProjection.SetPerspective(45.0f, (float)W / (float)H, 0.01f, 100.f);
	viewProjection.LookAt({ 3.0f, 5.0f, 7.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

	rootSignatureService->Add(myRootSignatureName, ctx->CreateRootSignature(myRootSignatureName));
	std::shared_ptr<transforms::Pipeline> myPipeline = std::make_shared<transforms::Pipeline>(
		L"transforms_vertex_shader.cso",
		L"transforms_pixel_shader.cso",
		rootSignatureService->Get(myRootSignatureName),
		ctx->GetDevice(),
		L"HelloWorldPipeline"
	);

	// Fill out the Viewport
	myPipeline->viewport.TopLeftX = 0;
	myPipeline->viewport.TopLeftY = 0;
	myPipeline->viewport.Width = W;
	myPipeline->viewport.Height = H;
	myPipeline->viewport.MinDepth = 0.0f;
	myPipeline->viewport.MaxDepth = 1.0f;

	// Fill out a scissor rect
	myPipeline->scissorRect.left = 0;
	myPipeline->scissorRect.top = 0;
	myPipeline->scissorRect.right = W;
	myPipeline->scissorRect.bottom = H;

	//create the scene
	std::shared_ptr<GameObject> obj1 = std::make_shared<GameObject>();
	obj1->position = { 0.0f, 0.0f, 0.0f };
	obj1->scale = { 1.0f, 1.0f, 1.0f };
	obj1->rotation = DirectX::XMQuaternionIdentity();
	obj1->mesh = 0;
	obj1->id = 0;
	std::shared_ptr<GameObject> obj2 = std::make_shared<GameObject>();
	obj2->position = { 2.0f, 0.0f, 0.0f };
	obj2->scale = { 1.0f, 1.0f, 1.0f };
	obj2->rotation = DirectX::XMQuaternionIdentity();
	obj2->mesh = 1;
	obj2->id = 1;

	gGameObjects = { obj1, obj2 };

	//handle window resizing
	window.mOnResize = [&myPipeline, &ctx](int newW, int newH) {
		//TODO: Resize is freezing the app
		ctx->Resize(newW, newH);
		myPipeline->viewport.Width = newW;
		myPipeline->viewport.Height = newH;
		myPipeline->scissorRect.right = newW;
		myPipeline->scissorRect.bottom = newH;
	};
	//set onIdle handle to deal with rendering
	window.mOnIdle = [&ctx, &rootSignatureService, myRootSignatureName,
		&myPipeline, &viewProjection]() {
		//wait until i can interact with this frame again
		ctx->WaitForPreviousFrame();
		//reset the allocator and the command list
		ctx->ResetCurrentCommandList();
		//the render target has to be in D3D12_RESOURCE_STATE_RENDER_TARGET state to be be used by the Output Merger to compose the scene
		ctx->TransitionCurrentRenderTarget(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//then we set the current target for the output merger
		ctx->SetCurrentOutputMergerTarget();
		static float red = 0.0f;
		if (red >= 1.0f) red = 0.0f;
		else red += 0.0001f;
		// Clear the render target by using the ClearRenderTargetView command
		ctx->ClearRenderTargetView({ red, 0.2f, 0.4f, 1.0f });
		//update the model matrix
		std::vector<transforms::Transform*> lstTransforms(gGameObjects.size());
		for (auto i = 0; i < lstTransforms.size(); i++)
		{
			lstTransforms[i] = gGameObjects[i].get();
		}
		//update the model matrix buffer in the gpu
		mModelMatrix->UploadData(lstTransforms, ctx->GetFrameIndex(), ctx->GetCommandList());
		viewProjection.StoreInBuffer();
		//bind root signature
		ctx->BindRootSignature(rootSignatureService->Get(myRootSignatureName), 
			viewProjection,
			*mModelMatrix);
		//bind the pipeline
		ctx->BindPipeline(myPipeline);
		//draw the meshes using the pipeline
		for (auto i = 0; i < lstTransforms.size(); i++)
		{
			auto go = reinterpret_cast<GameObject*>(lstTransforms[i]);
			auto mesh = gMeshTable[go->mesh];
			ctx->GetCommandList()->SetGraphicsRoot32BitConstant(0, i, 0);
			myPipeline->DrawInstanced(ctx->GetCommandList(),
				mesh->VertexBufferView(),
				mesh->IndexBufferView(),
				mesh->NumberOfIndices());
		}
		//...
		//now that we drew everything, transition the rtv to presentation
		ctx->TransitionCurrentRenderTarget(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		//Submit the commands and present
		ctx->Present();
		};
	//fire main loop
	window.MainLoop();
	ctx->WaitForPreviousFrame();

	rootSignatureService.reset();
	ctx.reset();
	for (auto& m : gMeshTable) {
		m.second = nullptr;
	}
	gMeshTable.clear();
	return 0;
}

void LoadMeshes(transforms::Context& ctx) {
	auto monkeyMeshData = common::LoadMeshes("assets/monkey.glb")[0];
	std::shared_ptr<common::Mesh> monkeyMesh = std::make_shared<common::Mesh>(
		monkeyMeshData,
		ctx.GetDevice(), ctx.GetCommandQueue());
	gMeshTable.insert({ 0, monkeyMesh });

	auto cubeMeshData = common::LoadMeshes("assets/cube.glb")[0];
	std::shared_ptr<common::Mesh> cubeMesh = std::make_shared<common::Mesh>(
		cubeMeshData,
		ctx.GetDevice(), 
		ctx.GetCommandQueue());
	gMeshTable.insert({ 1, cubeMesh });

	auto sphereMeshData = common::LoadMeshes("assets/sphere.glb")[0];
	std::shared_ptr<common::Mesh> sphereMesh = std::make_shared<common::Mesh>(sphereMeshData,
		ctx.GetDevice(), ctx.GetCommandQueue());
	gMeshTable.insert({ 2, sphereMesh });

}
