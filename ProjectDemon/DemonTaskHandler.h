// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class PROJECTDEMON_API DemonTaskHandler : UObject
{
private:
	static FTimerHandle timerHandler;
public:
	DemonTaskHandler();
	~DemonTaskHandler();
	
	static void SubmitTask(TFunction<void()> func);
	static void SubmitTask(ENamedThreads::Type namedThread, TFunction<void()> func);
	static void ThreadWait(float seconds);
};
