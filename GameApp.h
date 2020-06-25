#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "LightHelper.h"
#include "Geometry.h"
#include "Camera.h"

class GameApp final : public D3DApp
{
public:
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

	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT4 eyePos;
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely
	{
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		Material material;
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;		// 打包保证16字节对齐
	};

	// 一个尽可能小的游戏对象类
	class GameObject
	{
	public:
		GameObject();

		// 获取物体变换
		Transform& GetTransform();
		// 获取物体变换
		const Transform& GetTransform() const;

		// 平移
		void Strafe(float d);
		// 直行(平面移动)
		void Walk(float d);

		// 设置缓冲区
		template<class VertexType, class IndexType>
		void SetBuffer(ID3D11Device* device, const Geometry::MeshData<VertexType, IndexType>& meshData);
		// 设置纹理
		void SetTexture(ID3D11ShaderResourceView* texture);

		// 绘制
		void Draw(ID3D11DeviceContext* deviceContext);

		// 设置调试对象名
		// 若缓冲区被重新设置，调试对象名也需要被重新设置
		void SetDebugObjectName(const std::string& name) const;
	private:
		Transform m_Transform;								// 物体变换信息
		ComPtr<ID3D11ShaderResourceView> m_pTexture;		// 纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;				// 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 索引缓冲区
		UINT m_VertexStride;								// 顶点字节大小
		UINT m_IndexCount;								    // 索引数目	
	};

	// IMGUI
	class ImguiPanel
	{
	public:
		// 渲染 IMGUI 面板
		void Draw() const;
		// 绘制 IMGUI 面板,必须在 其他2D/3D部分渲染完毕 之后且在 SwapChain->Present 前调用,不然不会显示面板
		static void Present();

		DirectX::XMFLOAT3 up{};
		DirectX::XMFLOAT3 right{};
		DirectX::XMFLOAT3 look{};
	};

	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson };

private:
	bool InitEffect();
	bool InitResource();

	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;	    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;					// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;		// 文本格式
	
	ComPtr<ID3D11InputLayout> m_pVertexLayout2D;	// 用于2D的顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;	// 用于3D的顶点输入布局
	ComPtr<ID3D11Buffer> m_pConstantBuffers[4];		    // 常量缓冲区

	GameObject m_WoodCrate;									    // 木盒
	GameObject m_Floor;										    // 地板
	std::vector<GameObject> m_Walls;							// 墙
	ImguiPanel m_imgui_panel;									// 面板

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;				    // 用于3D的像素着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;				// 用于2D的顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;				    // 用于2D的像素着色器

	CBChangesEveryFrame m_CBFrame;							    // 该缓冲区存放仅在每一帧进行更新的变量
	CBChangesOnResize m_CBOnResize;							    // 该缓冲区存放仅在窗口大小变化时更新的变量
	CBChangesRarely m_CBRarely;								    // 该缓冲区存放不会再进行修改的变量

	ComPtr<ID3D11SamplerState> m_pSamplerState;				    // 采样器状态

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式
};

#endif
