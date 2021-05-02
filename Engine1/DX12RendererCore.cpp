#include "DX12RendererCore.h"

#include "Settings.h"

#include <array>
#include <chrono>

using namespace Engine1;
using namespace Microsoft::WRL;

namespace /*anonymous*/
{
void throwIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw std::exception();
    }
}

void enableDebugLayer()
{
    ComPtr<ID3D12Debug> debugInterface;
    throwIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));

    debugInterface->EnableDebugLayer();
}

ComPtr<IDXGIAdapter4> getAdapter(const bool useWARP)
{
    ComPtr<IDXGIFactory4> factory4;
    UINT createFactoryFlags = 0;
    #if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    #endif

    throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4)));

    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIAdapter4> adapter4;

    if (useWARP)
    {
        throwIfFailed(factory4->EnumWarpAdapter(IID_PPV_ARGS(&adapter1)));
        throwIfFailed(adapter1.As(&adapter4));
    }
    else
    {
        SIZE_T maxDedicatedVideoMemory = 0;
        for (auto i = 0u; factory4->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            adapter1->GetDesc1(&dxgiAdapterDesc1);

            // Check to see if the adapter can create a D3D12 device without actually 
            // creating it. The adapter with the largest dedicated video memory
            // is favored.
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(adapter1.Get(),
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                throwIfFailed(adapter1.As(&adapter4));
            }
        }
    }

    return adapter4;
}

ComPtr<ID3D12Device2> createDevice(const ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> device;
    throwIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

    return device;
}

void configureWarnings(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(device.As(&pInfoQueue)))
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

        // Suppress whole categories of messages
        std::array<D3D12_MESSAGE_CATEGORY, 0> Categories = {};

        // Suppress messages based on their severity level
        std::array<D3D12_MESSAGE_SEVERITY, 1> Severities =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        std::array<D3D12_MESSAGE_ID, 0> DenyIds = {
            //D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            //D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            //D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        NewFilter.DenyList.NumCategories  = static_cast<UINT>(Categories.size());
        NewFilter.DenyList.pCategoryList  = (!Categories.empty() ? &Categories.front() : nullptr);
        NewFilter.DenyList.NumSeverities  = static_cast<UINT>(Severities.size());
        NewFilter.DenyList.pSeverityList  = (!Severities.empty() ? &Severities.front() : nullptr);
        NewFilter.DenyList.NumIDs         = static_cast<UINT>(DenyIds.size());
        NewFilter.DenyList.pIDList        = (!DenyIds.empty() ? &DenyIds.front() : nullptr);

        throwIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }
}

ComPtr<ID3D12CommandQueue> createCommandQueue(
    ComPtr<ID3D12Device2> device,
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandQueue> cmdQueue;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    throwIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue)));

    return cmdQueue;
}

bool checkTearingSupport()
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}

ComPtr<IDXGISwapChain4> createSwapChain(
    HWND hWnd,
    ComPtr<ID3D12CommandQueue> commandQueue,
    uint32_t width,
    uint32_t height,
    uint32_t bufferCount)
{
    ComPtr<IDXGISwapChain4> swapChain4;
    ComPtr<IDXGIFactory4>   factory4;
    UINT createFactoryFlags = 0u;

    #if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    #endif

    throwIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // It is recommended to always allow tearing if tearing support is available.
    swapChainDesc.Flags = checkTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    ComPtr<IDXGISwapChain1> swapChain1;
    throwIfFailed(factory4->CreateSwapChainForHwnd(
        commandQueue.Get(), hWnd,
        &swapChainDesc,
        nullptr, nullptr,
        &swapChain1));

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    throwIfFailed(factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

    throwIfFailed(swapChain1.As(&swapChain4));

    return swapChain4;
}

ComPtr<ID3D12DescriptorHeap> createDescriptorHeap(
    ComPtr<ID3D12Device2> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptors)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    throwIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

WRL::ComPtr<ID3D12Resource> createBackBufferRTV(
    ComPtr<ID3D12Device2> device,
    ComPtr<IDXGISwapChain4> swapChain,
    ComPtr<ID3D12DescriptorHeap> descriptorHeap,
    unsigned int backBufferIdx,
    unsigned int descriptorIdxInHeap)
{
    const auto descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    
    rtvHandle.Offset(descriptorSize * descriptorIdxInHeap);

    ComPtr<ID3D12Resource> backBuffer;
    throwIfFailed(swapChain->GetBuffer(backBufferIdx, IID_PPV_ARGS(&backBuffer)));

    device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

    return backBuffer;
}

ComPtr<ID3D12CommandAllocator> createCommandAllocator(
    ComPtr<ID3D12Device2> device,
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    throwIfFailed(device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator)));

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList> createCommandList(
    ComPtr<ID3D12Device2> device,
    ComPtr<ID3D12CommandAllocator> commandAllocator,
    D3D12_COMMAND_LIST_TYPE type)
{
    ComPtr<ID3D12GraphicsCommandList> commandList;
    throwIfFailed(device->CreateCommandList(0, type, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

    // Before the command list can be reset, it must first be closed.
    // So we close it now as if we had already drew one of the frames.
    throwIfFailed(commandList->Close());

    return commandList;
}

ComPtr<ID3D12Fence> createFence(ComPtr<ID3D12Device2> device)
{
    ComPtr<ID3D12Fence> fence;

    throwIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    return fence;
}

HANDLE createEventHandle()
{
    HANDLE fenceEvent;

    fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent && "Failed to create fence event.");

    return fenceEvent;
}

uint64_t signal(
    ComPtr<ID3D12CommandQueue> commandQueue, 
    ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue)
{
    uint64_t fenceValueForSignal = ++fenceValue;
    throwIfFailed(commandQueue->Signal(fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void waitForFenceValue(
    ComPtr<ID3D12Fence> fence, 
    uint64_t fenceValue, 
    HANDLE fenceEvent,
    std::chrono::milliseconds duration = std::chrono::milliseconds::max())
{
    if (fence->GetCompletedValue() < fenceValue)
    {
        throwIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));
        ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void flush(
    ComPtr<ID3D12CommandQueue> commandQueue, 
    ComPtr<ID3D12Fence> fence,
    uint64_t& fenceValue, 
    HANDLE fenceEvent)
{
    uint64_t fenceValueForSignal = signal(commandQueue, fence, fenceValue);
    waitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}
}

DX12RendererCore::DX12RendererCore()
{
}


DX12RendererCore::~DX12RendererCore()
{
    // Make sure the GPU has finished all commands before closing.
    flush(m_cmdQueue, m_fence, m_fenceValue, m_fenceEvent);

    ::CloseHandle(m_fenceEvent);

}

void DX12RendererCore::initialize(HWND windowHandle)
{
    #if defined(_DEBUG)
    enableDebugLayer();
    #endif

    const auto tearingSupport = checkTearingSupport();

    auto adapter = getAdapter(settings().main.useWARP);

    m_device = createDevice(adapter);

    m_frameObjects.resize(settings().main.backBufferFrameCount);

    #if defined(_DEBUG)
    configureWarnings(m_device);
    #endif

    m_cmdQueue = createCommandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_swapChain = createSwapChain(
        windowHandle, 
        m_cmdQueue, 
        settings().main.screenDimensions.x,
        settings().main.screenDimensions.y,
        settings().main.backBufferFrameCount);

    m_currentBackBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

    m_RTVDescriptorHeap = createDescriptorHeap(
        m_device,
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 
        settings().main.backBufferFrameCount);

    m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (auto backBufferIdx = 0; backBufferIdx < settings().main.backBufferFrameCount; ++backBufferIdx)
    {
        m_frameObjects[backBufferIdx].backBuffer = 
            createBackBufferRTV(
                m_device,
                m_swapChain,
                m_RTVDescriptorHeap,
                backBufferIdx,
                backBufferIdx);

        m_frameObjects[backBufferIdx].cmdAllocator = 
            createCommandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    }

    m_cmdList = createCommandList(
        m_device,
        m_frameObjects[m_currentBackBufferIdx].cmdAllocator, 
        D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_fence      = createFence(m_device);
    m_fenceEvent = createEventHandle();
}

void DX12RendererCore::render(bool tearingSupportedDoSthWithMe)
{
    auto& frameObjects = m_frameObjects[m_currentBackBufferIdx];

    frameObjects.cmdAllocator->Reset();

    m_cmdList->Reset(frameObjects.cmdAllocator.Get(), nullptr);

    // Clear the render target.
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            frameObjects.backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, 
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_cmdList->ResourceBarrier(1, &barrier);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIdx, 
            m_RTVDescriptorSize);

        m_cmdList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // Present
    {
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            frameObjects.backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, 
            D3D12_RESOURCE_STATE_PRESENT);

        m_cmdList->ResourceBarrier(1, &barrier);

        throwIfFailed(m_cmdList->Close());

        std::array<ID3D12CommandList*, 1> cmdLists = {
            m_cmdList.Get()
        };

        m_cmdQueue->ExecuteCommandLists(
            static_cast<UINT>(cmdLists.size()), 
            &cmdLists.front());

        UINT syncInterval = settings().main.verticalSync ? 1u : 0u;
        UINT presentFlags = tearingSupportedDoSthWithMe && !settings().main.verticalSync
            ? DXGI_PRESENT_ALLOW_TEARING 
            : 0;

        throwIfFailed(m_swapChain->Present(syncInterval, presentFlags));

        frameObjects.fenceValue = signal(m_cmdQueue, m_fence, m_fenceValue);

        m_currentBackBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

        // #TODO: Should second param be "fenceValue" or "nextFrameObjects.fenceValue"?
        // they will be different as we have just updated "m_currentBackBufferIdx".
        waitForFenceValue(m_fence, frameObjects.fenceValue, m_fenceEvent);
    }
}

void DX12RendererCore::resizeFrame(uint2 newDimensions)
{
    // Don't allow 0 size swap chain back buffers.
    newDimensions.x = std::max(1u, newDimensions.x);
    newDimensions.y = std::max(1u, newDimensions.y);

    // Flush the GPU queue to make sure the swap chain's back buffers
    // are not being referenced by an in-flight command list.
    flush(m_cmdQueue, m_fence, m_fenceValue, m_fenceEvent);

    const auto& currentFrameObjects = m_frameObjects[m_currentBackBufferIdx];

    for (auto& frameObjects : m_frameObjects)
    {
        // Any references to the back buffers must be released
        // before the swap chain can be resized.
        frameObjects.backBuffer.Reset();
        frameObjects.fenceValue = currentFrameObjects.fenceValue;
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    throwIfFailed(m_swapChain->GetDesc(&swapChainDesc));

    throwIfFailed(m_swapChain->ResizeBuffers(
        static_cast<UINT>(m_frameObjects.size()),
        newDimensions.x, 
        newDimensions.y,
        swapChainDesc.BufferDesc.Format, 
        swapChainDesc.Flags));

    m_currentBackBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

    for (auto backBufferIdx = 0; backBufferIdx < settings().main.backBufferFrameCount; ++backBufferIdx)
    {
        m_frameObjects[backBufferIdx].backBuffer = createBackBufferRTV(
            m_device,
            m_swapChain,
            m_RTVDescriptorHeap,
            backBufferIdx,
            backBufferIdx);
    }
}


