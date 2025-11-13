#include "pch.h"
#include "Hot3dxBlankApp2Main.h"
#include "Common\DirectXHelper.h"

#include <winrt/base.h>
#include <winrt/windows.system.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.Threading.h>
#include <concrt.h>
#include <atomic>
#include <thread>

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


///////////////////////////////////////////////////////////
// Replace StartRenderLoop and StopRenderLoop implementations with the versions below.
// Key changes:
// - Use m_runRenderLoop atomic to control loop lifetime.
// - Avoid holding a Concurrency critical_section across Update()/Render() calls.
// - Snapshot pointers/flags locally to avoid races.
// - Ensure worker is cancelled and waited on cleanly in StopRenderLoop().
// Replace / update StartRenderLoop, StopRenderLoop and OnSuspending implementations with these versions.
// Keep other code as-is.

void Hot3dxBlankApp2Main::StartRenderLoop()
{
	if (!m_deviceResources || !m_sceneRenderer)
	{
#ifndef _DEBUG
		OutputDebugStringW(L"StartRenderLoop: deviceResources or sceneRenderer not ready\n");
#endif
		return;
	}

	// signal running and reset finished flag
	m_runRenderLoop.store(true, std::memory_order_release);
	m_workerFinished.store(false, std::memory_order_release);
#ifndef _DEBUG
	OutputDebugStringA("Hot3dxBlankApp2Main::StartRenderLoop - starting worker\n");
#endif
	auto handler = WorkItemHandler{ [this](wf::IAsyncAction const& action)
	{
			// Worker loop
			while (action.Status() == wf::AsyncStatus::Started && m_runRenderLoop.load(std::memory_order_acquire))
			{
				auto localDeviceResources = m_deviceResources;
				auto localSceneRenderer = m_sceneRenderer;

				if (!localDeviceResources || !localSceneRenderer)
				{
					Sleep(1);
					continue;
				}

				m_pointerLocationX = m_pointerXAtomic.load(std::memory_order_relaxed);
				m_pointerLocationY = m_pointerYAtomic.load(std::memory_order_relaxed);

				try
				{
					m_timer.Tick([&]()
					{
						localSceneRenderer->Update(m_timer);
					});
				}
				catch (...)
				{
				}

				try
				{
					if (localDeviceResources->m_isSwapPanelVisible && localSceneRenderer)
					{
						bool sceneRendered = localSceneRenderer->Render();
						if (sceneRendered)
						{
							localDeviceResources->Present();
						}
					}
				}
				catch (...)
				{
				}
			}

			// mark finished for StopRenderLoop to observe
			m_workerFinished.store(true, std::memory_order_release);
#ifndef _DEBUG
			OutputDebugStringA("Hot3dxBlankApp2Main::StartRenderLoop - worker exiting\n");
#endif
		} };

	m_renderLoopWorker = ThreadPool::RunAsync(handler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Hot3dxBlankApp2Main::StopRenderLoop()
{
#ifndef _DEBUG
	OutputDebugStringA("Hot3dxBlankApp2Main::StopRenderLoop - requested\n");
#endif // !_DEBUG
	// Signal worker to stop
	m_runRenderLoop.store(false, std::memory_order_release);

	if (m_renderLoopWorker)
	{
		try
		{
			m_renderLoopWorker.Cancel();
		}
		catch (...) {}

		// Wait until worker signals finished or timeout
		const int maxWaitMs = 5000;
		int waited = 0;
		while (!m_workerFinished.load(std::memory_order_acquire) && waited < maxWaitMs)
		{
			Sleep(1);
			++waited;
		}

		if (!m_workerFinished.load(std::memory_order_acquire))
		{
#ifndef _DEBUG
			OutputDebugStringA("WARN: StopRenderLoop timeout waiting for worker to finish\n");
#endif
		}
		else
		{
#ifndef _DEBUG
			OutputDebugStringA("Hot3dxBlankApp2Main::StopRenderLoop - worker finished\n");
#endif
		}

		// Ensure GPU idle before continuing to release resources
		try
		{
			if (m_deviceResources)
			{
				m_deviceResources->WaitForGpu();
			}
		}
		catch (...) {}

		m_renderLoopWorker = nullptr;
	}
	else
	{
		try { if (m_deviceResources) m_deviceResources->WaitForGpu(); }
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
#ifndef _DEBUG
		OutputDebugStringA(sceneRendered ? "INFO: SceneRenderer::Render returned TRUE\n" : "INFO: SceneRenderer::Render returned FALSE\n");
#endif
	}

	// Make sure we actually present the swap chain
	if (m_deviceResources && sceneRendered)
	{
#ifndef _DEBUG
		OutputDebugStringA("INFO: Calling DeviceResources::Present()\n");
#endif
		m_deviceResources->Present();
	}
	else
	{
#ifndef _DEBUG
		OutputDebugStringA("WARN: Skipping Present (no deviceResources or nothing rendered)\n");
#endif
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
#ifndef _DEBUG
	OutputDebugStringA("Hot3dxBlankApp2Main::OnSuspending - begin\n");
#endif
	// Stop render thread, wait for GPU and save state.
	try
	{
		StopRenderLoop();

		if (m_deviceResources)
		{
			m_deviceResources->WaitForGpu();

			try
			{
				m_deviceResources->Trim();
#ifndef _DEBUG
	OutputDebugStringA("Hot3dxBlankApp2Main::OnSuspending - DeviceResources::Trim called\n");
#endif // !_DEBUG
			}
			catch (...)
			{
#ifndef _DEBUG
				OutputDebugStringA("Hot3dxBlankApp2Main::OnSuspending - Trim threw\n");
#endif // !_DEBUG
			}
		}

		if (m_sceneRenderer)
		{
			m_sceneRenderer->SaveState();
		}
	}
	catch (...)
	{
		// best-effort
	}

	// Existing suspend work can follow.
	// TODO: Replace this with your app's suspending logic.

	// Process lifetime management may terminate suspended apps at any time, so it is
	// good practice to save any state that will allow the app to restart where it left off.

	//m_sceneRenderer->SaveState();

	// If your application uses video memory allocations that are easy to re-create,
	// consider releasing that memory to make it available to other applications.

#ifndef _DEBUG
	OutputDebugStringA("Hot3dxBlankApp2Main::OnSuspending - end\n");
#endif
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

void winrt::Hot3dxBlankApp2::Hot3dxBlankApp2Main::CreateWindowSizedDependentResources()
{
	if (m_sceneRenderer->GetLoadingComplete() == false)return;
		m_sceneRenderer->CreateWindowSizeDependentResources();
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

void Hot3dxBlankApp2::Hot3dxBlankApp2Main::ProcessInput(float x, float y) noexcept
{
	// TODO: Add per frame input handling here.
	m_pointerXAtomic.store(x, std::memory_order_relaxed);
	m_pointerYAtomic.store(y, std::memory_order_relaxed);

	m_sceneRenderer->TrackingUpdate(m_pointerLocationX, m_pointerLocationY);
}