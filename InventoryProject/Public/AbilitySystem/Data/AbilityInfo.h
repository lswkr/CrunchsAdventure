// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

/**
 * 
 */
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAbilityStackInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 StackSize = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

USTRUCT(BlueprintType)
struct FAbilityLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RequiredLevel = 0;
};

USTRUCT(BlueprintType)
struct FIPAbilityInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AbilityName = TEXT("None");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// TArray<FAbilityStackInfo> PreAbilityTags = TArray<FAbilityStackInfo>();
	
	/*추가 Start*/
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// TMap<FGameplayTag, int32> PreAbilityMap = TMap<FGameplayTag, int32>();
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	// TMap<FGameplayTag, int32> PostAbilityMap = TMap<FGameplayTag, int32>();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAbilityLevelInfo> PreAbilitiesRequiringLevelInfo;
	
	/*추가 End*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FGameplayTag> PostAbilityTags = TArray<FGameplayTag>();
	
	// UPROPERTY(BlueprintReadOnly)
	// FGameplayTag InputTag = FGameplayTag();

	// UPROPERTY(BlueprintReadOnly)
	// FGameplayTag StatusTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();

	// UPROPERTY(BlueprintReadOnly)
	// int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PlayerLevelRequirement = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) //패시브, 액티브 등
	FGameplayTag AbilityType = FGameplayTag();

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) //AbilityInfo의 어빌리티 맵 사용하기 위해 폐기
	// TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine="true"))
	FString EquippedExplanation = FString();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine="true"))
	FString RequirementExplanation = FString();
};

UCLASS()
class INVENTORYPROJECT_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> AbilityMap;

	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	// TArray<FIPAbilityInfo> AbilityInformation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TMap<FGameplayTag, FIPAbilityInfo> AbilityInfoMap;
	
	UFUNCTION(BlueprintCallable, Category = "AbilityInformation")
	FIPAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;

	UFUNCTION()
	TSubclassOf<UGameplayAbility> GetAbilityFromTag(const FGameplayTag& AbilityTag) const;

	UFUNCTION()
	bool IsPreAbilitiesAndPlayerLevelQualified(const FGameplayTag& AbilityTag, const TMap<FGameplayTag, int>& CurrentAbilitiesStatus, const int32 PlayerLevel) const;
};
