//***************************************************************************************
// Author: X_Jun(MKXJun)(MIT License)
//
// Modified By: life4gal(NiceT)(MIT License)
//
// 基于 X_Jun 的 Camera ,做了较大改动
//
// 提供第一人称(自由视角)和第三人称摄像机
// Provide 1st person(free view) and 3rd person cameras.
//***************************************************************************************

#ifndef CAMERA_H
#define CAMERA_H

#include <d3d11_1.h>
#include <DirectXMath.h>

#include "BasicTransform.h"

// 经过考虑,我们认为让 Camera 继承 BasicTransform 是合理的
class Camera : public BasicTransform
{
public:
	explicit Camera(DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f }, DirectX::XMFLOAT3 rotation = {}, DirectX::XMFLOAT3 position = {});
	virtual ~Camera() = default;

	Camera(const Camera& other) = default;
	Camera(Camera && other) noexcept = default;
	Camera& operator=(const Camera & other) = default;
	Camera& operator=(Camera && other) noexcept = default;

	//
	// 获取摄像机旋转
	//

	// 获取绕X轴旋转的欧拉角弧度
	float GetRotationX() const;
	// 获取绕Y轴旋转的欧拉角弧度
	float GetRotationY() const;

	//
	// 获取矩阵
	//

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetProjMatrix() const;
	DirectX::XMMATRIX GetViewProjMatrix() const;

	// 获取视口
	D3D11_VIEWPORT GetViewPort() const;

	// 设置视锥体
	void SetFrustum(float fovY, float aspect, float nearZ, float farZ);

	// 设置视口
	void SetViewPort(const D3D11_VIEWPORT& viewPort);
	void SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

protected:
	// 视锥体属性
	float m_nearZ;
	float m_farZ;
	float m_aspect;
	float m_fovY;

	// 当前视口
	D3D11_VIEWPORT m_viewPort;
};

class FirstPersonCamera final : public Camera
{
public:
	FirstPersonCamera() = default;
	~FirstPersonCamera() override = default;

	FirstPersonCamera(const FirstPersonCamera& other) = default;
	FirstPersonCamera(FirstPersonCamera&& other) noexcept = default;
	FirstPersonCamera& operator=(const FirstPersonCamera& other) = default;
	FirstPersonCamera& operator=(FirstPersonCamera&& other) noexcept = default;
	
	// 设置第一人称摄像机的朝向
	void LookAt(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);
	void LookTo(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);
	
	// 平移
	void Strafe(float d);
	// 直行(平面移动)
	void Walk(float d);
	// 前进(朝前向移动)
	void MoveForward(float d);
	// 上下观察
	// 正rad值向上观察
	// 负rad值向下观察
	void Pitch(float rad);
	// 左右观察
	// 正rad值向左观察
	// 负rad值向右观察
	void RotateY(float rad);
};

class ThirdPersonCamera final : public Camera
{
public:
	ThirdPersonCamera() = default;
	~ThirdPersonCamera() override = default;

	ThirdPersonCamera(const ThirdPersonCamera& other) = default;
	ThirdPersonCamera(ThirdPersonCamera&& other) noexcept = default;
	ThirdPersonCamera& operator=(const ThirdPersonCamera& other) = default;
	ThirdPersonCamera& operator=(ThirdPersonCamera&& other) noexcept = default;
	
	// 获取当前跟踪物体的位置
	DirectX::XMFLOAT3 GetTargetPosition() const;
	// 获取与物体的距离
	float GetDistance() const;
	
	// 绕物体垂直旋转(注意绕x轴旋转欧拉角弧度限制在[0, pi/3])
	void RotateX(float rad);
	// 绕物体水平旋转
	void RotateY(float rad);
	// 拉近物体
	void Approach(float dist);
	
	// 设置初始绕X轴的弧度(注意绕x轴旋转欧拉角弧度限制在[0, pi/3])
	void SetRotationX(float rad);
	// 设置初始绕Y轴的弧度
	void SetRotationY(float rad);
	
	// 设置并绑定待跟踪物体的位置并确定是否看向目标,如果不设置lookTo,则会默认看向Z轴正方向且以Y轴为上方向,主要是为了避免摄像机视角切换导致方向变化
	// void SetTarget(const DirectX::XMFLOAT3& target, bool lookTo = false, const DirectX::XMFLOAT3& to = {0, 0, 1}, const DirectX::XMFLOAT3& up = {0, 1, 0});
	// 默认我们不看向目标,如果需要看向目标自己指定即可
	void SetTarget(const DirectX::XMFLOAT3& target);
	// 设置初始距离
	void SetDistance(float dist);
	// 设置最小最大允许距离
	void SetDistanceMinMax(float minDist, float maxDist);

private:
	DirectX::XMFLOAT3 m_target{};
	float m_distance = 0.0f;
	// 最小允许距离，最大允许距离
	float m_minDist = 0.0f;
	float m_maxDist = 0.0f;
};

#endif
