#include "pch.h"

#include "App.h"
#include "ScenariosConfigurations.h"
#include "MainPage.h"
#include <debugapi.h>
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Navigation;
using namespace winrt::Hot3dxBlankApp2;
using namespace winrt::Hot3dxBlankApp2::implementation;

// These placeholder functions are used if the sample does not
// implement the corresponding methods. This allows us to simulate
// C# partial methods in C++.

namespace
{
    [[maybe_unused]] void App_Construct(App*) {}
    [[maybe_unused]] bool App_OverrideOnLaunched(LaunchActivatedEventArgs const&) { return false; }
    [[maybe_unused]] void App_LaunchCompleted(LaunchActivatedEventArgs const&) {}
    [[maybe_unused]] void App_OnActivated(IActivatedEventArgs const&) {}
    [[maybe_unused]] void App_OnFileActivated(FileActivatedEventArgs const&) {}
    [[maybe_unused]] void App_OnBackgroundActivated(BackgroundActivatedEventArgs const&) {}
}

/// <summary>
/// Creates the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
    InitializeComponent();

    Suspending({ this, &App::OnSuspending });

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
        if (IsDebuggerPresent())
        {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif

    App_Construct(this);
}

winrt::Windows::UI::Xaml::Controls::Frame winrt::Hot3dxBlankApp2::implementation::App::CreateRootFrame()
{
    Frame rootFrame{ nullptr };
    auto content = Window::Current().Content();
    if (content)
    {
        rootFrame = content.try_as<Frame>();
    }

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = Frame();

        rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });

        // Place the frame in the current Window
        Window::Current().Content(rootFrame);
    }

    return rootFrame;
}

/// <summary>
/// Invoked when the application is launched normally by the end user.  Other entry points
/// will be used such as when the application is launched to open a specific file.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs const& e)
{
    Frame rootFrame{ nullptr };
    auto content = Window::Current().Content();
    if (content)
    {
        rootFrame = content.try_as<Frame>();
    }

    // Do not repeat app initialization when the Window already has content,
    // just ensure that the window is active
    if (rootFrame == nullptr)
    {
        // Create a Frame to act as the navigation context and associate it with
        // a SuspensionManager key
        rootFrame = Frame();

        rootFrame.NavigationFailed({ this, &App::OnNavigationFailed });

        if (e.PreviousExecutionState() == ApplicationExecutionState::Terminated)
        {
            // Restore the saved session state only when appropriate, scheduling the
            // final launch steps after the restore is complete
        }

        if (e.PrelaunchActivated() == false)
        {
            if (rootFrame.Content() == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame.Navigate(xaml_typename<Hot3dxBlankApp2::MainPage>(), box_value(e.Arguments()));
            }
            // Place the frame in the current Window
            Window::Current().Content(rootFrame);
            // Ensure the current window is active
            Window::Current().Activate();
        }
    }
    else
    {
        if (e.PrelaunchActivated() == false)
        {
            if (rootFrame.Content() == nullptr)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter
                rootFrame.Navigate(xaml_typename<Hot3dxBlankApp2::MainPage>(), box_value(e.Arguments()));
            }
            // Ensure the current window is active
            Window::Current().Activate();
        }
    }
}

void winrt::Hot3dxBlankApp2::implementation::App::OnActivated(Windows::ApplicationModel::Activation::IActivatedEventArgs const& e)
{
    App_OnActivated(e);
}

void winrt::Hot3dxBlankApp2::implementation::App::OnFileActivated(Windows::ApplicationModel::Activation::FileActivatedEventArgs const& e)
{
    App_OnFileActivated(e);
}

void winrt::Hot3dxBlankApp2::implementation::App::OnBackgroundActivated(Windows::ApplicationModel::Activation::BackgroundActivatedEventArgs const& e)
{
    App_OnBackgroundActivated(e);
}

/// <summary>
/// Invoked when application execution is being suspended.  Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
/// <param name="sender">The source of the suspend request.</param>
/// <param name="e">Details about the suspend request.</param>
void App::OnSuspending([[maybe_unused]] IInspectable const&, [[maybe_unused]] winrt::Windows::ApplicationModel::SuspendingEventArgs const&)
{
    // Save application state and stop any background activity
}

/// <summary>
/// Invoked when Navigation to a certain page fails
/// </summary>
/// <param name="sender">The Frame which failed navigation</param>
/// <param name="e">Details about the navigation failure</param>
void App::OnNavigationFailed(IInspectable const&, NavigationFailedEventArgs const& e)
{
    throw hresult_error(E_FAIL, hstring(L"Failed to load Page ") + e.SourcePageType().Name);
}