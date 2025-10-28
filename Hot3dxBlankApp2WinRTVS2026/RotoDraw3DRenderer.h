#pragma once

#include "DeviceResources.h"
#include <vector>

namespace Hot3dxBlankApp2WinRTVS2026
{
    struct DrawPoint
    {
        DirectX::XMFLOAT2 position;
        DirectX::XMFLOAT4 color;
    };

    class RotoDraw3DRenderer
    {
    public:
        RotoDraw3DRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        ~RotoDraw3DRenderer();

        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Update();
        void Render();

        void AddPoint(DirectX::XMFLOAT2 point);
        void ClearPoints();
        void SetSymmetryCount(int count);
        void SetBrushSize(float size);
        void SetBrushColor(DirectX::XMFLOAT4 color);
        void SetMirrorMode(bool enabled);

        int GetSymmetryCount() const { return m_symmetryCount; }
        float GetBrushSize() const { return m_brushSize; }
        DirectX::XMFLOAT4 GetBrushColor() const { return m_brushColor; }
        bool GetMirrorMode() const { return m_mirrorMode; }

    private:
        void CreatePipeline();
        void CreateVertexBuffer();
        void UpdateVertexBuffer();
        std::vector<DrawPoint> GenerateSymmetricalPoints(const DirectX::XMFLOAT2& point);

        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

        std::vector<DrawPoint> m_points;
        int m_symmetryCount;
        float m_brushSize;
        DirectX::XMFLOAT4 m_brushColor;
        bool m_mirrorMode;

        DirectX::XMFLOAT2 m_center;
        bool m_loadingComplete;
    };
}
