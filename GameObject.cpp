#include "GameObject.h"

using namespace DirectX;

Transform& GameObject::GetTransform()
{
	return m_transform;
}

const Transform& GameObject::GetTransform() const
{
	return m_transform;
}


BoundingBox GameObject::GetBoundingBox() const
{
	BoundingBox box;
	m_model.boundingBox.Transform(box, m_transform.GetLocalToWorldMatrixXM());
	return box;
}

BoundingBox GameObject::GetLocalBoundingBox() const
{
	return m_model.boundingBox;
}

BoundingOrientedBox GameObject::GetBoundingOrientedBox() const
{
	BoundingOrientedBox box;
	BoundingOrientedBox::CreateFromBoundingBox(box, m_model.boundingBox);
	box.Transform(box, m_transform.GetLocalToWorldMatrixXM());
	return box;
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
	UINT strides = m_model.vertexStride;
	UINT offsets = 0;

	for (auto& part : m_model.modelParts)
	{
		// 设置顶点/索引缓冲区
		deviceContext->IASetVertexBuffers(0, 1, part.vertexBuffer.GetAddressOf(), &strides, &offsets);
		deviceContext->IASetIndexBuffer(part.indexBuffer.Get(), part.indexFormat, 0);

		// 更新数据并应用
		effect.SetWorldMatrix(m_transform.GetLocalToWorldMatrixXM());
		effect.SetTextureDiffuse(part.texDiffuse.Get());
		effect.SetMaterial(part.material);

		effect.Apply(deviceContext);

		deviceContext->DrawIndexed(part.indexCount, 0, 0);
	}
}

void GameObject::SetDebugObjectName(const std::string& name)
{
	m_model.SetDebugObjectName(name);
}