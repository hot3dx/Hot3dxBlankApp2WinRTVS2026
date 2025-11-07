#pragma once

#include "Generated Files\Scenario2.g.h"


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
    struct Scenario2 : Scenario2T<Scenario2>
    {
        public:
            Scenario2();

            void OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);

    };
}

namespace winrt::Hot3dxBlankApp2::factory_implementation
{
    // Factory shim required by generated code
    struct Scenario2 : Scenario2T<Scenario2, winrt::Hot3dxBlankApp2::implementation::Scenario2>
    {
    };
}

