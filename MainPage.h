#pragma once

//#include "Generated Files\MainPage.xaml.g.h"
#include "Generated Files\MainPage.g.h"
#include "Scenario1.h"
#include "Scenario2.h"
#include "ScenariosConfigurations.h"
#include "Common/DeviceResources.h"
#include "Hot3dxBlankApp2Main.h"
#include <hstring.h>

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
#include <winrt/Windows.Graphics.Effects.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.System.Threading.Core.h>
 
using namespace winrt;
using namespace winrt::Windows::Foundation::Collections;

namespace winrt::Hot3dxBlankApp2::implementation
{

    enum class NotifyType
    {
        StatusMessage,
        ErrorMessage
    };

    // Use the projected enum from the IDL (Hot3dxBlankApp2::NotifyType)
    // instead of declaring a duplicate enum in the implementation namespace.
    using NotifyType = winrt::Hot3dxBlankApp2::implementation::NotifyType;

    struct MainPage : MainPageT<MainPage>
    {
        MainPage();

        static hstring FEATURE_NAME();

        // Fully-qualify the projected WinRT Scenario type so lookup inside the implementation
        // namespace resolves correctly.
        //std::vector<winrt::Hot3dxBlankApp2::Scenario> scenarios() { return scenariosInner; }
        
        static winrt::Hot3dxBlankApp2::MainPage Current() { return current; }
        
		void SaveInternalState(winrt::Windows::Foundation::Collections::IPropertySet const& state);
		void LoadInternalState(winrt::Windows::Foundation::Collections::IPropertySet const& state);
        inline static winrt::Windows::Foundation::Collections::IVector<winrt::Hot3dxBlankApp2::ScenarioInfo> scenarios2() { return scenariosIn; }
        
        // Fully-qualify WinRT MainPage to avoid ambiguity.
        void OnSwapChainPanelSizeChangedXaml(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
        void OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);

        int32_t MyProperty();
        void MyProperty(int32_t value);

        // event handler for swapChainPanel Loaded
        void OnSwapChainPanelLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
		void OnSwapChainPanelResized(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args);
		void OnDPIChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnOrientationChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
		void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args);
		void OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void OnNavigatedFrom(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& /* e */);

        
        
        //void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
        
        void Button1ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void Button2ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void Button_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        void ScenarioControl_SelectionChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        
        void FooterTrademarks_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

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

        
        void OnKeyDown(winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs const& args);
        void OnKeyUp(winrt::Windows::UI::Xaml::Input::KeyRoutedEventArgs  const& args);
        
        void ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        void OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);

        void Navigate(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName, winrt::Windows::Foundation::IInspectable const& parameter);

        // These methods are public so they can be called by binding.
        void NotifyUser(hstring strMessage, winrt::Hot3dxBlankApp2::implementation::NotifyType type);
        
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }

	private:
        
        void NavigateTo(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName, winrt::Windows::Foundation::IInspectable const& parameter);
        void UpdateStatus(hstring strMessage, winrt::Hot3dxBlankApp2::implementation::NotifyType type);
        void OnResizeTimerTick(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);
        
        //void RotateYaw(float deg);

        //void RotatePitch(float deg);
        
		inline static winrt::Windows::Foundation::Collections::IVector<Hot3dxBlankApp2::ScenarioInfo> scenariosIn{ nullptr };
		//std::vector<winrt::Hot3dxBlankApp2::Scenario> scenarios;
        // Resources used to render the DirectX content in the XAML page background.
       
        winrt::Windows::UI::Xaml::Controls::SwapChainPanel m_swapChainPanel{ nullptr }; // member, not namespace-scope

        std::shared_ptr<DeviceResources> m_deviceResources;
        std::unique_ptr<Hot3dxBlankApp2Main> m_main;
        bool m_windowVisible{ false };

        static winrt::Hot3dxBlankApp2::MainPage current;
        bool navigating = false;
       
        Concurrency::critical_section m_criticalSection;

        winrt::Windows::UI::Xaml::DispatcherTimer m_resizeTimer{};

    };
}

namespace winrt::Hot3dxBlankApp2::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
