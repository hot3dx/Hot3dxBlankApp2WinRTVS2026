#pragma once

#include "pch.h"
#include "Common/DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "Common/StepTimer.h"

namespace winrt::Hot3dxBlankApp2
{
	// This sample renderer instantiates a basic rendering pipeline.
	class Sample3DSceneRenderer
	{
	public:
		Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Sample3DSceneRenderer();
		winrt::Windows::Foundation::IAsyncAction CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void Update(DX::StepTimer const& timer);
		bool Render();
		void Clear();
		void SaveState();

		void StartTracking();
		void TrackingUpdate(float positionX, float positionY);
		void StopTracking();
		bool IsTracking() { return m_tracking; }


		void OnDeviceLost();
		void OnDeviceRestored();


		bool GetLoadingComplete() const { return m_loadingComplete; }
		winrt::com_ptr<ID3D12GraphicsCommandList> GetCommandList() { return m_commandList; }
		void LoadState();
		void Rotate(float radians);

	private:
		// Constant buffers must be 256-byte aligned.
		static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		winrt::com_ptr<ID3D12GraphicsCommandList>	m_commandList;
		winrt::com_ptr<ID3D12RootSignature>			m_rootSignature;
		winrt::com_ptr<ID3D12PipelineState>			m_pipelineState;
		winrt::com_ptr<ID3D12DescriptorHeap>		m_cbvHeap;
		winrt::com_ptr<ID3D12Resource>				m_vertexBuffer;
		winrt::com_ptr<ID3D12Resource>				m_indexBuffer;
		winrt::com_ptr<ID3D12Resource>				m_constantBuffer;
		ModelViewProjectionConstantBuffer					m_constantBufferData;
		UINT8* m_mappedConstantBuffer;
		UINT												m_cbvDescriptorSize;
		D3D12_RECT											m_scissorRect;
		std::vector<BYTE>          							m_vertexShader;
		std::vector<BYTE>           						m_pixelShader;
		D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		float	m_radiansPerSecond;
		float	m_angle;
		bool	m_tracking;
	};
}