#pragma once
#include "pch.h"
#include "../Common/d3d_utils.h"
namespace rtt
{
	template<typename T, std::size_t N>
	class InstanceData
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer, instanceUploadBuffer;
		D3D12_VERTEX_BUFFER_VIEW instanceBufferView = {};
		std::array<T, N> instanceData;
		void* mappedData;
		int cursor = INT_MAX;
	public:
		const UINT instanceBufferSize;
		InstanceData(
			Microsoft::WRL::ComPtr<ID3D12Device> device,
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue
		) :
			instanceBufferSize(static_cast<UINT>(N * sizeof(T)))
		{
			instanceBuffer = common::CreateBuffer(device.Get(), 
				commandQueue.Get(), 
				instanceData.data(), 
				instanceBufferSize, 
				instanceUploadBuffer);
			instanceBufferView.BufferLocation = instanceBuffer->GetGPUVirtualAddress();
			instanceBufferView.SizeInBytes = instanceBufferSize;
			instanceBufferView.StrideInBytes = sizeof(T);
			instanceUploadBuffer->Map(0, nullptr, &mappedData);
			//The shader facing buffer is created as D3D12_RESOURCE_STATE_COMMON, we have to 
			//change to what BeginStore expects
			common::RunCommands(device.Get(), commandQueue.Get(), [this](auto commandList) {
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					instanceBuffer.Get(),
					D3D12_RESOURCE_STATE_COMMON,
					D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
				);
				commandList->ResourceBarrier(1, &barrier);
			});
		}
		Microsoft::WRL::ComPtr<ID3D12Resource> InstanceBuffer()const {
			return instanceBuffer;
		}
		D3D12_VERTEX_BUFFER_VIEW InstanceBufferView()const {
			return instanceBufferView;
		}

		void BeginStore(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {
			//clear memory
			void* baseAddress = mappedData;
			ZeroMemory(baseAddress, instanceBufferSize);
			cursor = 0;
			//changes the buffer from D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER to D3D12_RESOURCE_STATE_COPY_DEST
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				instanceBuffer.Get(),
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				D3D12_RESOURCE_STATE_COPY_DEST
			);
			commandList->ResourceBarrier(1, &barrier);
		}
		void Store(const T& value)
		{
			assert(cursor != INT_MAX);
			instanceData[cursor] = value;
			cursor++;
		}
		void EndStore(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
		{
			//copy to the upload buffer addr
			memcpy(mappedData, instanceData.data(), sizeof(T) * cursor);
			//upload to buffer, using the staging buffer
			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = mappedData;
			subresourceData.RowPitch = sizeof(T) * N;
			subresourceData.SlicePitch = subresourceData.RowPitch;
			UpdateSubresources(commandList.Get(),
				instanceBuffer.Get(),
				instanceUploadBuffer.Get(),
				0, 0, 1, &subresourceData);
			cursor = INT_MAX;
			//changes the buffer from D3D12_RESOURCE_STATE_COPY_DEST to D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				instanceBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			);
			commandList->ResourceBarrier(1, &barrier);
		}
	};

}

