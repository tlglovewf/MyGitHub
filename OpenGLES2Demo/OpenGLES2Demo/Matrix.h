#ifndef _MATRIX_H_H
#define _MATRIX_H_H
class Matrix
{
public:
	Matrix();
	~Matrix();
	
	void scale(float sx, float sy, float sz);
	void translate(float tx, float ty, float tz);
	void rotate(float angle, float x, float y, float z);
	void frustum(float left, float right, float bottom, float top, float nearz, float farz);
	void prespective(float fovy, float aspect, float nearz, float farz);
	void ortho(float left, float right, float bottom, float top, float nearz, float farz);

	static void multiMatrix(Matrix &result, const Matrix &srcA, const Matrix &srcB);
	static void loadIdentity(Matrix &mat);

	float* getMatrix(void){ return &m[0][0]; }
private:
	float m[4][4];
};

#endif