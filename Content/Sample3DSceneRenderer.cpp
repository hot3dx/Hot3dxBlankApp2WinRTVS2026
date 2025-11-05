#include "pch.h"

#include "Common\d3dx12.h"
#include "Sample3DSceneRenderer.h"

#include "..\Common\DirectXHelper.h"

//#include <synchapi.h>

#include <winrt/base.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.applicationmodel.h>

#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.controls.h>
#include<winrt/windows.storage.h>
#include<winrt/windows.storage.streams.h>

using namespace winrt::Hot3dxBlankApp2;



using namespace DirectX;
using namespace Microsoft::WRL;
namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;


// Indices into the application state map.
winrt::param::hstring const& AngleKey = L"Angle";
winrt::param::hstring const& TrackingKey = L"Tracking";

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
Sample3DSceneRenderer::Sample3DSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_radiansPerSecond(XM_PIDIV4),	// rotate 45 degrees per second
	m_angle(0),
	m_tracking(false),
	m_mappedConstantBuffer(nullptr),
	m_deviceResources(deviceResources)
{
	LoadState();
	ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));
	CreateWindowSizeDependentResources();
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

Sample3DSceneRenderer::~Sample3DSceneRenderer()
{
	m_constantBuffer->Unmap(0, nullptr);
	m_mappedConstantBuffer = nullptr;
}

winrt::Windows::Foundation::IAsyncAction Sample3DSceneRenderer::CreateDeviceDependentResources()
{
	auto d3dDevice = m_deviceResources->GetD3DDevice();
	// Optional: before creating device resources, log whether device is valid
	{
		if (!d3dDevice)
		{
			OutputDebugStringA("ERROR: D3D device is null in CreateDeviceDependentResources()\n");
		}
		else
		{
			OutputDebugStringA("INFO: D3D device valid in CreateDeviceDependentResources()\n");
		}
	}
	// Create a root signature with a single constant buffer slot.
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;

		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

		winrt::com_ptr<ID3DBlob> pSignature;
		ID3DBlob* sig = pSignature.get();
		//winrt::com_ptr<ID3DBlob> pSignature;
		winrt::com_ptr<ID3DBlob> pError;
		ID3DBlob* err = pError.get();
		//winrt::com_ptr<ID3DBlob> pError;
		HRESULT srs = D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &err);
			DX::ThrowIfFailed(srs);
		
		auto rootSig = m_rootSignature.get();
		
		HRESULT rs = d3dDevice->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rootSig));
		m_rootSignature.attach(rootSig);
		DX::ThrowIfFailed(rs);
        //NAME_D3D12_OBJECT(m_rootSignature.get());
		
		//m_rootSignature->SetName(L"m_rootSignature");
		
	}

	// Load shaders asynchronously.
	const wchar_t* s1 = L"SampleVertexShader.cso";
	const wchar_t* s2 = L"SamplePixelShader.cso";
	const winrt::param::hstring& shaderV( s1);
	const winrt::param::hstring& shaderP( s2);
	winrt::Windows::Storage::Streams::IBuffer bufferV = co_await DX::ReadMyDataAsync(shaderV);
	winrt::com_array<uint8_t> vertData(bufferV.Length());
	winrt::Windows::Storage::Streams::DataReader::FromBuffer(bufferV).ReadBytes(vertData);
	
	m_vertexShader.clear();
	m_vertexShader.resize(bufferV.Length());
	for(size_t i = 0;i < bufferV.Length(); i++)
	{
		m_vertexShader.at(i) = static_cast<BYTE>(vertData.at(static_cast<uint32_t>(i)));
	}
	{
		char msg[256];
		sprintf_s(msg, "INFO: Loaded vertex shader bytes=%u\n", static_cast<unsigned>(bufferV.Length()));
		OutputDebugStringA(msg);
	}
	winrt::Windows::Storage::Streams::IBuffer bufferP = co_await DX::ReadMyDataAsync(shaderP);
	winrt::com_array<uint8_t> pixelData(bufferP.Length());
	winrt::Windows::Storage::Streams::DataReader::FromBuffer(bufferP).ReadBytes(pixelData);
	
	m_pixelShader.clear();
	m_pixelShader.resize(bufferP.Length());
	for (size_t i = 0; i < bufferP.Length(); i++)
	{
		m_pixelShader.at(i) = static_cast<BYTE>(pixelData.at(static_cast<uint32_t>(i)));
	}
	{
		char msg[256];
		sprintf_s(msg, "INFO: Loaded pixel shader bytes=%u\n", static_cast<unsigned>(bufferP.Length()));
		OutputDebugStringA(msg);
	}
	// Create the pipeline state once the shaders are loaded.
	//auto createPipelineStateTask = (createPSTask && createVSTask).then([this]() {

		static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.InputLayout = { inputLayout, _countof(inputLayout) };
		state.pRootSignature = m_rootSignature.get();
        state.VS = CD3DX12_SHADER_BYTECODE(&m_vertexShader[0], m_vertexShader.size());
        state.PS = CD3DX12_SHADER_BYTECODE(&m_pixelShader[0], m_pixelShader.size());
		state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.SampleMask = UINT_MAX;
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		state.NumRenderTargets = 1;
		state.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
		state.DSVFormat = m_deviceResources->GetDepthBufferFormat();
		state.SampleDesc.Count = 1;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&m_pipelineState)));

		// Shader data can be deleted once the pipeline state is created.
		m_vertexShader.clear();
		m_pixelShader.clear();
	//});

	// Create and upload cube geometry resources to the GPU.
	// auto createAssetsTask = createPipelineStateTask.then([this]() {
		d3dDevice = m_deviceResources->GetD3DDevice();

		// Create a command list.
		DX::ThrowIfFailed(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.get(), IID_PPV_ARGS(&m_commandList)));
        //NAME_D3D12_OBJECT(m_commandList);
		m_commandList->SetName(L"m_commandList");

		// Cube vertices. Each vertex has a position and a color.
		VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		const UINT vertexBufferSize = sizeof(cubeVertices);

		// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		winrt::com_ptr<ID3D12Resource> vertexBufferUpload;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COMMON, // create in COMMON to avoid creation-time state ignored warning
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));

		m_vertexBuffer->SetName(L"vertexBuffer");

		// Upload the vertex buffer to the GPU.
		{
			// Transition default buffer from COMMON -> COPY_DEST before uploading
			CD3DX12_RESOURCE_BARRIER beforeCopy =
				CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.get(),
					D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			m_commandList->ResourceBarrier(1, &beforeCopy);

			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
			vertexData.RowPitch = vertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresources(m_commandList.get(), m_vertexBuffer.get(), vertexBufferUpload.get(), 0, 0, 1, &vertexData);

			// Then transition from COPY_DEST -> VERTEX_AND_CONSTANT_BUFFER for use by IA
			CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.get(),
					D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
		}

		// Load mesh indices. Each trio of indices represents a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes 0, 2 and 1 from the vertex buffer compose the
		// first triangle of this mesh.
		unsigned short cubeIndices[] =
		{
			0, 2, 1, // -x
			1, 2, 3,

			4, 5, 6, // +x
			5, 7, 6,

			0, 1, 5, // -y
			0, 5, 4,

			2, 6, 7, // +y
			2, 7, 3,

			0, 4, 6, // -z
			0, 6, 2,

			1, 3, 7, // +z
			1, 7, 5,
		};

		const UINT indexBufferSize = sizeof(cubeIndices);

		// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		winrt::com_ptr<ID3D12Resource> indexBufferUpload;

		CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COMMON, // create in COMMON to avoid creation-time state ignored warning
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)));

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBufferUpload)));

		m_indexBuffer->SetName(L"indexBuffer");

		// Upload the index buffer to the GPU.
		{
			// Transition default buffer from COMMON -> COPY_DEST before uploading
			CD3DX12_RESOURCE_BARRIER beforeCopy =
				CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.get(),
					D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
			m_commandList->ResourceBarrier(1, &beforeCopy);

			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
			indexData.RowPitch = indexBufferSize;
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresources(m_commandList.get(), m_indexBuffer.get(), indexBufferUpload.get(), 0, 0, 1, &indexData);

			CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.get(),
					D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
		}

		// Create a descriptor heap for the constant buffers.
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = DX::c_frameCount;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

           //NAME_D3D12_OBJECT(m_cbvHeap);
			m_cbvHeap->SetName(L"m_cbvHeap");
		}

		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(DX::c_frameCount) * c_alignedConstantBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

        //NAME_D3D12_OBJECT(m_constantBuffer);
		m_constantBuffer->SetName(L"m_constantBuffer");

		// Create constant buffer views to access the upload buffer.
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		m_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (int n = 0; n < DX::c_frameCount; n++)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = c_alignedConstantBufferSize;
			d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

			cbvGpuAddress += desc.SizeInBytes;
			cbvCpuHandle.Offset(m_cbvDescriptorSize);
		}

		// Map the constant buffers.
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount * c_alignedConstantBufferSize);
		// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

		// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.

		DX::ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.get() };
		
		ID3D12CommandQueue* cq = m_deviceResources->GetCommandQueue();
		cq->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Create vertex/index buffer views.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
		m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

		// Wait for the command list to finish executing; the vertex/index buffers need to be uploaded to the GPU before the upload resources go out of scope.
		m_deviceResources->WaitForGpu();
	//});

	//createAssetsTask.then([this]() {
		m_loadingComplete = true;
	//});
}

// Initializes view parameters when the window size changes.
void Sample3DSceneRenderer::CreateWindowSizeDependentResources()
{
	wf::Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
	m_scissorRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height)};

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();
	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void Sample3DSceneRenderer::Update(DX::StepTimer const& timer)
{
	if (m_loadingComplete)
	{
		if (!m_tracking)
		{
			// Rotate the cube a small amount.
			m_angle += static_cast<float>(timer.GetElapsedSeconds()) * m_radiansPerSecond;

			Rotate(m_angle);
		}

		// Update the constant buffer resource.
		UINT8* destination = m_mappedConstantBuffer + (m_deviceResources->GetCurrentFrameIndex() * c_alignedConstantBufferSize);
		memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));
	}
}

// Saves the current state of the renderer.
void Sample3DSceneRenderer::SaveState()
{
	winrt::Windows::Foundation::Collections::IPropertySet
	state = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values();

	if (state.TryLookup(AngleKey))
	{
		state.TryRemove(AngleKey);
	}
	if (state.TryLookup(TrackingKey))
	{
		state.TryRemove(TrackingKey);
	}

	// Use Insert for IPropertySet, not operator[]
	//state.Insert(AngleKey, winrt::Windows::Foundation::PropertyValue::CreateSingle(m_angle));
	//state.Insert(TrackingKey, winrt::Windows::Foundation::PropertyValue::CreateBoolean(m_tracking));
}

// Restores the previous state of the renderer.
void Sample3DSceneRenderer::LoadState()
{
	winrt::Windows::Foundation::Collections::IPropertySet
	state = winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values();
	if (state.TryLookup(AngleKey))
	{
		m_angle = static_cast<float>(winrt::unbox_value<float>(state.TryLookup(AngleKey)));
		state.TryRemove(AngleKey);
	}
	if (state.TryLookup(TrackingKey))
	{
		//m_tracking = state.Lookup(TrackingKey).as<winrt::Windows::Foundation::IPropertyValue>().GetBoolean();
		m_tracking = static_cast<float>(winrt::unbox_value<float>(state.TryLookup(TrackingKey)));
		state.TryRemove(TrackingKey);
	}
}

// Rotate the 3D cube model a set amount of radians.
void Sample3DSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader.
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Sample3DSceneRenderer::StartTracking()
{
	m_tracking = true;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void Sample3DSceneRenderer::TrackingUpdate(float positionX, float positionY)
{
	if (m_tracking)
	{
		float radiansy = XM_2PI * 2.0f * positionY / m_deviceResources->GetOutputSize().Height;
		float radians = XM_2PI * 2.0f * positionX / m_deviceResources->GetOutputSize().Width;
		Rotate(radians);
		Rotate(radiansy);
	}
}

void Sample3DSceneRenderer::StopTracking()
{
	m_tracking = false;
}

void Sample3DSceneRenderer::OnDeviceLost()
{
	/*
	ReleaseDeviceDependentResources();
	m_bDDS_WIC_FLAGGridPicComplete = false;
	m_bDDS_WIC_FLAGGridPic = false;
	m_loadingComplete = false;
	m_loadingDrawnObjectComplete = false;

	if (m_bIsPBRModel == true)
	{
		m_resDescPile.reset();
	}
	m_bIsBasicModelColor = false;
	m_bIsBasicModel = false;
	m_bIsDualTextureModel = false;
	m_bIsPBRModel = false;
	m_bIsSculptWireframe = false;
	m_bIsVideoTextureModel = false;

	m_MousePosFont.reset();
	//m_CursorPosFont.reset();
	m_CameraEyeFont.reset();
	m_CameraAtFont.reset();
	m_CameraUpFont.reset();
	m_PointCountFont.reset();
	m_MAX_PointCountFont.reset();
	m_TotalPointCountFont.reset();
	m_FaceCountFont.reset();
	m_GroupCountFont.reset();
	m_SelectedPointNumberFont.reset();

	m_batch = nullptr;
	m_shape = nullptr;
	m_shapeTetra = nullptr;
	m_shapeGridPic = nullptr;
	m_model.reset();

	m_lineEffect = nullptr;

	m_cursorEffect = nullptr;

	m_shapeEffect = nullptr;

	m_shapeTetraEffect = nullptr;
	m_drawRectangleEffect = nullptr;
	m_DrawnMeshTexture1 = nullptr;
	m_shapeDrawnObjectEffect = nullptr;
	m_shapeDrawnObjectTex = nullptr;

	m_DrawnMeshTexture2 = nullptr;
	m_dualTextureEffect = nullptr;
	m_hot3dxDrawnObjectDual = nullptr;

	m_PBRTexture1 = nullptr;
	m_PBRTexture2 = nullptr;
	m_RMATexture = nullptr;
	m_NormalTexture = nullptr;
	m_radianceIBL = nullptr;
	m_irradianceIBL = nullptr;
	m_shapeDrawnObjectPBREffect = nullptr;
	m_hot3dxDrawnObjectPBR = nullptr;

	m_videoTexture = nullptr;
	m_shapeDrawnObjectVideoEffect = nullptr;
	m_shapeDrawnObjectVideoTex = nullptr;

	m_shapeDrawnObjectEffectSculpt = nullptr;
	m_shapeDrawnObjectSculpt = nullptr;
	m_shapeDrawnObjectColor = nullptr;
	//m_modelEffects.clear();
	//m_modelResources.reset();
	m_sprites.reset();
	m_resourceDescriptors.reset();

	m_states.reset();
	m_graphicsMemory.reset();
	pos.clear();
	pos.resize(0);

	posX = ref new Platform::Array<float>(10000);
	posY = ref new Platform::Array<float>(10000);
	posZ = ref new Platform::Array<float>(10000);
	box = ref new Platform::Array<float>(6);

	m_iGroupCount = 0;
	m_iPointCount = 0;
	m_iTotalPointCount = 0;

	m_iTempGroup = ref new Platform::Array<unsigned int>(10000);
	m_iTempMouseX = ref new Platform::Array<float>(10000);
	m_iTempMouseY = ref new Platform::Array<float>(10000);
	m_iTempGroupCount = 0;

	m_PtGroupList.~vector();

	vertices.~vector();
	vertexes.~vector();
	indices.~vector();
	textureU.~vector();
	textureV.~vector();
	verticesDual.~vector();
	verticesPBR.~vector();

	// not in original must watch
	m_loadingComplete = false;
	m_loadingDrawnObjectComplete = false;
	*/
	//CreateDeviceDependentResources();
}

void Sample3DSceneRenderer::OnDeviceRestored()
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
	
}

// Renders one frame using the vertex and pixel shaders.
bool Sample3DSceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		//OutputDebugString(L"\n Entered !m_loadingComplete m_sceneRenderer->Render()\n");
		return false;
	}
	//OutputDebugString(L"\n Entered m_sceneRenderer->Render()\n");

	// Ensure previous GPU work that used this allocator has finished before resetting it.
	// This prevents the COMMAND_ALLOCATOR_SYNC error.
	m_deviceResources->WaitForGpu();

	DX::ThrowIfFailed(m_deviceResources->GetCommandAllocator()->Reset());

	// The command list can be reset anytime after ExecuteCommandList() is called.
	DX::ThrowIfFailed(m_commandList->Reset(m_deviceResources->GetCommandAllocator(), m_pipelineState.get()));

	PIXBeginEvent(m_commandList.get(), 0, L"Draw the cube");
	{
		// Set the graphics root signature and descriptor heaps to be used by this frame.
		m_commandList->SetGraphicsRootSignature(m_rootSignature.get());
		ID3D12DescriptorHeap* ppHeaps[] = { m_cbvHeap.get() };
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		// Bind the current frame's constant buffer to the pipeline.
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), m_deviceResources->GetCurrentFrameIndex(), m_cbvDescriptorSize);
		m_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

		// Set the viewport and scissor rectangle.
		D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
		m_commandList->RSSetViewports(1, &viewport);
		m_commandList->RSSetScissorRects(1, &m_scissorRect);

		// Indicate this resource will be in use as a render target.
		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// Record drawing commands.
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
		m_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		m_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
		m_commandList->IASetIndexBuffer(&m_indexBufferView);
		m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		// Indicate that the render target will now be used to present when the command list is done executing.
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_commandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(m_commandList.get());

	DX::ThrowIfFailed(m_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.get() };
	m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}

void winrt::Hot3dxBlankApp2::Sample3DSceneRenderer::Clear()
{
	PIXBeginEvent(m_commandList.get(), PIX_COLOR_DEFAULT, L"Clear");

	// Record drawing commands.
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = m_deviceResources->GetRenderTargetView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = m_deviceResources->GetDepthStencilView();
	m_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
	m_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

	// Set the viewport and scissor rectangle.
	D3D12_VIEWPORT viewport = m_deviceResources->GetScreenViewport();
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	PIXEndEvent(m_commandList.get());
}

