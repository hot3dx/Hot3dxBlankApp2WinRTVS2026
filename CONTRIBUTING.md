# Contributing to RotoDraw3D

Thank you for your interest in contributing to RotoDraw3D! This document provides guidelines and information for contributors.

## Getting Started

### Prerequisites
- Visual Studio 2022 with UWP development tools
- Windows 10 SDK (10.0.17763.0 or later)
- Git for version control
- Basic knowledge of C++17/C++20, DirectX12, and XAML

### Setting Up Development Environment
1. Clone the repository
2. Open `Hot3dxBlankApp2WinRTVS2026.sln` in Visual Studio 2022
3. Restore NuGet packages if prompted
4. Build the solution (Ctrl+Shift+B)
5. Run the application (F5) to verify setup

## Code Style and Standards

### C++ Code Style
- Use C++17/C++20 features where appropriate
- Follow existing indentation (4 spaces, no tabs)
- Use meaningful variable and function names
- Add comments for complex algorithms
- Use `const` and `constexpr` where possible
- Prefer smart pointers over raw pointers

### Example
```cpp
// Good
void RotoDraw3DRenderer::SetSymmetryCount(int count)
{
    m_symmetryCount = std::max(1, std::min(count, 32));
}

// Avoid
void RotoDraw3DRenderer::SetSymmetryCount(int c) {
m_symmetryCount = c > 32 ? 32 : c < 1 ? 1 : c;
}
```

### XAML Style
- Use meaningful element names
- Follow existing naming conventions (PascalCase for events)
- Keep layout hierarchies flat when possible
- Use appropriate margins and padding

### File Organization
- Header files (.h) contain declarations
- Implementation files (.cpp) contain definitions
- Keep related functionality together
- Use forward declarations to reduce dependencies

## Making Changes

### Before You Start
1. Check existing issues for similar requests
2. Open an issue to discuss major changes
3. Fork the repository for your changes

### Development Workflow
1. Create a feature branch: `git checkout -b feature/your-feature-name`
2. Make your changes
3. Test thoroughly
4. Commit with clear messages
5. Push to your fork
6. Open a pull request

### Commit Messages
Use clear, descriptive commit messages:

```
Good:
- "Add color picker for custom brush colors"
- "Fix memory leak in vertex buffer management"
- "Improve symmetry calculation performance"

Avoid:
- "Fixed bug"
- "Updates"
- "Changes"
```

## Types of Contributions

### Bug Fixes
- Describe the bug clearly in the issue
- Include steps to reproduce
- Reference the issue in your commit
- Add tests if applicable

### New Features
- Discuss the feature in an issue first
- Consider backward compatibility
- Update documentation
- Add user-facing features to USER_GUIDE.md

### Performance Improvements
- Benchmark before and after
- Document the improvement
- Consider trade-offs

### Documentation
- Fix typos and errors
- Clarify unclear sections
- Add examples
- Keep diagrams updated

## Feature Ideas

### Easy (Good First Issues)
- Add keyboard shortcuts
- Implement additional color presets
- Add brush shape options (circle, square)
- Implement canvas background color selector

### Medium
- Add undo/redo functionality
- Implement layer support
- Add save/load functionality
- Create brush opacity control
- Add zoom and pan capabilities

### Advanced
- Implement GPU-accelerated symmetry calculation
- Add 3D mode with depth
- Create animation/time-lapse export
- Implement pressure sensitivity for stylus
- Add gradient brush support

## Testing Guidelines

### Manual Testing
Always test your changes:
1. **Build**: Ensure solution compiles without errors
2. **Run**: Launch application and verify it starts
3. **Functionality**: Test the specific feature/fix
4. **Regression**: Check that existing features still work
5. **Edge Cases**: Test boundary conditions

### Test Scenarios
- Different symmetry counts (1, 2, 8, 16)
- Mirror mode on/off
- All color options
- Various brush sizes
- Rapid drawing vs. slow drawing
- Clearing the canvas multiple times
- Drawing after window resize

### Performance Testing
- Monitor frame rate during drawing
- Check memory usage with large drawings
- Test on different hardware if possible

## Code Review Process

### What We Look For
- Code quality and readability
- Proper error handling
- Memory management
- Performance considerations
- Documentation updates
- Consistent style

### Review Checklist
- [ ] Code compiles without warnings
- [ ] Functionality works as intended
- [ ] No obvious performance issues
- [ ] Error handling is appropriate
- [ ] Code follows project style
- [ ] Documentation is updated
- [ ] No memory leaks introduced

## DirectX12 Guidelines

### Resource Management
- Always check HRESULT values
- Use ComPtr for D3D12 objects
- Release resources in reverse order of creation
- Properly synchronize CPU and GPU

### Best Practices
```cpp
// Good: Check for errors
DX::ThrowIfFailed(device->CreateCommandList(...));

// Good: Use ComPtr
Microsoft::WRL::ComPtr<ID3D12Device> m_device;

// Good: Proper cleanup
void Cleanup()
{
    WaitForGpu();  // Synchronize first
    m_resources.Reset();  // Then release
}
```

### Common Pitfalls
- Not waiting for GPU before releasing resources
- Incorrect synchronization with fences
- Forgetting to transition resource states
- Memory leaks from raw pointers

## XAML Guidelines

### Event Handlers
- Use meaningful names: `SymmetrySlider_ValueChanged`
- Keep handlers focused and simple
- Don't perform heavy work on UI thread
- Update UI elements appropriately

### Data Binding
- Use appropriate binding modes
- Consider performance implications
- Keep XAML clean and readable

## Building for Distribution

### Release Configuration
1. Switch to Release configuration
2. Select target architecture (x64 recommended)
3. Enable .NET Native tool chain
4. Build solution
5. Test thoroughly

### Creating Packages
1. Right-click project → Publish → Create App Packages
2. Choose sideload or store
3. Select architectures
4. Configure signing certificate
5. Build package

## Architecture Decisions

### Why DirectX12?
- Modern graphics API
- Better performance than DirectX11
- Educational value for learning modern graphics
- Prepares for future enhancements

### Why Point Primitives?
- Simple to implement
- Efficient for many small draws
- Scalable for future optimizations

### Why Upload Heap?
- Simplifies CPU-GPU transfers
- Acceptable for this use case
- Can be optimized later if needed

## Future Enhancements

### Planned Features
- Undo/redo system
- Save/load drawings
- Export to image files
- Custom color picker
- Keyboard shortcuts
- Brush presets

### Architectural Improvements
- GPU compute shader for symmetry
- Ring buffer for vertex data
- Texture-based rendering for layers
- Spatial data structure for culling

### Performance Optimizations
- Batch draw calls
- Reduce buffer recreations
- Implement level-of-detail
- Add frustum culling

## Getting Help

### Resources
- **DirectX12 Documentation**: https://docs.microsoft.com/en-us/windows/win32/direct3d12/
- **UWP Documentation**: https://docs.microsoft.com/en-us/windows/uwp/
- **C++/WinRT**: https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/

### Asking Questions
- Check documentation first
- Search existing issues
- Provide clear, detailed questions
- Include code snippets when relevant
- Describe what you've tried

## Code of Conduct

### Be Respectful
- Treat everyone with respect
- Welcome newcomers
- Accept constructive criticism
- Focus on the code, not the person

### Be Professional
- Use clear, professional language
- Stay on topic
- Provide constructive feedback
- Help others learn

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

## Recognition

Contributors will be recognized in:
- Git commit history
- Release notes
- Project documentation (if significant contribution)

## Thank You!

Your contributions help make RotoDraw3D better for everyone. Whether it's fixing a typo, adding a feature, or improving performance, every contribution is valued!

For questions or discussions, please open an issue on the repository.
