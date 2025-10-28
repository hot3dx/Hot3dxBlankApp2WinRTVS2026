#include "pch.h"
#include "MainPage.xaml.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace DirectX;

namespace winrt::Hot3dxBlankApp2WinRTVS2026::implementation
{
    MainPage::MainPage() :
        m_isDrawing(false),
        m_renderLoopWorker(nullptr)
    {
        InitializeComponent();

        m_deviceResources = std::make_shared<DX::DeviceResources>();
        m_renderer = std::make_unique<Hot3dxBlankApp2WinRTVS2026::RotoDraw3DRenderer>(m_deviceResources);

        Loaded({ this, &MainPage::OnLoaded });
        SizeChanged({ this, &MainPage::OnSizeChanged });

        auto resumingToken = Windows::ApplicationModel::Core::CoreApplication::Resuming({ this, &MainPage::OnResuming });
    }

    void MainPage::OnLoaded(IInspectable const&, RoutedEventArgs const&)
    {
        DrawingPanel().CompositionScaleChanged({ this, &MainPage::OnCompositionScaleChanged });
        
        // Get the raw pointer from the WinRT object
        auto panelNative = DrawingPanel().as<IUnknown>();
        Windows::UI::Xaml::Controls::SwapChainPanel^ panel = reinterpret_cast<Windows::UI::Xaml::Controls::SwapChainPanel^>(winrt::get_abi(DrawingPanel()));
        m_deviceResources->SetSwapChainPanel(panel);

        StartRenderLoop();
    }

    void MainPage::OnSizeChanged(IInspectable const&, SizeChangedEventArgs const& e)
    {
        m_deviceResources->SetLogicalSize(e.NewSize());
        m_renderer->CreateWindowSizeDependentResources();
    }

    void MainPage::OnCompositionScaleChanged(SwapChainPanel const& sender, IInspectable const&)
    {
        m_deviceResources->SetDpi(sender.CompositionScaleX() * 96.0f);
        m_renderer->CreateWindowSizeDependentResources();
    }

    void MainPage::OnResuming(IInspectable const&, IInspectable const&)
    {
        StartRenderLoop();
    }

    void MainPage::StartRenderLoop()
    {
        if (m_renderLoopWorker != nullptr && m_renderLoopWorker.Status() == AsyncStatus::Started)
        {
            return;
        }

        auto workItemHandler = ([this](IAsyncAction const& action)
        {
            while (action.Status() == AsyncStatus::Started)
            {
                m_renderer->Update();
                m_renderer->Render();
                m_deviceResources->Present();
            }
        });

        m_renderLoopWorker = Windows::System::Threading::ThreadPool::RunAsync(workItemHandler, Windows::System::Threading::WorkItemPriority::High, Windows::System::Threading::WorkItemOptions::TimeSliced);
    }

    void MainPage::StopRenderLoop()
    {
        if (m_renderLoopWorker)
        {
            m_renderLoopWorker.Cancel();
            m_renderLoopWorker = nullptr;
        }
    }

    void MainPage::SymmetrySlider_ValueChanged(IInspectable const&, Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
    {
        int value = static_cast<int>(e.NewValue());
        SymmetryText().Text(to_hstring(value));
        if (m_renderer)
        {
            m_renderer->SetSymmetryCount(value);
        }
    }

    void MainPage::BrushSizeSlider_ValueChanged(IInspectable const&, Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
    {
        if (m_renderer)
        {
            m_renderer->SetBrushSize(static_cast<float>(e.NewValue()));
        }
    }

    void MainPage::ClearButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_renderer)
        {
            m_renderer->ClearPoints();
        }
    }

    void MainPage::MirrorCheckBox_Changed(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_renderer)
        {
            m_renderer->SetMirrorMode(MirrorCheckBox().IsChecked().GetBoolean());
        }
    }

    void MainPage::ColorButton_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto button = sender.as<Button>();
        auto tag = unbox_value<hstring>(button.Tag());

        XMFLOAT4 color;
        if (tag == L"Magenta")
            color = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
        else if (tag == L"Cyan")
            color = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
        else if (tag == L"Yellow")
            color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
        else if (tag == L"Lime")
            color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
        else if (tag == L"Red")
            color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
        else
            color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        if (m_renderer)
        {
            m_renderer->SetBrushColor(color);
        }
    }

    void MainPage::DrawingPanel_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        m_isDrawing = true;
        auto point = e.GetCurrentPoint(DrawingPanel());
        if (m_renderer)
        {
            m_renderer->AddPoint(XMFLOAT2(static_cast<float>(point.Position().X), static_cast<float>(point.Position().Y)));
        }
    }

    void MainPage::DrawingPanel_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (m_isDrawing)
        {
            auto point = e.GetCurrentPoint(DrawingPanel());
            if (m_renderer)
            {
                m_renderer->AddPoint(XMFLOAT2(static_cast<float>(point.Position().X), static_cast<float>(point.Position().Y)));
            }
        }
    }

    void MainPage::DrawingPanel_PointerReleased(IInspectable const&, PointerRoutedEventArgs const&)
    {
        m_isDrawing = false;
    }
}
