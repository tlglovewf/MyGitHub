#include "Matrix.h"
#include "stdlib.h"
#include <string.h>
#include <math.h>

const float PI = 3.141592657;
Matrix::Matrix()
{
	memset(this, 0, sizeof(Matrix));
}


Matrix::~Matrix()
{
}

void Matrix::scale(float sx, float sy, float sz)
{
	this->m[0][0] *= sx;
	this->m[0][1] *= sx;
	this->m[0][2] *= sx;
	this->m[0][3] *= sx;

	this->m[1][0] *= sy;
	this->m[1][1] *= sy;
	this->m[1][2] *= sy;
	this->m[1][3] *= sy;

	this->m[2][0] *= sz;
	this->m[2][1] *= sz;
	this->m[2][2] *= sz;
	this->m[2][3] *= sz;
}

void Matrix::translate(float tx, float ty, float tz)
{
	this->m[0][3] += tx;
	this->m[1][3] += ty;
	this->m[2][3] += tz;
}

void Matrix::rotate(float angle, float x, float y, float z)
{
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	sinAngle = sinf(angle * PI / 180.0f);
	cosAngle = cosf(angle * PI / 180.0f);
	if (mag > 0.0f)
	{
		float xx, yy, zz, xy, yz, zx, xs, ys, zs;
		float oneMinusCos;
		Matrix rotMat;

		x /= mag;
		y /= mag;
		z /= mag;

		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;

		rotMat.m[0][0] = (oneMinusCos * xx) + cosAngle;
		rotMat.m[0][1] = (oneMinusCos * xy) - zs;
		rotMat.m[0][2] = (oneMinusCos * zx) + ys;
		rotMat.m[0][3] = 0.0F;

		rotMat.m[1][0] = (oneMinusCos * xy) + zs;
		rotMat.m[1][1] = (oneMinusCos * yy) + cosAngle;
		rotMat.m[1][2] = (oneMinusCos * yz) - xs;
		rotMat.m[1][3] = 0.0F;

		rotMat.m[2][0] = (oneMinusCos * zx) - ys;
		rotMat.m[2][1] = (oneMinusCos * yz) + xs;
		rotMat.m[2][2] = (oneMinusCos * zz) + cosAngle;
		rotMat.m[2][3] = 0.0F;

		rotMat.m[3][0] = 0.0F;
		rotMat.m[3][1] = 0.0F;
		rotMat.m[3][2] = 0.0F;
		rotMat.m[3][3] = 1.0F;

		multiMatrix(*this, rotMat, *this);
	}
}

void Matrix::frustum(float left, float right, float bottom, float top, float nearz, float farz)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farz - nearz;

	loadIdentity(*this);
	this->m[0][0] = 2 * nearz / deltaX;
	this->m[2][0] = (right + left) / deltaX;
	this->m[1][1] = 2 * nearz / deltaY;
	this->m[2][1] = (top + bottom) / deltaY;
	this->m[2][2] = -(farz + nearz) / deltaZ;
	this->m[3][2] = -2 * farz * nearz / deltaZ;
	this->m[2][3] = -1;
	this->m[3][3] = 0;
}

void Matrix::prespective(float fovy, float aspect, float nearz, float farz)
{
	float frustumW, frustumH;
	frustumH = tanf(fovy / 360.0f * 3.14159265) * nearz;
	frustumW = frustumH * aspect;

	frustum( -frustumW, frustumW, -frustumH, frustumH, nearz, farz);
}

void Matrix::ortho(float left, float right, float bottom, float top, float nearz, float farz)
{
	float       deltaX = right - left;
	float       deltaY = top - bottom;
	float       deltaZ = farz - nearz;

	if ((deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f))
		return;

	loadIdentity(*this);
	
	this->m[0][0] = 2.0 / deltaX;
	this->m[3][0] = -(right + left) / deltaX;
	this->m[1][1] = 2 / deltaY;
	this->m[3][1] = -(top + bottom) / (top - bottom);
	this->m[2][2] = -2 / deltaZ;
	this->m[3][2] = -(farz + nearz) / nearz;
}

void Matrix::multiMatrix(Matrix &result, const Matrix &srcA, const Matrix &srcB)
{
	Matrix tmp;
	int i = 0;
	for (i = 0; i < 4; ++i)
	{
		tmp.m[i][0] = (srcA.m[i][0] * srcB.m[0][0]) +
			(srcA.m[i][1] * srcB.m[1][0]) +
			(srcA.m[i][2] * srcB.m[2][0]) +
			(srcA.m[i][3] * srcB.m[3][0]);

		tmp.m[i][1] = (srcA.m[i][0] * srcB.m[0][1]) +
			(srcA.m[i][1] * srcB.m[1][1]) +
			(srcA.m[i][2] * srcB.m[2][1]) +
			(srcA.m[i][3] * srcB.m[3][1]);

		tmp.m[i][2] = (srcA.m[i][0] * srcB.m[0][2]) +
			(srcA.m[i][1] * srcB.m[1][2]) +
			(srcA.m[i][2] * srcB.m[2][2]) +
			(srcA.m[i][3] * srcB.m[3][2]);

		tmp.m[i][3] = (srcA.m[i][0] * srcB.m[0][3]) +
			(srcA.m[i][1] * srcB.m[1][3]) +
			(srcA.m[i][2] * srcB.m[2][3]) +
			(srcA.m[i][3] * srcB.m[3][3]);
	}
	memcpy(&result, &tmp, sizeof(Matrix));
}

void Matrix::loadIdentity(Matrix &mat)
{
	memset(&mat, 0, sizeof(Matrix));
	mat.m[0][0] = 1.0f;
	mat.m[1][1] = 1.0f;
	mat.m[2][2] = 1.0f;
	mat.m[3][3] = 1.0f;
}
