# RotoDraw3D Component Diagram

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        UWP Application                           │
│                     (Hot3dxBlankApp2WinRTVS2026)                │
└─────────────────────────────────────────────────────────────────┘
                                 │
                ┌────────────────┴────────────────┐
                │                                  │
┌───────────────▼──────────────┐   ┌──────────────▼──────────────┐
│      XAML UI Layer           │   │   DirectX12 Graphics Layer   │
│                              │   │                               │
│  ┌────────────────────────┐ │   │  ┌─────────────────────────┐│
│  │   MainPage.xaml        │ │   │  │   DeviceResources       ││
│  │  - Control Panel       │ │   │  │  - D3D12 Device         ││
│  │  - Sliders & Buttons   │ │   │  │  - Swap Chain           ││
│  │  - SwapChainPanel      │ │   │  │  - Command Queue        ││
│  └───────────┬────────────┘ │   │  │  - Descriptor Heaps     ││
│              │               │   │  └───────────┬─────────────┘│
│  ┌───────────▼────────────┐ │   │              │               │
│  │  MainPage.xaml.cpp     │◄├───┤  ┌───────────▼─────────────┐│
│  │  - Event Handlers      │ │   │  │  RotoDraw3DRenderer     ││
│  │  - Input Processing    │─┼───┼─►│  - Symmetry Engine      ││
│  │  - Render Loop Control │ │   │  │  - Vertex Buffers       ││
│  └────────────────────────┘ │   │  │  - Pipeline State       ││
│                              │   │  │  - Shader Management    ││
└──────────────────────────────┘   │  └─────────────────────────┘│
                                   └──────────────────────────────┘
```

## Data Flow Diagram

```
┌──────────────┐
│ User Input   │
│ (Touch/Mouse)│
└──────┬───────┘
       │
       │ Pointer Events
       │
       ▼
┌─────────────────────────────────┐
│  MainPage Event Handlers        │
│  - PointerPressed()             │
│  - PointerMoved()               │
│  - PointerReleased()            │
└────────────┬────────────────────┘
             │
             │ Screen Coordinates
             │
             ▼
┌─────────────────────────────────┐
│  RotoDraw3DRenderer             │
│  ::AddPoint(x, y)               │
└────────────┬────────────────────┘
             │
             │ Generate Symmetry
             │
             ▼
┌─────────────────────────────────┐
│  ::GenerateSymmetricalPoints()  │
│  - Normalize coordinates        │
│  - Apply rotation matrix        │
│  - Apply mirror transform       │
│  - Return transformed points    │
└────────────┬────────────────────┘
             │
             │ Point Array
             │
             ▼
┌─────────────────────────────────┐
│  ::UpdateVertexBuffer()         │
│  - Create GPU buffer            │
│  - Upload point data            │
│  - Update buffer view           │
└────────────┬────────────────────┘
             │
             │ GPU Buffer Ready
             │
             ▼
┌─────────────────────────────────┐
│  Render Loop (Async)            │
│  - Reset command allocator      │
│  - Record commands              │
│  - Execute command list         │
│  - Present swap chain           │
└────────────┬────────────────────┘
             │
             │ Frame
             │
             ▼
┌─────────────────────────────────┐
│  Display Output                 │
└─────────────────────────────────┘
```

## DirectX12 Pipeline

```
┌─────────────────────────────────────────────────────────┐
│                  DirectX12 Rendering Pipeline            │
└─────────────────────────────────────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        ▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│   Vertex     │  │   Vertex     │  │    Pixel     │
│   Input      │─►│   Shader     │─►│   Shader     │
│  Assembly    │  │              │  │              │
└──────────────┘  └──────────────┘  └──────┬───────┘
                                            │
                                            ▼
                                    ┌──────────────┐
                                    │  Rasterizer  │
                                    │   Stage      │
                                    └──────┬───────┘
                                            │
                                            ▼
                                    ┌──────────────┐
                                    │   Output     │
                                    │   Merger     │
                                    └──────┬───────┘
                                            │
                                            ▼
                                    ┌──────────────┐
                                    │ Render Target│
                                    │  (Back Buffer)│
                                    └──────┬───────┘
                                            │
                                            ▼
                                    ┌──────────────┐
                                    │  Swap Chain  │
                                    │   Present    │
                                    └──────────────┘
```

## Symmetry Algorithm Visualization

```
        Input Point (x, y)
              │
              ▼
     ┌─────────────────┐
     │ Normalize to    │
     │ Center Origin   │
     └────────┬────────┘
              │
    ┌─────────▼─────────┐
    │ For each symmetry │
    │ axis (0 to n-1)   │
    └─────────┬─────────┘
              │
    ┌─────────▼──────────┐
    │ angle = 2π/n * i   │
    └─────────┬──────────┘
              │
    ┌─────────▼──────────┐
    │ Rotation Matrix:   │
    │ x' = x·cos - y·sin │
    │ y' = x·sin + y·cos │
    └─────────┬──────────┘
              │
              ├──► Add Rotated Point
              │
    ┌─────────▼──────────┐
    │ If Mirror Mode:    │
    │ x'' = -x'          │
    │ y'' = y'           │
    └─────────┬──────────┘
              │
              └──► Add Mirrored Point

    Result: N points (or 2N if mirrored)
```

## Class Relationships

```
┌──────────────────────┐
│       App            │
│  (Application)       │
└──────┬───────────────┘
       │ owns
       │
       ▼
┌──────────────────────┐
│     MainPage         │
│   (UI Control)       │
└──────┬───┬───────────┘
       │   │ owns
       │   │
       │   ▼
       │ ┌──────────────────────┐
       │ │  DeviceResources     │
       │ │  (D3D12 Management)  │
       │ └──────────────────────┘
       │
       │ owns
       ▼
┌──────────────────────┐
│ RotoDraw3DRenderer   │
│  (Rendering Logic)   │
└──────┬───────────────┘
       │ uses
       │
       ▼
┌──────────────────────┐
│  DeviceResources     │
│  (shared_ptr)        │
└──────────────────────┘
```

## File Organization

```
Hot3dxBlankApp2WinRTVS2026/
│
├── Application Layer
│   ├── App.xaml.h/cpp          (Application lifecycle)
│   └── MainPage.xaml.h/cpp     (Main UI page)
│
├── Rendering Layer  
│   ├── DeviceResources.h/cpp   (DirectX12 device mgmt)
│   └── RotoDraw3DRenderer.h/cpp (Symmetry engine)
│
├── Support Files
│   ├── pch.h/cpp               (Precompiled headers)
│   ├── d3dx12.h                (D3D12 helpers)
│   └── Package.appxmanifest    (App manifest)
│
├── Assets/
│   └── *.png                   (App icons & logos)
│
└── Documentation
    ├── README.md               (Project overview)
    ├── BUILDING.md             (Build instructions)
    ├── ARCHITECTURE.md         (Technical docs)
    ├── USER_GUIDE.md           (User manual)
    └── DIAGRAMS.md             (This file)
```

## Memory Management

```
┌─────────────────────────────────────────────┐
│            CPU Memory                        │
│                                             │
│  ┌─────────────────────────────────────┐   │
│  │  std::vector<DrawPoint> m_points    │   │
│  │  - Stores all drawn points          │   │
│  │  - Grows dynamically                │   │
│  └─────────────┬───────────────────────┘   │
│                │ Copy to GPU                │
└────────────────┼───────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────┐
│            GPU Memory                        │
│                                             │
│  ┌─────────────────────────────────────┐   │
│  │  ID3D12Resource (Vertex Buffer)     │   │
│  │  - Upload heap                       │   │
│  │  - Contains point positions & colors│   │
│  └─────────────────────────────────────┘   │
│                                             │
│  ┌─────────────────────────────────────┐   │
│  │  Swap Chain Buffers (2x)            │   │
│  │  - Back buffer (rendering)           │   │
│  │  - Front buffer (display)            │   │
│  └─────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

## Threading Model

```
┌────────────────────┐         ┌────────────────────┐
│   UI Thread        │         │  Render Thread     │
│   (Main Thread)    │         │  (ThreadPool)      │
└─────────┬──────────┘         └────────┬───────────┘
          │                               │
          │ Input Events                  │
          ├──────────────────────────────►│
          │                               │
          │                               │ Continuous Loop
          │                               ├────────┐
          │                               │        │
          │                               ├◄───────┘
          │                               │
          │                               │ Update()
          │                               │ Render()
          │                               │ Present()
          │                               │
          │                               │
          │◄──────────────────────────────┤
          │ Frame Completed                │
          │                               │
          
     Sync Point: Fence & Event
```

## UI Layout

```
┌────────────────────────────────────────────────────────────┐
│  RotoDraw3D (Hot3dxSymmetry3D)                            │
├────────────────────────────────────────────────────────────┤
│  Symmetry: [====●====] 8    Brush: [===●====]             │
│  [Clear] ☑ Mirror    ●●●●● (color buttons)                │
├────────────────────────────────────────────────────────────┤
│                                                            │
│                                                            │
│                    Drawing Canvas                          │
│                  (SwapChainPanel)                          │
│                                                            │
│               DirectX12 Rendering Output                   │
│                                                            │
│                                                            │
└────────────────────────────────────────────────────────────┘
```

## Build Process

```
┌─────────────────┐
│  Source Files   │
│  (.cpp, .h)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  C++ Compiler   │
│  (MSVC 2022)    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Object Files   │
│  (.obj)         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    Linker       │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Executable     │
│  (.exe)         │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  App Packager   │
│  (MakeAppx)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  UWP Package    │
│  (.appx)        │
└─────────────────┘
```
