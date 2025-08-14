#include "Matrix4.h"


namespace Bamboo {

    Matrix4 Matrix4::operator*(const Matrix4 &other) const 
    {
        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m_data[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result.m_data[i * 4 + j] += m_data[i * 4 + k] * other.m_data[k * 4 + j];
                }
            }
        }
        return result;
    }

    Matrix4 Matrix4::operator+(const Matrix4 &other) const {
        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m_data[i] = m_data[i] + other.m_data[i];
        }
        return result;
    }

    Matrix4 Matrix4::operator-(const Matrix4 &other) const {
        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m_data[i] = m_data[i] - other.m_data[i];
        }
        return result;
    }

    Matrix4 Matrix4::operator/(float scalar) const {
        if(scalar == 0.0f){
            //TODO: throw exception
            return *this;
        }

        Matrix4 result;
        for (int i = 0; i < 16; i++) {
            result.m_data[i] = m_data[i] / scalar;
        }
        return result;
    }

    Matrix4 Matrix4::Transpose() const {
        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m_data[i * 4 + j] = m_data[j * 4 + i];
            }
        }
        return result;
    }

    float Matrix4::Determinant() const {
        float det = 0.0f;
        for (int i = 0; i < 4; i++) {
            det += m_data[i * 4 + 0] * (m_data[1 + (i + 1) % 4] * m_data[5 + (i + 2) % 4] - m_data[2 + (i + 2) % 4] * m_data[4 + (i + 1) % 4]);
        }
        return det;
    }

    Matrix4 Matrix4::Inverse() const {
        float det = Determinant();
        if (det == 0.0f) {
            //TODO: throw exception
            return *this;
        }

        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                float sign = ((i + j) % 2 == 0) ? 1.0f : -1.0f;
                float cofactor = (m_data[(j + 1) % 4 * 4 + i] * m_data[(j + 2) % 4 * 4 + (i + 1) % 4] - m_data[(j + 2) % 4 * 4 + i] * m_data[(j + 1) % 4 * 4 + (i + 1) % 4]) / det;
                result.m_data[i * 4 + j] = sign * cofactor;
            }
        }
        return result;
    }
}