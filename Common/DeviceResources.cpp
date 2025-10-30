﻿#include "pch.h"

#include "d3dx12.h"
#include "DeviceResources.h"
#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgiformat.h>
#include "DirectXHelper.h"
#include <windows.h>
#include <synchapi.h>
#include <unknwn.h>
#include <winrt/windows.foundation.h>
#include <algorithm>

#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.controls.h>

using namespace DirectX;

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Controls;

// Local minimal COM interface declaration for ISwapChainPanelNative.
// This resolves "pointer or reference to incomplete type" errors by providing
// the method signature used in this file.
// IID: 94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1
struct __declspec(uuid("94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1")) ISwapChainPanelNative : public ::IUnknown
{
    virtual HRESULT __stdcall SetSwapChain(::IDXGISwapChain* swapChain) = 0;
};

namespace DisplayMetrics
{
	static const bool SupportHighResolutions = false;
	static const float DpiThreshold = 192.0f;
	static const float WidthThreshold = 1920.0f;
	static const float HeightThreshold = 1080.0f;
};

// ScreenRotation constants (unchanged)
namespace ScreenRotation
{
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};

// Constructor for DeviceResources.
DX::DeviceResources::DeviceResources(DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat) :
	m_currentFrame(0),
	m_screenViewport(),
	m_rtvDescriptorSize(0),
	m_fenceEvent(nullptr),
	m_backBufferFormat(backBufferFormat),
	m_depthBufferFormat(depthBufferFormat),
	m_fenceValues{},
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_deviceRemoved(false)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CreateWindowSizeDependentResources();
}

void DX::DeviceResources::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel, winrt::Windows::UI::Core::CoreWindow const& window)
{
	m_swapChainPanel = panel;

	DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_logicalSize = winrt::Windows::Foundation::Size(window.Bounds().Width, window.Bounds().Height);
	m_nativeOrientation = currentDisplayInformation.NativeOrientation();
	m_currentOrientation = currentDisplayInformation.CurrentOrientation();
	m_dpi = currentDisplayInformation.LogicalDpi();

	//CreateWindowSizeDependentResources();
}

// Configures resources that don't depend on the Direct3D device.
void DX::DeviceResources::CreateDeviceIndependentResources()
{
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DX::DeviceResources::CreateDeviceResources()
{
#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers.
	{
		winrt::com_ptr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory)));

// Acquire adapter
IDXGIAdapter1* adapterRaw = nullptr;
GetHardwareAdapter(&adapterRaw);
winrt::com_ptr<IDXGIAdapter1> adapter;
if (adapterRaw)
{
	adapter.attach(adapterRaw);
}

// Create the Direct3D 12 API device object
HRESULT hr = D3D12CreateDevice(
	adapter.get(),                    // The hardware adapter.
	D3D_FEATURE_LEVEL_11_0,           // Minimum feature level this app can support.
	IID_PPV_ARGS(&m_D3dDevice)        // Returns the Direct3D device created.
);

#if defined(_DEBUG)
if (FAILED(hr))
{
	// If the initialization fails, fall back to the WARP device.
	winrt::com_ptr<IDXGIAdapter> WarpAdapter;
	winrt::check_hresult(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));
	hr = D3D12CreateDevice(WarpAdapter.get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_D3dDevice));
}
#endif

DX::ThrowIfFailed(hr);

// Create the command queue.
D3D12_COMMAND_QUEUE_DESC queueDesc = {};
queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
DX::ThrowIfFailed(m_D3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));

// Create descriptor heaps for render target views and depth stencil views.
D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
rtvHeapDesc.NumDescriptors = c_frameCount;
rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
DX::ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)));

m_rtvDescriptorSize = m_D3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
dsvHeapDesc.NumDescriptors = 1;
dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
DX::ThrowIfFailed(m_D3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap)));
m_DsvHeap->SetName(L"m_dsvHeap");

for (UINT n = 0; n < c_frameCount; n++)
{
	DX::ThrowIfFailed(
		m_D3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocators[n]))
	);
}

// Create synchronization objects.
DX::ThrowIfFailed(m_D3dDevice->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
m_fenceValues[m_currentFrame]++;

m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
if (m_fenceEvent == nullptr)
{
	DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
}
}

// These resources need to be recreated every time the window size is changed.
void DX::DeviceResources::CreateWindowSizeDependentResources()
{
	// Wait until all previous GPU work is complete.
	WaitForGpu();

	// Clear the previous window size specific content and update the tracked fence values.
	for (UINT n = 0; n < c_frameCount; n++)
	{
		m_RenderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[m_currentFrame];
	}

	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	UINT backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	UINT backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	if (m_SwapChain)
	{
		// If the swap chain already exists, resize it.
		HRESULT hrResize = m_SwapChain->ResizeBuffers(c_frameCount, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (hrResize == DXGI_ERROR_DEVICE_REMOVED || hrResize == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			m_deviceRemoved = true;
			return;
		}
		else
		{
			DX::ThrowIfFailed(hrResize);
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;						// Match the size of the window.
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = c_frameCount;					// Use triple-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		HRESULT hrCreate = m_DxgiFactory->CreateSwapChainForComposition(
			m_CommandQueue.get(),		// Swap chains need a reference to the command queue in DirectX 12.
			&swapChainDesc,
			nullptr,
			reinterpret_cast<IDXGISwapChain1**>(m_SwapChain.put())
		);

		DX::ThrowIfFailed(hrCreate);

		// Promote to IDXGISwapChain3
		winrt::com_ptr<IDXGISwapChain3> swapChain3;
		winrt::com_ptr<IDXGISwapChain1> swapChain1 = m_SwapChain;
		swapChain1.as(swapChain3);
		m_SwapChain = swapChain3;

		// Attach swap chain to SwapChainPanel via ISwapChainPanelNative (must be done on UI thread)

		if (m_swapChainPanel)
		{
			// Get the ISwapChainPanelNative interface from the SwapChainPanel
			winrt::com_ptr<::IUnknown> panelUnknown = nullptr;
			panelUnknown.attach(reinterpret_cast<::IUnknown*>(winrt::get_abi(m_swapChainPanel)));

			// ISwapChainPanelNative is a COM interface, not a WinRT interface.
			ISwapChainPanelNative* panelNative = nullptr;
			HRESULT hrQ = panelUnknown->QueryInterface(__uuidof(ISwapChainPanelNative), reinterpret_cast<void**>(&panelNative));
			if (SUCCEEDED(hrQ) && panelNative)
			{
				HRESULT hrSet = panelNative->SetSwapChain(m_SwapChain.get());
				if (FAILED(hrSet))
				{
					OutputDebugStringA("ERROR: ISwapChainPanelNative::SetSwapChain FAILED\n");
				}
				panelNative->Release();
			}
			else
			{
				OutputDebugStringA("ERROR: QueryInterface(ISwapChainPanelNative) FAILED\n");
			}
		}
		
	}

		// Set the proper orientation for the swap chain, and generate 3D matrix transformations.
		if (displayRotation == DXGI_MODE_ROTATION_UNSPECIFIED)
		{
			// Some platforms/drivers may return UNSPECIFIED (0). Map that to IDENTITY
			// so we don't call SetRotation with an invalid value.
			displayRotation = DXGI_MODE_ROTATION_IDENTITY;
		}

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		// Defensive fallback
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;
	}

	HRESULT hrRotate = m_SwapChain->SetRotation(displayRotation);
	if (FAILED(hrRotate))
	{
		char buf[128];
		sprintf_s(buf, "ERROR: IDXGISwapChain::SetRotation FAILED: %08X\n", static_cast<unsigned>(hrRotate));
		OutputDebugStringA(buf);
	}
	DX::ThrowIfFailed(hrRotate);



	// Create render target views of the swap chain back buffer.
	{
		m_currentFrame = m_SwapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT n = 0; n < c_frameCount; n++)
		{
			DX::ThrowIfFailed(m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n])));
			ID3D12Resource* renderTarget = m_RenderTargets[n].get();
			m_D3dDevice->CreateRenderTargetView(renderTarget, nullptr, rtvDescriptor);
			rtvDescriptor.Offset(m_rtvDescriptorSize);

			WCHAR name[32];
			if (swprintf_s(name, L"m_renderTargets[%u]", n) > 0)
			{
				DX::SetName(renderTarget, (LPCWSTR)name);
			}
		}
	}

	// Create a depth stencil and view.
	{
		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_depthBufferFormat, 1.0f, 0);

		DX::ThrowIfFailed(m_D3dDevice->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_DepthStencil)
		));

		m_D3dDevice->SetName(L"DepthStencil");

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_D3dDevice->CreateDepthStencilView(m_DepthStencil.get(), &dsvDesc, m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };
}

// Determine the dimensions of the render target and whether it will be scaled down.
void DX::DeviceResources::UpdateRenderTargetSize()
{
	m_effectiveDpi = m_dpi;

	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold)
	{
		float width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_dpi);
		float height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_dpi);

		if (max(width, height) > DisplayMetrics::WidthThreshold && min(width, height) > DisplayMetrics::HeightThreshold)
		{
			m_effectiveDpi /= 2.0f;
		}
	}

	// Calculate the necessary render target size in pixels.
	m_outputSize.Width = DX::ConvertDipsToPixels(m_logicalSize.Width, m_effectiveDpi);
	m_outputSize.Height = DX::ConvertDipsToPixels(m_logicalSize.Height, m_effectiveDpi);

	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = max(m_outputSize.Width, 1);
	m_outputSize.Height = max(m_outputSize.Height, 1);
}

// This method is called when the CoreWindow is created (or re-created).
void DX::DeviceResources::SetWindow(winrt::Windows::UI::Core::CoreWindow const& window)
{
	DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = winrt::Windows::Foundation::Size(window.Bounds().Width, window.Bounds().Height);
	m_nativeOrientation = currentDisplayInformation.NativeOrientation();
	m_currentOrientation = currentDisplayInformation.CurrentOrientation();
	m_dpi = currentDisplayInformation.LogicalDpi();

	//CreateWindowSizeDependentResources();
}

// This method is called in the event handler for the SizeChanged event.
void DX::DeviceResources::SetLogicalSize(winrt::Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DpiChanged event.
void DX::DeviceResources::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		if (m_window)
		{
			auto windowObj = m_window.get();
			m_logicalSize = winrt::Windows::Foundation::Size(windowObj.Bounds().Width, windowObj.Bounds().Height);
		}

		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the OrientationChanged event.
void DX::DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DX::DeviceResources::ValidateDevice()
{
	DXGI_ADAPTER_DESC previousDesc;
	{
		winrt::com_ptr<IDXGIAdapter1> previousDefaultAdapter;
		IDXGIAdapter1* prevDefaultAdapter = nullptr;
		DX::ThrowIfFailed(m_DxgiFactory->EnumAdapters1(0, &prevDefaultAdapter));
		previousDefaultAdapter.attach(prevDefaultAdapter);
		DX::ThrowIfFailed(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	DXGI_ADAPTER_DESC currentDesc;
	{
		winrt::com_ptr<IDXGIFactory4> currentDxgiFactory;
		DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		winrt::com_ptr<IDXGIAdapter1> currentDefaultAdapter;
		IDXGIAdapter1* curDefaultAdapter = nullptr;
		DX::ThrowIfFailed(currentDxgiFactory->EnumAdapters1(0, &curDefaultAdapter));
		currentDefaultAdapter.attach(curDefaultAdapter);
		DX::ThrowIfFailed(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_D3dDevice->GetDeviceRemovedReason()))
	{
		m_deviceRemoved = true;
	}
}

// Present the contents of the swap chain to the screen.
void DX::DeviceResources::Present()
{
	HRESULT hr = m_SwapChain->Present(1, 0);
	if (FAILED(hr))
	{
		OutputDebugStringA("ERROR: IDXGISwapChain::Present FAILED: ");
		char buf[64]; sprintf_s(buf, "%08X\n", static_cast<unsigned>(hr)); OutputDebugStringA(buf);
	}
	else
	{
		OutputDebugStringA("INFO: Present OK\n");
	}

	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		DX::ThrowIfFailed(hr);
		MoveToNextFrame();
	}
}

// Wait for pending GPU work to complete.
void DX::DeviceResources::WaitForGpu()
{
	if (m_Fence)
	{
		DX::ThrowIfFailed(m_CommandQueue->Signal(m_Fence.get(), m_fenceValues[m_currentFrame]));
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		m_fenceValues[m_currentFrame]++;
	}
}

void DX::DeviceResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify)
{
	m_deviceNotify = deviceNotify;
}

// Prepare to render the next frame.
void DX::DeviceResources::MoveToNextFrame()
{
	const UINT64 currentFenceValue = m_fenceValues[m_currentFrame];
	DX::ThrowIfFailed(m_CommandQueue->Signal(m_Fence.get(), currentFenceValue));

	m_currentFrame = m_SwapChain->GetCurrentBackBufferIndex();

	if (m_Fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
	{
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	m_fenceValues[m_currentFrame] = currentFenceValue + 1;
}

// This method determines the rotation between the display device's native Orientation and the
// current display orientation.
DXGI_MODE_ROTATION DX::DeviceResources::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;
		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;
		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;
		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;
		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}

// This method acquires the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
void DX::DeviceResources::GetHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	IDXGIAdapter1* adapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_DxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter;
}