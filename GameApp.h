#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

#include "Camera.h"
#include "GameObject.h"
#include "ObjReader.h"
#include "Collision.h"

#include "ImguiPanel.h"

class GameApp final : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson };
	
	explicit GameApp(HINSTANCE hInstance);
	~GameApp();

	GameApp(const GameApp&) = delete;
	GameApp(const GameApp&&) = delete;
	GameApp& operator=(GameApp&) = delete;
	GameApp& operator=(GameApp&&) = delete;

	bool Init() override;
	void OnResize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

private:
	bool InitResource();
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	//Player m_player;
	Tank m_player;
	std::array<GameObject, 4> m_walls;							// 墙壁
	GameObject m_ground;										// 地面

	GameObject m_sphere;										// 球
	GameObject m_cube;										    // 立方体
	GameObject m_cylinder;									    // 圆柱体
	GameObject m_house;										    // 房屋

	DirectX::BoundingSphere m_boundingSphere;				    // 球的包围盒
	Geometry::MeshData<> m_triangleMesh;						// 三角形网格模型
	std::wstring m_pickedObjStr;								// 已经拾取的对象名
	
	ImguiPanel m_imguiPanel;

	BasicEffect m_basicEffect;								    // 对象渲染特效管理
	
	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_cameraMode;									// 摄像机模式

	ObjReader m_objReader;									    // 模型读取对象
};

#endif
