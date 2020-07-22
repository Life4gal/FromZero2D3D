#ifndef BASICEFFECT_H
#define BASICEFFECT_H

#include "Effects.h"
#include "d3dUtil.h"
#include "EffectHelper.h"	// 必须晚于Effects.h和d3dUtil.h包含
#include "DXTrace.h"
#include "Vertex.h"

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

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world) const;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX view) const;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX proj) const;

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

	void SetTextureDiffuse(ID3D11ShaderResourceView* textureDiffuse) const;
	void SetTextureCube(ID3D11ShaderResourceView* textureCube) const;

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos) const;

	//
	// 状态开关设置
	//

	void SetReflectionEnabled(bool isEnable) const;

	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext* deviceContext) override;

private:
	class Impl;
	std::unique_ptr<Impl> m_pImpl;
};

#endif
