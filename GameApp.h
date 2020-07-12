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
	void CreateRandomTrees();
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	Player m_player;
	std::array<GameObject, 4> m_walls;							// 墙壁
	GameObject m_Trees;										    // 树
	GameObject m_ground;										// 地面
	std::vector<Transform> m_InstancedData;						// 树的实例数据
	
	ImguiPanel m_imguiPanel;

	BasicEffect m_basicEffect;								    // 对象渲染特效管理
	bool m_EnableFrustumCulling;								// 视锥体裁剪开启
	bool m_EnableInstancing;									// 硬件实例化开启
	
	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_cameraMode;									// 摄像机模式

	ObjReader m_objReader;									    // 模型读取对象
};

#endif
