#pragma once
#include "App.xaml.g.h"

namespace winrt::Hot3dxBlankApp2::implementation
{
    struct App : AppT<App>
    {
        App();
        winrt::Windows::UI::Xaml::Controls::Frame CreateRootFrame();

        void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
        void OnActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs const&);
        void OnFileActivated(Windows::ApplicationModel::Activation::FileActivatedEventArgs const&);
        void OnBackgroundActivated(Windows::ApplicationModel::Activation::BackgroundActivatedEventArgs const&);
        void OnSuspending(IInspectable const& sender, winrt::Windows::ApplicationModel::SuspendingEventArgs const&);
        void OnNavigationFailed(IInspectable const&, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
    };
}
