#pragma once

#include "pch.h"
#include <stdint.h>

#include <winrt/base.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.applicationmodel.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.xaml.controls.h>
#include<winrt/windows.storage.h>
#include<winrt/windows.storage.streams.h>

using namespace DirectX;
//using namespace Microsoft::WRL;
namespace wf = winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch Win32 API errors.
			throw winrt::hresult_error(hr);
		}
	}

	// Function that reads from a binary file asynchronously.

	inline winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::Streams::IBuffer> ReadMyDataAsync(winrt::param::hstring const& filename)
	{
		auto folder = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation();
		const winrt::Windows::Storage::StorageFile file = co_await folder.GetFileAsync(filename);
		winrt::Windows::Storage::Streams::IBuffer buffer = co_await winrt::Windows::Storage::FileIO::ReadBufferAsync(file);
		co_return buffer; // <-- Return the IBuffer directly
	}
	
	// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	// Assign a name to the object to aid with debugging.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif
}

// Naming helper function for ComPtr<T>.
// Assigns the name of the variable as the name of the object.
//#define NAME_D3D12_OBJECTRT(x) DX::SetName(x.get(), L#x)
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)
