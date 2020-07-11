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
		m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_direction(direction), m_pad() {}

	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular;
	DirectX::XMFLOAT3 m_direction;
	float m_pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 点光
struct PointLight
{
	PointLight() = default;
	
	PointLight(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position, const float range, const DirectX::XMFLOAT3& att) :
		m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_position(position), m_range(range), m_att(att), m_pad() {}

	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular;

	// 打包成4D向量: (position, range)
	DirectX::XMFLOAT3 m_position;
	float m_range;

	// 打包成4D向量: (A0, A1, A2, pad)
	DirectX::XMFLOAT3 m_att;
	float m_pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 聚光灯
struct SpotLight
{
	SpotLight() = default;
	
	SpotLight(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT3& position, const float range, const DirectX::XMFLOAT3& direction,
		const float spot, const DirectX::XMFLOAT3& att) :
		m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), 
		m_position(position), m_range(range), m_direction(direction), m_spot(spot), m_att(att), m_pad() {}

	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular;

	// 打包成4D向量: (position, range)
	DirectX::XMFLOAT3 m_position;
	float m_range;

	// 打包成4D向量: (direction, spot)
	DirectX::XMFLOAT3 m_direction;
	float m_spot;

	// 打包成4D向量: (att, pad)
	DirectX::XMFLOAT3 m_att;
	float m_pad; // 最后用一个浮点数填充使得该结构体大小满足16的倍数，便于我们以后在HLSL设置数组
};

// 物体表面材质
struct Material
{
	Material() = default;

	Material(const DirectX::XMFLOAT4& ambient, const DirectX::XMFLOAT4& diffuse, const DirectX::XMFLOAT4& specular,
		const DirectX::XMFLOAT4& reflect) :
		m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_reflect(reflect) {}

	DirectX::XMFLOAT4 m_ambient;
	DirectX::XMFLOAT4 m_diffuse;
	DirectX::XMFLOAT4 m_specular; // w = 镜面反射强度
	DirectX::XMFLOAT4 m_reflect;
};

#endif
