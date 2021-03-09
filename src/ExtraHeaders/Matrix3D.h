#pragma once
#include <YRPP.h>

template <typename T> class Vector4D
{
public:
	static const Vector4D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	T X, Y, Z, W;

	//methods are WIP
};

template <typename T>
const Vector4D<T> Vector4D<T>::Empty = { T(), T(), T(), T() };

class NOVTABLE Matrix3D
{
public:
	union
	{
		Vector4D<float> Row[3];
		float row[3][4];
		float Data[12];
	};

	// plain floats ctor
	Matrix3D::Matrix3D(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
		{ JMP_THIS(0x5AE630); }

	// column vector ctor
	Matrix3D::Matrix3D(
		Vector3D<float> const& vec0,
		Vector3D<float> const& vec1,
		Vector3D<float> const& vec2,
		Vector3D<float> const& vec3)
		{ JMP_THIS(0x5AE690); }

	// some other rotation ctor?
	Matrix3D::Matrix3D(float unknown1, float unknown2)
		{ JMP_THIS(0x5AE6F0); }

	// rotation ctor
	Matrix3D::Matrix3D(Vector3D<float>* axis, float angle)
		{ JMP_THIS(0x5AE750); }

	// copy ctor
	Matrix3D::Matrix3D(Matrix3D* copyOf)
		{ JMP_THIS(0x5AE610); }

	// 1-matrix
	void Matrix3D::MakeIdentity()
		{ JMP_THIS(0x5AE860); }

	void Matrix3D::Translate(float x, float y, float z)
		{ JMP_THIS(0x5AE890); }

	void Matrix3D::Translate(Vector3D<float> const& vec)
		{ JMP_THIS(0x5AE8F0); }

	void Matrix3D::TranslateX(float x)
		{ JMP_THIS(0x5AE980); }

	void Matrix3D::TranslateY(float y)
		{ JMP_THIS(0x5AE9B0); }

	void Matrix3D::TranslateZ(float z)
		{ JMP_THIS(0x5AE9E0); }
	/*
	void __thiscall Matrix3D::Scale(float factor)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::Scale(float x, float y, float z)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ScaleX(float factor)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ScaleY(float factor)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ScaleZ(float factor)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ShearYZ(float y, float z)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ShearXY(float x, float y)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::ShearXZ(float x, float z)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::PreRotateX(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::PreRotateY(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::PreRotateZ(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateX(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateX(float s, float c)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateY(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateY(float s, float c)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateZ(float theta)
		{ JMP_THIS(0x); }

	void __thiscall Matrix3D::RotateZ(float s, float c)
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetXVal()
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetYVal()
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetZVal()
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetXRotation()
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetXVal()
		{ JMP_THIS(0x); }

	float __thiscall Matrix3D::GetXVal()
		{ JMP_THIS(0x); }

	Vector3D<float> __thiscall Matrix3D::RotateVector(Vector3D<float> const& vec)
		{ JMP_THIS(0x); }

	// TODO add everything else

	*/
};