#include "pch.h"
#include "MainPage.h"
#include "Scenario1.h"
#include "Scenario2.h"
#include "Generated Files\MainPage.g.cpp"
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Automation.Peers.h>
// if you need native interop checks
//#include <windows.ui.xaml.media.dxinterop.h> CRASH US MAXIMUS!
#include <winrt/Windows.UI.Xaml.Interop.h>

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Hot3dxBlankApp2::implementation;

namespace winrt::Hot3dxBlankApp2::implementation
{
    MainPage* MainPage::Current = { nullptr };

    MainPage::MainPage()
    {
        // Initialize XAML parts first (must be called before manipulating Content() / named controls)
        InitializeComponent();
        
        // create DeviceResources
        m_deviceResources = std::make_shared<DeviceResources>();

        // instantiate the SwapChainPanel before using it
        if (!m_swapChainPanel)
        {
            m_swapChainPanel = swapChainPanel();
        }

        // attach Loaded handler
        m_swapChainPanel.Loaded({ this, &MainPage::OnSwapChainPanelLoaded });
       
        // Put the panel into the page's visual tree.
       // Wrap in try/catch to capture any HRESULT and print it.
        try
        {
            Content(m_swapChainPanel);
        }
        catch (winrt::hresult_error const& e)
        {
            // Build a readable message and show it in the visible status area instead of OutputDebugStringA.
            std::wstringstream ss;
            ss << L"ERROR: Setting Content failed: 0x" << std::hex << static_cast<unsigned>(e.code()) << L" - " << e.message().c_str();

            // NotifyUser updates the visible status panel (calls UpdateStatus on the UI thread).
            NotifyUser(winrt::hstring{ ss.str() }, NotifyType::ErrorMessage);

        }
        winrt::hresult_error const& e = {};
        e.code() = S_OK;
		e.message() = L"Setting Content succeeded";
        std::wstringstream ss;
        ss << L"ERROR_SUCCESS: Setting Content succeeded MainPage. cpp line 57" << std::hex << static_cast<unsigned>(e.code()) << L" - " << e.message().c_str();

        // NotifyUser updates the visible status panel (calls UpdateStatus on the UI thread).
        NotifyUser(winrt::hstring{ ss.str() }, NotifyType::StatusMessage);
        // mark window invisible until ready
        m_windowVisible = false;
    }

    void MainPage::OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const& /*sender*/,
        winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
        auto newSize = args.NewSize();
        float aw = static_cast<float>(newSize.Width);
        float ah = static_cast<float>(newSize.Height);
        if (aw <= 0.0f || ah <= 0.0f) return;

        // unregister handler if you only want first-time init
        m_swapChainPanel.SizeChanged(nullptr);

        m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(aw, ah));
        m_deviceResources->SetSwapChainPanel(m_swapChainPanel, Window::Current().CoreWindow());
        m_deviceResources->CreateWindowSizeDependentResources();

        m_main = std::make_unique<Hot3dxBlankApp2Main>(m_deviceResources);
        m_main->CreateRenderers(m_deviceResources);
        m_windowVisible = true;
        m_main->OnWindowSizeChanged();
    }

    void MainPage::OnSwapChainPanelLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        auto panel = m_swapChainPanel; // or sender.as<SwapChainPanel>()
        if (!panel) return;

        // CoreWindow for SetWindow
        auto window = Window::Current();
        auto coreWindow = window.CoreWindow();

        // 1) give DeviceResources the CoreWindow
        m_deviceResources->SetWindow(coreWindow);

        // 2) set logical size from panel measured DIPs (preferred)
        float aw = static_cast<float>(panel.ActualWidth());
        float ah = static_cast<float>(panel.ActualHeight());
        if (aw > 0 && ah > 0)
        {
            m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(aw, ah));
        }
        else
        {
            // fallback to CoreWindow bounds if necessary
            m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(coreWindow.Bounds().Width, coreWindow.Bounds().Height));
        }

        // 3) give DeviceResources the SwapChainPanel (attach swapchain to panel via ISwapChainPanelNative)
        m_deviceResources->SetSwapChainPanel(panel, coreWindow);

        // 4) create/resize swap chain and render targets
        m_deviceResources->CreateWindowSizeDependentResources();

        // 5) set up renderers and first frame
        m_main = std::make_unique<Hot3dxBlankApp2Main>(m_deviceResources);
        m_main->CreateRenderers(m_deviceResources);
        m_windowVisible = true;
        m_main->StartRenderLoop();
    }

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainPage::Button1ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked 1"));
    }

    void MainPage::Button2ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        myButton2().Content(box_value(L"Clicked 2"));
    }
    // For all methods with unreferenced formal parameters, mark them as unreferenced to suppress C4100 warnings.

    void MainPage::Button_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        Splitter().IsPaneOpen(!Splitter().IsPaneOpen());
    }
    /*
    void MainPage::ScenarioControl_SelectionChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {
        if (args.AddedItems().Size() > 0)
        {
            ScenarioControl().SelectedItem(args.AddedItems().GetAt(0));
            auto selectedItem = args.AddedItems().GetAt(0).as<Windows::UI::Xaml::Controls::ListBoxItem>();
            if (selectedItem)
            {
                auto scenarioName = winrt::unbox_value<hstring>(selectedItem.Content());
                if (scenarioName == L"Scenario1")
                {
                    //ScenarioFrame().Navigate(xaml_typename<Hot3dxBlankApp2::Scenario1>());
                }
                else if (scenarioName == L"Scenario2")
                {
                    //ScenarioFrame().Navigate(xaml_typename<Hot3dxBlankApp2::Scenario2>());
                }
            }
        }
    }
    */


    void MainPage::ScenarioControl_SelectionChanged(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {
        auto list = sender.as<Windows::UI::Xaml::Controls::Button>();
        if (args.AddedItems().Size() > 0)
        {
            ScenarioControl().SelectedItem(args.AddedItems().GetAt(0));
            auto selectedItem = args.AddedItems().GetAt(0).as<Windows::UI::Xaml::Controls::ListBoxItem>();
            if (selectedItem)
            {
                auto scenarioName = winrt::unbox_value<hstring>(selectedItem.Content());
                if (scenarioName == L"Scenario1")
                {
                    ScenarioFrame().Navigate(xaml_typename<Hot3dxBlankApp2::Scenario1>());
                }
                else if (scenarioName == L"Scenario2")
                {
                    ScenarioFrame().Navigate(xaml_typename<Hot3dxBlankApp2::Scenario2>());
                }
                // Add more scenarios as needed
            }
        }
    }
    
    void MainPage::Footer_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        try
        {
            auto uri = winrt::Windows::Foundation::Uri{ L"http://github.com/hot3dx/Hot3dxBlankApp2WinRTVS2026" };
            auto op = winrt::Windows::System::Launcher::LaunchUriAsync(uri);
            // Non-blocking: handle completion asynchronously
            op.Completed([](winrt::Windows::Foundation::IAsyncOperation<bool> const& asyncOp,
                winrt::Windows::Foundation::AsyncStatus const& status)
                {
                    if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
                    {
                        bool success = asyncOp.GetResults();
                        // Optionally log success/failure. Avoid blocking here.
                    }
                });
        }
        catch (...) // keep failures graceful for certification
        {
        }
    }

    void MainPage::Footer_Click2(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        try
        {
            auto uri = winrt::Windows::Foundation::Uri{ L"https://github.com/hot3dx/Hot3dxBlankApp2WinRTVS2026/blob/main/PrivacyPolicy.md" };
            auto op = winrt::Windows::System::Launcher::LaunchUriAsync(uri);
            // Non-blocking: handle completion asynchronously
            op.Completed([](winrt::Windows::Foundation::IAsyncOperation<bool> const& asyncOp,
                winrt::Windows::Foundation::AsyncStatus const& status)
                {
                    if (status == winrt::Windows::Foundation::AsyncStatus::Completed)
                    {
                        bool success = asyncOp.GetResults();
                        // Optionally log success/failure. Avoid blocking here.
                    }
                });
        }
        catch (...) // keep failures graceful for certification
        {
        }
    }


    void MainPage::OnPointerPressed(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerMoved(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerReleased(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerWheelChanged(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerEntered(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerExited(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnPointerCaptureLost(winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const e)
    {
        //if (sender) { 
        if (e) {}
        //}
    }
    void MainPage::OnKeyDown(Windows::UI::Xaml::Input::KeyRoutedEventArgs const& args)
    {
        if (args) {}
    }
    void MainPage::OnKeyUp(Windows::UI::Xaml::Input::KeyRoutedEventArgs const& args)
    {
        if (args) {}
    }

    void winrt::Hot3dxBlankApp2::implementation::MainPage::ToggleButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        Splitter().IsPaneOpen(!Splitter().IsPaneOpen());
    }

    void MainPage::UpdateStatus(hstring strMessage, NotifyType type)
    {
        using namespace winrt::Windows::UI;
        using namespace winrt::Windows::UI::Xaml;

        switch (type)
        {
        case NotifyType::StatusMessage:
        {
            winrt::Windows::UI::Xaml::Media::SolidColorBrush brush1;
            brush1.Color(Colors::DarkBlue());
            StatusBorder().Background(brush1);
        }
        break;

        case NotifyType::ErrorMessage:
        {
            winrt::Windows::UI::Xaml::Media::SolidColorBrush brush1;
            brush1.Color(Colors::Red());
            StatusBorder().Background(brush1);
        }
        break;

        default:
            break;
        }

        StatusBlock().Text(strMessage);

        // Collapse the StatusBlock if it has no text to conserve real estate.
        if (StatusBlock().Text() != L"")
        {
            StatusBorder().Visibility(Visibility::Visible);
            StatusPanel().Visibility(Visibility::Visible);
        }
        else
        {
            StatusBorder().Visibility(Visibility::Collapsed);
            StatusPanel().Visibility(Visibility::Collapsed);
        }

        // Raise an event if necessary to enable a screen reader to announce the status update.
        auto peer1 = winrt::Windows::UI::Xaml::Automation::Peers::FrameworkElementAutomationPeer::FromElement(StatusBlock());
        if (peer1)
        {
            peer1.RaiseAutomationEvent(winrt::Windows::UI::Xaml::Automation::Peers::AutomationEvents::LiveRegionChanged);
        }
    }
    /*
    void MainPage::UpdateStatus(hstring strMessage, NotifyType type)
    {
        switch (type)
        {
        case NotifyType::StatusMessage:
        {
            winrt::Windows::UI::Xaml::Media::SolidColorBrush brush1 = winrt::Windows::UI::Xaml::Media::SolidColorBrush();        brush1.Color() = { winrt::Windows::UI::Colors::DarkBlue()};
            brush1.Color() = { winrt::Windows::UI::Colors::DarkBlue().A, winrt::Windows::UI::Colors::DarkBlue().B, winrt::Windows::UI::Colors::DarkBlue().G, winrt::Windows::UI::Colors::DarkBlue().R };
            StatusBorder().Background() = brush1;
        };
            break;
        
        case NotifyType::ErrorMessage:
        {
            winrt::Windows::UI::Xaml::Media::SolidColorBrush brush1 = winrt::Windows::UI::Xaml::Media::SolidColorBrush();
            brush1.Color() = { winrt::Windows::UI::Colors::Red().A, winrt::Windows::UI::Colors::Red().B, winrt::Windows::UI::Colors::Red().G, winrt::Windows::UI::Colors::Red().R };
            StatusBorder().Background() = brush1;
        };
        break;
        
        default:
            break;
        }

        StatusBlock().Text() = strMessage;

        // Collapse the StatusBlock if it has no text to conserve real estate.
        if (StatusBlock().Text() != L"")
        {
            StatusBorder().Visibility({ winrt::Windows::UI::Xaml::Visibility::Visible });
            StatusPanel().Visibility({ winrt::Windows::UI::Xaml::Visibility::Visible });
        }
        else
        {
            StatusBorder().Visibility({ winrt::Windows::UI::Xaml::Visibility::Collapsed });
            StatusPanel().Visibility({ winrt::Windows::UI::Xaml::Visibility::Collapsed });
        }

        // Raise an event if necessary to enable a screen reader to announce the status update.
        
        winrt::Windows::UI::Xaml::Automation::Peers::FrameworkElementAutomationPeer apeer();
        auto peer1 = apeer().FromElement({StatusBlock()});
        if (peer1 != nullptr)
        {
            peer1.RaiseAutomationEvent({ winrt::Windows::UI::Xaml::Automation::Peers::AutomationEvents::LiveRegionChanged });
        }
    }
    */
    void MainPage::NotifyUser(hstring strMessage, NotifyType type)
    {
        if (Dispatcher().HasThreadAccess())
        {
            UpdateStatus(strMessage, type);
        }
        else
        {
            Dispatcher().RunAsync(CoreDispatcherPriority::Normal, DispatchedHandler([strMessage, type, this]()
                {
                    UpdateStatus(strMessage, type);

                }));
        }
    }
}