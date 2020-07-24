#ifndef SCRRENFADEEFFECT_H
#define SCRRENFADEEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

#include <array>

class ScreenFadeEffect final : public IEffect
{
public:
	ScreenFadeEffect();
	~ScreenFadeEffect() override;

	// 不允许拷贝，允许移动
	ScreenFadeEffect(const ScreenFadeEffect&) = delete;
	ScreenFadeEffect& operator=(const ScreenFadeEffect&) = delete;
	ScreenFadeEffect(ScreenFadeEffect&& moveFrom) noexcept;
	ScreenFadeEffect& operator=(ScreenFadeEffect&& moveFrom) noexcept;

	// 获取单例
	static ScreenFadeEffect& Get();

	// 初始化ScreenFade.hlsli所需资源并初始化渲染状态
	bool InitAll(ID3D11Device* device) const;

	// 
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext) const;

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX world, DirectX::CXMMATRIX view, DirectX::CXMMATRIX proj) const;
	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX worldViewProjMatrix) const;

	//
	// 淡入淡出设置
	//

	void SetFadeAmount(float fadeAmount) const;

	//
	// 纹理设置
	//

	void SetTexture(ID3D11ShaderResourceView* texture) const;

	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
