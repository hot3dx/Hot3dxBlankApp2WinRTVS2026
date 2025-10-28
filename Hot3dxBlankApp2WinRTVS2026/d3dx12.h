//*********************************************************
//
// D3D12 Helper Structures (Simplified)
// Based on Microsoft's d3dx12.h
//
//*********************************************************

#pragma once

#ifndef __D3DX12_H__
#define __D3DX12_H__

#include <d3d12.h>
#include <cstring>

//------------------------------------------------------------------------------------------------
struct CD3DX12_DEFAULT {};
extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

//------------------------------------------------------------------------------------------------
inline bool operator==( const D3D12_VIEWPORT& l, const D3D12_VIEWPORT& r ) noexcept
{
    return l.TopLeftX == r.TopLeftX && l.TopLeftY == r.TopLeftY && l.Width == r.Width &&
        l.Height == r.Height && l.MinDepth == r.MinDepth && l.MaxDepth == r.MaxDepth;
}

//------------------------------------------------------------------------------------------------
inline bool operator!=( const D3D12_VIEWPORT& l, const D3D12_VIEWPORT& r ) noexcept
{ return !( l == r ); }

//------------------------------------------------------------------------------------------------
struct CD3DX12_RECT : public D3D12_RECT
{
    CD3DX12_RECT() = default;
    explicit CD3DX12_RECT( const D3D12_RECT& o ) noexcept :
        D3D12_RECT( o )
    {}
    explicit CD3DX12_RECT(
        LONG Left,
        LONG Top,
        LONG Right,
        LONG Bottom ) noexcept
    {
        left = Left;
        top = Top;
        right = Right;
        bottom = Bottom;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_VIEWPORT : public D3D12_VIEWPORT
{
    CD3DX12_VIEWPORT() = default;
    explicit CD3DX12_VIEWPORT( const D3D12_VIEWPORT& o ) noexcept :
        D3D12_VIEWPORT( o )
    {}
    explicit CD3DX12_VIEWPORT(
        FLOAT topLeftX,
        FLOAT topLeftY,
        FLOAT width,
        FLOAT height,
        FLOAT minDepth = D3D12_MIN_DEPTH,
        FLOAT maxDepth = D3D12_MAX_DEPTH ) noexcept
    {
        TopLeftX = topLeftX;
        TopLeftY = topLeftY;
        Width = width;
        Height = height;
        MinDepth = minDepth;
        MaxDepth = maxDepth;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_HEAP_PROPERTIES : public D3D12_HEAP_PROPERTIES
{
    CD3DX12_HEAP_PROPERTIES() = default;
    explicit CD3DX12_HEAP_PROPERTIES( const D3D12_HEAP_PROPERTIES& o ) noexcept :
        D3D12_HEAP_PROPERTIES( o )
    {}
    CD3DX12_HEAP_PROPERTIES(
        D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
        D3D12_MEMORY_POOL memoryPoolPreference,
        UINT creationNodeMask = 1,
        UINT nodeMask = 1 ) noexcept
    {
        Type = D3D12_HEAP_TYPE_CUSTOM;
        CPUPageProperty = cpuPageProperty;
        MemoryPoolPreference = memoryPoolPreference;
        CreationNodeMask = creationNodeMask;
        VisibleNodeMask = nodeMask;
    }
    explicit CD3DX12_HEAP_PROPERTIES(
        D3D12_HEAP_TYPE type,
        UINT creationNodeMask = 1,
        UINT nodeMask = 1 ) noexcept
    {
        Type = type;
        CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        CreationNodeMask = creationNodeMask;
        VisibleNodeMask = nodeMask;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_RESOURCE_DESC : public D3D12_RESOURCE_DESC
{
    CD3DX12_RESOURCE_DESC() = default;
    explicit CD3DX12_RESOURCE_DESC( const D3D12_RESOURCE_DESC& o ) noexcept :
        D3D12_RESOURCE_DESC( o )
    {}
    CD3DX12_RESOURCE_DESC(
        D3D12_RESOURCE_DIMENSION dimension,
        UINT64 alignment,
        UINT64 width,
        UINT height,
        UINT16 depthOrArraySize,
        UINT16 mipLevels,
        DXGI_FORMAT format,
        UINT sampleCount,
        UINT sampleQuality,
        D3D12_TEXTURE_LAYOUT layout,
        D3D12_RESOURCE_FLAGS flags ) noexcept
    {
        Dimension = dimension;
        Alignment = alignment;
        Width = width;
        Height = height;
        DepthOrArraySize = depthOrArraySize;
        MipLevels = mipLevels;
        Format = format;
        SampleDesc.Count = sampleCount;
        SampleDesc.Quality = sampleQuality;
        Layout = layout;
        Flags = flags;
    }
    static inline CD3DX12_RESOURCE_DESC Buffer(
        const D3D12_RESOURCE_ALLOCATION_INFO& resAllocInfo,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE ) noexcept
    {
        return CD3DX12_RESOURCE_DESC( D3D12_RESOURCE_DIMENSION_BUFFER, resAllocInfo.Alignment, resAllocInfo.SizeInBytes,
            1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags );
    }
    static inline CD3DX12_RESOURCE_DESC Buffer(
        UINT64 width,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        UINT64 alignment = 0 ) noexcept
    {
        return CD3DX12_RESOURCE_DESC( D3D12_RESOURCE_DIMENSION_BUFFER, alignment, width, 1, 1, 1,
            DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags );
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER
{
    CD3DX12_RESOURCE_BARRIER() = default;
    explicit CD3DX12_RESOURCE_BARRIER( const D3D12_RESOURCE_BARRIER& o ) noexcept :
        D3D12_RESOURCE_BARRIER( o )
    {}
    static inline CD3DX12_RESOURCE_BARRIER Transition(
        _In_ ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter,
        UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
        D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE ) noexcept
    {
        CD3DX12_RESOURCE_BARRIER result = {};
        D3D12_RESOURCE_BARRIER &barrier = result;
        result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        result.Flags = flags;
        barrier.Transition.pResource = pResource;
        barrier.Transition.StateBefore = stateBefore;
        barrier.Transition.StateAfter = stateAfter;
        barrier.Transition.Subresource = subresource;
        return result;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_RANGE : public D3D12_RANGE
{
    CD3DX12_RANGE() = default;
    explicit CD3DX12_RANGE( const D3D12_RANGE& o ) noexcept :
        D3D12_RANGE( o )
    {}
    CD3DX12_RANGE(
        SIZE_T begin,
        SIZE_T end ) noexcept
    {
        Begin = begin;
        End = end;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_DESCRIPTOR_RANGE : public D3D12_DESCRIPTOR_RANGE
{
    CD3DX12_DESCRIPTOR_RANGE() = default;
    explicit CD3DX12_DESCRIPTOR_RANGE( const D3D12_DESCRIPTOR_RANGE& o ) noexcept :
        D3D12_DESCRIPTOR_RANGE( o )
    {}
    CD3DX12_DESCRIPTOR_RANGE(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ) noexcept
    {
        Init( rangeType, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart );
    }

    inline void Init(
        D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
        UINT numDescriptors,
        UINT baseShaderRegister,
        UINT registerSpace = 0,
        UINT offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND ) noexcept
    {
        RangeType = rangeType;
        NumDescriptors = numDescriptors;
        BaseShaderRegister = baseShaderRegister;
        RegisterSpace = registerSpace;
        OffsetInDescriptorsFromTableStart = offsetInDescriptorsFromTableStart;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_ROOT_SIGNATURE_DESC : public D3D12_ROOT_SIGNATURE_DESC
{
    CD3DX12_ROOT_SIGNATURE_DESC() = default;
    explicit CD3DX12_ROOT_SIGNATURE_DESC( const D3D12_ROOT_SIGNATURE_DESC& o ) noexcept :
        D3D12_ROOT_SIGNATURE_DESC( o )
    {}
    CD3DX12_ROOT_SIGNATURE_DESC(
        UINT numParameters,
        _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
        UINT numStaticSamplers = 0,
        _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE ) noexcept
    {
        Init( numParameters, _pParameters, numStaticSamplers, _pStaticSamplers, flags );
    }
    CD3DX12_ROOT_SIGNATURE_DESC( CD3DX12_DEFAULT ) noexcept
    {
        Init( 0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE );
    }

    inline void Init(
        UINT numParameters,
        _In_reads_opt_(numParameters) const D3D12_ROOT_PARAMETER* _pParameters,
        UINT numStaticSamplers = 0,
        _In_reads_opt_(numStaticSamplers) const D3D12_STATIC_SAMPLER_DESC* _pStaticSamplers = nullptr,
        D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE ) noexcept
    {
        NumParameters = numParameters;
        pParameters = _pParameters;
        NumStaticSamplers = numStaticSamplers;
        pStaticSamplers = _pStaticSamplers;
        Flags = flags;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE() = default;
    explicit CD3DX12_CPU_DESCRIPTOR_HANDLE( const D3D12_CPU_DESCRIPTOR_HANDLE& o ) noexcept :
        D3D12_CPU_DESCRIPTOR_HANDLE( o )
    {}
    CD3DX12_CPU_DESCRIPTOR_HANDLE( CD3DX12_DEFAULT ) noexcept { ptr = 0; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE(
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other,
        INT offsetScaledByIncrementSize ) noexcept
    {
        InitOffsetted( other, offsetScaledByIncrementSize );
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE(
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other,
        INT offsetInDescriptors,
        UINT descriptorIncrementSize ) noexcept
    {
        InitOffsetted( other, offsetInDescriptors, descriptorIncrementSize );
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset(
        INT offsetInDescriptors,
        UINT descriptorIncrementSize ) noexcept
    {
        ptr = SIZE_T( INT64( ptr ) + INT64( offsetInDescriptors ) * INT64( descriptorIncrementSize ) );
        return *this;
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE& Offset( INT offsetScaledByIncrementSize ) noexcept
    {
        ptr = SIZE_T( INT64( ptr ) + INT64( offsetScaledByIncrementSize ) );
        return *this;
    }
    inline void InitOffsetted(
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
        INT offsetScaledByIncrementSize ) noexcept
    {
        InitOffsetted( *this, base, offsetScaledByIncrementSize );
    }
    inline void InitOffsetted(
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
        INT offsetInDescriptors,
        UINT descriptorIncrementSize ) noexcept
    {
        InitOffsetted( *this, base, offsetInDescriptors, descriptorIncrementSize );
    }
    static inline void InitOffsetted(
        _Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle,
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
        INT offsetScaledByIncrementSize ) noexcept
    {
        handle.ptr = SIZE_T( INT64( base.ptr ) + INT64( offsetScaledByIncrementSize ) );
    }
    static inline void InitOffsetted(
        _Out_ D3D12_CPU_DESCRIPTOR_HANDLE& handle,
        _In_ const D3D12_CPU_DESCRIPTOR_HANDLE& base,
        INT offsetInDescriptors,
        UINT descriptorIncrementSize ) noexcept
    {
        handle.ptr = SIZE_T( INT64( base.ptr ) + INT64( offsetInDescriptors ) * INT64( descriptorIncrementSize ) );
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_RASTERIZER_DESC : public D3D12_RASTERIZER_DESC
{
    CD3DX12_RASTERIZER_DESC() = default;
    explicit CD3DX12_RASTERIZER_DESC( const D3D12_RASTERIZER_DESC& o ) noexcept :
        D3D12_RASTERIZER_DESC( o )
    {}
    explicit CD3DX12_RASTERIZER_DESC( CD3DX12_DEFAULT ) noexcept
    {
        FillMode = D3D12_FILL_MODE_SOLID;
        CullMode = D3D12_CULL_MODE_BACK;
        FrontCounterClockwise = FALSE;
        DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DepthClipEnable = TRUE;
        MultisampleEnable = FALSE;
        AntialiasedLineEnable = FALSE;
        ForcedSampleCount = 0;
        ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_BLEND_DESC : public D3D12_BLEND_DESC
{
    CD3DX12_BLEND_DESC() = default;
    explicit CD3DX12_BLEND_DESC( const D3D12_BLEND_DESC& o ) noexcept :
        D3D12_BLEND_DESC( o )
    {}
    explicit CD3DX12_BLEND_DESC( CD3DX12_DEFAULT ) noexcept
    {
        AlphaToCoverageEnable = FALSE;
        IndependentBlendEnable = FALSE;
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
        {
            FALSE,FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL,
        };
        for ( UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i )
            RenderTarget[ i ] = defaultRenderTargetBlendDesc;
    }
};

//------------------------------------------------------------------------------------------------
struct CD3DX12_SHADER_BYTECODE : public D3D12_SHADER_BYTECODE
{
    CD3DX12_SHADER_BYTECODE() = default;
    explicit CD3DX12_SHADER_BYTECODE( const D3D12_SHADER_BYTECODE& o ) noexcept :
        D3D12_SHADER_BYTECODE( o )
    {}
    CD3DX12_SHADER_BYTECODE(
        _In_ ID3DBlob* pShaderBlob ) noexcept
    {
        pShaderBytecode = pShaderBlob->GetBufferPointer();
        BytecodeLength = pShaderBlob->GetBufferSize();
    }
    CD3DX12_SHADER_BYTECODE(
        const void* _pShaderBytecode,
        SIZE_T bytecodeLength ) noexcept
    {
        pShaderBytecode = _pShaderBytecode;
        BytecodeLength = bytecodeLength;
    }
};

#endif //__D3DX12_H__
