#ifndef DEBUGEFFECT_H
#define DEBUGEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

#include <array>

class DebugEffect final : public IEffect, public IEffectTransform, public IEffectTextureDiffuse
{
public:
	DebugEffect();
	~DebugEffect() override;

	DebugEffect(const DebugEffect& other) = delete;
	DebugEffect& operator=(const DebugEffect& other) = delete;
	
	DebugEffect(DebugEffect&& moveFrom) noexcept;
	DebugEffect& operator=(DebugEffect&& moveFrom) noexcept;

	// 获取单例
	static DebugEffect& Get();

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
	// DebugEffect
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext) const;

	// 绘制单通道(0-R, 1-G, 2-B)
	void SetRenderOneComponent(ID3D11DeviceContext* deviceContext, int index) const;

	// 绘制单通道，但以灰度的形式呈现(0-R, 1-G, 2-B, 3-A)
	void SetRenderOneComponentGray(ID3D11DeviceContext* deviceContext, int index) const;

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
