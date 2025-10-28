#pragma once

#include "App.xaml.g.h"

namespace winrt::Hot3dxBlankApp2WinRTVS2026::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
        void OnSuspending(IInspectable const&, Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnNavigationFailed(IInspectable const&, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
    };
}

namespace winrt::Hot3dxBlankApp2WinRTVS2026::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
