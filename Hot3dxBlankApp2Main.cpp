#include "pch.h"
#include "Hot3dxBlankApp2Main.h"
#include "Common\DirectXHelper.h"

#include <winrt/base.h>
#include <winrt/windows.system.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.Threading.h>

using namespace winrt::Hot3dxBlankApp2;
using namespace winrt;
namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::System;
using namespace winrt::Windows::System::Threading;
using namespace Concurrency;

// The DirectX 12 Application template is documented at https://go.microsoft.com/fwlink/?LinkID=613670&clcid=0x409

// Loads and initializes application assets when the application is loaded.
Hot3dxBlankApp2Main::Hot3dxBlankApp2Main(const std::shared_ptr<DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_pointerLocationX(0.0f),
	m_pointerLocationY(0.0f),
	m_visible{ false },
	m_renderNeeded{ false },
	m_pauseRequested{ false },
	m_pressComplete{ false },
	m_renderLoopWorker(nullptr),
	m_sceneRenderer(nullptr),
	m_timer{},
	m_key(),
	m_haveFocus{ false },
	m_activationState{ false }
{
	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	//SetSceneRenderer();

}

// Creates and initializes the renderers.
void Hot3dxBlankApp2Main::CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
	// TODO: Replace this with your app's content initialization.
	m_deviceResources = deviceResources;
	SetSceneRenderer();
	OnWindowSizeChanged();
}

// Updates the application state once per frame.
void Hot3dxBlankApp2Main::Update()
{
	//ProcessInput();
	// Update scene objects.
	m_timer.Tick([&]()
		{
			// TODO: Replace this with your app's content update functions.
			m_sceneRenderer->Update(m_timer);
		});

	Render();
}

void Hot3dxBlankApp2Main::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker && m_renderLoopWorker.Status() == wf::AsyncStatus::Started)
	{
		return;
	}

	// Create a WorkItemHandler that will be run on a background thread.
	auto handler = WorkItemHandler{ [this](wf::IAsyncAction const& action)
	{
		while (action.Status() == wf::AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);

			auto commandQueue = m_deviceResources->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				Update();
			}
			PIXEndEvent(commandQueue);

			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (m_deviceResources->m_isSwapPanelVisible == true)
				{
					m_sceneRenderer->Render();
				}
			}
			PIXEndEvent(commandQueue);
		}
	} };

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(handler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Hot3dxBlankApp2Main::StopRenderLoop()
{
	if (m_renderLoopWorker)
	{
		try
		{
			m_renderLoopWorker.Cancel();
		}
		catch (...) {}
	}
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool Hot3dxBlankApp2Main::Render()
{

	// Call scene renderer first
	bool sceneRendered = false;
	if (m_sceneRenderer)
	{
		sceneRendered = m_sceneRenderer->Render();
		OutputDebugStringA(sceneRendered ? "INFO: SceneRenderer::Render returned TRUE\n" : "INFO: SceneRenderer::Render returned FALSE\n");
	}

	// Make sure we actually present the swap chain
	if (m_deviceResources && sceneRendered)
	{
		OutputDebugStringA("INFO: Calling DeviceResources::Present()\n");
		m_deviceResources->Present();
	}
	else
	{
		OutputDebugStringA("WARN: Skipping Present (no deviceResources or nothing rendered)\n");
	}

	return sceneRendered;
}

void Hot3dxBlankApp2::Hot3dxBlankApp2Main::Clear()
{
	m_sceneRenderer->Clear();
}

// Updates application state when the window's size changes (e.g. device orientation change)
void Hot3dxBlankApp2Main::OnWindowSizeChanged()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	if (m_sceneRenderer->GetLoadingComplete() == false)return;
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Notifies the app that it is being suspended.
void Hot3dxBlankApp2Main::OnSuspending()
{
	// TODO: Replace this with your app's suspending logic.

	// Process lifetime management may terminate suspended apps at any time, so it is
	// good practice to save any state that will allow the app to restart where it left off.

	m_sceneRenderer->SaveState();

	// If your application uses video memory allocations that are easy to re-create,
	// consider releasing that memory to make it available to other applications.
}

// Notifes the app that it is no longer suspended.
void Hot3dxBlankApp2Main::OnResuming()
{
	// TODO: Replace this with your app's resuming logic.
}

// Notifies renderers that device resources need to be released.
void Hot3dxBlankApp2Main::OnDeviceRemoved()
{
	// TODO: Save any necessary application or renderer state and release the renderer
	// and its resources which are no longer valid.
	m_sceneRenderer->SaveState();
	m_sceneRenderer = nullptr;
}

void Hot3dxBlankApp2Main::OnDeviceLost()
{
	m_sceneRenderer->OnDeviceLost();
}

void Hot3dxBlankApp2Main::OnDeviceRestored()
{
	m_sceneRenderer->OnDeviceRestored();
}

void Hot3dxBlankApp2Main::WindowActivationChanged(winrt::Windows::UI::Core::CoreWindowActivationState activationState)
{
	unsigned int val = static_cast<unsigned int>(activationState);
	switch (val)
	{
	case 0://Windows::UI::Core::CoreWindowActivationState::CodeActivated:
		break;
	case 1://Windows::UI::Core::CoreWindowActivationState::Deactivated:
		break;
	case 2://Windows::UI::Core::CoreWindowActivationState::PointerActivated:
		break;
	default:
		break;
	};
}

void Hot3dxBlankApp2Main::KeyDown(winrt::Windows::System::VirtualKey const& key)
{
	int Key = static_cast<int>(key);
	switch (Key)
	{
	case 20:
	default:
		break;
	};
}

void Hot3dxBlankApp2Main::KeyUp(winrt::Windows::System::VirtualKey const& key)
{
	int Key = static_cast<int>(key);
	switch (Key)
	{
	case 20:
	default:
		break;
	};
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX, m_pointerLocationY);
}

void Hot3dxBlankApp2::Hot3dxBlankApp2Main::ProcessInput()
{
	// TODO: Add per frame input handling here.
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX, m_pointerLocationY);
}