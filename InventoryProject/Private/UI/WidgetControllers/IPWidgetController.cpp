// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetControllers/IPWidgetController.h"

#include "AbilitySystem/IPAbilitySystemComponent.h"
#include "AbilitySystem/IPAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Components/IPInventorySystemComponent.h"
#include "Player/IPPlayerController.h"
#include "Player/IPPlayerState.h"

void UIPWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
	IPInventorySystemComponent = WCParams.InventorySystemComponent;
	IPQuickSlotComponent = WCParams.QuickSlotComponent;
}

void UIPWidgetController::BroadcastInitialValues()
{
}

void UIPWidgetController::BindCallbacksToDependencies()
{
}


int32 UIPWidgetController::GetInventorySize() 
{
	return GetIPISC()->GetInventorySize();
}


AIPPlayerController* UIPWidgetController::GetIPPC()
{
	if (IPPlayerController == nullptr)
	{
		IPPlayerController = Cast<AIPPlayerController>(PlayerController);
	}
	return IPPlayerController;
}

AIPPlayerState* UIPWidgetController::GetIPPS()
{
	if (IPPlayerState == nullptr)
	{
		IPPlayerState = Cast<AIPPlayerState>(PlayerState);
	}
	return IPPlayerState;
}

UIPAbilitySystemComponent* UIPWidgetController::GetIPASC()
{
	if (IPAbilitySystemComponent == nullptr)
	{
		IPAbilitySystemComponent = Cast<UIPAbilitySystemComponent>(AbilitySystemComponent);
	}
	return IPAbilitySystemComponent;
}

UIPAttributeSet* UIPWidgetController::GetIPAS()
{
	if (IPAttributeSet == nullptr)
	{
		IPAttributeSet = Cast<UIPAttributeSet>(AttributeSet);
	}
	return IPAttributeSet;
}

UIPInventorySystemComponent* UIPWidgetController::GetIPISC()
{
	return IPInventorySystemComponent;
}

UIPQuickSlotComponent* UIPWidgetController::GetIPQSC()
{
	return IPQuickSlotComponent;
}
