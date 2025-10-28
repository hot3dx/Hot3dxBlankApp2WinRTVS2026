# RotoDraw3D Quick Reference

## Quick Start
1. Launch RotoDraw3D
2. Adjust symmetry slider (1-16)
3. Select a color
4. Draw on the canvas
5. Enable mirror mode for kaleidoscope effects
6. Click Clear to start over

## UI Controls at a Glance

| Control | Location | Function | Range |
|---------|----------|----------|-------|
| Symmetry Slider | Top, Center-Left | Sets rotation count | 1-16 |
| Brush Size Slider | Top, Center | Adjusts stroke size | 1-20 |
| Clear Button | Top, Center-Right | Clears canvas | - |
| Mirror Checkbox | Top, Center-Right | Enables mirror mode | On/Off |
| Color Buttons | Top, Right | Selects brush color | 5 colors |

## Color Palette
- 🟣 **Magenta** - Vibrant purple-pink
- 🔵 **Cyan** - Cool light blue  
- 🟡 **Yellow** - Bright yellow
- 🟢 **Lime** - Bright green
- 🔴 **Red** - Bold red

## Symmetry Count Guide

| Count | Effect | Best For |
|-------|--------|----------|
| 1 | No symmetry | Regular drawing |
| 2 | Bilateral | Butterflies, faces |
| 3 | Tri-radial | Triangular patterns |
| 4 | Quad-radial | Squares, crosses |
| 5 | Penta-radial | Flowers, stars |
| 6 | Hexa-radial | Snowflakes, honeycombs |
| 8 | Octa-radial | Mandalas (default) |
| 12+ | High-density | Complex geometric |

## Drawing Techniques

### Basic Patterns
- **Circles**: Create mandala centers
- **Radial Lines**: Make star/flower petals
- **Spirals**: Generate whirlpool effects
- **Random**: Create organic patterns

### Pro Tips
- Start from center for balanced designs
- Vary brush size for depth
- Mix colors for vibrant art
- Use mirror mode sparingly for best effect
- Draw slowly for smooth lines
- Draw quickly for energetic strokes

## Common Tasks

### Create a Mandala
1. Symmetry: 8-12
2. Mirror: On
3. Start from center
4. Draw circular patterns outward

### Create a Flower
1. Symmetry: 5-6
2. Mirror: Off
3. Draw petal shapes from center

### Create Abstract Art
1. Symmetry: 2-4
2. Toggle mirror mode
3. Experiment freely

### Create Geometric Pattern
1. Symmetry: 12-16
2. Mirror: On
3. Draw straight lines and angles

## Keyboard Shortcuts
*Not currently implemented - future feature*

| Key | Function |
|-----|----------|
| Ctrl+Z | Undo (planned) |
| Ctrl+Y | Redo (planned) |
| Ctrl+C | Clear (planned) |
| 1-5 | Select color (planned) |
| +/- | Adjust symmetry (planned) |

## File Locations

### Source Code
```
Hot3dxBlankApp2WinRTVS2026/
├── App.xaml.cpp/h              - Application
├── MainPage.xaml.cpp/h         - UI & Events
├── DeviceResources.cpp/h       - DirectX12
├── RotoDraw3DRenderer.cpp/h    - Rendering
└── d3dx12.h                    - D3D12 Helpers
```

### Documentation
- `README.md` - Project overview
- `BUILDING.md` - Build instructions
- `ARCHITECTURE.md` - Technical details
- `USER_GUIDE.md` - Complete manual
- `CONTRIBUTING.md` - For developers
- `DIAGRAMS.md` - Visual diagrams

## Performance Tips
- Clear canvas periodically if it slows down
- Use Release build for better performance
- Draw at consistent speed for smooth lines
- Restart app if experiencing issues

## Troubleshooting Quick Fixes

| Problem | Solution |
|---------|----------|
| App won't launch | Enable Developer Mode in Windows Settings |
| Slow performance | Clear canvas or restart app |
| Jagged lines | Draw more slowly |
| No symmetry visible | Check slider is above 1 |
| Color not changing | Click color button, draw new stroke |

## System Requirements
- **OS**: Windows 10 (17763+) or Windows 11
- **Graphics**: DirectX12-compatible GPU
- **RAM**: 2GB minimum, 4GB recommended
- **Storage**: 50MB

## Saving Your Work
*Built-in save not yet implemented*

Current options:
1. **Windows Game Bar**: Win+G → Screenshot
2. **Snipping Tool**: Win+Shift+S
3. **Print Screen**: PrtScn → Paste in image editor

## API Quick Reference

### RotoDraw3DRenderer Methods
```cpp
void AddPoint(XMFLOAT2 point);           // Add a point
void ClearPoints();                      // Clear all points
void SetSymmetryCount(int count);        // Set 1-32
void SetBrushSize(float size);           // Set 1-50
void SetBrushColor(XMFLOAT4 color);      // Set RGBA
void SetMirrorMode(bool enabled);        // Toggle mirror
```

### DeviceResources Methods
```cpp
void SetLogicalSize(Size size);          // Window size change
void SetDpi(float dpi);                  // DPI change
void Present();                          // Show frame
void WaitForGpu();                       // Synchronize
```

## Common DirectX12 Objects

| Object | Purpose |
|--------|---------|
| ID3D12Device | Main D3D12 interface |
| ID3D12CommandQueue | Submits commands |
| ID3D12CommandList | Records commands |
| ID3D12Resource | GPU memory resource |
| IDXGISwapChain | Display management |

## Mathematics

### Rotation Formula
```
x' = x * cos(θ) - y * sin(θ)
y' = x * sin(θ) + y * cos(θ)
```
Where: `θ = 2π / symmetryCount * index`

### Mirror Formula
```
x'' = -x'
y'' = y'
```

## Build Configurations

| Config | Use Case | Optimization |
|--------|----------|--------------|
| Debug | Development | None |
| Release | Deployment | Full |

## Useful Visual Studio Shortcuts

| Shortcut | Function |
|----------|----------|
| F5 | Start Debugging |
| Ctrl+F5 | Start Without Debugging |
| Ctrl+Shift+B | Build Solution |
| F7 | Build Project |
| Ctrl+Break | Stop Debugging |

## Project Statistics
- **Language**: C++17/C++20
- **Graphics API**: DirectX12
- **UI Framework**: XAML
- **Platform**: UWP (Universal Windows Platform)
- **Lines of Code**: ~1,500
- **Files**: ~20

## Version Information
- **Project**: Hot3dxBlankApp2WinRTVS2026
- **App Name**: RotoDraw3D (Hot3dxSymmetry3D)
- **Version**: 1.0.0.0
- **Target SDK**: Windows 10 (10.0.17763.0)
- **Visual Studio**: 2022

## Related Technologies
- **C++/WinRT**: Modern C++ projection for Windows Runtime
- **XAML**: Extensible Application Markup Language
- **DirectX12**: Low-level graphics API
- **UWP**: Universal Windows Platform

## Getting More Help
1. Read the complete USER_GUIDE.md
2. Check ARCHITECTURE.md for technical details
3. Review BUILDING.md for setup issues
4. See CONTRIBUTING.md for development info

## Example Code Snippets

### Adding a Point
```cpp
XMFLOAT2 point(100.0f, 200.0f);
m_renderer->AddPoint(point);
```

### Changing Settings
```cpp
m_renderer->SetSymmetryCount(8);
m_renderer->SetBrushSize(10.0f);
m_renderer->SetBrushColor(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f));
m_renderer->SetMirrorMode(true);
```

### Clearing Canvas
```cpp
m_renderer->ClearPoints();
```

## Links & Resources
- Microsoft DirectX12 Docs: https://docs.microsoft.com/windows/win32/direct3d12
- UWP Development: https://docs.microsoft.com/windows/uwp
- C++/WinRT: https://docs.microsoft.com/windows/uwp/cpp-and-winrt-apis

---

**Quick Tip**: For the best experience, start with symmetry=8, mirror mode off, and experiment from there!
