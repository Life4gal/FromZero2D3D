#include "Camera.h"

using namespace DirectX;

Camera::~Camera() = default;

XMFLOAT3 Camera::GetPosition() const
{
	return m_transform.GetPositionFloat3();
}

XMVECTOR Camera::GetPositionXM() const
{
	return m_transform.GetPositionVector();
}

float Camera::GetRotationX() const
{
	return m_transform.GetRotationFloat3().x;
}

float Camera::GetRotationY() const
{
	return m_transform.GetRotationFloat3().y;
}

XMFLOAT3 Camera::GetRightAxis() const
{
	XMFLOAT3 right{};
	XMStoreFloat3(&right, m_transform.GetRightAxisVector());
	return right;
}

XMVECTOR Camera::GetRightAxisXM() const
{
	return m_transform.GetRightAxisVector();
}

XMFLOAT3 Camera::GetUpAxis() const
{
	XMFLOAT3 up{};
	XMStoreFloat3(&up, m_transform.GetUpAxisVector());
	return up;
}

XMVECTOR Camera::GetUpAxisXM() const
{
	return m_transform.GetUpAxisVector();
}

XMFLOAT3 Camera::GetLookAxis() const
{
	XMFLOAT3 look{};
	XMStoreFloat3(&look, m_transform.GetForwardAxisVector());
	return look;
}

XMVECTOR Camera::GetLookAxisXM() const
{
	return m_transform.GetForwardAxisVector();
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	m_transform.SetPosition(pos);
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	m_transform.SetPosition(pos);
}

void Camera::SetPosition(float x, float y, float z)
{
	m_transform.SetPosition(x, y, z);
}

XMMATRIX Camera::GetViewXM() const
{
	return m_transform.GetWorldToLocalMatrix();
}

XMMATRIX Camera::GetProjXM() const
{
	return XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}

XMMATRIX Camera::GetViewProjXM() const
{
	return GetViewXM() * GetProjXM();
}

D3D11_VIEWPORT Camera::GetViewPort() const
{
	return m_viewPort;
}

void Camera::SetFrustum(float fovY, float aspect, float nearZ, float farZ)
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

void Camera::SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	m_viewPort.TopLeftX = topLeftX;
	m_viewPort.TopLeftY = topLeftY;
	m_viewPort.Width = width;
	m_viewPort.Height = height;
	m_viewPort.MinDepth = minDepth;
	m_viewPort.MaxDepth = maxDepth;
}


// ******************
// 第一人称/自由视角摄像机
//

FirstPersonCamera::~FirstPersonCamera() = default;

void FirstPersonCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	m_transform.SetPosition(pos);
	m_transform.LookAt(target, up);
}

void FirstPersonCamera::LookAt(const XMVECTOR& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	m_transform.SetPosition(pos);
	m_transform.LookAt(target, up);
}

void FirstPersonCamera::LookTo(const XMFLOAT3& pos, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_transform.SetPosition(pos);
	m_transform.LookTo(to, up);
}

void FirstPersonCamera::LookTo(const XMVECTOR& pos, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_transform.SetPosition(pos);
	m_transform.LookTo(to, up);
}

void FirstPersonCamera::Strafe(float d)
{
	m_transform.Translate(m_transform.GetRightAxisVector(), d);
}

void FirstPersonCamera::Walk(float d)
{
	// 右轴叉积上轴并单位向量化得到前轴(Z轴)
	m_transform.Translate(XMVector3Normalize(XMVector3Cross(m_transform.GetRightAxisVector(), g_XMIdentityR1)), d);
}

void FirstPersonCamera::MoveForward(float d)
{
	m_transform.Translate(m_transform.GetForwardAxisVector(), d);
}

void FirstPersonCamera::Pitch(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	// 将绕x轴旋转弧度限制在[-7pi/18, 7pi/18]之间
	rotation.x += rad;
	if (rotation.x > XM_PI * 7 / 18)
		rotation.x = XM_PI * 7 / 18;
	else if (rotation.x < -XM_PI * 7 / 18)
		rotation.x = -XM_PI * 7 / 18;

	m_transform.SetRotation(rotation);
}

void FirstPersonCamera::RotateY(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	rotation.y = XMScalarModAngle(rotation.y + rad);
	m_transform.SetRotation(rotation);
}


// ******************
// 第三人称摄像机
//

ThirdPersonCamera::~ThirdPersonCamera() = default;

XMFLOAT3 ThirdPersonCamera::GetTargetPosition() const
{
	return m_Target;
}

float ThirdPersonCamera::GetDistance() const
{
	return m_distance;
}

void ThirdPersonCamera::RotateX(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	rotation.x += rad;
	if (rotation.x < 0.0f)
		rotation.x = 0.0f;
	else if (rotation.x > XM_PI / 3)
		rotation.x = XM_PI / 3;

	m_transform.SetRotation(rotation);
	m_transform.SetPosition(m_Target);
	m_transform.Translate(m_transform.GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::RotateY(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	rotation.y = XMScalarModAngle(rotation.y + rad);

	m_transform.SetRotation(rotation);
	m_transform.SetPosition(m_Target);
	m_transform.Translate(m_transform.GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::Approach(float dist)
{
	m_distance += dist;
	// 限制距离在[m_MinDist, m_MaxDist]之间
	if (m_distance < m_minDist)
		m_distance = m_minDist;
	else if (m_distance > m_maxDist)
		m_distance = m_maxDist;

	m_transform.SetPosition(m_Target);
	m_transform.Translate(m_transform.GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::SetRotationX(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	rotation.x = rad;
	if (rotation.x < 0.0f)
		rotation.x = 0.0f;
	else if (rotation.x > XM_PI / 3)
		rotation.x = XM_PI / 3;

	m_transform.SetRotation(rotation);
	m_transform.SetPosition(m_Target);
	m_transform.Translate(m_transform.GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::SetRotationY(float rad)
{
	XMFLOAT3 rotation = m_transform.GetRotationFloat3();
	rotation.y = XMScalarModAngle(rad);
	m_transform.SetRotation(rotation);
	m_transform.SetPosition(m_Target);
	m_transform.Translate(m_transform.GetForwardAxisVector(), -m_distance);
}

void ThirdPersonCamera::SetTarget(const XMFLOAT3& target, bool lookTo, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_Target = target;
	if(lookTo)
	{
		m_transform.LookTo(to, up);
	}
}

void ThirdPersonCamera::SetTarget(const XMVECTOR& target, bool lookTo, const XMFLOAT3& to, const XMFLOAT3& up)
{
	XMStoreFloat3(&m_Target, target);
	if (lookTo)
	{
		m_transform.LookTo(to, up);
	}
}

void ThirdPersonCamera::SetDistance(float dist)
{
	m_distance = dist;
}

void ThirdPersonCamera::SetDistanceMinMax(float minDist, float maxDist)
{
	m_minDist = minDist;
	m_maxDist = maxDist;
}

