#include "pch.h"
#include "MainPage.h"
#include "Scenario1.h"
#include "Generated Files\Scenario1.g.cpp"
#include <winrt/base.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Hot3dxBlankApp2;
using namespace winrt::Hot3dxBlankApp2::implementation;

namespace winrt::Hot3dxBlankApp2::implementation
{
    
    Scenario1::Scenario1()
    {
        InitializeComponent();

        
    }
        
    void Scenario1::OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        windowSizeChangedToken = Window::Current().SizeChanged([this](auto&&, auto&&)
            {
                UpdateContent();
            });
        UpdateContent();
    }

    void Scenario1::OnNavigatedFrom(NavigationEventArgs const&)
    {
        Window::Current().SizeChanged(windowSizeChangedToken);
    }

    void Scenario1::Scenario1_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto view = ApplicationView::GetForCurrentView();
        auto page = winrt::Hot3dxBlankApp2::implementation::MainPage::Current();
        auto notifyType = winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage;
        auto impl = winrt::get_self<winrt::Hot3dxBlankApp2::implementation::MainPage>(page);

        if (view.TryResizeView({ 600, 500 }))
        {
            impl->NotifyUser(L"Resizing to 600 \u00D7 500", notifyType);
            // The SizeChanged event will be raised when the resize is complete.
        }
        else
        {
            // The system does not support resizing, or the provided size is out of range.
            impl->NotifyUser(L"Failed to resize view", NotifyType::ErrorMessage);
        }
    }

    void Scenario1::SetMinimumSize_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto view = ApplicationView::GetForCurrentView();
        /*
        if (SetMinimumSize().IsChecked().Value())
        {
            // If this size is not permitted by the system, the nearest permitted value is used.
            view.SetPreferredMinSize({ 300, 200 });
        }
        else
        {
            // Passing width = height = 0 returns to the default system behavior.
            view.SetPreferredMinSize({ 0, 0 });
        }
        */
    }

    void Scenario1::UpdateContent()
    {
        Rect bounds = Window::Current().Bounds();
        //().Text(to_hstring(bounds.Width));
       // ReportViewHeight().Text(to_hstring(bounds.Height));
    }
}