// Fill out your copyright notice in the Description page of Project Settings.

#include "Kismet/GameplayStatics.h"
#include "DemonTaskHandler.h"

DemonTaskHandler::DemonTaskHandler()
{
}

DemonTaskHandler::~DemonTaskHandler()
{
}

void DemonTaskHandler::SubmitTask(TFunction<void()> func)
{
	AsyncTask(ENamedThreads::AnyHiPriThreadNormalTask, func);
}
void DemonTaskHandler::SubmitTask(ENamedThreads::Type namedThread, TFunction<void()> func)
{
	AsyncTask(namedThread, func);
}
void DemonTaskHandler::ThreadWait(float Seconds)
{
	FPlatformProcess::Sleep(Seconds);
}
