#include"DirectXCommon.h"

#include <cassert>
#include <thread>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp)
{
    //NULL���o
    assert(winApp);
    //�����o�ϐ��ɋL�^
    this->winApp = winApp;

    //FPS�Œ菉����
    InitializeFixFPS();
    
    /// <summary>
    /// �f�o�C�X������
    /// </summary>
    InitializeDevice();
    /// <summary>
    /// �R�}���h�֘A�̏�����
    /// </summary>
    InitializeCommon();
    /// <summary>
    /// �X���b�v�`�F�[���̏�����
    /// </summary>
    InitializeSwapchain();
    /// <summary>
    /// �����_�[�^�[�Q�b�g�r���[�̏�����
    /// </summary>
    InitializeRenderTargetView();
    /// <summary>
    /// �[�x�o�b�t�@�̏�����
    /// </summary>
    InitializeDepthBuffer();
    /// <summary>
    /// �t�F���X�̏�����
    /// </summary>
    InitializeFence();

    

    // DirectX�����������@��������
    HRESULT result;
   

   

  

    

    // DirectX�����������@�����܂�
}

void DirectXCommon::PreDraw()
{
    // �o�b�N�o�b�t�@�̔ԍ����擾�i2�Ȃ̂�0�Ԃ�1�ԁj
    UINT bbIndex = swapChain->GetCurrentBackBufferIndex();

    // �P�D���\�[�X�o���A�ŏ������݉\�ɕύX
    D3D12_RESOURCE_BARRIER barrierDesc{};
    barrierDesc.Transition.pResource = backBuffers[bbIndex].Get(); // �o�b�N�o�b�t�@���w��
    barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;      // �\����Ԃ���
    barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`���Ԃ�
    commandList->ResourceBarrier(1, &barrierDesc);

    // �Q�D�`���̕ύX
    // �����_�[�^�[�Q�b�g�r���[�̃n���h�����擾
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += bbIndex * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // �[�x�X�e���V���r���[�p�f�X�N���v�^�q�[�v�̃n���h�����擾
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    // �R�D��ʃN���A           R     G     B    A
    FLOAT clearColor[] = { 0.1f,0.25f, 0.5f,0.0f }; // ���ۂ��F
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // �S�D�`��R�}���h��������
    // �r���[�|�[�g�ݒ�R�}���h
    D3D12_VIEWPORT viewport{};
    viewport.Width = WinApp::window_width;
    viewport.Height = WinApp::window_height;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    // �r���[�|�[�g�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
    commandList->RSSetViewports(1, &viewport);

    // �V�U�[��`
    D3D12_RECT scissorRect{};
    scissorRect.left = 0;                                       // �؂蔲�����W��
    scissorRect.right = scissorRect.left + WinApp::window_width;        // �؂蔲�����W�E
    scissorRect.top = 0;                                        // �؂蔲�����W��
    scissorRect.bottom = scissorRect.top + WinApp::window_height;       // �؂蔲�����W��
    // �V�U�[��`�ݒ�R�}���h���A�R�}���h���X�g�ɐς�
    commandList->RSSetScissorRects(1, &scissorRect);
}

void DirectXCommon::PostDraw()
{
    HRESULT result;
    // �o�b�N�o�b�t�@�̔ԍ����擾�i2�Ȃ̂�0�Ԃ�1�ԁj
    UINT bbIndex = swapChain->GetCurrentBackBufferIndex();
    // �P�D���\�[�X�o���A�ŏ������݉\�ɕύX
    D3D12_RESOURCE_BARRIER barrierDesc{};
    barrierDesc.Transition.pResource = backBuffers[bbIndex].Get(); // �o�b�N�o�b�t�@���w��

    // �T�D���\�[�X�o���A��߂�
    barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET; // �`���Ԃ���
    barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;        // �\����Ԃ�
    commandList->ResourceBarrier(1, &barrierDesc);

    // ���߂̃N���[�Y
    result = commandList->Close();
    assert(SUCCEEDED(result));
    // �R�}���h���X�g�̎��s
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(1, commandLists);

    // ��ʂɕ\������o�b�t�@���t���b�v�i���\�̓��ւ��j
    result = swapChain->Present(1, 0);
    assert(SUCCEEDED(result));

    // �R�}���h�̎��s������҂�
    commandQueue->Signal(fence.Get(), ++fenceVal);
    if (fence->GetCompletedValue() != fenceVal) {
        HANDLE event = CreateEvent(nullptr, false, false, nullptr);
        fence->SetEventOnCompletion(fenceVal, event);
        WaitForSingleObject(event, INFINITE);
        CloseHandle(event);
    }

    //FPS�Œ�X�V   
    UpdateFixFPS();

    // �L���[���N���A
    result = commandAllocator->Reset();
    assert(SUCCEEDED(result));
    // �ĂуR�}���h���X�g�𒙂߂鏀��
    result = commandList->Reset(commandAllocator.Get(), nullptr);
    assert(SUCCEEDED(result));

}

void DirectXCommon::InitializeDevice()
{
    HRESULT result;

#ifdef _DEBUG
    //�f�o�b�O���C���[���I����
    ComPtr<ID3D12Debug1> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
    {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    // DXGI�t�@�N�g���[�̐���
    result = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
    assert(SUCCEEDED(result));

    // �A�_�v�^�[�̗񋓗p
    std::vector<ComPtr<IDXGIAdapter4>> adapters;
    // �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
    ComPtr<IDXGIAdapter4> tmpAdapter;

    // �p�t�H�[�}���X���������̂��珇�ɁA�S�ẴA�_�v�^�[��񋓂���
    for (UINT i = 0;
        dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) != DXGI_ERROR_NOT_FOUND;
        i++) {
        // ���I�z��ɒǉ�����
        adapters.push_back(tmpAdapter);
    }

    // �Ó��ȃA�_�v�^��I�ʂ���
    for (size_t i = 0; i < adapters.size(); i++) {
        DXGI_ADAPTER_DESC3 adapterDesc;
        // �A�_�v�^�[�̏����擾����
        adapters[i]->GetDesc3(&adapterDesc);

        // �\�t�g�E�F�A�f�o�C�X�����
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            // �f�o�C�X���̗p���ă��[�v�𔲂���
            tmpAdapter = adapters[i];
            break;
        }
    }

    // �Ή����x���̔z��
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL featureLevel;

    for (size_t i = 0; i < _countof(levels); i++) {
        // �̗p�����A�_�v�^�[�Ńf�o�C�X�𐶐�
        result = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&device));
        if (result == S_OK) {
            // �f�o�C�X�𐶐��ł������_�Ń��[�v�𔲂���
            featureLevel = levels[i];
            break;
        }
    }

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        // �}������G���[
        D3D12_MESSAGE_ID denyIds[] = {
            /*
             * Windows11�ł�DXGI�f�o�b�O���C���[��DX12�f�o�b�O���C���[�̑��ݍ�p�o�O�ɂ��G���[���b�Z�[�W
             * https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
             */
            D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE };
        // �}������\�����x��
        D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
        D3D12_INFO_QUEUE_FILTER filter{};
        filter.DenyList.NumIDs = _countof(denyIds);
        filter.DenyList.pIDList = denyIds;
        filter.DenyList.NumSeverities = _countof(severities);
        filter.DenyList.pSeverityList = severities;
        // �w�肵���G���[�̕\����}������
        infoQueue->PushStorageFilter(&filter);
        // �G���[���Ƀu���[�N�𔭐�������
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    }
#endif

}

void DirectXCommon::InitializeCommon()
{

    HRESULT result;
    // �R�}���h�A���P�[�^�𐶐�
    result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    assert(SUCCEEDED(result));

    // �R�}���h���X�g�𐶐�
    result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    assert(SUCCEEDED(result));

    //�R�}���h�L���[�̐ݒ�
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    //�R�}���h�L���[�𐶐�
    result = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
    assert(SUCCEEDED(result));

}

void DirectXCommon::InitializeSwapchain()
{
    HRESULT result;
    // �X���b�v�`�F�[���̐ݒ�
    
    swapChainDesc.Width = 1280;
    swapChainDesc.Height = 720;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // �F���̏���
    swapChainDesc.SampleDesc.Count = 1; // �}���`�T���v�����Ȃ�
    swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // �o�b�N�o�b�t�@�p
    swapChainDesc.BufferCount = 2;  // �o�b�t�@�����Q�ɐݒ�
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �t���b�v��͔j��
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    ComPtr<IDXGISwapChain1> swapChain1;
    // �X���b�v�`�F�[���̐���
    result = dxgiFactory->CreateSwapChainForHwnd(
        commandQueue.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain1);
    assert(SUCCEEDED(result));

    // SwapChain4�𓾂�
    swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
    assert(SUCCEEDED(result));
}

void DirectXCommon::InitializeRenderTargetView()
{
    // �f�X�N���v�^�q�[�v�̐ݒ�
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // �����_�[�^�[�Q�b�g�r���[
    rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;    // ���\�̂Q��
    // �f�X�N���v�^�q�[�v�̐���
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

    // �o�b�N�o�b�t�@

    backBuffers.resize(swapChainDesc.BufferCount);

    // �X���b�v�`�F�[���̑S�Ẵo�b�t�@�ɂ��ď�������
    for (size_t i = 0; i < backBuffers.size(); i++) {
        // �X���b�v�`�F�[������o�b�t�@���擾
        swapChain->GetBuffer((UINT)i, IID_PPV_ARGS(&backBuffers[i]));
        // �f�X�N���v�^�q�[�v�̃n���h�����擾
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        // �����\���ŃA�h���X�������
        rtvHandle.ptr += i * device->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
        // �����_�[�^�[�Q�b�g�r���[�̐ݒ�
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        // �V�F�[�_�[�̌v�Z���ʂ�SRGB�ɕϊ����ď�������
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        // �����_�[�^�[�Q�b�g�r���[�̐���
        device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);
    }
}

void DirectXCommon::InitializeDepthBuffer()
{
    HRESULT result;
    // ���\�[�X�ݒ�
    D3D12_RESOURCE_DESC depthResourceDesc{};
    depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResourceDesc.Width = WinApp::window_width; // �����_�[�^�[�Q�b�g�ɍ��킹��
    depthResourceDesc.Height = WinApp::window_height; // �����_�[�^�[�Q�b�g�ɍ��킹��
    depthResourceDesc.DepthOrArraySize = 1;
    depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT; // �[�x�l�t�H�[�}�b�g
    depthResourceDesc.SampleDesc.Count = 1;
    depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // �f�v�X�X�e���V��

    // �[�x�l�p�q�[�v�v���p�e�B
    D3D12_HEAP_PROPERTIES depthHeapProp{};
    depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
    // �[�x�l�̃N���A�ݒ�
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f; // �[�x�l1.0f�i�ő�l�j�ŃN���A
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // �[�x�l�t�H�[�}�b�g
    // ���\�[�X����
   
    result = device->CreateCommittedResource(
        &depthHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &depthResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, // �[�x�l�������݂Ɏg�p
        &depthClearValue,
        IID_PPV_ARGS(&depthBuff));

    // �[�x�r���[�p�f�X�N���v�^�q�[�v�쐬
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.NumDescriptors = 1; // �[�x�r���[��1��
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // �f�v�X�X�e���V���r���[
    result = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));

    // �[�x�r���[�쐬
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // �[�x�l�t�H�[�}�b�g
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    device->CreateDepthStencilView(
        depthBuff.Get(),
        &dsvDesc,
        dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::InitializeFence()
{
    HRESULT result;
    // �t�F���X�̐���
   
  

    result = device->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    assert(SUCCEEDED(result));
}

void DirectXCommon::InitializeFixFPS()
{
    //���݂̎��Ԃ��L�^����
    reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS()
{
    //�@1/60�b�҂�����̎���
    const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
    // 1/60�b���킸���ɒZ������
    const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

    //���ݎ��Ԃ��擾����
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    //�O��L�^����̌o�ߎ��Ԃ��擾����
    std::chrono::microseconds elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

    // 1/60�b(���킸���ɒZ������)�o���Ă��Ȃ��ꍇ
    if (elapsed < kMinCheckTime) {
        // 1/60�b�o�߂���܂Ŕ����ȃX���[�v���J��Ԃ�
        while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
            // 1�}�C�N���b�X���[�v
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    //���݂̎��Ԃ��L�^����
    reference_ = std::chrono::steady_clock::now();
}
