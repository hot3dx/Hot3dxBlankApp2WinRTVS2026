#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <pix.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <memory>
#include <vector>
#include <agile.h>
#include <concrt.h>
#include <algorithm>

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw Platform::Exception::CreateException(hr);
        }
    }
}

#include "App.xaml.h"
