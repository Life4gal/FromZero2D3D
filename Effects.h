//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
// 
// 简易特效管理框架
// Simple effect management framework.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include <memory>
#include "LightHelper.h"
#include "RenderStates.h"

class IEffect
{
public:
	// 使用模板别名(C++11)简化类型名
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	IEffect() = default;
	// 不应该对这个类使用多态,这个类也不应该(也不需要)自行定义析构函数(但是IDE一直有警告就很烦....)
	virtual ~IEffect() = default;
	
	// 不允许拷贝，允许移动
	IEffect(const IEffect&) = delete;
	IEffect& operator=(const IEffect&) = delete;
	IEffect(IEffect&&) = default;
	IEffect& operator=(IEffect&&) = default;

	// 更新并绑定常量缓冲区
	virtual void Apply(ID3D11DeviceContext* deviceContext) = 0;
};

class BasicEffect final : public IEffect
{
public:

	enum class RenderType { RenderObject, RenderInstance };
	
	BasicEffect();
	// 不应该对这个类使用多态,这个类也只需要默认的合成析构就够了(不需要自己特别指定default dtor)(但是IDE一直有警告就很烦....)
	~BasicEffect() override;

	// 不允许拷贝，允许移动
	BasicEffect(const BasicEffect&) = delete;
	BasicEffect& operator=(const BasicEffect&) = delete;
	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	// 获取单例
	static BasicEffect& Get();

	// 初始化所需资源
	bool InitAll(ID3D11Device* device) const;

	//
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext, RenderType type) const;

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldMatrix(const DirectX::FXMMATRIX& world) const;
	void XM_CALLCONV SetViewMatrix(const DirectX::FXMMATRIX& view) const;
	void XM_CALLCONV SetProjMatrix(const DirectX::FXMMATRIX& proj) const;

	//
	// 光照、材质和纹理相关设置
	//

	// 各种类型灯光允许的最大数目
	static const int MaxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight) const;
	void SetPointLight(size_t pos, const PointLight& pointLight) const;
	void SetSpotLight(size_t pos, const SpotLight& spotLight) const;

	void SetMaterial(const Material& material) const;

	void SetTextureUsed(bool isUsed) const;
	
	void SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const;

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos) const;
	
	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;
	
private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif