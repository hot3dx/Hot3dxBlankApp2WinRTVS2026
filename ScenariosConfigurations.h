//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
// Explanation:
// Use the projected WinRT 'Scenario' (winrt::Hot3dxBlankApp2::Scenario) here instead
// of the implementation type. IVector<T> requires a projected WinRT type and using the
// projected type avoids generated-header template / name-lookup errors.
#pragma once 

#include "pch.h"
#include <winrt/Windows.Foundation.Collections.h>
#include "MainPage.h"


namespace winrt::Hot3dxBlankApp2
{

}

namespace winrt::Hot3dxBlankApp2::implementation
{
	// Populate the provided vector with ScenarioInfo entries.
	void PopulateScenarios(winrt::Windows::Foundation::Collections::IVector<Hot3dxBlankApp2::ScenarioInfo>& outVector);

	struct ScenarioInfo
	{
		hstring Title;
		winrt::Windows::UI::Xaml::Interop::TypeName ClassType;
	};
}
