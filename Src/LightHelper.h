//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
// 
//***************************************************************************************

#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

#include <DirectXMath.h>

// 方向光
struct DirectionalLight
{
	DirectionalLight() = default;
	
	DirectionalLight(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& direction) :
		ambient(ambient), diffuse(diffuse), specular(specular), direction(direction), pad() {}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT3 direction;
	float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 点光
struct PointLight
{
	PointLight() = default;
	
	PointLight(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position, const float range, const DirectX::XMFLOAT3& att) :
		ambient(ambient), diffuse(diffuse), specular(specular), position(position), range(range), att(att), pad() {}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	// 打包成4D向量: (position, range)
	DirectX::XMFLOAT3 position;
	float range;

	// 打包成4D向量: (A0, A1, A2, pad)
	DirectX::XMFLOAT3 att;
	float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 聚光灯
struct SpotLight
{
	SpotLight() = default;
	
	SpotLight(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position, const float range, const DirectX::XMFLOAT3& direction,
		const float spot, const DirectX::XMFLOAT3& att) :
		ambient(ambient), diffuse(diffuse), specular(specular), 
		position(position), range(range), direction(direction), spot(spot), att(att), pad() {}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;

	// 打包成4D向量: (position, range)
	DirectX::XMFLOAT3 position;
	float range;

	// 打包成4D向量: (direction, spot)
	DirectX::XMFLOAT3 direction;
	float spot;

	// 打包成4D向量: (att, pad)
	DirectX::XMFLOAT3 att;
	float pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 物体表面材质
struct Material
{
	Material() = default;

	Material(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT4& reflect) :
		ambient(ambient), diffuse(diffuse), specular(specular), reflect(reflect) {}

	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular; // w = 镜面反射强度
	DirectX::XMFLOAT4 reflect;
};

#endif
