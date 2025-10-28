# RotoDraw3D Architecture

## Overview
RotoDraw3D is a modern UWP application that combines XAML UI with DirectX12 rendering to create an interactive symmetrical drawing experience.

## Architecture Layers

### 1. Presentation Layer (XAML)
**Files**: `MainPage.xaml`, `MainPage.xaml.h/cpp`

- Provides user interface controls
- Handles user input events (pointer pressed/moved/released)
- Manages UI state (sliders, buttons, checkboxes)
- Hosts the SwapChainPanel for DirectX rendering

**Key Components**:
- Control panel with symmetry, brush size, and color controls
- SwapChainPanel for DirectX12 output
- Event handlers for user interactions

### 2. Application Layer
**Files**: `App.xaml`, `App.xaml.h/cpp`

- Application lifecycle management
- Window activation and suspension
- Navigation between pages
- Exception handling

### 3. Rendering Layer (DirectX12)
**Files**: `DeviceResources.h/cpp`, `RotoDraw3DRenderer.h/cpp`

#### DeviceResources
Manages DirectX12 device and resources:
- **Device Creation**: Initializes D3D12 device and factory
- **Swap Chain Management**: Creates and manages the swap chain for presenting frames
- **Command Infrastructure**: Command queue, allocators, and lists
- **Descriptor Heaps**: RTV (Render Target View) heap for rendering
- **Synchronization**: Fences and events for GPU synchronization
- **Window Size Management**: Handles DPI and size changes

#### RotoDraw3DRenderer
Implements the drawing engine:
- **Symmetry Algorithm**: Generates rotational and mirror symmetry
- **Vertex Management**: Creates and updates vertex buffers for points
- **Pipeline State**: Graphics pipeline with shaders
- **Rendering**: Draws points to the back buffer

### 4. Helper Layer
**Files**: `d3dx12.h`, `pch.h`

- **d3dx12.h**: DirectX12 helper structures and classes
- **pch.h**: Precompiled headers for faster compilation

## Data Flow

```
User Input (Touch/Mouse)
    ↓
MainPage Event Handler
    ↓
RotoDraw3DRenderer::AddPoint()
    ↓
GenerateSymmetricalPoints()
    ↓
UpdateVertexBuffer()
    ↓
Render Loop
    ↓
DirectX12 Command List
    ↓
GPU Rendering
    ↓
Swap Chain Present
    ↓
Display Output
```

## Key Algorithms

### Symmetry Generation
```cpp
For each input point:
1. Convert screen coords to normalized space [0,1]
2. Translate to center-origin coordinate system
3. For each symmetry axis:
   a. Calculate rotation angle = 2π / symmetryCount * axisIndex
   b. Apply rotation matrix:
      x' = x * cos(angle) - y * sin(angle)
      y' = x * sin(angle) + y * cos(angle)
   c. If mirror mode:
      - Create mirrored point by negating x coordinate
   d. Translate back to [0,1] space
4. Add all transformed points to vertex buffer
```

### DirectX12 Rendering Pipeline
```
1. Command Allocator Reset
2. Command List Reset
3. Set Root Signature
4. Set Viewport and Scissor Rect
5. Transition Resource (PRESENT → RENDER_TARGET)
6. Clear Render Target
7. Set Render Targets
8. Set Pipeline State
9. Set Vertex Buffer
10. Draw Call
11. Transition Resource (RENDER_TARGET → PRESENT)
12. Close Command List
13. Execute Command List
14. Present Swap Chain
15. Signal Fence
16. Move to Next Frame
```

## Threading Model

### UI Thread
- Handles XAML events
- Updates UI controls
- Calls into renderer for drawing operations

### Render Thread
- Dedicated thread pool worker for rendering
- Runs continuously in a loop
- Updates and renders at display refresh rate
- Synchronized with GPU via fences

### Synchronization
- **Fences**: GPU-CPU synchronization for frame completion
- **Events**: Win32 events for fence signaling
- **Double Buffering**: Two frame buffers for smooth rendering

## Memory Management

### GPU Resources
- **Vertex Buffers**: Upload heap, recreated on each update
- **Render Targets**: Created from swap chain buffers
- **Descriptor Heaps**: RTV heap for render target views

### CPU Resources
- **Smart Pointers**: ComPtr for COM objects, std::shared_ptr/unique_ptr for C++ objects
- **RAII**: Automatic cleanup on destruction
- **Vector**: std::vector for dynamic point storage

## Performance Considerations

### Optimizations
1. **Precompiled Headers**: Reduces compilation time
2. **Upload Heap**: Direct CPU-GPU data transfer
3. **Point Primitives**: Efficient for many small draw calls
4. **Double Buffering**: Prevents tearing and allows async work

### Bottlenecks
1. **Vertex Buffer Updates**: Recreated on every point add (could be optimized with ring buffer)
2. **CPU Symmetry Calculation**: Done per-point (could be GPU compute shader)
3. **Draw Calls**: One per frame (efficient for this use case)

## Extensibility Points

### Adding New Features
1. **New Brush Types**: Extend vertex structure, modify shaders
2. **Layers**: Add multiple vertex buffers and render in sequence
3. **Undo/Redo**: Store point history in memory
4. **Export**: Add texture render target and save to file
5. **3D Mode**: Extend to 3D coordinates and add depth buffer

### Customization
1. **Colors**: Modify brush color palette in MainPage.xaml
2. **Symmetry Range**: Change min/max in slider definition
3. **Background**: Modify clear color in RotoDraw3DRenderer::Render()
4. **Shaders**: Edit inline HLSL in RotoDraw3DRenderer::CreateDeviceDependentResources()

## Error Handling

### Strategy
- **HRESULT Checking**: All DirectX calls checked with DX::ThrowIfFailed()
- **Exceptions**: COM exceptions propagated to app level
- **Debug Layer**: Enabled in debug builds for validation
- **Device Removal**: Detected and could trigger resource recreation

### Common Errors
1. **DXGI_ERROR_DEVICE_REMOVED**: GPU crash or driver issue
2. **E_OUTOFMEMORY**: GPU memory exhausted
3. **Invalid Handle**: Usually a synchronization issue with fences
