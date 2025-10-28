# Hot3dxBlankApp2WinRTVS2026
RotoDraw3D (Hot3dxSymmetry3D) - A Symmetrical Drawing Application in WinRT XAML C++17/C++20 with DirectX12

## Overview
This is a Universal Windows Platform (UWP) application that enables users to create beautiful symmetrical drawings using DirectX12 rendering. The application features radial symmetry with customizable parameters and real-time drawing on a DirectX12-powered canvas.

## Features
- **Radial Symmetry Drawing**: Create patterns with 1-16 fold rotational symmetry
- **Mirror Mode**: Enable mirroring for kaleidoscope-like effects
- **Brush Customization**: Adjust brush size and choose from multiple colors
- **Real-time DirectX12 Rendering**: Hardware-accelerated drawing with smooth performance
- **Touch and Mouse Support**: Draw with either touch or mouse input

## Technology Stack
- **Language**: C++17/C++20
- **Platform**: Universal Windows Platform (UWP)
- **Graphics API**: DirectX12
- **UI Framework**: XAML
- **IDE**: Visual Studio 2022

## Project Structure
```
Hot3dxBlankApp2WinRTVS2026/
├── Hot3dxBlankApp2WinRTVS2026.sln          # Visual Studio Solution
└── Hot3dxBlankApp2WinRTVS2026/             # Main Project
    ├── App.xaml                             # Application definition
    ├── App.xaml.cpp/h                       # Application code
    ├── MainPage.xaml                        # Main UI page
    ├── MainPage.xaml.cpp/h                  # Main page code
    ├── DeviceResources.cpp/h                # DirectX12 device management
    ├── RotoDraw3DRenderer.cpp/h             # Symmetry drawing engine
    ├── d3dx12.h                             # DirectX12 helper structures
    ├── pch.h/cpp                            # Precompiled headers
    ├── Package.appxmanifest                 # App manifest
    └── Assets/                              # Application assets
```

## Building the Project
1. Open `Hot3dxBlankApp2WinRTVS2026.sln` in Visual Studio 2022
2. Ensure you have the following installed:
   - Windows 10 SDK (10.0.17763.0 or later)
   - Universal Windows Platform development workload
   - C++ Universal Windows Platform tools
3. Select your target platform (x64, x86, or ARM64)
4. Build the solution (F7) or press Ctrl+Shift+B

## Running the Application
1. Set the platform to x64 or x86
2. Select "Local Machine" as the deployment target
3. Press F5 to run with debugging or Ctrl+F5 to run without debugging

## Usage
1. **Drawing**: Click/touch and drag on the canvas to draw
2. **Symmetry Control**: Use the slider to adjust the number of symmetry axes (1-16)
3. **Brush Size**: Adjust the brush size slider to change stroke thickness
4. **Color Selection**: Click on any color button to change the drawing color
5. **Mirror Mode**: Enable the checkbox to add mirror symmetry for kaleidoscope effects
6. **Clear Canvas**: Click the "Clear" button to start fresh

## Technical Details

### DirectX12 Implementation
- Uses DirectX12 for hardware-accelerated rendering
- Implements efficient vertex buffer management for drawing points
- Double-buffered swap chain for smooth presentation
- Command lists and command allocators for GPU command submission

### Symmetry Algorithm
The symmetry engine generates multiple instances of each drawn point by:
1. Converting screen coordinates to normalized space
2. Calculating rotation angles based on symmetry count
3. Applying rotation transformations around the center point
4. Optionally applying mirror transformations for enhanced effects

### Performance
- Optimized for real-time drawing with minimal latency
- Efficient GPU utilization through DirectX12
- Asynchronous rendering loop for responsive UI

## System Requirements
- Windows 10 version 17763 or later
- DirectX12-compatible graphics card
- Visual Studio 2022 (for building from source)

## License
Copyright © 2025 Hot3dx. All rights reserved.

## Contributing
This is a demonstration project showcasing UWP DirectX12 development with C++17/C++20.
