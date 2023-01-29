#include"Input.h"
#include<cassert>
//#include<wrl.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


//using namespace Microsoft::WRL;
void Input::Initialize(WinApp* winApp)
{
    HRESULT result;
    //�؂�Ă���winApp�̃C���X�^���X���L�^
    this->winApp = winApp;




    // DirectInput�̏�����
    result = DirectInput8Create(
        this->winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));



   
    result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
    // ���̓f�[�^�`���̃Z�b�g
    result = keyboard->SetDataFormat(&c_dfDIKeyboard); // �W���`��
    assert(SUCCEEDED(result));
    // �r�����䃌�x���̃Z�b�g
    result = keyboard->SetCooperativeLevel(
        this->winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));
}

void Input::Update()
{
    HRESULT result;

    memcpy(keyPre, key, sizeof(key));
    // �L�[�{�[�h���̎擾�J�n
    result = keyboard->Acquire();
    // �S�L�[�̓��͏�Ԃ��擾����
    result = keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE KeyNumber)
{
    if (key [KeyNumber])
    {
        return true;
    }
    return false;
}

bool Input::TriggerKey(BYTE KeyNumber)
{
    if (key[KeyNumber]&&!keyPre[KeyNumber])
    {
        return true;
    }
    return false;
}
