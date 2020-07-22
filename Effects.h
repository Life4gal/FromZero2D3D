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

#endif