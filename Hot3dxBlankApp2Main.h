#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include <atomic> // add at top of file with other includes

using namespace DX;
namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;
using namespace Concurrency;
//using namespace winrt::Hot3dxBlankApp2::implementation;

// Renders Direct3D content on the screen.
namespace winrt::Hot3dxBlankApp2
{
	class Hot3dxBlankApp2Main : public IDeviceNotify
	{
	public:
		Hot3dxBlankApp2Main(const std::shared_ptr<DeviceResources>& deviceResources);
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update();
		void StartRenderLoop();
		void StopRenderLoop();
		void StartRenderLoopOrig();
		
		// Renders the current frame according to the current application state.
		bool Render();

		void Clear();
		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

		// Properties for input handling.
			// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();
		void CreateWindowSizedDependentResources();
		void ProcessInput(float x, float y) noexcept;
		// Accessors

		Sample3DSceneRenderer* GetSceneRenderer() { return m_sceneRenderer; }
		void SetSceneRenderer() {
			m_sceneRenderer = new Sample3DSceneRenderer(m_deviceResources);
		}

		void PauseRequested() {
			//m_timer.Stop();
			m_pauseRequested = true;
		};
		void PressComplete() {// if (m_updateState == RotoDraw3DX12::UpdateEngineState::WaitingForPress) 
			m_pressComplete = true;
		};


		void WindowActivationChanged(winrt::Windows::UI::Core::CoreWindowActivationState activationState);

		void KeyDown(winrt::Windows::System::VirtualKey const& key);
		void KeyUp(winrt::Windows::System::VirtualKey const& key);

		StepTimer GetTimer() const { return m_timer; }



	private:

		// Cached pointer to device resources.
		std::shared_ptr<DeviceResources> m_deviceResources;
		// TODO: Replace with your own content renderers.
		Sample3DSceneRenderer* m_sceneRenderer;

		winrt::Windows::Foundation::IAsyncAction m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;
		
		// Render-loop control flag used to stop/start the background worker safely.
		// Replaces holding a long-lived lock across Update/Render calls.
		std::atomic<bool> m_runRenderLoop{ false };      // already present in some edits; ensure exists
		std::atomic<bool> m_workerFinished{ true };      // becomes false while worker runs, true when exited


		// Rendering loop timer.
		DX::StepTimer m_timer;

		///////////////////////////////////////////////////
		bool                                                m_pauseRequested;
		bool                                                m_pressComplete;
		bool                                                m_renderNeeded;
		bool                                                m_haveFocus;
		bool                                                m_visible;

		//Hot3dxController^ m_controller;

		///////////////////////////////////////////////////

		// Track current input pointer position.
		float m_pointerLocationX;
		float m_pointerLocationY;
		std::atomic<float> m_pointerXAtomic{ 0.0f };
		std::atomic<float> m_pointerYAtomic{ 0.0f };
		// Add near other atomic members in Hot3dxBlankApp2Main
		
		winrt::Windows::UI::Core::CoreWindowActivationState m_activationState;
		winrt::Windows::System::VirtualKey m_key;
	};
}