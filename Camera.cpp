#include "Camera.h"

using namespace DirectX;

Camera::Camera(const XMFLOAT3 scale, const XMFLOAT3 rotation, const XMFLOAT3 position)
	:
	BasicTransform(scale, rotation, position),
	m_nearZ(),
	m_farZ(),
	m_aspect(),
	m_fovY(),
	m_viewPort()
{
}

float Camera::GetRotationX() const
{
	return GetRotationFloat3().x;
}

float Camera::GetRotationY() const
{
	return GetRotationFloat3().y;
}

XMMATRIX Camera::GetViewMatrix() const
{
	return GetWorldToLocalMatrix();
}

XMMATRIX Camera::GetProjMatrix() const
{
	return XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}

XMMATRIX Camera::GetViewProjMatrix() const
{
	return GetViewMatrix() * GetProjMatrix();
}

D3D11_VIEWPORT Camera::GetViewPort() const
{
	return m_viewPort;
}

void Camera::SetFrustum(const float fovY, const float aspect, const float nearZ, const float farZ)
{
	m_fovY = fovY;
	m_aspect = aspect;
	m_nearZ = nearZ;
	m_farZ = farZ;
}

void Camera::SetViewPort(const D3D11_VIEWPORT & viewPort)
{
	m_viewPort = viewPort;
}

void Camera::SetViewPort(const float topLeftX, const float topLeftY, const float width, const float height, const float minDepth, const float maxDepth)
{
	m_viewPort.TopLeftX = topLeftX;
	m_viewPort.TopLeftY = topLeftY;
	m_viewPort.Width = width;
	m_viewPort.Height = height;
	m_viewPort.MinDepth = minDepth;
	m_viewPort.MaxDepth = maxDepth;
}


// ******************
// 第一人称
//

void XM_CALLCONV FirstPersonCamera::LookAt(FXMVECTOR position, FXMVECTOR target, FXMVECTOR up)
{
	SetPosition(position);
	Camera::LookAt(target, up);
}

void XM_CALLCONV FirstPersonCamera::LookTo(FXMVECTOR position, FXMVECTOR to, FXMVECTOR up)
{
	SetPosition(position);
	Camera::LookTo(to, up);
}

void FirstPersonCamera::Strafe(const float d)
{
	Translate(GetRightAxisVector(), d);
}

void FirstPersonCamera::Walk(const float d)
{
	// 右轴叉积上轴并单位向量化得到前轴(Z轴)
	Translate(XMVector3Normalize(XMVector3Cross(GetRightAxisVector(), g_XMIdentityR1)), d);
}

void FirstPersonCamera::MoveForward(const float d)
{
	Translate(GetForwardAxisVector(), d);
}

void FirstPersonCamera::Pitch(const float rad)
{
	// 将绕x轴旋转弧度限制在[-7pi/18, 7pi/18]之间
	m_rotation.x += rad;
	if (m_rotation.x > XM_PI * 7 / 18)
		m_rotation.x = XM_PI * 7 / 18;
	else if (m_rotation.x < -XM_PI * 7 / 18)
		m_rotation.x = -XM_PI * 7 / 18;
}

void FirstPersonCamera::RotateY(const float rad)
{
	m_rotation.y = XMScalarModAngle(m_rotation.y + rad);
}

// ******************
// 第三人称摄像机
//

XMFLOAT3 ThirdPersonCamera::GetTargetPosition() const
{
	return m_target;
}

float ThirdPersonCamera::GetDistance() const
{
	return m_distance;
}

void ThirdPersonCamera::RotateX(const float rad)
{
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	m_rotation.x += rad;
	if (m_rotation.x < 0.0f)
		m_rotation.x = 0.0f;
	else if (m_rotation.x > XM_PI / 3)
		m_rotation.x = XM_PI / 3;

	SetPosition(m_target);
	Translate(GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::RotateY(const float rad)
{
	m_rotation.y = XMScalarModAngle(m_rotation.y + rad);

	SetPosition(m_target);
	Translate(GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::Approach(const float dist)
{
	m_distance += dist;
	// 限制距离在[m_MinDist, m_MaxDist]之间
	if (m_distance < m_minDist)
		m_distance = m_minDist;
	else if (m_distance > m_maxDist)
		m_distance = m_maxDist;

	SetPosition(m_target);
	Translate(GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::SetRotationX(const float rad)
{
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	m_rotation.x = rad;
	if (m_rotation.x < 0.0f)
		m_rotation.x = 0.0f;
	else if (m_rotation.x > XM_PI / 3)
		m_rotation.x = XM_PI / 3;

	SetPosition(m_target);
	Translate(GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::SetRotationY(const float rad)
{
	m_rotation.y = XMScalarModAngle(rad);

	SetPosition(m_target);
	Translate(GetForwardAxisVector(), -m_distance);
}

/*
void ThirdPersonCamera::SetTarget(const XMFLOAT3& target, bool lookTo, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_target = target;
	if(lookTo)
	{
		LookTo(to, up);
	}
}
*/

void ThirdPersonCamera::SetTarget(const XMFLOAT3& target)
{
	m_target = target;
}

void ThirdPersonCamera::SetDistance(const float dist)
{
	m_distance = dist;
}

void ThirdPersonCamera::SetDistanceMinMax(const float minDist, const float maxDist)
{
	m_minDist = minDist;
	m_maxDist = maxDist;
}
