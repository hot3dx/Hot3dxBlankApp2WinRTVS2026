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


using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Hot3dxBlankApp2::implementation;

namespace winrt::Hot3dxBlankApp2::implementation
{
    Hot3dxBlankApp2::MainPage MainPage::current{ nullptr };

    MainPage::MainPage()
    {
        // Initialize XAML parts first (must be called before manipulating Content() / named controls)
        InitializeComponent();

        if (!MainPage::scenariosIn)
        {
            MainPage::scenariosIn = single_threaded_vector<Hot3dxBlankApp2::ScenarioInfo>();
        }
        PopulateScenarios(MainPage::scenariosIn);

        Hot3dxBlankApp2().Text(FEATURE_NAME());
        
        // set static instance pointer
        MainPage::current = *this;
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
            NotifyUser(winrt::hstring{ ss.str() }, winrt::Hot3dxBlankApp2::implementation::NotifyType::ErrorMessage);

        }
        winrt::hresult_error const& e = {};
        e.code() = S_OK;
        e.message() = L"Setting Content succeeded";
        std::wstringstream ss;
        ss << L"ERROR_SUCCESS: Setting Content succeeded MainPage. cpp line 57" << std::hex << static_cast<unsigned>(e.code()) << L" - " << e.message().c_str();

        // NotifyUser updates the visible status panel (calls UpdateStatus on the UI thread).
        NotifyUser(winrt::hstring{ ss.str() }, winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage);
        // mark window invisible until ready

        // Update inside MainPage::MainPage() after m_swapChainPanel.Loaded(...)
        m_swapChainPanel.SizeChanged({ this, &MainPage::OnSwapChainPanelSizeChanged });

        // Handle composition scale changes (DPI / compositor scaling).
        m_swapChainPanel.CompositionScaleChanged([this](auto const& sender, auto const&)
            {
                // Compute logical size in DIPs using measured ActualWidth/ActualHeight.
                float aw = static_cast<float>(sender.ActualWidth());
                float ah = static_cast<float>(sender.ActualHeight());
                if (aw <= 0.0f || ah <= 0.0f) return;

                // Update DeviceResources and recreate size-dependent resources.
                m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(aw, ah));
                m_deviceResources->SetSwapChainPanel(sender, Window::Current().CoreWindow());
                m_deviceResources->CreateWindowSizeDependentResources();

                if (m_main)
                {
                    m_main->OnWindowSizeChanged();
                }
            });
        m_windowVisible = false;

        // in constructor (or lazy-create) on UI thread:
        m_resizeTimer = winrt::Windows::UI::Xaml::DispatcherTimer();
        m_resizeTimer.Interval(std::chrono::milliseconds(100)); // debounce interval
        m_resizeTimer.Tick({ this, &MainPage::OnResizeTimerTick });
        m_resizeTimer.Stop(); // start stopped; start when first event arrives

    }

    void MainPage::SaveInternalState(winrt::Windows::Foundation::Collections::IPropertySet const& state)
    {
        critical_section::scoped_lock lock(m_criticalSection);
		m_deviceResources->WaitForGpu();
        try
        {
            // Stop debounce timer (no further resize callbacks).
            if (m_resizeTimer)
            {
                m_resizeTimer.Stop();
            }

            // Stop the render loop first so the background worker won't submit more GPU work.
            if (m_main)
            {
            
              // m_deviceResources->Trim(); add later an empty function for now if you have ideas on what to put into Trim()...
               m_main->StopRenderLoop();
            } 
            // Ensure GPU work is finished before we release or snapshot GPU-backed resources.
            if (m_deviceResources)
            {
                m_deviceResources->WaitForGpu();

                // Unregister device notifications to avoid callbacks after page is suspended/closed.
                m_deviceResources->RegisterDeviceNotify(nullptr);
            }

            // Let app-level logic save renderer state (if implemented).
            if (m_main)
            {
                m_main->OnSuspending(); // best-effort; protects per-renderer state
            }

            // Persist a couple of simple values to the provided IPropertySet so the app can restore later.
            // Use box_value to create IPropertyValue objects.
            try
            {
                state.Insert(L"WindowVisible", winrt::box_value(m_windowVisible));
                // Add other keys as needed, e.g. camera angle, tracking flag, etc.
                // state.Insert(L"SomeKey", winrt::box_value(someValue));
            }
            catch (...)
            {
                // Ignore property-set failures (best-effort persistence).
            }
        }
        catch (...)
        {
            // Best-effort cleanup on save; do not rethrow from lifecycle callbacks.
        }
    }

    void MainPage::LoadInternalState(winrt::Windows::Foundation::Collections::IPropertySet const& state)
    {
		m_main->StartRenderLoop();
    }

    void MainPage::OnSwapChainPanelSizeChangedXaml(winrt::Windows::Foundation::IInspectable const& /* sender */,
        winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
        // Handle in OnSwapChainPanelSizeChanged
        critical_section::scoped_lock lock(m_criticalSection);
        // Safe way to get a SwapChainPanel from an IInspectable sender (use in any event handler)
		winrt::Windows::Foundation::Size newSize = args.NewSize();
        m_deviceResources->SetLogicalSize(newSize);
        m_main->CreateWindowSizedDependentResources();
    }

    void MainPage::OnSwapChainPanelSizeChanged(winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
        // Get SwapChainPanel from sender safely
        auto panel = sender.try_as<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>();
        if (!panel) return;

        float aw = static_cast<float>(args.NewSize().Width);
        float ah = static_cast<float>(args.NewSize().Height);
        if (aw <= 0.0f || ah <= 0.0f) return;

        // Ensure UI thread
        if (!Dispatcher().HasThreadAccess())
        {
            Dispatcher().RunAsync(CoreDispatcherPriority::Normal, winrt::Windows::UI::Core::DispatchedHandler([this, panel, aw, ah, args]()
                {
                    OnSwapChainPanelSizeChanged(panel, args);///*fabricated args not needed*/ winrt::Windows::UI::Xaml::SizeChangedEventArgs());
                }));
            return;
        }

        // Debounce: start/refresh timer to coalesce rapid events (DispatcherTimer)
        if (!m_resizeTimer)
        {
            m_resizeTimer = winrt::Windows::UI::Xaml::DispatcherTimer();
            m_resizeTimer.Interval(std::chrono::milliseconds(100));
            m_resizeTimer.Tick([this](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
                {
                    m_resizeTimer.Stop();

                    // Stop render loop & wait for GPU
                    if (m_main) m_main->StopRenderLoop();
                    m_deviceResources->WaitForGpu();

                    // Update DeviceResources with panel and size on UI thread
                    m_deviceResources->SetSwapChainPanel(m_swapChainPanel, Window::Current().CoreWindow());
                    m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(static_cast<float>(m_swapChainPanel.ActualWidth()), static_cast<float>(m_swapChainPanel.ActualHeight())));
                    m_deviceResources->SetCompositionScale(m_swapChainPanel.CompositionScaleX(), m_swapChainPanel.CompositionScaleY());
                    m_deviceResources->CreateWindowSizeDependentResources();

                    if (!m_main)
                    {
                        m_main = std::make_unique<Hot3dxBlankApp2Main>(m_deviceResources);
                        m_main->CreateRenderers(m_deviceResources);
                    }
                    else
                    {
                        m_main->OnWindowSizeChanged();
                    }

                    m_main->StartRenderLoop();
                    m_windowVisible = true;
                });
        }

        // store panel and restart timer
        m_swapChainPanel = panel;
        m_resizeTimer.Stop();
        m_resizeTimer.Start();
    }


    void MainPage::OnSwapChainPanelLoaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        auto panel = m_swapChainPanel;
        if (!panel) return;

        auto coreWindow = Window::Current().CoreWindow();

        // Give DeviceResources the CoreWindow and the SwapChainPanel.
        m_deviceResources->SetWindow(coreWindow);

        float aw = static_cast<float>(panel.ActualWidth());
        float ah = static_cast<float>(panel.ActualHeight());
        if (aw > 0 && ah > 0)
        {
            m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(aw, ah));
        }
        else
        {
            m_deviceResources->SetLogicalSize(winrt::Windows::Foundation::Size(coreWindow.Bounds().Width, coreWindow.Bounds().Height));
        }

        m_deviceResources->SetSwapChainPanel(panel, coreWindow);
        m_deviceResources->CreateWindowSizeDependentResources();

        // Create and start the renderer only once.
        if (!m_main)
        {
            m_main = std::make_unique<Hot3dxBlankApp2Main>(m_deviceResources);
            m_main->CreateRenderers(m_deviceResources);
            m_main->StartRenderLoop();
            m_windowVisible = true;
        }
        else
        {
            // If renderer already exists (unlikely during load), update it safely.
            m_main->StopRenderLoop();
            m_deviceResources->WaitForGpu();
            m_main->OnWindowSizeChanged();
            m_main->StartRenderLoop();
            m_windowVisible = true;
        }
    }

    void MainPage::OnSwapChainPanelResized(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args)
    {
		// Handle in OnSwapChainPanelSizeChanged
        critical_section::scoped_lock lock(m_criticalSection);
        // Safe way to get a SwapChainPanel from an IInspectable sender (use in any event handler)
        auto panel = sender.try_as<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>();
        if (!panel)
        {
            // Not a SwapChainPanel (or null) — handle gracefully
            return;
        }

        // Now `panel` is a valid SwapChainPanel you can use on the UI thread.
        m_swapChainPanel = panel;

		m_deviceResources->SetCompositionScale(m_swapChainPanel.CompositionScaleX(), m_swapChainPanel.CompositionScaleY());
        m_main->CreateWindowSizedDependentResources();
    }

    void MainPage::OnDPIChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args)
    {
        // Handle in OnSwapChainPanelSizeChanged
        critical_section::scoped_lock lock(m_criticalSection);
        m_deviceResources->SetDpi(sender.LogicalDpi());
        m_main->CreateWindowSizedDependentResources();
    }

    void MainPage::OnOrientationChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args)
    {
        // Handle in OnSwapChainPanelSizeChanged
		critical_section::scoped_lock lock(m_criticalSection);
		m_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
        m_main->CreateWindowSizedDependentResources();
    }

    void MainPage::OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const& sender, winrt::Windows::Foundation::IInspectable const& args)
    {
        // Handle in OnSwapChainPanelSizeChanged
        critical_section::scoped_lock lock(m_criticalSection);
		m_deviceResources->ValidateDevice();
    }

    void MainPage::OnCompositionScaleChanged(winrt::Windows::UI::Xaml::Controls::SwapChainPanel const& sender, winrt::Windows::Foundation::IInspectable const& args)
    {
        // Handle in OnSwapChainPanelSizeChanged
        critical_section::scoped_lock lock(m_criticalSection);
        
        // Safe way to get a SwapChainPanel from an IInspectable sender (use in any event handler)
        auto panel = sender.try_as<winrt::Windows::UI::Xaml::Controls::SwapChainPanel>();
        if (!panel)
        {
            // Not a SwapChainPanel (or null) — handle gracefully
            return;
        }

        // Now `panel` is a valid SwapChainPanel you can use on the UI thread.
        m_swapChainPanel = panel;

        m_deviceResources->SetCompositionScale(m_swapChainPanel.CompositionScaleX(), m_swapChainPanel.CompositionScaleY());
        m_main->CreateWindowSizedDependentResources();

    }

    void MainPage::OnNavigatedFrom(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const&)
    {
        // Stop render loop and ensure GPU is idle before tearing down UI or device resources.
        try
        {
            if (m_main)
            {
                m_main->StopRenderLoop();
            }

            if (m_deviceResources)
            {
                m_deviceResources->WaitForGpu();
                // Unregister device notifications so we won't get device callbacks after page unload.
                m_deviceResources->RegisterDeviceNotify(nullptr);
            }
        }
        catch (...)
        {
            // swallow - best-effort cleanup on navigation away
        }

        // Stop any debounce timers and detach panel reference to reduce races.
        if (m_resizeTimer)
        {
            m_resizeTimer.Stop();
        }

        m_swapChainPanel = nullptr;
        m_windowVisible = false;

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

    void MainPage::Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
    {
        Splitter().IsPaneOpen(!Splitter().IsPaneOpen());
    }

    void MainPage::Navigate(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName, winrt::Windows::Foundation::IInspectable const& parameter)
    {
        int index;
        for (index = static_cast<int>(scenarios2().Size()) - 1; index >= 0; --index)
            // for (index = static_cast<int>(scenarios().size()) - 1; index >= 0; --index)
        {
            if (scenarios2().GetAt(index).ClassType == typeName)
                //if (scenarios().at(index).ClassType == typeName)
            {
                break;
            }
        }
        navigating = true;
        ScenarioControl().SelectedIndex(index);
        navigating = false;

        NavigateTo(typeName, parameter);
    }

    void MainPage::ScenarioControl_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const& args)
    {
        using winrt::Hot3dxBlankApp2::ScenarioInfo;

        auto list = sender.as<winrt::Windows::UI::Xaml::Controls::ListBox>();
        if (!list)
        {
            return;
        }

        // If selection was changed programmatically we don't want to navigate again.
        if (navigating)
        {
            return;
        }

        // Prefer index-based access into the scenarios vector (safer than unbox in mixed templates).
        int32_t selectedIndex = static_cast<int32_t>(list.SelectedIndex());
        if (selectedIndex >= 0)
        {
            try
            {
                auto scenarios = MainPage::scenarios2();
                if (scenarios && selectedIndex < static_cast<int32_t>(scenarios.Size()))
                {
                    auto s = scenarios.GetAt(static_cast<uint32_t>(selectedIndex)); // copy of ScenarioInfo
                    // Provide feedback for debugging (optional)
                    NotifyUser(winrt::hstring{ L"Selected: " + s.Title }, winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage);

                    // Navigate to the scenario page
                    NavigateTo(s.ClassType, nullptr);
                    return;
                }
            }
            catch (...)
            {
                NotifyUser(winrt::hstring{ L"Failed to navigate (index path)" }, winrt::Hot3dxBlankApp2::implementation::NotifyType::ErrorMessage);
            }
        }

        // Fallback: try unboxing the selected item (works if you boxed ScenarioInfo into the ListBox)
        try
        {
            auto selectedItem = list.SelectedItem();
            if (selectedItem)
            {
                auto s = winrt::unbox_value<winrt::Hot3dxBlankApp2::ScenarioInfo>(selectedItem);
                NotifyUser(winrt::hstring{ L"Selected (unbox): " + s.Title }, winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage);
                NavigateTo(s.ClassType, nullptr);
            }
        }
        catch (...)
        {
            NotifyUser(winrt::hstring{ L"Selection change: unable to resolve scenario" }, winrt::Hot3dxBlankApp2::implementation::NotifyType::ErrorMessage);
        }
    }

    void MainPage::FooterTrademarks_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        try
        {
            auto uri = winrt::Windows::Foundation::Uri{ L"http://github.com/hot3dx/Hot3dxBlankApp2WinRTVS2026/blob/main/Assets/Square256x256.png" };
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

    void MainPage::Footer_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
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

    void MainPage::Footer_Click2(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args)
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

    // Replace the OnNavigatedTo implementation with a guarded, index-based loop
// to avoid iterating a null IVector and to prevent iterator-related issues.
    void MainPage::OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        // Create an observable collection for the ListBox items.
        auto itemCollection = single_threaded_observable_vector<IInspectable>();

        auto scenarios = MainPage::scenarios2();

        // Guard: if scenarios is null or empty, set empty ItemsSource and return.
        if (!scenarios || scenarios.Size() == 0)
        {
            ScenarioControl().ItemsSource(itemCollection);
            return;
        }

        // Use index-based access (GetAt) which is safer for WinRT collections in some interop contexts.
        int i = 1;
        for (uint32_t idx = 0; idx < scenarios.Size(); ++idx)
        {
            auto s = scenarios.GetAt(idx); // copies a ScenarioInfo struct
            s.Title = to_hstring(i++) + L") " + s.Title;
            itemCollection.Append(box_value(s));
        }

        // Set the newly created itemCollection as the ListBox ItemSource.
        ScenarioControl().ItemsSource(itemCollection);
        /*
        int startingScenarioIndex;

        if (Window::Current().Bounds().Width < 640)
        {
            startingScenarioIndex = -1;
        }
        else
        {
            startingScenarioIndex = 0;
        }
        */
        int startingScenarioIndex = Window::Current().Bounds().Width < 640 ? -1 : 0;

        ScenarioControl().SelectedIndex(startingScenarioIndex);
        ScenarioControl().ScrollIntoView(ScenarioControl().SelectedItem());

		// Navigate to the starting scenario.
        if (startingScenarioIndex >= 0 && startingScenarioIndex < static_cast<int>(scenarios.Size()))
        {
            NavigateTo(scenarios.GetAt(startingScenarioIndex).ClassType, IInspectable{ nullptr });
		}


    }

    void MainPage::NavigateTo(winrt::Windows::UI::Xaml::Interop::TypeName const& typeName, winrt::Windows::Foundation::IInspectable const& parameter)
    {
        // Clear the status block when changing scenarios
        NotifyUser(hstring(), winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage);

        // Navigate to the selected scenario.
        ScenarioFrame().Navigate(typeName, parameter);

        if (Window::Current().Bounds().Width < 640)
        {
            Splitter().IsPaneOpen(false);
        }
    }

    void MainPage::UpdateStatus(hstring strMessage, winrt::Hot3dxBlankApp2::implementation::NotifyType type)
    {
        using namespace winrt::Windows::UI;
        using namespace winrt::Windows::UI::Xaml;

        switch (type)
        {
        case winrt::Hot3dxBlankApp2::implementation::NotifyType::StatusMessage:
        {
            winrt::Windows::UI::Xaml::Media::SolidColorBrush brush1;
            brush1.Color(Colors::DarkBlue());
            StatusBorder().Background(brush1);
        }
        break;

        case winrt::Hot3dxBlankApp2::implementation::NotifyType::ErrorMessage:
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

    void MainPage::OnResizeTimerTick(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
    {
        m_resizeTimer.Stop();

        // Must be on UI thread here (DispatcherTimer guarantees this).
        // Stop render loop, wait for GPU, update DeviceResources, then restart loop.
        if (m_main)
        {
            m_main->StopRenderLoop();
        }

        m_deviceResources->WaitForGpu();

        m_deviceResources->SetSwapChainPanel(m_swapChainPanel, Window::Current().CoreWindow());
        m_deviceResources->SetLogicalSize(m_deviceResources->GetLogicalSize());// m_newLogicalSize);
        m_deviceResources->SetCompositionScale(m_swapChainPanel.CompositionScaleX(), m_swapChainPanel.CompositionScaleY());
        m_deviceResources->CreateWindowSizeDependentResources();

        if (!m_main)
        {
            m_main = std::make_unique<Hot3dxBlankApp2Main>(m_deviceResources);
            m_main->CreateRenderers(m_deviceResources);
        }
        else
        {
            m_main->OnWindowSizeChanged();
        }

        m_main->StartRenderLoop();
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
    void MainPage::NotifyUser(hstring strMessage, winrt::Hot3dxBlankApp2::implementation::NotifyType type)
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

