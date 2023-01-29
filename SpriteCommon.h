#pragma once
#include <wrl.h>
#include <d3d12.h>


#include"DirectXCommon.h"

//�X�v���C�g���ʕ�
class SpriteCommon
{
public://�����o�֐�
	//������
	void Initialize(DirectXCommon*_dxCommon);
	//�`��
	void PreDraw();
	//Getter
	DirectXCommon* GetDirectXCommon() { return dxCommon; }
private:
	DirectXCommon* dxCommon;

	Microsoft::WRL::ComPtr<ID3D12PipelineState>pipelineState;

	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature;

};

