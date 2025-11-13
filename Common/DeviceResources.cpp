#include "pch.h"
#include "MainPage.h"
#include "d3dx12.h"
#include "DeviceResources.h"
#include <d3d12.h>
#include <intsafe.h>
#include <dxgidebug.h>
#include <dxgiformat.h>
#include "DirectXHelper.h"
#include "DirectXIUnknowns.h"
#include <windows.h>
#include <synchapi.h>
#include <unknwn.h>
#include <winrt/windows.foundation.h>
#include <algorithm>
#include <winrt/base.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.controls.h>

using namespace DirectX;

namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Controls;

#define LOG_STATUS(isError, fmt, ...) LogStatusImpl(__FILE__, __LINE__, isError, fmt, ##__VA_ARGS__)

// add near top of file (replace previous LogStatus)
#include <cstdio>
#include <string>
// Macro to capture source location automatically


inline static void LogStatusImpl(const char* file, int line, bool isError, const char* fmt, ...)
{

	char msgBuf[1024];
	va_list ap;
	va_start(ap, fmt);
#if defined(_MSC_VER)
	vsnprintf_s(msgBuf, _countof(msgBuf), _TRUNCATE, fmt, ap);
#else
	vsnprintf(msgBuf, sizeof(msgBuf), fmt, ap);
#endif
	va_end(ap);

	// Shorten file path to filename
	std::string fname = file ? file : "unknown";
	size_t pos = fname.find_last_of("\\/");
	if (pos != std::string::npos) fname = fname.substr(pos + 1);

	// Build final ANSI/UTF-8 message: "filename(line): message"
	char finalBuf[1280];
	snprintf(finalBuf, sizeof(finalBuf), "%s(%d): %s", fname.c_str(), line, msgBuf);

	// Convert to wide string assuming UTF-8 input (use CP_UTF8). Change code page if needed.
	std::wstring wbuf;
	int wlen = MultiByteToWideChar(CP_UTF8, 0, finalBuf, -1, nullptr, 0);
	if (wlen > 0)
	{
		wbuf.resize(static_cast<size_t>(wlen - 1));
		MultiByteToWideChar(CP_UTF8, 0, finalBuf, -1, &wbuf[0], wlen);
	}

	// Forward to visible status panel when available, else fallback to debug output.


	auto page = winrt::Hot3dxBlankApp2::implementation::MainPage::Current();
	if (page)
	{
		auto notifyType = isError
			? winrt::Hot3dxBlankApp2::implementation::NotifyType::ErrorMessage
			: winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage;

		// get_self gives access to the implementation instance so we can call the
		// implementation-only NotifyUser() member that isn't projected on the public type.
		auto impl = winrt::get_self<winrt::Hot3dxBlankApp2::implementation::MainPage>(page);
		impl->NotifyUser(winrt::hstring{ wbuf }, notifyType);
#ifndef _DEBUG
		OutputDebugStringW((wbuf + L"\n").c_str());
#endif
	}
	else
	{
#ifndef _DEBUG
		OutputDebugStringW((wbuf + L"\n").c_str());
#endif
	}
}

// Local minimal COM interface declaration for ISwapChainPanelNative.
// This resolves "pointer or reference to incomplete type" errors by providing
// the method signature used in this file.
// IID: 94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1
//struct __declspec(uuid("94D99BDB-F1F8-4AB0-B236-7DA0170EDAB1")) ISwapChainPanelNative : public ::IUnknown
//{
//    virtual HRESULT __stdcall SetSwapChain(::IDXGISwapChain* swapChain) = 0;
//};

namespace DisplayMetrics
{
	inline static const bool SupportHighResolutions = false;
	inline static const float DpiThreshold = 192.0f;
	inline static const float WidthThreshold = 1920.0f;
	inline static const float HeightThreshold = 1080.0f;
};

// ScreenRotation constants (unchanged)
namespace ScreenRotation
{
	inline static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	inline static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	inline static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	inline static const XMFLOAT4X4 Rotation270(
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
	
}

void DX::DeviceResources::SetSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel, winrt::Windows::UI::Core::CoreWindow const& window)
{
    // Must be called on UI thread
	// Validate UI thread and marshal or bail out if not on UI thread.
	// Ensure this is called on the UI thread by using the provided CoreWindow's dispatcher.
	// Do not call CoreApplication::MainView() here (that can trigger C3779 when headers/order make
	// the function-generic 'auto' return unavailable at this translation unit).
	auto dispatcher = window.Dispatcher();
	if (!dispatcher.HasThreadAccess())
	{
		LOG_STATUS(true, "SetSwapChainPanel must be called on the UI thread");
		return;
	}

    
        // Caller should marshal to UI thread; fail fast here or assert in debug
       // Must be called on UI thread
    

	m_swapChainPanel = panel;

	// update logical size and DPI/rotation
	float aw = static_cast<float>(panel.ActualWidth());
	float ah = static_cast<float>(panel.ActualHeight());

	// Prefer the SwapChainPanel measured size for logical size (in DIPs) when available.
	// Fallback to CoreWindow bounds when panel measurement is not ready.
	if (aw > 0.0f && ah > 0.0f)
	{
		m_logicalSize = winrt::Windows::Foundation::Size(aw, ah);
	}
	else
	{
		m_logicalSize = winrt::Windows::Foundation::Size(window.Bounds().Width, window.Bounds().Height);
	}

	DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();
	m_nativeOrientation = currentDisplayInformation.NativeOrientation();
	m_currentOrientation = currentDisplayInformation.CurrentOrientation();
	m_dpi = currentDisplayInformation.LogicalDpi();

	m_isSwapPanelVisible = true;
	//CreateWindowSizeDependentResources();
	
	if (m_SwapChain && m_swapChainPanel)
		{
			HRESULT hr = AttachSwapChainToSwapChainPanel(m_swapChainPanel, m_SwapChain.get());
			if (FAILED(hr))
			{
				LOG_STATUS(true, "AttachSwapChainToSwapChainPanel failed: 0x%08X", static_cast<unsigned>(hr));
			}
		}
	
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
	char buf[256];
	sprintf_s(buf, "WARN: D3D12CreateDevice failed: 0x%08X\n", static_cast<unsigned>(hr));
#ifndef _DEBUG	
	OutputDebugStringA(buf);
#endif


	// Try WARP but log result
	winrt::com_ptr<IDXGIAdapter> WarpAdapter;
	HRESULT hrEnum = m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter));
	sprintf_s(buf, "INFO: EnumWarpAdapter returned: 0x%08X\n", static_cast<unsigned>(hrEnum));
#ifndef _DEBUG
	OutputDebugStringA(buf);
#endif

	if (SUCCEEDED(hrEnum))
	{
		HRESULT hrWarp = D3D12CreateDevice(WarpAdapter.get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_D3dDevice));
		sprintf_s(buf, "INFO: D3D12CreateDevice(WARP) returned: 0x%08X\n", static_cast<unsigned>(hrWarp));
#ifndef _DEBUG
		OutputDebugStringA(buf);
#endif
		hr = hrWarp;
	}
}
DX::ThrowIfFailed(hr);
#endif

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

void DX::DeviceResources::SetCompositionScale(float CompositionScaleX, float CompositionScaleY)
{
	if (m_compositionScaleX != CompositionScaleX ||
		m_compositionScaleY != CompositionScaleY)
	{
		m_compositionScaleX = CompositionScaleX;
		m_compositionScaleY = CompositionScaleY;
		CreateWindowSizeDependentResources();
	}
}

// These resources need to be recreated every time the window size is changed.
void DX::DeviceResources::CreateWindowSizeDependentResources()
{
#ifndef _DEBUG
#pragma warning(suppress : 4996)
	OutputDebugStringA("DeviceResources::CreateWindowSizeDependentResources - begin\n");
#endif
	// Suppress noisy warnings in release builds
	// 
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

	// If we have a SwapChainPanel, account for its composition scale (the panel can be DPI/scaled).
	if (m_SwapChain)
	{
		try
		{
			float csx = static_cast<float>(m_swapChainPanel.CompositionScaleX());
			float csy = static_cast<float>(m_swapChainPanel.CompositionScaleY());

			// CompositionScale multiplies DIPs to compositor pixels; apply before creating swap chain
			// (convert logical DIPs -> pixels already happened via UpdateRenderTargetSize using DPI).
			UINT scaledWidth = static_cast<UINT>(max(1u, static_cast<UINT>(std::lround(backBufferWidth * csx))));
			UINT scaledHeight = static_cast<UINT>(max(1u, static_cast<UINT>(std::lround(backBufferHeight * csy))));

			char buf[256];
			sprintf_s(buf, "INFO: Panel composition scale applied: csx=%.2f csy=%.2f -> pixel W=%u H=%u (pre=%u,%u)\n",
				csx, csy, scaledWidth, scaledHeight, backBufferWidth, backBufferHeight);
#ifndef _DEBUG 
			OutputDebugStringA(buf);
#endif

			backBufferWidth = scaledWidth;
			backBufferHeight = scaledHeight;
		}
		catch (...)
		{
#ifndef _DEBUG
			OutputDebugStringA("\nline 362 DevRes.cppWARN: Failed to read CompositionScale from SwapChainPanel; using default sizes\n");
#endif
		}
	}

	if (m_SwapChain)
	{
		// If the swap chain already exists, resize it.
		HRESULT hrResize = m_SwapChain->ResizeBuffers(c_frameCount, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);
		if (hrResize != S_OK)
		{
			char buf[256];
			sprintf_s(buf, "\nINFO: SwapChain ResizeBuffers W=%u H=%u hr=0x%08X\n",
				backBufferWidth, backBufferHeight, static_cast<unsigned>(hrResize));
#ifndef _DEBUG
			OutputDebugStringA(buf);
#endif
		}
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
		{
			if (m_SwapChain)
			{
				DXGI_SWAP_CHAIN_DESC1 desc = {};
				if (SUCCEEDED(m_SwapChain->GetDesc1(&desc)))
				{
					char buf[256];
					sprintf_s(buf, "INFO: SwapChainDesc Width=%u Height=%u Buffers=%u Format=%u SwapEffect=%u\n",
						desc.Width, desc.Height, desc.BufferCount, desc.Format, desc.SwapEffect);
#ifndef _DEBUG
					OutputDebugStringA(buf);
#endif
				}
				else
				{
#ifndef _DEBUG
					OutputDebugStringA("WARN: Failed to get SwapChainDesc1\n");
#endif
				}

				// Confirm GetBuffer / RTV creation
				for (UINT n = 0; n < c_frameCount; ++n)
				{
					winrt::com_ptr<ID3D12Resource> buf;
					HRESULT hr = m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&buf));
					char bufMsg[128];
					sprintf_s(bufMsg, "INFO: GetBuffer(%u) hr=%08X ptr=%p\n", n, static_cast<unsigned>(hr), static_cast<void*>(buf.get()));
#ifndef _DEBUG
					OutputDebugStringA(bufMsg);
#endif
				}
			}
			else
			{
#ifndef _DEBUG
				OutputDebugStringA("ERROR: m_SwapChain is null after creation\n");
#endif
			}
		}
	}
		// Promote to IDXGISwapChain3
		winrt::com_ptr<IDXGISwapChain3> swapChain3;
		winrt::com_ptr<IDXGISwapChain1> swapChain1 = m_SwapChain;
		swapChain1.as(swapChain3);
		m_SwapChain = swapChain3;
		/*
		// Attach swap chain to SwapChainPanel via ISwapChainPanelNative (must be done on UI thread)
		m_swapChainPanel.Dispatcher().RunAsync(
			winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
			[this]() {
				HRESULT hrQ = S_OK;
			});
		*/
		// NEW: Attach swap chain to the SwapChainPanel if available
		if (m_swapChainPanel && m_SwapChain)
		{
			HRESULT hr = AttachSwapChainToSwapChainPanel(m_swapChainPanel, m_SwapChain.get());
			if (FAILED(hr))
			{
				// optional: log via your LogStatusImpl or OutputDebugString
				char buf[256];
				sprintf_s(buf, "WARN: AttachSwapChainToSwapChainPanel failed: 0x%08X\n", static_cast<unsigned>(hr));
#ifndef _DEBUG
				OutputDebugStringA(buf);
#endif
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
#ifndef _DEBUG
		OutputDebugStringA(buf);
#endif
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
#ifndef _DEBUG
	OutputDebugStringA("DeviceResources::CreateWindowSizeDependentResources - end\n");
#endif // !_DEBUG

}
void DX::DeviceResources::HandleDeviceLost()
{
	// Ensure any GPU work is finished before we release resources.
	// This prevents final-release while GPU still references resources.
	try
	{
		if (m_CommandQueue && m_Fence && m_fenceEvent)
		{
			// Signal and wait for GPU to finish outstanding work.
			WaitForGpu();
		}
	}
	catch (...)
	{
		// Best-effort — continue with cleanup even if WaitForGpu fails.
	}

	if (m_deviceNotify)
	{
		m_deviceNotify->OnDeviceLost();
	}

	// Reset command allocators safely.
	for (UINT n = 0; n < c_frameCount; n++)
	{
		if (m_CommandAllocators[n])
		{
			// Reset may fail if allocator is already in a bad state; ignore failures here.
			HRESULT hr = m_CommandAllocators[n]->Reset();
			(void)hr;
		}
		m_RenderTargets[n] = nullptr;
	}

	// Release size-dependent and core D3D objects in a deterministic order.
	m_DepthStencil = nullptr;

	// Make sure command queue is flushed and released after WaitForGpu above.
	m_CommandQueue = nullptr;

	m_Fence = nullptr;
	m_RtvHeap = nullptr;
	m_DsvHeap = nullptr;

	// Release swap chain and device last.
	m_SwapChain = nullptr;
	m_D3dDevice = nullptr;
	m_DxgiFactory = nullptr;

#ifdef _DEBUG
	{
		winrt::com_ptr<IDXGIDebug1> dxgiDebug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}
#endif

	// Recreate device and window-size dependent resources.
	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	if (m_deviceNotify)
	{
		m_deviceNotify->OnDeviceRestored();
	}
}

/*
void DX::DeviceResources::HandleDeviceLost()
{

		if (m_deviceNotify)
		{
			m_deviceNotify->OnDeviceLost();
		}

		for (UINT n = 0; n < c_frameCount; n++)
		{
			m_CommandAllocators[n].get()->Reset();
			m_RenderTargets[n] = nullptr;
		}

		m_DepthStencil = nullptr;
		m_CommandQueue = nullptr;
		m_Fence = nullptr;
		m_RtvHeap = nullptr;
		m_DsvHeap = nullptr;
		m_SwapChain = nullptr;
		m_D3dDevice = nullptr;
		m_DxgiFactory = nullptr;

#ifdef _DEBUG
		{
			winrt::com_ptr<IDXGIDebug1> dxgiDebug;
			//ComPtr<IDXGIDebug1> dxgiDebug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
			{
				dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
			}
		}
#endif

		CreateDeviceResources();
		CreateWindowSizeDependentResources();

		if (m_deviceNotify)
		{
			m_deviceNotify->OnDeviceRestored();
		}
	
}
*/
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
	{
		char buf[128];
		sprintf_s(buf, "INFO: SwapChain::Present returned 0x%08X CurrentBackBufferIndex=%u\n",
			static_cast<unsigned>(hr), m_SwapChain ? m_SwapChain->GetCurrentBackBufferIndex() : 0xFFFFFFFF);
#ifndef _DEBUG
		OutputDebugStringA(buf);
#endif
	}

	if (FAILED(hr))
	{
#ifndef _DEBUG
		OutputDebugStringA("ERROR: IDXGISwapChain::Present FAILED: ");
#endif
		char buf[64]; sprintf_s(buf, "%08X\n", static_cast<unsigned>(hr));
#ifndef _DEBUG
		OutputDebugStringA(buf);
#endif
	}
	else
	{
#ifndef _DEBUG
		OutputDebugStringA("INFO: Present OK\n");
#endif
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


// Temporary test: clear backbuffer and present once (call after CreateWindowSizeDependentResources())
void DX::DeviceResources::TestClearAndPresentOnce()
{
	// reset allocator & command list
	m_CommandAllocators[m_currentFrame]->Reset();
	winrt::com_ptr<ID3D12GraphicsCommandList> commandList;
	DX::ThrowIfFailed(m_D3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_currentFrame].get(), nullptr, IID_PPV_ARGS(&commandList)));

	// Transition RT to RENDER_TARGET
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	// Clear to bright red so it's obvious
	D3D12_CPU_DESCRIPTOR_HANDLE rtv = GetRenderTargetView();
	FLOAT clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

	// Transition back to present
	CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier2);

	DX::ThrowIfFailed(commandList->Close());
	ID3D12CommandList* lists[] = { commandList.get() };
	m_CommandQueue->ExecuteCommandLists(1, lists);
	const char* file = "\nTestClearAndPresentOnce";
#ifndef _DEBUG
	OutputDebugStringA("\nOutputDebugString begin\n");
#endif
	LogStatusImpl(file, 815, false, " success\n");
#ifndef _DEBUG
	OutputDebugStringA("\nOutputDebugString end\n");
#endif
	// Present and wait so we see the result
	//OutputDebugStringA("INFO: TestClearAndPresentOnce - calling Present()\n");
	Present();
	WaitForGpu();
}

// Wait for pending GPU work to complete.
void DX::DeviceResources::WaitForGpu()
{
#ifndef _DEBUG
OutputDebugStringA("DeviceResources::WaitForGpu - begin\n");
#endif
	// Schedule a Signal command in the GPU queue.

	if (m_Fence && m_CommandQueue)
	{
		DX::ThrowIfFailed(m_CommandQueue->Signal(m_Fence.get(), m_fenceValues[m_currentFrame]));
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
		m_fenceValues[m_currentFrame]++;
	}
#ifndef _DEBUG
	OutputDebugStringA("DeviceResources::WaitForGpu - end\n");
#endif
}

void DX::DeviceResources::Trim()
{
#ifndef _DEBUG
	OutputDebugStringA("DeviceResources::Trim - begin\n");
#endif
	winrt::com_ptr<IDXGIDevice3> dxgiDevice;
	m_D3dDevice.as(dxgiDevice);
	
		dxgiDevice->Trim();
	

	// Best-effort: stop GPU work and release large transient resources.

	try
	{
		// Ensure GPU is idle before releasing resources.
		WaitForGpu();
	}
	catch (...)
	{
#ifndef _DEBUG
		OutputDebugStringA("DeviceResources::Trim - WaitForGpu threw\n");
#endif
	}

	// Release size-dependent resources but keep device & factory alive.
	for (UINT n = 0; n < c_frameCount; n++)
	{
		m_RenderTargets[n] = nullptr;
	}
	m_DepthStencil = nullptr;

	// Optionally release descriptor heaps to further free memory (they will be recreated later)
	// Note: recreating descriptor heaps is cheap relative to textures/buffers.
	m_RtvHeap = nullptr;
	m_DsvHeap = nullptr;
#ifndef _DEBUG
	OutputDebugStringA("DeviceResources::Trim - released render targets, depth stencil and heaps\n");
#endif
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
	const char* file = "adapter is a success";
#ifndef _DEBUG
	OutputDebugStringA("\nOutputDebugString begin\n");
#endif
	LogStatusImpl(file, 815, false, "adapter is a success");
#ifndef _DEBUG
	OutputDebugStringA("\nOutputDebugString end\n");
#endif
	*ppAdapter = adapter;
}



