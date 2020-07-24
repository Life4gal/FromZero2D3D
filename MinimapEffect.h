#ifndef MINIMAPEFFECT_H
#define MINIMAPEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

#include <array>

class MinimapEffect : public IEffect
{
public:
	MinimapEffect();
	~MinimapEffect() override;

	// 不允许拷贝，允许移动
	MinimapEffect(const MinimapEffect&) = delete;
	MinimapEffect& operator=(const MinimapEffect&) = delete;
	MinimapEffect(MinimapEffect&& moveFrom) noexcept;
	MinimapEffect& operator=(MinimapEffect&& moveFrom) noexcept;

	// 获取单例
	static MinimapEffect& Get();

	// 初始化Minimap.hlsli所需资源并初始化渲染状态
	bool InitAll(ID3D11Device* device) const;

	// 
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext* deviceContext) const;

	//
	// 状态设置
	//

	void SetFogState(bool isOn) const;
	void SetVisibleRange(float range) const;
	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos) const;
	void XM_CALLCONV SetMinimapRect(DirectX::FXMVECTOR rect) const; // (Left, Front, Right, Back)
	void XM_CALLCONV SetInvisibleColor(DirectX::FXMVECTOR color) const;

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
