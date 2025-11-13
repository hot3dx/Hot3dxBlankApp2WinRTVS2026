#pragma once

#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <Unknwnbase.h>
// Add this before any #include <winrt/...> lines
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include "winrt/Windows.UI.Xaml.Automation.Peers.h"
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include "winrt/Windows.UI.Xaml.Documents.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>


#include <wrl.h>
#include <wrl/client.h>
#include <wrl/module.h>    // <-- use WRL's StaticStorage / Module implementation
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "Common\d3dx12.h"
#include <pix.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>

#include <algorithm>
#include <concrt.h>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace winrt;

#if defined(_DEBUG)
 #include <dxgidebug.h>
#endif