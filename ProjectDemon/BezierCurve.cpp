// Fill out your copyright notice in the Description page of Project Settings.


#include "BezierCurve.h"

void UBezierCurve::DrawCurve(FVector point1, FVector point2, FVector point3)
{
	auto P = UBezierCurve::GetBezierPoints(point3, point2, point3);
	auto prev = P[0];
}
void UBezierCurve::DrawSimpleCurve()
{
}
TArray<FVector> UBezierCurve::GetBezierPoints(FVector point0, FVector point1, FVector point2, int NumberOfPoints)
{
	TArray<FVector> points;
	const float T = 1.0;
	auto p0l = point0;
	auto p1l = point1;
	auto p2l = point2;
	auto Q0 = (1 - 0.0) * p0l + 0.0 * p1l;
	auto Q1 = (1 - 0.0) * p1l + 0.0 * p2l;
	auto c = (1 - 0.0) * Q0 + 0.0 * Q1;
	
	double d = T/NumberOfPoints;
	for (int i = 0; i <= NumberOfPoints; i++)
	{
		float n = d * i;
		Q0 = (1 - n) * p0l + n * p1l;
		Q1 = (1 - n) * p1l + n * p2l;
		c = (1 - n) * Q0 + n * Q1;
		points.Add(c);
	}
	return points;
}
FVector UBezierCurve::GetTangentAtPoint(FVector point0, FVector point1, FVector point2, int point , int NumberOfPoints)
{
	TArray<FVector> points = GetBezierPoints(point0, point1, point2 , NumberOfPoints);
	FVector tangent = FVector::ZeroVector;
	if (points.IsValidIndex(point) && points.IsValidIndex(point + 1))
	{
		tangent = points[point + 1] - points[point];
	}
	return tangent;
}
FVector UBezierCurve::GetPointAtRatio(FVector point0, FVector point1, FVector point2, float ratio)
{
	return FMath::Pow(1 - ratio, 2) * point0 + 2* ratio * (1 - ratio) * point1 + ratio * ratio * point2;
}
FVector UBezierCurve::GetTangentAtPoint(TArray<FVector> points, int point)
{
	FVector tangent = FVector::ZeroVector;
	if (points.IsValidIndex(point) && points.IsValidIndex(point + 1))
	{
		tangent = points[point + 1] - points[point];
	}
	return tangent;
}
FVector UBezierCurve::GetTangentAtPoint(TArray<FVector> points, FVector pointlocation)
{
	FVector tangent = FVector::ZeroVector;
	int index = 0;
	for (FVector point : points)
	{
		if (point.Equals(pointlocation))
		{
			if (points.IsValidIndex(index) && points.IsValidIndex(index + 1))
			{
				return points[index + 1] - points[index];
			}
		}
	}
	
	return tangent;
}
float UBezierCurve::GetCurveLength(TArray<FVector> points)
{
	float length = 0.0;
	if (points.IsValidIndex(0) && points.IsValidIndex(1))
	{
		length = FVector::Dist( points[0] , points[1]) * points.Num();
	}
	return length;
}
TArray<FVector> UBezierCurve::GetBezierPoints(TArray<FVector> Points,float subdivisions)
{
	auto ControlPoints = Points;
	auto factorial = [](int num) {
		int ret = 1;
		for (int i = num; i > 0; i--)
		{
			ret *= i;
		}
		return ret;
	};
	auto InitialPoint = FVector::ZeroVector;
	auto num = ControlPoints.Num();
	auto n = num - 1;
	for (size_t i = 0; i < num; i++)
	{
		auto p = ControlPoints[i] * FMath::Pow(0.0, i) * FMath::Pow(1 - 0.0, num - i - 1);
		float top = factorial(num - 1);
		float bottom = factorial(i) * factorial(num - 1 - i);
		auto BiNomCo = top / bottom;

		InitialPoint += p * BiNomCo;

	}

	auto curr = InitialPoint;

	auto inc = 1 / subdivisions;


	for (float t = inc; t <= 1.0; t += inc)
	{
		auto c = FVector::ZeroVector;
		for (size_t i = 0; i < num; i++)
		{
			auto p = ControlPoints[i] * FMath::Pow(t, i) * FMath::Pow(1 - t, num - i - 1);
			int top = factorial(num - 1);
			int bottom = factorial(i) * factorial(num - 1 - i);
			auto BiNomCo = top / bottom;

			c += p * BiNomCo;
		}

		curr = c;
	}
	return TArray<FVector>();
}

int UBezierCurve::Factorial(int num)
{
	int ret = 1;
	for (int i = num; i > 0; i--)
	{
		ret *= i;
	}
	return ret;
}