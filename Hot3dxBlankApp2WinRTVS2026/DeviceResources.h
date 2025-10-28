#pragma once

namespace DX
{
    class DeviceResources
    {
    public:
        DeviceResources();
        ~DeviceResources();

        void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
        void SetLogicalSize(Windows::Foundation::Size logicalSize);
        void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
        void SetDpi(float dpi);
        void ValidateDevice();
        void Present();
        void WaitForGpu();

        Windows::Foundation::Size GetOutputSize() const { return m_outputSize; }
        Windows::Foundation::Size GetLogicalSize() const { return m_logicalSize; }

        ID3D12Device* GetD3DDevice() const { return m_d3dDevice.Get(); }
        IDXGISwapChain3* GetSwapChain() const { return m_swapChain.Get(); }
        ID3D12Resource* GetRenderTarget() const { return m_renderTargets[m_currentFrame].Get(); }
        ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.Get(); }
        ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocators[m_currentFrame].Get(); }
        ID3D12DescriptorHeap* GetRtvHeap() const { return m_rtvHeap.Get(); }
        DXGI_FORMAT GetBackBufferFormat() const { return m_backBufferFormat; }
        D3D12_VIEWPORT GetScreenViewport() const { return m_screenViewport; }
        UINT GetCurrentFrameIndex() const { return m_currentFrame; }
        UINT GetRtvDescriptorSize() const { return m_rtvDescriptorSize; }

    private:
        void CreateDeviceIndependentResources();
        void CreateDeviceResources();
        void CreateWindowSizeDependentResources();
        void UpdateRenderTargetSize();
        void MoveToNextFrame();

        static const UINT c_frameCount = 2;

        UINT m_currentFrame;
        Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
        Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[c_frameCount];
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[c_frameCount];
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
        UINT64 m_fenceValues[c_frameCount];
        Microsoft::WRL::Wrappers::Event m_fenceEvent;

        D3D12_VIEWPORT m_screenViewport;
        UINT m_rtvDescriptorSize;
        DXGI_FORMAT m_backBufferFormat;

        Windows::Foundation::Size m_d3dRenderTargetSize;
        Windows::Foundation::Size m_outputSize;
        Windows::Foundation::Size m_logicalSize;
        Windows::Graphics::Display::DisplayOrientations m_nativeOrientation;
        Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
        float m_dpi;
        float m_effectiveDpi;

        Platform::Agile<Windows::UI::Xaml::Controls::SwapChainPanel> m_swapChainPanel;
    };
}
