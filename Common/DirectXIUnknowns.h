#pragma once

#include <winrt/base.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <d3d12.h>
#include <dxgi1_6.h>

// Attach a DXGI swap chain to a XAML SwapChainPanel with no hidden side-effects.
// - Returns an HRESULT from ISwapChainPanelNative::SetSwapChain.
// - Must be called on the UI thread.
inline HRESULT AttachSwapChainToSwapChainPanel(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& panel, IDXGISwapChain3* swapChain) noexcept
{
    if (!panel || !swapChain)
    {
        OutputDebugString(L"\n DirectXInUknowns.h if (!panel || !swapChain) return E_INVALIDARG;\n");
        return E_INVALIDARG;
    }

    // get_unknown returns a raw ::IUnknown* (non-owning). AddRef then attach to com_ptr to manage lifetime.
    ::IUnknown* rawUnknown = winrt::get_unknown(panel);
    if (!rawUnknown)
    {
        OutputDebugString(L"\n DirectXInUknowns.h ::IUnknown* rawUnknown = winrt::get_unknown(panel); if (!rawUnknown) return E_FAIL;\n");
        return E_FAIL;
    }

    rawUnknown->AddRef();
    winrt::com_ptr<::IUnknown> panelUnknown;
    panelUnknown.attach(rawUnknown); // takes ownership

    // Query for the native ISwapChainPanelNative and set the swap chain.
    winrt::com_ptr<ISwapChainPanelNative> panelNative = panelUnknown.as<ISwapChainPanelNative>();
    if (!panelNative)
    {
        OutputDebugString(L"\n DirectXInUknowns.h  winrt::com_ptr<ISwapChainPanelNative> panelNative = panelUnknown.as<ISwapChainPanelNative>(); if (!panelNative) return E_NOINTERFACE\n");
        return E_NOINTERFACE;
    }
    OutputDebugString(L"\n DirectXInUknowns.h  panelNative->SetSwapChain(swapChain) SUCCESS\n");
    return panelNative->SetSwapChain(swapChain);
}