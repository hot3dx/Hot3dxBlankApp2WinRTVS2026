#pragma once

#include "MainPage.g.h"
#include "Common/DeviceResources.h"
#include "Hot3dxBlankApp2Main.h"

#include <Unknwn.h>
#include <Unknwnbase.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Interactions.h>
#include <winrt/Windows.UI.Composition.Core.h>
#include <winrt/Windows.UI.Composition.Effects.h>
#include <winrt/Windows.UI.Composition.Scenes.h>
//#include <winrt/Windows.UI.Composition.AnimationMetrics.h>
//#include <winrt/Windows.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.Graphics.Effects.h>
//#include <winrt/Microsoft.Graphics.Canvas.h>
//#include <winrt/Microsoft.Graphics.Canvas.Effects.h>
//#include <winrt/Microsoft.Graphics.Canvas.UI.Composition.h>
//#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
//#include <winrt/Microsoft.Graphics.Canvas.Text.h>
//#include <winrt/Microsoft.Graphics.Canvas.Brushes.h>
//#include <winrt/Microsoft.Graphics.Canvas.Geometry.h>
//#include <winrt/Microsoft.Graphics.Canvas.UI.h>


namespace winrt::Hot3dxBlankApp2::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        void OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);

        static MainPage* Current;
        int32_t MyProperty();
        void MyProperty(int32_t value);

        // event handler for swapChainPanel Loaded
        void OnSwapChainPanelLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);


        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void Button_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void ScenarioControl_SelectionChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args);

        void Footer_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void Footer_Click2(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        // Independent input handling functions.
        void OnPointerPressed(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerMoved(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerReleased(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerWheelChanged(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerEntered(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerExited(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);
        void OnPointerCaptureLost(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e);

        
        void OnKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs const& args);
        void OnKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs  const& args);
        
        void ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        ///////////////////////
	private:
        
        //void RotateYaw(float deg);

        //void RotatePitch(float deg);

        
        // Resources used to render the DirectX content in the XAML page background.
       
        winrt::Windows::UI::Xaml::Controls::SwapChainPanel m_swapChainPanel{ nullptr }; // member, not namespace-scope
        std::shared_ptr<DeviceResources> m_deviceResources;
        std::unique_ptr<Hot3dxBlankApp2Main> m_main;
        bool m_windowVisible{ false };
       
       
    };
}

namespace winrt::Hot3dxBlankApp2::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
