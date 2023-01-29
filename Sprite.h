#pragma once
#include<DirectXMath.h>
#include "SpriteCommon.h"


//�X�v���C�g
class Sprite
{
public://�����o�֐�
	//������
	void Initialize(SpriteCommon*_spriteCommon);

	void Draw();
private:
	SpriteCommon* spriteCommon = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertBuff;

	D3D12_VERTEX_BUFFER_VIEW vbView{};

};

