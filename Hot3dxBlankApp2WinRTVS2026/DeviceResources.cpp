#include "pch.h"
#include "DeviceResources.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Platform;

namespace DX
{
    DeviceResources::DeviceResources() :
        m_currentFrame(0),
        m_screenViewport{},
        m_rtvDescriptorSize(0),
        m_fenceEvent(nullptr),
        m_backBufferFormat(DXGI_FORMAT_B8G8R8A8_UNORM),
        m_d3dRenderTargetSize{},
        m_outputSize{},
        m_logicalSize{},
        m_nativeOrientation(DisplayOrientations::None),
        m_currentOrientation(DisplayOrientations::None),
        m_dpi(-1.0f),
        m_effectiveDpi(-1.0f)
    {
        CreateDeviceIndependentResources();
        CreateDeviceResources();
    }

    DeviceResources::~DeviceResources()
    {
        WaitForGpu();
    }

    void DeviceResources::CreateDeviceIndependentResources()
    {
    }

    void DeviceResources::CreateDeviceResources()
    {
        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        DX::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

        ComPtr<IDXGIAdapter1> adapter;
        DX::ThrowIfFailed(m_dxgiFactory->EnumAdapters1(0, &adapter));

        DX::ThrowIfFailed(D3D12CreateDevice(
            adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3dDevice)
        ));

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

        for (UINT n = 0; n < c_frameCount; n++)
        {
            DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&m_commandAllocators[n])
            ));
        }

        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = c_frameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        DX::ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValues[m_currentFrame]++;

        m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (m_fenceEvent.Get() == nullptr)
        {
            DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    void DeviceResources::SetSwapChainPanel(SwapChainPanel^ panel)
    {
        m_swapChainPanel = panel;

        m_logicalSize = Size(static_cast<float>(panel->ActualWidth), static_cast<float>(panel->ActualHeight));
        m_nativeOrientation = DisplayOrientations::Landscape;
        m_currentOrientation = DisplayOrientations::Landscape;
        m_dpi = 96.0f;
        m_effectiveDpi = m_dpi;

        CreateWindowSizeDependentResources();
    }

    void DeviceResources::SetLogicalSize(Size logicalSize)
    {
        if (m_logicalSize != logicalSize)
        {
            m_logicalSize = logicalSize;
            CreateWindowSizeDependentResources();
        }
    }

    void DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
    {
        if (m_currentOrientation != currentOrientation)
        {
            m_currentOrientation = currentOrientation;
            CreateWindowSizeDependentResources();
        }
    }

    void DeviceResources::SetDpi(float dpi)
    {
        if (dpi != m_dpi)
        {
            m_dpi = dpi;
            m_effectiveDpi = dpi;
            CreateWindowSizeDependentResources();
        }
    }

    void DeviceResources::CreateWindowSizeDependentResources()
    {
        WaitForGpu();

        for (UINT n = 0; n < c_frameCount; n++)
        {
            m_renderTargets[n] = nullptr;
            m_fenceValues[n] = m_fenceValues[m_currentFrame];
        }

        UpdateRenderTargetSize();

        UINT width = static_cast<UINT>(m_d3dRenderTargetSize.Width);
        UINT height = static_cast<UINT>(m_d3dRenderTargetSize.Height);

        if (m_swapChain != nullptr)
        {
            HRESULT hr = m_swapChain->ResizeBuffers(c_frameCount, width, height, m_backBufferFormat, 0);

            if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            {
                return;
            }
            else
            {
                DX::ThrowIfFailed(hr);
            }
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = m_backBufferFormat;
            swapChainDesc.Stereo = false;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = c_frameCount;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.Flags = 0;
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

            ComPtr<IDXGISwapChain1> swapChain;
            DX::ThrowIfFailed(m_dxgiFactory->CreateSwapChainForComposition(
                m_commandQueue.Get(),
                &swapChainDesc,
                nullptr,
                &swapChain
            ));

            DX::ThrowIfFailed(swapChain.As(&m_swapChain));

            ComPtr<ISwapChainPanelNative> panelNative;
            DX::ThrowIfFailed(reinterpret_cast<IUnknown*>(m_swapChainPanel.Get())->QueryInterface(IID_PPV_ARGS(&panelNative)));
            DX::ThrowIfFailed(panelNative->SetSwapChain(m_swapChain.Get()));
        }

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT n = 0; n < c_frameCount; n++)
        {
            DX::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(m_rtvDescriptorSize);
        }

        m_screenViewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    }

    void DeviceResources::UpdateRenderTargetSize()
    {
        m_effectiveDpi = m_dpi;
        m_d3dRenderTargetSize.Width = m_logicalSize.Width * m_effectiveDpi / 96.0f;
        m_d3dRenderTargetSize.Height = m_logicalSize.Height * m_effectiveDpi / 96.0f;
        m_d3dRenderTargetSize.Width = std::max(m_d3dRenderTargetSize.Width, 1.0f);
        m_d3dRenderTargetSize.Height = std::max(m_d3dRenderTargetSize.Height, 1.0f);
        m_outputSize = m_logicalSize;
    }

    void DeviceResources::Present()
    {
        HRESULT hr = m_swapChain->Present(1, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
            MoveToNextFrame();
        }
    }

    void DeviceResources::WaitForGpu()
    {
        DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_currentFrame]));

        DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);

        m_fenceValues[m_currentFrame]++;
    }

    void DeviceResources::MoveToNextFrame()
    {
        const UINT64 currentFenceValue = m_fenceValues[m_currentFrame];
        DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

        m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

        if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
        {
            DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent.Get()));
            WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
        }

        m_fenceValues[m_currentFrame] = currentFenceValue + 1;
    }

    void DeviceResources::ValidateDevice()
    {
    }
}
