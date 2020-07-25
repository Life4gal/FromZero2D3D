#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include "Collision.h"
#include "NormalTank.h"

class Player
{
	// 允许IMGUI面板自由访问数据
	friend class ImguiPanel;
	
public:
	struct VehicleInfo;

	explicit Player();

	void Init(ID3D11Device* device);
	
	static Player& Get();
	
	void Walk(float d);
	void Strafe(float d);
	Ray Shoot() const;
	// 转动炮管,大于0向右转
	void Turn(float d);

	DirectX::XMFLOAT3 GetPosition() const;
	void SetPosition(const DirectX::XMFLOAT3& position);
	
	void XM_CALLCONV AdjustPosition(DirectX::FXMVECTOR minCoordinate, DirectX::FXMVECTOR maxCoordinate);

	// 绘制
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);

private:

	NormalTank m_tank;
};

#endif
