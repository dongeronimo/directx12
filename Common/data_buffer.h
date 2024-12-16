#pragma once
#include "pch.h"
#include "../Common/d3d_utils.h"
namespace common
{
	/// <summary>
	/// The data buffer holds data that can be passed as shader resource views or 
	/// as vertex buffer view (for instance data).
	/// The data is written to a staging buffer and then pushed to the gpu-facing buffer. 
	/// </summary>
	/// <typeparam name="T">The type: prefer primitive types and simple structs because they'll be memcpyed to the gpu</typeparam>
	/// <typeparam name="N">How many. Once set there's no way to change.</typeparam>
	template<typename T, std::size_t N>
	class DataBuffer
	{
	private:
		/// <summary>
		/// GPU-Facing buffer
		/// </summary>
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceBuffer;
		/// <summary>
		/// Staging buffer
		/// </summary>
		Microsoft::WRL::ComPtr<ID3D12Resource> instanceUploadBuffer;
		/// <summary>
		/// CPU-side data storage
		/// </summary>
		std::array<T, N> instanceData;
		/// <summary>
		/// ptr to the staging buffer
		/// </summary>
		void* mappedData;
		/// <summary>
		/// Where we are.
		/// </summary>
		int cursor = INT_MAX;
	public:
		/// <summary>
		/// Size of the buffer in bytes
		/// </summary>
		const UINT instanceBufferSize;
		/// <summary>
		/// Creates the buffer. The gpu-facing buffer will be in D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER state,
		/// the staging buffer will be in D3D12_RESOURCE_STATE_GENERIC_READ.
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandQueue"></param>
		DataBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device,Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue):
			instanceBufferSize(static_cast<UINT>(N * sizeof(T))) 
		{
			instanceBuffer = common::CreateBuffer(device.Get(),
				commandQueue.Get(),
				instanceData.data(),
				instanceBufferSize,
				instanceUploadBuffer);
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
		/// <summary>
		/// the instance buffer
		/// </summary>
		/// <returns></returns>
		Microsoft::WRL::ComPtr<ID3D12Resource> InstanceBuffer()const {
			return instanceBuffer;
		}
		/// <summary>
		/// Call this to begin the process of passing data. It resets the cursor, clear
		/// the memory (with ZeroMemory) and transition the gpu-facing buffer to copy dest
		/// </summary>
		/// <param name="commandList"></param>
		void BeginStore(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) {
			//clear memory
			void* baseAddress = mappedData;
			ZeroMemory(baseAddress, instanceBufferSize);
			cursor = 0;
			//changes the buffer from D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER to D3D12_RESOURCE_STATE_COPY_DEST
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				instanceBuffer.Get(),
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,//TODO: see the original model_matrix
				D3D12_RESOURCE_STATE_COPY_DEST
			);
			commandList->ResourceBarrier(1, &barrier);
		}
		/// <summary>
		/// Store the data, advance the cursor.
		/// </summary>
		/// <param name="value"></param>
		void Store(const T& value)
		{
			assert(cursor != INT_MAX);
			instanceData[cursor] = value;
			cursor++;
		}
		/// <summary>
		/// Upload data to the gpu-facing buffer, makes the cursor invalid, change the state of 
		/// the gpu facing buffer.
		/// </summary>
		/// <param name="commandList"></param>
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

