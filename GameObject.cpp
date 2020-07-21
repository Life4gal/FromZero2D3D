#include "GameObject.h"

using namespace DirectX;

void GameObject::AddChild(GameObject* child)
{
	m_children.insert(child);
}

BasicTransform& GameObject::GetTransform()
{
	return m_transform;
}

const BasicTransform& GameObject::GetTransform() const
{
	return m_transform;
}

BoundingBox GameObject::GetLocalBoundingBox() const
{
	return m_model.boundingBox;
}

BoundingBox GameObject::GetBoundingBox() const
{
	BoundingBox box;
	m_model.boundingBox.Transform(box, m_transform.GetLocalToWorldMatrix());
	return box;
}

BoundingOrientedBox GameObject::GetBoundingOrientedBox() const
{
	BoundingOrientedBox box;
	BoundingOrientedBox::CreateFromBoundingBox(box, m_model.boundingBox);
	box.Transform(box, m_transform.GetLocalToWorldMatrix());
	return box;
}

size_t GameObject::GetCapacity() const
{
	return m_capacity;
}

void GameObject::ResizeBuffer(ID3D11Device* device, const size_t count)
{
	// 设置实例缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = static_cast<UINT>(count) * sizeof(InstancedData);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 创建实例缓冲区
	HR(device->CreateBuffer(&vbd, nullptr, m_pInstancedBuffer.ReleaseAndGetAddressOf()));

	// 重新调整m_Capacity
	m_capacity = count;
}

void GameObject::SetModel(Model&& model)
{
	std::swap(m_model, model);
	model.modelParts.clear();
	model.boundingBox = BoundingBox();
}

void GameObject::SetModel(const Model& model)
{
	m_model = model;
}

void GameObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect)
{
	Draw(deviceContext, effect, XMMatrixIdentity(), XMMatrixIdentity());
}

void GameObject::DrawInstanced(ID3D11DeviceContext* deviceContext, BasicEffect& effect, const std::vector<BasicTransform>& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;
	const UINT numInstances = static_cast<UINT>(data.size());
	// 若传入的数据比实例缓冲区还大，需要重新分配
	if (numInstances > m_capacity)
	{
		ComPtr<ID3D11Device> device;
		deviceContext->GetDevice(device.GetAddressOf());
		ResizeBuffer(device.Get(), numInstances);
	}

	HR(deviceContext->Map(m_pInstancedBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	auto iter = reinterpret_cast<InstancedData*>(mappedData.pData);
	for (auto& transform : data)
	{
		const XMMATRIX world = transform.GetLocalToWorldMatrix();
		iter->world = XMMatrixTranspose(world);
		iter->worldInvTranspose = XMMatrixInverse(nullptr, world);	// 两次转置抵消
		++iter;
	}

	deviceContext->Unmap(m_pInstancedBuffer.Get(), 0);

	UINT strides[2] = { m_model.vertexStride, sizeof(InstancedData) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer* buffers[2] = { nullptr, m_pInstancedBuffer.Get() };
	for (auto& part : m_model.modelParts)
	{
		buffers[0] = part.vertexBuffer.Get();

		// 设置顶点/索引缓冲区
		deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		deviceContext->IASetIndexBuffer(part.indexBuffer.Get(), part.indexFormat, 0);

		// 更新数据并应用
		effect.SetTextureDiffuse(part.texDiffuse.Get());
		effect.SetMaterial(part.material);
		effect.Apply(deviceContext);

		deviceContext->DrawIndexedInstanced(part.indexCount, numInstances, 0, 0, 0);
	}
}

void GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)

	m_model.SetDebugObjectName(name);
	if (m_pInstancedBuffer)
	{
		D3D11SetDebugObjectName(m_pInstancedBuffer.Get(), name + ".InstancedBuffer");
	}

#endif
}

void GameObject::Draw(ID3D11DeviceContext* deviceContext, BasicEffect& effect, FXMMATRIX scale, CXMMATRIX toRoot)
{
	const XMMATRIX parentScale = XMMatrixScalingFromVector(m_transform.GetScaleVector());
	const XMMATRIX parentRotation = XMMatrixRotationRollPitchYawFromVector(m_transform.GetRotationVector());
	const XMMATRIX parentTranslation = XMMatrixTranslationFromVector(m_transform.GetPositionVector());

	UINT strides = m_model.vertexStride;
	UINT offsets = 0;

	for (auto& part : m_model.modelParts)
	{
		// 设置顶点/索引缓冲区
		deviceContext->IASetVertexBuffers(0, 1, part.vertexBuffer.GetAddressOf(), &strides, &offsets);
		deviceContext->IASetIndexBuffer(part.indexBuffer.Get(), part.indexFormat, 0);

		// Rotation*Translation矩阵可以让物体从物体自身的局部坐标系变换到世界坐标系
		effect.SetWorldMatrix(scale * parentScale * parentRotation * parentTranslation * toRoot);
		effect.SetTextureDiffuse(part.texDiffuse.Get());
		effect.SetMaterial(part.material);

		effect.Apply(deviceContext);

		deviceContext->DrawIndexed(part.indexCount, 0, 0);
	}

	// 子物体绘制
	for(const GameObject* child : m_children)
	{
		// 子物体的RT矩阵可以让子物体从子物体自身的局部坐标系变换到父物体的局部坐标系,然后再乘上父物体的Rotation*Translation矩阵变换到世界坐标系
		const_cast<GameObject*>(child)->Draw(deviceContext, effect, scale * parentScale, parentRotation * parentTranslation * toRoot);
	}
}
