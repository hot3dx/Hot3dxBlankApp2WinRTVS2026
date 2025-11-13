#include "pch.h"
#include "MainPage.h"
#include "Scenario1.h"
#include "Scenario2.h"
#include <hstring.h>
#include <winrt/Hot3dxBlankApp2.h>
#include "ScenariosConfigurations.h"


using namespace winrt;
using namespace winrt::Windows::Foundation::Collections;
using namespace Hot3dxBlankApp2;
//using namespace winrt::Hot3dxBlankApp2::implementation;

hstring implementation::MainPage::FEATURE_NAME()
{
    return L"Hot3dx Blank App";
}

// Use the projected WinRT 'Scenario' type (not implementation::Scenario).

winrt::Windows::Foundation::Collections::IVector<winrt::Hot3dxBlankApp2::ScenarioInfo> scenariosIn =
single_threaded_observable_vector<winrt::Hot3dxBlankApp2::ScenarioInfo>(
    {
        winrt::Hot3dxBlankApp2::ScenarioInfo{ L"Resizing the view", xaml_typename<winrt::Hot3dxBlankApp2::Scenario1>() },
        winrt::Hot3dxBlankApp2::ScenarioInfo{ L"Launching at a custom size", xaml_typename<winrt::Hot3dxBlankApp2::Scenario2>() },
    });


#include <winrt/Windows.UI.Xaml.h>

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;

void winrt::Hot3dxBlankApp2::implementation::PopulateScenarios(winrt::Windows::Foundation::Collections::IVector<Hot3dxBlankApp2::ScenarioInfo>& outVector)
{
    if (!outVector)
    {
        outVector = single_threaded_vector<Hot3dxBlankApp2::ScenarioInfo>();
    }

    outVector.Clear();

    Hot3dxBlankApp2::ScenarioInfo s1{};
    s1.Title = L"Scenario 1";
    s1.ClassType = xaml_typename<Hot3dxBlankApp2::Scenario1>();

    Hot3dxBlankApp2::ScenarioInfo s2{};
    s2.Title = L"Scenario 2";
    s2.ClassType = xaml_typename<Hot3dxBlankApp2::Scenario2>();

    outVector.Append(s1);
    outVector.Append(s2);
}