// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/IPAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "IPGameplayTags.h"
#include "AbilitySystem/IPAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/IPGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interfaces/CombatInterface.h"
#include "Interfaces/PlayerInterface.h"

void UIPAbilitySystemComponent::AbilityActorInfoSet()
{
	FIPGameplayTags GameplayTags = FIPGameplayTags::Get();

	for (const TPair<FGameplayTag, FIPAbilityInfo>& Info : UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor())->
	     AbilityInfoMap)
	{
		CurrentAbilitiesLevel.Add(Info.Key, 0);
	}
	//OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UIPAbilitySystemComponent::EffectApplied);
}

void UIPAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	UE_LOG(LogTemp, Warning, TEXT("AddCharacterAbilitiesFromSaveData Activated"));
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		UE_LOG(LogTemp, Warning, TEXT("SavedAbility: %s"), *Data.AbilityTag.ToString());
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);

		if (Data.AbilityType == FIPGameplayTags::Get().Abilities_Type_Active) //다른 어빌리티는 넣기만
		{
			//UE_LOG(LogTemp, Warning, TEXT("saved Ability: %s"), *Data.AbilityTag.ToString());
			GiveAbility(LoadedAbilitySpec);
		}
		else //ListenForEvent같은 PassiveAbility는 바로 Activate 
		{
			GiveAbilityAndActivateOnce(LoadedAbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	OnAbilitiesGivenDelegate.Broadcast();
}

void UIPAbilitySystemComponent::AddCharacterAllAbilities()
{
	UAbilityInfo* AbilityInfo = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	if (AbilityInfo)
	{
		for (const TPair<FGameplayTag, TSubclassOf<UGameplayAbility>> AbilityClass : AbilityInfo->AbilityMap)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass.Value, 1);
			if (const UIPGameplayAbility* IPAbility = Cast<UIPGameplayAbility>(AbilitySpec.Ability))
			{
				AbilitySpec.DynamicAbilityTags.AddTag(IPAbility->StartupInputTag);
				AbilitySpec.DynamicAbilityTags.AddTag(FIPGameplayTags::Get().Abilities_Status_Equipped); //시작 스킬은 바로 장착상태

				GiveAbility(AbilitySpec);
			}
		}
	}
}
void UIPAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UIPGameplayAbility* IPAbility = Cast<UIPGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(IPAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FIPGameplayTags::Get().Abilities_Status_Equipped); //시작 스킬은 바로 장착상태

			GiveAbility(AbilitySpec);
		}
	}

	if (!bStartupAbilitiesGiven) //시작 때에만 한 번 브로드캐스트하고 그 이후는 없도록
	{
		bStartupAbilitiesGiven = true;
		OnAbilitiesGivenDelegate.Broadcast();
	}
}

void UIPAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FIPGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec); //패시브 스킬은 처음에 한 번 써서 지속적으로 적용되도록
	}
}

void UIPAbilitySystemComponent::AddCharacterAbilities_forBoss(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UIPGameplayAbility* IPAbility = Cast<UIPGameplayAbility>(AbilitySpec.Ability))
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s"),*IPAbility->GetName());;
			GiveAbility(AbilitySpec);
		}
	}
}

void UIPAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle,
				                      AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

void UIPAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UIPAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle,
			                      AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UIPAbilitySystemComponent::ForEachAbility(const FForEachAbilityDelegate& Delegate) //원하는 델리게이트 넣어서 모든 어빌리티에 적용하는 함수
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
	//UpdateAbilitiesInfo();
}

void UIPAbilitySystemComponent::UpdateAbilitiesInfo()
{
	UAbilityInfo* AbilityInfo = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	FIPGameplayTags GameplayTags = FIPGameplayTags::Get();

	for (const TPair<FGameplayTag, FIPAbilityInfo>& Info : AbilityInfo->AbilityInfoMap)
	{
		FGameplayTag Status = GetStatusFromAbilityTag(Info.Key);
		if (Status != GameplayTags.Abilities_Status_Equipped && Status != GameplayTags.Abilities_Status_Unlocked &&
			Status != GameplayTags.Abilities_Status_Eligible)
		{
			OnAbilityStatusChangedDelegate.Broadcast(Info.Key, Status, 0);
		}
		else
		{
			OnAbilityStatusChangedDelegate.Broadcast(Info.Key, Status, GetSpecFromAbilityTag(Info.Key)->Level);
		}
	}
}

FGameplayTag UIPAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag UIPAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UIPAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* UIPAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this); //어빌리티 변경 방지 락(반복문 도는 동안 변경 없도록)
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

FGameplayTag UIPAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag UIPAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

bool UIPAbilitySystemComponent::DoesAbilityHaveSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

bool UIPAbilitySystemComponent::DoesAbilityHaveSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
//포인터 쓰이는 상황에 사용(오류 방지)
{
	for (FGameplayTag Tag : Spec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}


bool UIPAbilitySystemComponent::DoesAbilityHaveAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

bool UIPAbilitySystemComponent::IsSlotEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (DoesAbilityHaveSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}


FGameplayAbilitySpec* UIPAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

void UIPAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.DynamicAbilityTags.AddTag(Slot);
}

void UIPAbilitySystemComponent::UpdateAbilitiesStatusAtLevelChange(int32 Level) //레벨 업 했을 때 전체 스킬 업데이트
{
	const UAbilityInfo* AbilityInfo = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	int idx = 0; //디버그 로그용 변수

	for (const TPair<FGameplayTag, FIPAbilityInfo>& Info : AbilityInfo->AbilityInfoMap)
	{
		idx++;
		if (!Info.Key.IsValid()) //어빌리티 태그 제대로 설정 안 해둔 경우
		{
			UE_LOG(LogTemp, Warning, TEXT("#%d Ability has no valid tag. Please set AbilityTag at AbilityInfo."), idx);
			continue;
		}
		// if (Level < Info.Value.PlayerLevelRequirement) continue;
		//UE_LOG(LogTemp, Warning, TEXT("is %s qualified?"),*Info.Key.ToString());
		if (IsAbilityQualified(Info.Key, AbilityInfo, Level))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Yes! %s is qualified"),*Info.Key.ToString());
			FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(Info.Key);

			if (Spec == nullptr) //아직 등록되지 않은 경우
			{
				TSubclassOf<UGameplayAbility> AbilityClass = AbilityInfo->GetAbilityFromTag(Info.Key);
				FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
				AbilitySpec.DynamicAbilityTags.AddTag(FIPGameplayTags::Get().Abilities_Status_Eligible);
				//Info.StatusTag = FIPGameplayTags::Get().Abilities_Status_Eligible;
				GiveAbility(AbilitySpec);
				//MarkAbilitySpecDirty(AbilitySpec);

				OnAbilityStatusChangedDelegate.Broadcast(Info.Key, FIPGameplayTags::Get().Abilities_Status_Eligible, 1);
			}
			// else  //폐기 예정-사유: 레벨업 할 때의 시점에서는 어빌리티를 새로 등록하는 행동 밖에 할 것이 없다. 즉, 기존에 등록된 것을 다시 찾올 이유가 없는 것이다.
			// {
			// 	FGameplayTag CurrentStatus = GetStatusFromAbilityTag(Info.Key);
			// 	AbilityStatusChanged.Broadcast(Info.Key, CurrentStatus, Spec->Level);
			// }
		}
	}
	// for (FIPAbilityInfo& Info : AbilityInfo->AbilityInformation)
	// {
	// 	idx++;
	// 	if (!Info.AbilityTag.IsValid())//어빌리티 태그 제대로 설정 안 해둔 경우
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("#%d Ability has no valid tag. Please set AbilityTag at AbilityInfo."), idx);
	// 		
	// 		continue;
	// 	}
	// 	if (Level < Info.LevelRequirement) continue;
	// 	
	// 	if (IsAbilityQualified(Info.AbilityTag, AbilityInfo))
	// 	{
	// 		//UE_LOG(LogTemp, Warning, TEXT("%s is qualified"),*Info.AbilityTag.ToString());
	// 		FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(Info.AbilityTag);
	// 		
	// 		if (Spec == nullptr) //아직 등록되지 않은 경우
	// 		{
	// 			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
	// 			AbilitySpec.DynamicAbilityTags.AddTag(FIPGameplayTags::Get().Abilities_Status_Eligible);
	// 			//Info.StatusTag = FIPGameplayTags::Get().Abilities_Status_Eligible;
	// 			GiveAbility(AbilitySpec);
	// 			//MarkAbilitySpecDirty(AbilitySpec);
	// 			
	// 			AbilityStatusChanged.Broadcast(Info.AbilityTag,FIPGameplayTags::Get().Abilities_Status_Eligible, 1);
	// 		}
	// 		else //등록된 경우
	// 		{
	// 			//TODO: Spec찾아서 StatusTag추가하기
	// 			FGameplayTag CurrentStatus = GetStatusFromAbilityTag(Info.AbilityTag);
	// 			Info.StatusTag = CurrentStatus;
	// 			AbilityStatusChanged.Broadcast(Info.AbilityTag,CurrentStatus, Spec->Level);
	// 		}
	// 	}
	// }
}

void UIPAbilitySystemComponent::SpendSkillPoint(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		int32 CurrentPlayerLevel = 0;
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSkillPoints(GetAvatarActor(), -1);
		}
		if (GetAvatarActor()->Implements<UCombatInterface>())
		{
			CurrentPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(GetAvatarActor());
			UE_LOG(LogTemp, Warning, TEXT("CurrentPlayerLevel: %d"),CurrentPlayerLevel);
		}

		const FIPGameplayTags GameplayTags = FIPGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible)) //장착 가능 상태에서 스킬 포인트 찍을 시 Unlocked
		{
			//UE_LOG(LogTemp,Warning, TEXT("%s is Eligible"), *AbilityTag.ToString());
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped)
			|| Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			//어빌리티 레벨 1 이후에는 레벨 올리기
			AbilitySpec->Level++;
		}
		CurrentAbilitiesLevel[AbilityTag]++;
		OnAbilityStatusChangedDelegate.Broadcast(AbilityTag, Status, AbilitySpec->Level);
		//MarkAbilitySpecDirty(*AbilitySpec);//멀티플레이 구현 시 이 라인 추가하기

		//TArray<FIPAbilityInfo> AbilityInfoArray = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor())->AbilityInformation;
		UAbilityInfo* AbilityInfo = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

		if (AbilityInfo)
		{
			for (const FGameplayTag& PostAbilityTag : AbilityInfo->AbilityInfoMap[AbilityTag].PostAbilityTags)
			{
				const FGameplayTag& PostAbilityStatus = GetStatusFromAbilityTag(PostAbilityTag);
				if (PostAbilityStatus == GameplayTags.Abilities_Status_Equipped || PostAbilityStatus == GameplayTags.Abilities_Status_Unlocked) //이미 해제된 상태라면
					continue;

				if (AbilityInfo->IsPreAbilitiesAndPlayerLevelQualified(PostAbilityTag, CurrentAbilitiesLevel, CurrentPlayerLevel)) //현재 PostAbility의 PreAbility가 모두 레벨을 충족한다면
				{
					if (GetSpecFromAbilityTag(PostAbilityTag) == nullptr)
					{
						TSubclassOf<UGameplayAbility> PostAbilitySubclass = AbilityInfo->GetAbilityFromTag(PostAbilityTag);
						checkf(PostAbilitySubclass != nullptr, TEXT("AbilitySubclass isn't set at AbilityInfo"));
						FGameplayAbilitySpec PostSpec = FGameplayAbilitySpec(PostAbilitySubclass, 1); //초기 Level1
						PostSpec.DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Eligible);
						
						GiveAbility(PostSpec);
						OnAbilityStatusChangedDelegate.Broadcast(AbilityTag,GameplayTags.Abilities_Status_Eligible, 0);
					}
				}
			}
		}
		//선행 스킬 레벨 변경 후 조건맞는 Post스킬 ASC에 넣기
		// for (const FIPAbilityInfo& Info : AbilityInfoArray)
		// {
		// 	if (Info.AbilityTag == AbilityTag)
		// 	{
		// 		for (FAbilityStackInfo StackInfo : Info.PostAbilityTags)
		// 		{
		// 			if (IsAbilityQualified(StackInfo.AbilityTag, AbilityInfo))
		// 			{
		// 				if (GetSpecFromAbilityTag(StackInfo.AbilityTag) == nullptr)
		// 				{
		// 					FGameplayAbilitySpec PostSpec = FGameplayAbilitySpec(StackInfo.Ability, 1); //초기 Level1
		// 					PostSpec.DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Eligible);
		//
		// 					GiveAbility(PostSpec);
		// 					//MarkAbilitySpecDirty(PostSpec);
		// 					OnAbilityStatusChangedDelegate.Broadcast(StackInfo.AbilityTag, GameplayTags.Abilities_Status_Eligible, 0);
		// 				}
		// 			}
		// 		}
		// 	}
		// }
		UpdateAbilitiesInfo();
	}
}

void UIPAbilitySystemComponent::EquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& NewSlot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) //메뉴창에서 고른 스킬
	{
		const FIPGameplayTags& GameplayTags = FIPGameplayTags::Get();
		//Equip하려는 Ability의 현 상태
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.
			Abilities_Status_Unlocked;

		if (bStatusValid) //등록이 가능한 어빌리티인지 확인
		{
			if (!IsSlotEmpty(NewSlot)) //넣으려는 슬롯(InputTag)에 이미 Ability가 있다면 지우기
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(NewSlot); //Slot을 점유하고 있는 어빌리티 찾기

				if (SpecWithSlot) //그 슬롯에 어빌리티가 있다면
				{
					//해당 슬롯에 있는 어빌리티가 등록하려는 어빌리티와 같다면 빠른 return;
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						OnAbilityEquippedDelegate.Broadcast(AbilityTag, GameplayTags.Abilities_Status_Equipped, NewSlot,
						                                    PrevSlot);
						return;
					}
					//다르다면 그 어빌리티를 슬롯에서 해제
					else
					{
						//기존에 ClearSlot이었으나 같은 탐색 연산이 한 번 더 있어 단일 연산으로 변경 
						SpecWithSlot->DynamicAbilityTags.RemoveTag(NewSlot); //슬롯 해제
						SpecWithSlot->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Equipped); //장착된 상태 해제
						SpecWithSlot->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
					}
				}
			}
			//슬롯 비우고 난 이후
			if (!DoesAbilityHaveAnySlot(*AbilitySpec)) //해당 어빌리티가 어떤 슬롯에도 등록되어 있지 않다면
			{
				AbilitySpec->DynamicAbilityTags.RemoveTag(Status); //기존 상태를 지우고
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped); //장착된 상태로 변경
			}
			AssignSlotToAbility(*AbilitySpec, NewSlot); //슬롯에 장착(InputTag부여)
			//MarkAbilitySpecDirty(*AbilitySpec);
			OnAbilityEquippedDelegate.Broadcast(AbilityTag, GameplayTags.Abilities_Status_Equipped, NewSlot, PrevSlot);
		}
	}
}

void UIPAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UIPAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (DoesAbilityHaveSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}


bool UIPAbilitySystemComponent::IsAbilityQualified(const FGameplayTag& AbilityTag, const UAbilityInfo* AbilityInfo, const int32 PlayerLevel)
{
	FIPGameplayTags GameplayTags = FIPGameplayTags::Get();

	FGameplayTag Status = GetStatusFromAbilityTag(AbilityTag);
	if (Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked)
		return true; //이미 해제된 스킬이라면 true 

	return AbilityInfo->IsPreAbilitiesAndPlayerLevelQualified(AbilityTag, CurrentAbilitiesLevel, PlayerLevel);
	for (const FAbilityLevelInfo AbilityLevelInfo : AbilityInfo->AbilityInfoMap[AbilityTag].
	     PreAbilitiesRequiringLevelInfo) //선행 스킬 자격요건 충족하는지 찾기
	{
		if (CurrentAbilitiesLevel[AbilityTag] < AbilityLevelInfo.RequiredLevel) return false; //레벨 낮은 것이 있다면 false 
	}
	return true; //모두 레벨제한을 넘으면 true
	// 	
	// for (const FIPAbilityInfo& Info : AbilityInfo->AbilityInformation)
	// {
	// 	if (Info.AbilityTag == AbilityTag)
	// 	{
	// 		FGameplayTag Status = GetStatusFromAbilityTag(AbilityTag);
	// 		
	// 		if (Status ==  GameplayTags.Abilities_Status_Equipped || Status ==  GameplayTags.Abilities_Status_Unlocked) return true; //이미 처리된 경우 빠른 리턴
	//
	//
	// 		for (const FAbilityStackInfo& StackInfo : Info.PreAbilityTags) //선행 스킬의 조건을 모두 충족시키는지 확인
	// 		{
	// 			if (CurrentAbilitiesLevel[StackInfo.AbilityTag] < StackInfo.StackSize) //충족 못 시킨 경우 false
	// 			{
	// 			//	UE_LOG(LogTemp, Warning, TEXT("%s is not qualified"),*Info.AbilityTag.ToString());
	// 				return false;
	// 			}
	// 			
	// 		}
	// 		return true;//모두 충족 시켰으면 true
	// 	}
	// }
	// return false;
}

void UIPAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0) //포인트가 0보다 많다면
		{
			FGameplayEventData Payload;
			Payload.EventTag = AttributeTag;
			Payload.EventMagnitude = 1.f; //해당 Attribute에 +1

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

			if (GetAvatarActor()->Implements<UPlayerInterface>())
			{
				IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1); //총 포인트에서 -1
			}
		}
	}
}

// void UIPAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent,
//                                               const FGameplayEffectSpec& EffectSpec,
//                                               FActiveGameplayEffectHandle ActiveEffectHandle)
// {
// 	FGameplayTagContainer TagContainer;
// 	EffectSpec.GetAllAssetTags(TagContainer);
//
// 	EffectAssetTagsDelegate.Broadcast(TagContainer);
// }

// bool UIPAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription)
// {
// 	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
// 	{
// 		if(UIPGameplayAbility* AuraAbility = Cast<UIPGameplayAbility>(AbilitySpec->Ability))
// 		{
// 			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
// 			return true;
// 		}
// 	}
// 	
// 	const UAbilityInfo* AbilityInfo = UIPAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
// 	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FIPGameplayTags::Get().Abilities_None))
// 	{
// 		OutDescription = FString();
// 	}
// 	else
// 	{
// 		OutDescription = UIPGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
// 	}
// 	
//
// 	return false;
// }
