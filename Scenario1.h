#pragma once

#include "Generated Files\Scenario1.g.h"
#include "Generated Files\Scenario1.xaml.g.h"
#include "MainPage.g.h"

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


#include <winrt/Windows.System.h>
#include <winrt/Windows.System.Threading.h>
#include <winrt/Windows.System.Threading.Core.h>

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Hot3dxBlankApp2;
using namespace winrt::Hot3dxBlankApp2::implementation;

namespace winrt::Hot3dxBlankApp2::implementation
{
    //struct MainPage : MainPageT<MainPage>
    struct Scenario1 : Scenario1T<Scenario1>
    {
        public:
            Scenario1();

            void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);
            void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);

            // These methods are public so they can be called by binding.
            void Scenario1_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);
            void SetMinimumSize_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e);

    private:
        Hot3dxBlankApp2::MainPage rootPage{ MainPage::Current() };
        event_token windowSizeChangedToken;

        void UpdateContent();
           
    };
}

namespace winrt::Hot3dxBlankApp2::factory_implementation
{
    // Factory shim required by generated code
    struct Scenario1 : Scenario1T<Scenario1, winrt::Hot3dxBlankApp2::implementation::Scenario1>
    {
    };
}
    
