#include "pch.h"
#include "Scenario2.h"
#include "Generated Files\Scenario2.g.cpp"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Hot3dxBlankApp2;
using namespace winrt::Hot3dxBlankApp2::implementation;

namespace winrt::Hot3dxBlankApp2::implementation
{

    Scenario2::Scenario2()
    {
        InitializeComponent();

        LaunchAtSize().IsChecked(ApplicationView::PreferredLaunchWindowingMode() != ApplicationViewWindowingMode::Auto);

        
    }

    void Scenario2::LaunchAtSize_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        if (LaunchAtSize().IsChecked().Value())
        {
            // For best results, set the PreferredLaunchViewSize before setting
            // the PreferredLaunchWindowingMode.
            ApplicationView::PreferredLaunchViewSize({ 1400, 1200 });
            ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::PreferredLaunchViewSize);
        }
        else
        {
            ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::Auto);
        }
    }

    void Scenario2::OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
    }
}