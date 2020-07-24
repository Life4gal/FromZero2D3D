#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

#include "Camera.h"
#include "GameObject.h"
#include "Effect.h"
#include "Render.h"

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

	void DrawSceneObject(bool drawCenterSphere);
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	Player m_player;											// 玩家
	
	GameObject m_ground;										// 地面
	GameObject m_cylinder;									    // 圆柱体
	
	GameObject m_sphere;										// 球
	float m_sphereRad;											// 球体旋转弧度

	ComPtr<ID3D11ShaderResourceView> m_bricksNormalMap;		    // 砖块法线贴图
	ComPtr<ID3D11ShaderResourceView> m_stonesNormalMap;		    // 地面法线贴图
	
	BasicEffect m_basicEffect;								    // 对象渲染特效管理
	SkyEffect m_skyEffect;									    // 天空盒特效管理
	
	std::unique_ptr<DynamicSkyRender> m_pDaylight;				// 天空盒(白天)
	
	std::shared_ptr<Camera> m_pCamera;						    // 摄像机

	CameraMode m_cameraMode;									// 摄像机模式

	ImguiPanel m_imguiPanel;
};

#endif
