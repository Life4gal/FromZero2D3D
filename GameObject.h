//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 简易游戏对象
// Simple game object.
//***************************************************************************************

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Model.h"
#include "BasicTransform.h"

#include <set>

class GameObject
{
public:
	// 使用模板别名(C++11)简化类型名
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 添加子对象
	void AddChild(GameObject* child);
	
	// 获取物体变换
	BasicTransform& GetTransform();
	// 获取物体变换
	const BasicTransform& GetTransform() const;

	//
	// 获取包围盒
	//

	DirectX::BoundingBox GetLocalBoundingBox() const;
	DirectX::BoundingBox GetBoundingBox() const;
	DirectX::BoundingOrientedBox GetBoundingOrientedBox() const;

	//
	// 设置实例缓冲区
	//

	// 获取缓冲区可容纳实例的数目
	size_t GetCapacity() const;
	// 重新设置实例缓冲区可容纳实例的数目
	void ResizeBuffer(ID3D11Device* device, size_t count);

	//
	// 设置模型
	//

	void SetModel(Model&& model);
	void SetModel(const Model& model);

	//
	// 绘制
	//

	// 绘制对象
	void Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect);
	// 绘制实例
	void DrawInstanced(ID3D11DeviceContext* deviceContext, BasicEffect& effect, const std::vector<BasicTransform>& data);

	//
	// 调试 
	//

	// 设置调试对象名
	// 若模型被重新设置，调试对象名也需要被重新设置
	void SetDebugObjectName(const std::string& name);

private:
	void XM_CALLCONV Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect, DirectX::FXMMATRIX scale, DirectX::CXMMATRIX toRoot);
	
	struct InstancedData
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
	};

	// 子对象
	std::set<GameObject*> m_children;
	
	Model m_model{};							// 模型
	BasicTransform m_transform{};

	ComPtr<ID3D11Buffer> m_pInstancedBuffer = nullptr;				// 实例缓冲区
	size_t m_capacity = 0;
};

#endif
