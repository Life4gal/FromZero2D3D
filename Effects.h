//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
// 
// 简易特效管理框架
// Simple effect management framework.
//
// 总体使用方式基本相同
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
	template <class T>
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
	virtual void Apply(ID3D11DeviceContext * deviceContext) = 0;
};

class BasicEffect : public IEffect
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
	bool InitAll(ID3D11Device * device) const;

	//
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext * deviceContext) const;
	// Alpha混合绘制
	void SetRenderAlphaBlend(ID3D11DeviceContext * deviceContext) const;
	// 绘制闪电动画所需要的特效，关闭深度测试
	void SetDrawBoltAnimNoDepthTest(ID3D11DeviceContext* deviceContext) const;
	// 绘制闪电动画所需要的特效，关闭深度写入
	void SetDrawBoltAnimNoDepthWrite(ID3D11DeviceContext* deviceContext) const;
	// 无二次混合
	void SetRenderNoDoubleBlend(ID3D11DeviceContext * deviceContext, UINT stencilRef) const;
	// 仅写入模板值
	void SetWriteStencilOnly(ID3D11DeviceContext * deviceContext, UINT stencilRef) const;
	// 对指定模板值的区域进行绘制，采用默认状态
	void SetRenderDefaultWithStencil(ID3D11DeviceContext * deviceContext, UINT stencilRef) const;
	// 对指定模板值的区域进行绘制，采用Alpha混合
	void SetRenderAlphaBlendWithStencil(ID3D11DeviceContext * deviceContext, UINT stencilRef) const;
	// 绘制闪电动画所需要的特效，关闭深度测试，对指定模板值区域进行绘制
	void SetDrawBoltAnimNoDepthTestWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef) const;
	// 绘制闪电动画所需要的特效，关闭深度写入，对指定模板值区域进行绘制
	void SetDrawBoltAnimNoDepthWriteWithStencil(ID3D11DeviceContext* deviceContext, UINT stencilRef) const;
	// 2D默认状态绘制
	void Set2DRenderDefault(ID3D11DeviceContext * deviceContext) const;
	// 2D混合绘制
	void Set2DRenderAlphaBlend(ID3D11DeviceContext * deviceContext) const;

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W) const;
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V) const;
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P) const;

	void XM_CALLCONV SetReflectionMatrix(DirectX::FXMMATRIX R) const;
	void XM_CALLCONV SetShadowMatrix(DirectX::FXMMATRIX S) const;
	void XM_CALLCONV SetRefShadowMatrix(DirectX::FXMMATRIX RefS) const;
	
	//
	// 光照、材质和纹理相关设置
	//

	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight) const;
	void SetPointLight(size_t pos, const PointLight& pointLight) const;
	void SetSpotLight(size_t pos, const SpotLight& spotLight) const;

	void SetMaterial(const Material& material) const;

	void SetTexture(ID3D11ShaderResourceView * texture) const;

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos) const;

	//
	// 状态开关设置
	//

	void SetReflectionState(bool isOn) const;
	void SetShadowState(bool isOn) const;
	

	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext * deviceContext) override;
	
private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

#endif