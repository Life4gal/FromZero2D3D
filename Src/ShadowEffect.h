#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

#include <array>

class ShadowEffect final : public IEffect, public IEffectTransform, public IEffectTextureDiffuse
{
public:
	ShadowEffect();
	~ShadowEffect() override;

	ShadowEffect(const ShadowEffect& other) = delete;
	ShadowEffect& operator=(const ShadowEffect& other) = delete;
	
	ShadowEffect(ShadowEffect&& moveFrom) noexcept;
	ShadowEffect& operator=(ShadowEffect&& moveFrom) noexcept;

	// 获取单例
	static ShadowEffect& Get();

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

	// 设置漫反射纹理
	void SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const override;

	// 
	// ShadowEffect
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext, RenderType type) const;

	// Alpha裁剪绘制(处理具有透明度的物体)
	void SetRenderAlphaClip(ID3D11DeviceContext* deviceContext, RenderType type) const;

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
