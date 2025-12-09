#ifndef CAMERA_H
#define CAMERA_H

#include <DirectXMath.h>
#include <memory>
#include "Window.h"

class Camera
{
public:
	Camera();

	~Camera();

	static Camera& GetInstance();

	/// <summary>
	/// カメラの現在座標を返す
	/// </summary>
	/// <returns>カメラの座標</returns>
	DirectX::XMVECTOR GetPosition();

	DirectX::XMVECTOR GetRotation();

	/// <summary>
	/// カメラの現在向いている方向を単位ベクトルで返す
	/// </summary>
	/// <returns>単位ベクトル</returns>
	DirectX::XMVECTOR GetTargetVec();

	/// <summary>
	/// カメラが現在フォーカスしている現在座標
	/// </summary>
	/// <returns>フォーカスしている座標</returns>
	DirectX::XMVECTOR GetTargetPos();

	/// <summary>
    /// カメラの座標を設定する
    /// </summary>
    /// <param name="position">カメラ位置</param>
	void SetCameraPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// カメラの回転をEulerで設定する
	/// </summary>
	/// <param name="rotation">Euler回転</param>
	void SetCameraRotation(DirectX::XMVECTOR rotation);

	DirectX::XMVECTOR GetPosWorldToScreen(DirectX::XMVECTOR pos);

	DirectX::XMVECTOR GetPosScreenToWorld(DirectX::XMVECTOR pos);

	float GetFov();

	float GetNearZ();

	float GetFarZ();

	float GetAspect();

	float GetOrthoDiameter();

	DirectX::XMMATRIX GetViewMat();
	DirectX::XMMATRIX GetProjMat();

	void SetFov(float v);

	void SetNearZ(float v);

	void SetFarZ(float v);

	void SetAspect(float v);

	void SetOrthoDiameter(float v);

	bool isPerspective = true;

	DirectX::XMVECTOR shadowMapPos = DirectX::XMVectorSet(0, 10, -10, 0);

	DirectX::XMVECTOR shadowMapRot = DirectX::XMVectorSet(55, 0, 0, 0);
private:
	float fov = DirectX::XMConvertToRadians(70);

	float nearZ = 1.0f;

	float farZ = 1000.0f;

	float aspect = (float)Window::width / (float)Window::height;

	float orthoDiameter = 10;

	/// <summary>
	/// カメラの座標位置
	/// </summary>
	DirectX::XMVECTOR camPosition = DirectX::XMVectorSet(0, 2, -4, 0);

	/// <summary>
	/// カメラのEuler回転
	/// </summary>
	DirectX::XMVECTOR camRotation = DirectX::XMVectorSet(0, 0, 10, 0);
protected:

	/// <summary>
	/// カメラの回転を四元数で表したもの
	/// </summary>
	DirectX::XMVECTOR camQuaternion = DirectX::XMQuaternionRotationRollPitchYaw(
	    DirectX::XMConvertToRadians(DirectX::XMVectorGetX(camRotation)), 
		DirectX::XMConvertToRadians(DirectX::XMVectorGetY(camRotation)),
	    DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(camRotation))
	);

	// 四元数から角度を伴った単位ベクトルを得るためのZに1が代入された前方向のベクトル
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	/// <summary>
	/// 角度を単位ベクトルで表したもの
	/// </summary>
	DirectX::XMVECTOR targetVec = DirectX::XMVector3Rotate(forward, camQuaternion);

	/// <summary>
	/// カメラがフォーカスをしている現在の座標
	/// </summary>
	DirectX::XMVECTOR targetPos = DirectX::XMVectorAdd(camPosition , targetVec);

	/// <summary>
	/// カメラのシングルトンのインスタンス
	/// </summary>
	static std::unique_ptr<Camera> instance;
};

#endif