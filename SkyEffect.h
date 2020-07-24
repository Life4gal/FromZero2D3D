#ifndef SKYEFFECT_H
#define SKYEFFECT_H

#include "d3dUtil.h"
#include "EffectHelper.h"
#include "DXTrace.h"
#include "Vertex.h"

class SkyEffect final : public IEffect
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
	// 纹理立方体映射设置
	//

	void SetTextureCube(ID3D11ShaderResourceView* textureCube) const;


	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
