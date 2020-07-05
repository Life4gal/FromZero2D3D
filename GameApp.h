#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Camera.h"
#include "GameObject.h"

class GameApp final : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson };
	
	GameApp(HINSTANCE hInstance);
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
	
	GameObject m_WoodCrate;									    // 木盒
	GameObject m_Floor;										    // 地板
	std::vector<GameObject> m_Walls;							// 墙壁
	GameObject m_Mirror;										// 镜面

	ImguiPanel m_ImguiPanel;
	
	Material m_ShadowMat;									    // 阴影材质
	Material m_WoodCrateMat;									// 木盒材质

	BasicEffect m_BasicEffect;								    // 对象渲染特效管理

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式
};

#endif
