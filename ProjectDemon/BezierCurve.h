// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UBezierCurve
{
	TArray<FVector> UpdatedPoints;
	int lagFramesBy = 15;
	int Slowdown = 0;
	bool bEnableSnakeManRegular = 0;
	float initialDistance = 0;
	bool bEnableOscilliation = false;
	bool bEnableLegWhip = false;
	bool bEnaableLookAtLocation;
	bool bEnableEndPointForwardBezier = false;
	bool bEnableRandomArmToLocation = false;
	/// <summary>
	/// Will Draw Curves using the higher order points. 
	/// Don't recommended because it comes weird.
	/// </summary>
	void DrawCurve();
	void DrawCurve(FVector p1, FVector p2, FVector p3);
	void DrawSimpleCurve();
	bool bEnableBoneReverseStretch = false;

public:	
	static TArray<FVector> GetBezierPoints(FVector p0, FVector p1, FVector p2,int NumberOfPoints = 20);
	static FVector GetTangentAtPoint(FVector point0, FVector point1, FVector point2, int point, int NumberOfPoints);
	static FVector GetPointAtRatio(FVector point0, FVector point1, FVector point2, float ratio);
	static FVector GetTangentAtPoint(TArray<FVector> points, int point);
	static FVector GetTangentAtPoint(TArray<FVector> points, FVector pointlocation);
	static float GetCurveLength(TArray<FVector> points);
	static TArray<FVector> GetBezierPoints(TArray<FVector>, float Subdivision = 15);
	int Factorial(int num);
	/// <summary>
	/// Must enter three points in order to function. Make sure to Use ActivateCurves function to display any thing. Doesn't have to be bone names. 
	/// Returns if succesfull or not.
	/// </summary>
	/// <param name="P1">
	/// Initial point
	/// </param>
	/// <param name="P2">
	/// The middle portion. 
	/// </param>
	/// <param name="P3">
	/// Will be used to stetch and point
	/// </param>

	void SetDistance(float);
	void ResetParameters();

};
