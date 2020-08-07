#ifndef SKYEFFECT_H
#define SKYEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

class SkyEffect final : public IEffect, public IEffectTransform
{
public:
	SkyEffect();
	~SkyEffect() override;

	// 不允许拷贝，允许移动
	SkyEffect(const SkyEffect&) = delete;
	SkyEffect& operator=(const SkyEffect&) = delete;
	SkyEffect(SkyEffect&& moveFrom) noexcept;
	SkyEffect& operator=(SkyEffect&& moveFrom) noexcept;

	// 获取单例
	static SkyEffect& Get();

	// 初始化所需资源
	bool InitAll(ID3D11Device* device) const;

	//
	// IEffectTransform
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world) const override;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX view) const override;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX proj) const override;
	
	// 
	// SkyEffect
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext) const;

	// 设置天空盒
	void SetTextureCube(ID3D11ShaderResourceView* textureCube) const;

	//
	// 矩阵设置
	//

	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
