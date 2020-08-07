//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 定义了一些顶点结构体和输入布局
// Defines vertex structures and input layouts.
//***************************************************************************************

#ifndef VERTEX_H
#define VERTEX_H

#include <d3d11_1.h>
#include <DirectXMath.h>

struct VertexPos
{
	VertexPos() = default;

	explicit constexpr VertexPos(const DirectX::XMFLOAT3& pos) : pos(pos) {}

	DirectX::XMFLOAT3 pos;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[1];
};

struct VertexPosColor
{
	VertexPosColor() = default;

	constexpr VertexPosColor(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT4& color) :
		pos(pos), color(color) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[2];
};

struct VertexPosTex
{
	VertexPosTex() = default;

	constexpr VertexPosTex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& tex) :
		pos(pos), tex(tex) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[2];
};

struct VertexPosSize
{
	VertexPosSize() = default;

	constexpr VertexPosSize(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& size) :
		pos(pos), size(size) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 size;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[2];
};

struct VertexPosNormalColor
{
	VertexPosNormalColor() = default;

	constexpr VertexPosNormalColor(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal,
		const DirectX::XMFLOAT4& color) :
		pos(pos), normal(normal), color(color) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 color;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[3];
};


struct VertexPosNormalTex
{
	VertexPosNormalTex() = default;

	constexpr VertexPosNormalTex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal,
		const DirectX::XMFLOAT2& tex) :
		pos(pos), normal(normal), tex(tex) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[3];
};

struct VertexPosNormalTangentTex
{
	VertexPosNormalTangentTex() = default;

	constexpr VertexPosNormalTangentTex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& normal,
		const DirectX::XMFLOAT4& tangent, const DirectX::XMFLOAT2& tex) :
		pos(pos), normal(normal), tangent(tangent), tex(tex) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 tex;
	static const D3D11_INPUT_ELEMENT_DESC InputLayout[4];
};

#endif
