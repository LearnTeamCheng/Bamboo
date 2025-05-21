#pragma once

#include <cmath>
#include <algorithm>
#include "MathDefs.h"
namespace Bamboo::Math
{

		inline bool IsZero(float x)
		{
			return x < B_EPSILON && x > -B_EPSILON;
		}

		inline bool IsEqual(float a, float b, float epsilon)
		{
			return std::fabs(a - b) < epsilon;
		}

		// 限制x在min和max之间
		inline float Clamp(float x, float min, float max)
		{
			return std::fmax(std::fmin(x, max), min);
		}

		// 线性插值 a + (b - a) * t
		inline float Lerp(float a, float b, float t)
		{
			return a + (b - a) * t;
		}

		//绝对值
		inline float Abs(float x)
		{
			return std::fabs(x);
		}

		inline int Abs(int x)
		{
			return std::abs(x);
		}

		inline float Sqrt(float x)
		{
			return std::sqrtf(x);
		}

		inline float Floor(float x)
		{
			return std::floorf(x);
		}

		inline float Ceil(float x)
		{
			return std::ceilf(x);
		}

		inline float Round(float x)
		{
			return std::roundf(x);
		}

		inline float Sin(float x)
		{
			return std::sinf(x);
		}

		inline float Cos(float x)
		{
			return std::cosf(x);
		}

		inline float Tan(float x)
		{
			return std::tanf(x);
		}

		inline float Asin(float x)
		{
			return std::asinf(x);
		}

		inline float Acos(float x)
		{
			return std::acosf(x);
		}

		inline float Atan(float x)
		{
			return std::atanf(x);
		}

		inline float Atan2(float y, float x)
		{
			return std::atan2f(y, x);
		}

		inline float Pow(float x, float y)
		{
			return std::powf(x, y);
		}

		//最大值
		inline float Max(float a, float b)
		{
			return std::fmax(a, b);
		}

		inline int Max(int a, int b)
		{
			return std::max(a, b);
		}

		inline float Min(float a, float b)
		{
			return std::fmin(a, b);
		}

		inline int Min(int a, int b)
		{
			return std::min(a, b);
		}
	

}; // namespace Bamboo
