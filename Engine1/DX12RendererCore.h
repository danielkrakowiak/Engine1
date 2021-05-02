#pragma once

#include <exception>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h>

// D3D12 extension library.
#include "d3dx12.h"

#include "uint2.h"

namespace WRL = Microsoft::WRL;

namespace Engine1
{
    class DX12RendererCore
    {

    public:

        DX12RendererCore();
        ~DX12RendererCore();

        void initialize(HWND windowHandle);

        void render(bool tearingSupportedDoSthWithMe);

        void resizeFrame(uint2 newDimensions);

    private:

        WRL::ComPtr<ID3D12Device2>             m_device;
        WRL::ComPtr<ID3D12CommandQueue>        m_cmdQueue;
        WRL::ComPtr<IDXGISwapChain4>           m_swapChain;
        WRL::ComPtr<ID3D12DescriptorHeap>      m_RTVDescriptorHeap;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;

        struct FrameObjects
        {
            WRL::ComPtr<ID3D12Resource>         backBuffer;
            WRL::ComPtr<ID3D12CommandAllocator> cmdAllocator;
            uint64_t                            fenceValue;
        };

        // 2 objects for double buffering, 3 for triple buffering.
        std::vector< FrameObjects > m_frameObjects;
        
        unsigned int m_RTVDescriptorSize = 0;

        unsigned int m_currentBackBufferIdx = 0;

        // Synchronization objects.
        WRL::ComPtr<ID3D12Fence> m_fence;
        uint64_t m_fenceValue = 0;
        HANDLE m_fenceEvent;
    };
}
