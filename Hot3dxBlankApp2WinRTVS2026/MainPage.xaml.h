#pragma once

#include "MainPage.g.h"
#include "DeviceResources.h"
#include "RotoDraw3DRenderer.h"

namespace winrt::Hot3dxBlankApp2WinRTVS2026::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void SymmetrySlider_ValueChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void BrushSizeSlider_ValueChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);
        void ClearButton_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void MirrorCheckBox_Changed(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void ColorButton_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void DrawingPanel_PointerPressed(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DrawingPanel_PointerMoved(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DrawingPanel_PointerReleased(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e);

    private:
        void OnLoaded(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
        void OnSizeChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::SizeChangedEventArgs const& e);
        void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel const& sender, Windows::Foundation::IInspectable const& args);
        void OnResuming(Windows::Foundation::IInspectable const& sender, Windows::Foundation::IInspectable const& args);
        void StartRenderLoop();
        void StopRenderLoop();

        std::shared_ptr<DX::DeviceResources> m_deviceResources;
        std::unique_ptr<Hot3dxBlankApp2WinRTVS2026::RotoDraw3DRenderer> m_renderer;
        bool m_isDrawing;
        Windows::Foundation::IAsyncAction m_renderLoopWorker;
    };
}

namespace winrt::Hot3dxBlankApp2WinRTVS2026::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
