// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "IPAbilitySystemComponent.generated.h"

/**
 * 
 */
class ULoadScreenSaveGame;
class UAbilityInfo;
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTagsDelegate, const FGameplayTagContainer& /*AssetTags*/);
DECLARE_MULTICAST_DELEGATE(FOnAbilitiesGivenDelegate);
DECLARE_DELEGATE_OneParam(FForEachAbilityDelegate, const FGameplayAbilitySpec&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAbilityStatusChangedDelegate, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnAbilityEquippedDelegate, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeactivatePassiveAbilityDelegate, const FGameplayTag& /*AbilityTag*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActivatePassiveEffectDelegate, const FGameplayTag& /*AbilityTag*/, bool /*bActivate*/);

UCLASS()
class INVENTORYPROJECT_API UIPAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void AbilityActorInfoSet();

	FEffectAssetTagsDelegate EffectAssetTagsDelegate;
 	FOnAbilitiesGivenDelegate OnAbilitiesGivenDelegate;
 	FOnAbilityStatusChangedDelegate OnAbilityStatusChangedDelegate;
 	FOnAbilityEquippedDelegate OnAbilityEquippedDelegate;

	void AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData);
 	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
	void AddCharacterAbilities_forBoss(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
 	bool bStartupAbilitiesGiven = false;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
 	void ForEachAbility(const FForEachAbilityDelegate& Delegate);

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
	FGameplayTag GetSlotFromAbilityTag(const FGameplayTag& AbilityTag);
	bool IsSlotEmpty(const FGameplayTag& Slot);
	static bool DoesAbilityHaveSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);
	static bool DoesAbilityHaveAnySlot(const FGameplayAbilitySpec& Spec);
	FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);
	static void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);

	UFUNCTION()
 	void UpgradeAttribute(const FGameplayTag& AttributeTag);
	
	void UpdateAbilitiesStatusAtLevelChange(int32 Level);

	UFUNCTION()
	void SpendSkillPoint(const FGameplayTag& AbilityTag);

	UFUNCTION()
	void UpdateAbilitiesInfo();

	void EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);
	void AddCharacterAllAbilities();
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription);

 	static void ClearSlot(FGameplayAbilitySpec* Spec);
 	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);
 	static bool DoesAbilityHaveSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);
	bool IsAbilityQualified(const FGameplayTag& AbilityTag,  const UAbilityInfo* AbilityInfo, const int32 PlayerLevel);
	
protected:
	//UFUNCTION() //EffectAssetTags를 Broadcast하는 함수. OverlayWidgetController에서 메시지 위젯 출력하도록 한다.
	//void EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

private:
	UPROPERTY()
	TMap<FGameplayTag, int32> CurrentAbilitiesLevel;//어빌리티의 스킬 포인트 수 체크하는 맵
};
