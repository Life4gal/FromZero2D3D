#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"

#include "Camera.h"
#include "Player.h"

#include "Effect.h"
#include "Render.h"

class GameApp final : public D3DApp
{
public:
	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
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
	void DrawScene(BasicEffect* pBasicEffect);
	void DrawScene(ShadowEffect* pShadowEffect);

private:
	bool InitResource();

	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	bool m_enableDebug;											// 开启调试模式
	bool m_grayMode;											// 深度值以灰度形式显示
	size_t m_slopeIndex;										// 斜率索引
	
	Player m_player;											// 玩家
	
	GameObject m_ground;										// 地面
	
	GameObject m_cylinder;									    // 圆柱体
	std::vector<BasicTransform> m_cylinderTransforms;			// 圆柱体变换信息
	GameObject m_sphere;										// 球
	std::vector<BasicTransform> m_sphereTransforms;				// 球体变换信息

	GameObject m_debugQuad;										// 调试用四边形

	DirectionalLight m_dirLights[3];						// 方向光
	DirectX::XMFLOAT3 m_originalLightDirs[3];				// 初始光方向
	
	std::unique_ptr<BasicEffect> m_pBasicEffect;				// 基础特效
	std::unique_ptr<ShadowEffect> m_pShadowEffect;				// 阴影特效
	std::unique_ptr<SkyEffect> m_pSkyEffect;					// 天空盒特效
	std::unique_ptr<DebugEffect> m_pDebugEffect;				// 调试用显示纹理的特效

	std::unique_ptr<TextureRender> m_pShadowMap;				// 阴影贴图
	std::unique_ptr<SkyRender> m_pDaylight;						// 天空盒(白天)
	
	std::shared_ptr<Camera> m_pCamera;						    // 摄像机

	CameraMode m_cameraMode;									// 摄像机模式
};

#endif
