# Building RotoDraw3D

## Prerequisites
1. **Visual Studio 2022** (or later)
   - Universal Windows Platform development workload
   - C++ Universal Windows Platform tools
   - Windows 10 SDK (10.0.17763.0 or later)

2. **Windows 10/11** 
   - Version 17763 or later
   - Developer mode enabled (for deployment)

## Build Steps

### Using Visual Studio
1. Open `Hot3dxBlankApp2WinRTVS2026.sln` in Visual Studio 2022
2. Select the target platform:
   - **x64** (recommended for most PCs)
   - **x86** (for older 32-bit systems)
   - **ARM64** (for ARM-based devices)
3. Select build configuration:
   - **Debug** (for development)
   - **Release** (for deployment)
4. Build the solution:
   - Menu: Build → Build Solution
   - Keyboard: Ctrl+Shift+B

### First-Time Build
On the first build, Visual Studio will:
- Generate C++/WinRT projection headers
- Download NuGet packages (if any)
- Compile all source files
- Link the executable
- Package the application

This may take 2-5 minutes depending on your system.

## Running the Application

### Local Machine
1. Set deployment target to "Local Machine"
2. Press F5 (run with debugging) or Ctrl+F5 (run without debugging)
3. The application will deploy and launch automatically

### Troubleshooting

#### Build Errors
- **Error: Windows SDK not found**
  - Install the required Windows 10 SDK version from Visual Studio Installer

- **Error: C++/WinRT headers not found**
  - Clean and rebuild the solution
  - Ensure the C++/WinRT NuGet package is installed

- **Error: Cannot find certificate**
  - Right-click the project → Publish → Create App Packages
  - Or: Create a test certificate manually

#### Runtime Errors
- **App doesn't launch**
  - Enable Developer Mode in Windows Settings
  - Check Windows Event Viewer for deployment errors

- **DirectX errors**
  - Ensure your graphics drivers are up to date
  - Verify DirectX12 support: run `dxdiag` and check Feature Levels

## Deployment

### Creating an App Package
1. Right-click the project in Solution Explorer
2. Select "Publish" → "Create App Packages"
3. Follow the wizard to create an APPX/MSIX package
4. The package can be sideloaded on other Windows devices

### Sideloading
To install on another device:
1. Enable Developer Mode on the target device
2. Double-click the .appx or .msix package
3. Click "Install"

## Performance Tips
- Use Release configuration for better performance
- Enable "Use .NET Native tool chain" for optimized builds
- Build for x64 on 64-bit systems for best performance
