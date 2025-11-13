
Quick setup for Visual Studio 2026 environment and package restore.

1. Visual Studio workloads
   - Install via __Visual Studio Installer__ the UWP and C++ workloads:
     - __C++ (v142) and (v143) and (v145 for VS2026) Universal Windows Platform tools__

2. NuGet packages used by the project (from `packages.config`)
   - `Microsoft.Web.WebView2` — version 1.0.3595.46
   - `Microsoft.Windows.CppWinRT` — version 2.0.250303.1
   - `WinPixEventRuntime` — version 1.0.240308001

3. Restore and build
   - Recommended: run `nuget restore Hot3dxBlankApp2.sln`.
   - Open `Hot3dxBlankApp2.sln` and build Release.

4. Security scanning
   - Dependabot can be used for NuGet updates.
   - CodeQL analysis can run via GitHub Actions.

