#pragma once
#include "pch.h"
namespace myd3d
{
	/// <summary>
	/// Run the commands in the callback function.
	/// RunCommands create a new command allocator and list, rewind it, call callback passing the 
	/// list just created as parameter. It expects that the function passed in callback will use
	/// the command list. After callback finishes it submits the command list to the queue and
	/// waits for completition.
	/// </summary>
	/// <param name="device">a valid device</param>
	/// <param name="commandQueue">a valid command queue</param>
	/// <param name="callback">the callback with the commands</param>
	void RunCommands(
		ID3D12Device* device,
		ID3D12CommandQueue* commandQueue,
		std::function<void(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>)> callback);
}
