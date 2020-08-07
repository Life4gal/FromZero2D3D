#ifndef BASICEFFECT_H
#define BASICEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

#include <array>

class BasicEffect final : public IEffect, public IEffectTransform, public IEffectTextureDiffuse
{
public:
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
	// IEffectTransform
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world) const override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX view) const override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX proj) const override;

	//
	// IEffectTextureDiffuse
	//

	void SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const override;

	//
	// BasicEffect
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext, RenderType type) const;
	// 带法线贴图的绘制
	void SetRenderWithNormalMap(ID3D11DeviceContext* deviceContext, RenderType type) const;

	void XM_CALLCONV SetShadowTransformMatrix(DirectX::FXMMATRIX shadow) const;

	//
	// 光照、材质和纹理相关设置
	//

	// 各种类型灯光允许的最大数目
	static const int MaxLights = 5;

	void SetDirLight(size_t position, const DirectionalLight& dirLight) const;
	void SetPointLight(size_t position, const PointLight& pointLight) const;
	void SetSpotLight(size_t position, const SpotLight& spotLight) const;
	void SetMaterial(const Material& material) const;

	void SetTextureUsed(bool isUsed) const;
	void SetShadowEnabled(bool enabled) const;
	void SetTextureNormalMap(ID3D11ShaderResourceView* textureNormalMap) const;
	void SetTextureShadowMap(ID3D11ShaderResourceView* textureShadowMap) const;
	void SetTextureCube(ID3D11ShaderResourceView* textureCube) const;

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos) const;

	//
	// IEffect
	//

	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
