#include "pch.h"
#include "RotoDraw3DRenderer.h"
#include <d3dcompiler.h>

using namespace Hot3dxBlankApp2WinRTVS2026;
using namespace DirectX;
using namespace Microsoft::WRL;

RotoDraw3DRenderer::RotoDraw3DRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources),
    m_symmetryCount(8),
    m_brushSize(5.0f),
    m_brushColor(1.0f, 0.0f, 1.0f, 1.0f),
    m_mirrorMode(false),
    m_center(0.5f, 0.5f),
    m_loadingComplete(false)
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

RotoDraw3DRenderer::~RotoDraw3DRenderer()
{
    ReleaseDeviceDependentResources();
}

void RotoDraw3DRenderer::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    DX::ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

    static const char vertexShader[] =
        "struct VSInput\n"
        "{\n"
        "    float2 position : POSITION;\n"
        "    float4 color : COLOR;\n"
        "};\n"
        "struct PSInput\n"
        "{\n"
        "    float4 position : SV_POSITION;\n"
        "    float4 color : COLOR;\n"
        "};\n"
        "PSInput main(VSInput input)\n"
        "{\n"
        "    PSInput result;\n"
        "    result.position = float4(input.position.x * 2.0 - 1.0, 1.0 - input.position.y * 2.0, 0.0, 1.0);\n"
        "    result.color = input.color;\n"
        "    return result;\n"
        "}\n";

    static const char pixelShader[] =
        "struct PSInput\n"
        "{\n"
        "    float4 position : SV_POSITION;\n"
        "    float4 color : COLOR;\n"
        "};\n"
        "float4 main(PSInput input) : SV_TARGET\n"
        "{\n"
        "    return input.color;\n"
        "}\n";

    ComPtr<ID3DBlob> vertexShaderBlob;
    ComPtr<ID3DBlob> pixelShaderBlob;

    DX::ThrowIfFailed(D3DCompile(vertexShader, strlen(vertexShader), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &error));
    DX::ThrowIfFailed(D3DCompile(pixelShader, strlen(pixelShader), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &error));

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_deviceResources->GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;

    DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

    DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_deviceResources->GetCommandAllocator(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
    DX::ThrowIfFailed(m_commandList->Close());

    m_loadingComplete = true;
}

void RotoDraw3DRenderer::CreateWindowSizeDependentResources()
{
    auto outputSize = m_deviceResources->GetOutputSize();
    m_center = XMFLOAT2(outputSize.Width / 2.0f, outputSize.Height / 2.0f);
}

void RotoDraw3DRenderer::ReleaseDeviceDependentResources()
{
    m_loadingComplete = false;
    m_pipelineState.Reset();
    m_rootSignature.Reset();
    m_commandList.Reset();
    m_vertexBuffer.Reset();
}

void RotoDraw3DRenderer::Update()
{
}

void RotoDraw3DRenderer::Render()
{
    if (!m_loadingComplete)
    {
        return;
    }

    auto device = m_deviceResources->GetD3DDevice();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    DX::ThrowIfFailed(commandAllocator->Reset());
    DX::ThrowIfFailed(m_commandList->Reset(commandAllocator, m_pipelineState.Get()));

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_deviceResources->GetScreenViewport());

    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(m_deviceResources->GetOutputSize().Width), static_cast<LONG>(m_deviceResources->GetOutputSize().Height) };
    m_commandList->RSSetScissorRects(1, &scissorRect);

    auto renderTarget = m_deviceResources->GetRenderTarget();
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_deviceResources->GetRtvHeap()->GetCPUDescriptorHandleForHeapStart(),
        m_deviceResources->GetCurrentFrameIndex(),
        m_deviceResources->GetRtvDescriptorSize()
    );

    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    if (m_points.size() > 0 && m_vertexBuffer)
    {
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->DrawInstanced(static_cast<UINT>(m_points.size()), 1, 0, 0);
    }

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);

    DX::ThrowIfFailed(m_commandList->Close());

    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void RotoDraw3DRenderer::AddPoint(XMFLOAT2 point)
{
    auto symmetricalPoints = GenerateSymmetricalPoints(point);
    m_points.insert(m_points.end(), symmetricalPoints.begin(), symmetricalPoints.end());
    UpdateVertexBuffer();
}

void RotoDraw3DRenderer::ClearPoints()
{
    m_points.clear();
    m_vertexBuffer.Reset();
}

void RotoDraw3DRenderer::SetSymmetryCount(int count)
{
    m_symmetryCount = std::max(1, std::min(count, 32));
}

void RotoDraw3DRenderer::SetBrushSize(float size)
{
    m_brushSize = std::max(1.0f, std::min(size, 50.0f));
}

void RotoDraw3DRenderer::SetBrushColor(XMFLOAT4 color)
{
    m_brushColor = color;
}

void RotoDraw3DRenderer::SetMirrorMode(bool enabled)
{
    m_mirrorMode = enabled;
}

std::vector<DrawPoint> RotoDraw3DRenderer::GenerateSymmetricalPoints(const XMFLOAT2& point)
{
    std::vector<DrawPoint> points;
    auto outputSize = m_deviceResources->GetOutputSize();

    float normalizedX = point.x / outputSize.Width;
    float normalizedY = point.y / outputSize.Height;

    float dx = normalizedX - 0.5f;
    float dy = normalizedY - 0.5f;

    float angleStep = XM_2PI / m_symmetryCount;

    for (int i = 0; i < m_symmetryCount; ++i)
    {
        float angle = angleStep * i;
        float cosAngle = cosf(angle);
        float sinAngle = sinf(angle);

        float rotatedX = dx * cosAngle - dy * sinAngle + 0.5f;
        float rotatedY = dx * sinAngle + dy * cosAngle + 0.5f;

        DrawPoint drawPoint;
        drawPoint.position = XMFLOAT2(rotatedX, rotatedY);
        drawPoint.color = m_brushColor;
        points.push_back(drawPoint);

        if (m_mirrorMode)
        {
            float mirroredX = -dx * cosAngle - dy * sinAngle + 0.5f;
            float mirroredY = -dx * sinAngle + dy * cosAngle + 0.5f;

            DrawPoint mirroredPoint;
            mirroredPoint.position = XMFLOAT2(mirroredX, mirroredY);
            mirroredPoint.color = m_brushColor;
            points.push_back(mirroredPoint);
        }
    }

    return points;
}

void RotoDraw3DRenderer::UpdateVertexBuffer()
{
    if (m_points.empty())
    {
        return;
    }

    auto device = m_deviceResources->GetD3DDevice();
    UINT bufferSize = static_cast<UINT>(m_points.size() * sizeof(DrawPoint));

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    DX::ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)));

    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);
    DX::ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, m_points.data(), bufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(DrawPoint);
    m_vertexBufferView.SizeInBytes = bufferSize;
}
