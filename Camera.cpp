#include "Camera.h"
using namespace DirectX;

Camera::~Camera() = default;

XMFLOAT3 Camera::GetPosition() const
{
	return m_Transform.GetPosition();
}

XMVECTOR Camera::GetPositionXM() const
{
	return m_Transform.GetPositionXM();
}

float Camera::GetRotationX() const
{
	return m_Transform.GetRotation().x;
}

float Camera::GetRotationY() const
{
	return m_Transform.GetRotation().y;
}

XMFLOAT3 Camera::GetRightAxis() const
{
	XMFLOAT3 right{};
	XMStoreFloat3(&right, m_Transform.GetRightAxisXM());
	return right;
}

XMVECTOR Camera::GetRightAxisXM() const
{
	return m_Transform.GetRightAxisXM();
}

XMFLOAT3 Camera::GetUpAxis() const
{
	XMFLOAT3 up{};
	XMStoreFloat3(&up, m_Transform.GetUpAxisXM());
	return up;
}

XMVECTOR Camera::GetUpAxisXM() const
{
	return m_Transform.GetUpAxisXM();
}

XMFLOAT3 Camera::GetLookAxis() const
{
	XMFLOAT3 look{};
	XMStoreFloat3(&look, m_Transform.GetForwardAxisXM());
	return look;
}

XMVECTOR Camera::GetLookAxisXM() const
{
	return m_Transform.GetForwardAxisXM();
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	m_Transform.SetPosition(pos);
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	m_Transform.SetPosition(pos);
}

void Camera::SetPosition(float x, float y, float z)
{
	m_Transform.SetPosition(x, y, z);
}

XMMATRIX Camera::GetViewXM() const
{
	return m_Transform.GetWorldToLocalMatrixXM();
}

XMMATRIX Camera::GetProjXM() const
{
	return XMMatrixPerspectiveFovLH(m_FovY, m_Aspect, m_NearZ, m_FarZ);
}

XMMATRIX Camera::GetViewProjXM() const
{
	return GetViewXM() * GetProjXM();
}

D3D11_VIEWPORT Camera::GetViewPort() const
{
	return m_ViewPort;
}

void Camera::SetFrustum(float fovY, float aspect, float nearZ, float farZ)
{
	m_FovY = fovY;
	m_Aspect = aspect;
	m_NearZ = nearZ;
	m_FarZ = farZ;
}

void Camera::SetViewPort(const D3D11_VIEWPORT & viewPort)
{
	m_ViewPort = viewPort;
}

void Camera::SetViewPort(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
{
	m_ViewPort.TopLeftX = topLeftX;
	m_ViewPort.TopLeftY = topLeftY;
	m_ViewPort.Width = width;
	m_ViewPort.Height = height;
	m_ViewPort.MinDepth = minDepth;
	m_ViewPort.MaxDepth = maxDepth;
}


// ******************
// 第一人称/自由视角摄像机
//

FirstPersonCamera::~FirstPersonCamera() = default;

void FirstPersonCamera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookAt(target, up);
}

void FirstPersonCamera::LookAt(const XMVECTOR& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookAt(target, up);
}

void FirstPersonCamera::LookTo(const XMFLOAT3& pos, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookTo(to, up);
}

void FirstPersonCamera::LookTo(const XMVECTOR& pos, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_Transform.SetPosition(pos);
	m_Transform.LookTo(to, up);
}

void FirstPersonCamera::Strafe(float d)
{
	m_Transform.Translate(m_Transform.GetRightAxisXM(), d);
}

void FirstPersonCamera::Walk(float d)
{
	// 右轴叉积上轴并单位向量化得到前轴(Z轴)
	m_Transform.Translate(XMVector3Normalize(XMVector3Cross(m_Transform.GetRightAxisXM(), g_XMIdentityR1)), d);
}

void FirstPersonCamera::MoveForward(float d)
{
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), d);
}

void FirstPersonCamera::Pitch(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	// 将绕x轴旋转弧度限制在[-7pi/18, 7pi/18]之间
	rotation.x += rad;
	if (rotation.x > XM_PI * 7 / 18)
		rotation.x = XM_PI * 7 / 18;
	else if (rotation.x < -XM_PI * 7 / 18)
		rotation.x = -XM_PI * 7 / 18;

	m_Transform.SetRotation(rotation);
}

void FirstPersonCamera::RotateY(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rotation.y + rad);
	m_Transform.SetRotation(rotation);
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
	return m_Distance;
}

void ThirdPersonCamera::RotateX(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	rotation.x += rad;
	if (rotation.x < 0.0f)
		rotation.x = 0.0f;
	else if (rotation.x > XM_PI / 3)
		rotation.x = XM_PI / 3;

	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), -m_Distance);
}

void ThirdPersonCamera::RotateY(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rotation.y + rad);

	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), -m_Distance);
}

void ThirdPersonCamera::Approach(float dist)
{
	m_Distance += dist;
	// 限制距离在[m_MinDist, m_MaxDist]之间
	if (m_Distance < m_MinDist)
		m_Distance = m_MinDist;
	else if (m_Distance > m_MaxDist)
		m_Distance = m_MaxDist;

	m_Transform.SetPosition(m_Target);
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), -m_Distance);
}

void ThirdPersonCamera::SetRotationX(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	// 将绕x轴旋转弧度限制在[0, pi/3]之间
	rotation.x = rad;
	if (rotation.x < 0.0f)
		rotation.x = 0.0f;
	else if (rotation.x > XM_PI / 3)
		rotation.x = XM_PI / 3;

	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), -m_Distance);
}

void ThirdPersonCamera::SetRotationY(float rad)
{
	XMFLOAT3 rotation = m_Transform.GetRotation();
	rotation.y = XMScalarModAngle(rad);
	m_Transform.SetRotation(rotation);
	m_Transform.SetPosition(m_Target);
	m_Transform.Translate(m_Transform.GetForwardAxisXM(), -m_Distance);
}

void ThirdPersonCamera::SetTarget(const XMFLOAT3& target, bool lookTo, const XMFLOAT3& to, const XMFLOAT3& up)
{
	m_Target = target;
	if(lookTo)
	{
		m_Transform.LookTo(to, up);
	}
}

void ThirdPersonCamera::SetTarget(const XMVECTOR& target, bool lookTo, const XMFLOAT3& to, const XMFLOAT3& up)
{
	XMStoreFloat3(&m_Target, target);
	if (lookTo)
	{
		m_Transform.LookTo(to, up);
	}
}

void ThirdPersonCamera::SetDistance(float dist)
{
	m_Distance = dist;
}

void ThirdPersonCamera::SetDistanceMinMax(float minDist, float maxDist)
{
	m_MinDist = minDist;
	m_MaxDist = maxDist;
}

