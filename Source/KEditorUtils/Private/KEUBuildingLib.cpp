#include "KEUBuildingLib.h"

#include "FGIconLibrary.h"
#include "Buildables/FGBuildable.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "Buildable/KPCLExtractorBase.h"
#include "Buildable/KPCLProducerBase.h"
#include "Desc/KLCleanerDescriptor.h"
#include "Resources/FGAnyUndefinedDescriptor.h"
#include "Resources/FGBuildDescriptor.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "Resources/FGNoneDescriptor.h"
#include "Resources/FGOverflowDescriptor.h"
#include "Unlocks/FGUnlockRecipe.h"
#include "Unlocks/FGUnlockScannableObject.h"
#include "Unlocks/FGUnlockScannableResource.h"
#include "Unlocks/FGUnlockSchematic.h"
#include "Unlocks/UKLUnlockCleanerItem.h"


class UUKLUnlockCleanerItem;

void UKEUBuildingLib::SetDefaultStateColors( AFGBuildable* Buildable, TArray<FLinearColor> Colors, TArray< ENewProductionState > States )
{
	if( Colors.Num() == States.Num() )
	{
		for( int32 Idx = 0; Idx < Colors.Num(); ++Idx )
		{
			if( AKPCLExtractorBase* ExtractorBase = Cast< AKPCLExtractorBase >( Buildable ) )
			{
				ExtractorBase->mStateColors[ static_cast< uint8 >( States[ Idx ] ) ] = Colors[ Idx ];
			}
			
			if( AKPCLProducerBase* ProducerBase = Cast< AKPCLProducerBase >( Buildable ) )
			{
				ProducerBase->mStateColors[ static_cast< uint8 >( States[ Idx ] ) ] = Colors[ Idx ];
			}
		}

		ObjectIsDirty( Buildable );
	}
}

void UKEUBuildingLib::MakeStaticMeshesToInstances( AFGBuildable* Buildable, TEnumAsByte<EComponentMobility::Type> Mobility )
{
	if( !IsValid( Buildable ) )
	{
		return;
	}
	
	TArray< UStaticMeshComponent* > Components;
	Buildable->GetDefaultComponents( Components );

	if( Components.Num() > 0 )
	{
		int32 CurrentIndex = 0;
		if( !IsValid( Buildable->mInstanceDataCDO ) )
		{
			Buildable->mInstanceDataCDO = NewObject< UAbstractInstanceDataObject >( Buildable );
#if WITH_EDITORONLY_DATA
			Buildable->mInstanceData = Buildable->mInstanceDataCDO;
#endif
		}
		Buildable->mCanContainLightweightInstances = IsValid( Buildable->mInstanceDataCDO );
		
		for ( UStaticMeshComponent* Component : Components )
		{
			if( Cast< UFGProductionIndicatorInstanceComponent >( Component ) )
			{
				continue;
			}
			
			if( IsValid( Component->GetStaticMesh() ) )
			{
				int32 NumOfFloats = 20;
				
				if( UKPCLBetterIndicator* BetterIndicator = Cast< UKPCLBetterIndicator >( Component ) )
				{
					NumOfFloats += 4;
					
					if( AKPCLExtractorBase* ExtractorBase = Cast< AKPCLExtractorBase >( Buildable ) )
					{
						ExtractorBase->mCustomIndicatorHandleIndexes.Add( CurrentIndex );
					}
			
					if( AKPCLProducerBase* ProducerBase = Cast< AKPCLProducerBase >( Buildable ) )
					{
						ProducerBase->mCustomIndicatorHandleIndexes.Add( CurrentIndex );
					}
				}
				else if( UKPCLColoredStaticMesh* ColoredInstance = Cast< UKPCLColoredStaticMesh >( Component ) )
				{
					NumOfFloats += ColoredInstance->mCustomExtraData.Num();
				}
				
				FInstanceData Data;
				Data.Mobility = Mobility;
				Data.StaticMesh = Component->GetStaticMesh();
				Data.NumCustomDataFloats = NumOfFloats;
				Data.CollisionProfileName = Component->GetCollisionProfileName();
				Data.RelativeTransform = Component->GetRelativeTransform();

#if WITH_EDITORONLY_DATA
				Buildable->mInstanceDataCDO->Instances.Add( Data );
#endif
				CurrentIndex++;
			}
		}

		ObjectIsDirty( Buildable );
#if WITH_EDITORONLY_DATA
		UE_LOG( LogTemp, Error, TEXT("Add %d InstanceDatas to %s"), Buildable->mInstanceData->GetInstanceData().Num(), *Buildable->GetClass()->GetName() )
#endif
	}
	else
	{
#if WITH_EDITORONLY_DATA
		UE_LOG( LogTemp, Error, TEXT("MakeStaticMeshesToInstances %s generate no InstanceHandles"), *Buildable->GetClass()->GetName() )
#endif
	}
}

void UKEUBuildingLib::EditInstanceMobility( AFGBuildable* Buildable, TEnumAsByte<EComponentMobility::Type> Mobility )
{
	if( IsValid( Buildable ) )
	{
	#if WITH_EDITOR
		if( IsValid( Buildable->mInstanceData ) )
		{
			for( int32 Idx = 0; Idx < Buildable->mInstanceData->Instances.Num(); ++Idx )
			{
				Buildable->mInstanceDataCDO->Instances[ Idx ].Mobility = Mobility;
			}
		}
		Buildable->mInstanceDataCDO = Buildable->mInstanceData;
		ObjectIsDirty( Buildable );
	#endif
	}
}

void UKEUBuildingLib::ClassIsDirty( UClass* Class )
{
	if( IsValid( Class ) )
	{
		Class->GetDefaultObject()->MarkPackageDirty();
		Class->MarkPackageDirty();
	}
}

void UKEUBuildingLib::ObjectIsDirty( UObject* Object )
{
	if( IsValid( Object ) )
	{
		Object->MarkPackageDirty();
		ClassIsDirty( Object->GetClass() );
	}
}

void UKEUBuildingLib::DebugInstanceHandleArray( AFGBuildable* Buildable, bool ShouldClear )
{
	if( IsValid( Buildable ) )
	{
#if WITH_EDITOR
		UE_LOG( LogTemp, Warning, TEXT("DebugInstanceHandleArray %s Has %d Handels!"), *Buildable->GetClass()->GetName(), Buildable->mInstanceHandles.Num() )
		if( ShouldClear )
		{
			Buildable->mInstanceHandles.Empty(  );
			UE_LOG( LogTemp, Warning, TEXT("DebugInstanceHandleArray %s Has %d Handels after clearing the Handels!"), *Buildable->GetClass()->GetName(), Buildable->mInstanceHandles.Num() )
			ObjectIsDirty( Buildable );
		}
#endif
	}
}


void UKEUBuildingLib::GenerateModdedIconDataBase( TSubclassOf<UFGIconLibrary> DefaulticonLibraryClass, TSubclassOf<UFGIconLibrary> iconLibraryClass, TArray<TSubclassOf<UFGSchematic>> SchematicsToScans, TArray< TSubclassOf< UFGItemDescriptor > > ForcedItemDesc )
{
	if( IsValid( iconLibraryClass ) && IsValid( DefaulticonLibraryClass ) )
	{
		UFGIconLibrary* OldLib = DefaulticonLibraryClass.GetDefaultObject();
		UFGIconLibrary* NewLib = iconLibraryClass.GetDefaultObject();
		
		TArray<TSubclassOf<UFGItemDescriptor>> ScannedItemDesc;
		TArray<TArray< FIconDataInformation >> Textures;

		TArray<TArray<FIconData>> IconData;
		
		IconData.Add( OldLib->mIconData );
		IconData.Add( OldLib->mMonochromeIconData );
		IconData.Add( OldLib->mCustomIconData );
		IconData.Add( OldLib->mMaterialIconData );
		IconData.Add( OldLib->mMapStampIconData );
		
		TArray<TArray<FIconData>> NewIconData;
		
		NewIconData.Add( NewLib->mIconData );
		NewIconData.Add( NewLib->mMonochromeIconData );
		NewIconData.Add( NewLib->mCustomIconData );
		NewIconData.Add( NewLib->mMaterialIconData );
		NewIconData.Add( NewLib->mMapStampIconData );

		Textures.SetNum( IconData.Num() );
		
		for ( int32 Idx = 0; Idx < Textures.Num(); ++Idx )
		{
			for (FIconData Data : NewIconData[ Idx ] )
			{
				if( ( Data.Texture.IsPending() || !Data.Texture.IsNull() || Data.Texture.IsValid() ) && Data.ID >= 2000 )
				{
					FIconDataInformation Info;
					Info.Texture = Data.Texture.LoadSynchronous();
					Info.IconName = Data.IconName;
					Info.IconType = Data.IconType;
					Info.ID = Data.ID;
					Textures[ Idx ].Add( Info );
				}
				else if( Data.ID > INDEX_NONE )
				{
					FIconDataInformation Info;
					Info.Texture = nullptr;
					Info.IconName = Data.IconName;
					Info.IconType = Data.IconType;
					Info.ID = Data.ID;
					Textures[ Idx ].Add( Info );
				}
			}
		}
		
		int32 NextIndex = 2000 + Textures.Num();
		
		for ( int32 Idx = 0; Idx < Textures.Num(); ++Idx )
		{
			for (FIconData Data : IconData[ Idx ] )
			{
				if( ( Data.Texture.IsPending() || !Data.Texture.IsNull() || Data.Texture.IsValid() ) && Data.ID > INDEX_NONE )
				{
					FIconDataInformation Info;
					Info.Texture = Data.Texture.LoadSynchronous();
					Info.IconName = Data.IconName;
					Info.IconType = Data.IconType;
					Info.ID = Data.ID;
					Textures[ Idx ].Add( Info );
				}
			}
		}

		for (TSubclassOf<UFGSchematic> SchematicsToScan : SchematicsToScans )
		{
			GetIconsForSchematic( SchematicsToScan, ScannedItemDesc, Textures[0], NextIndex, IconData[ 0 ] );
		}

		for (TSubclassOf<UFGItemDescriptor> ItemDesc : ForcedItemDesc )
		{
			GetIconsForItemDesc( ItemDesc, ScannedItemDesc, Textures[0], NextIndex, IconData[ 0 ] );
		}

		for( int32 Idx = 0; Idx < Textures[ 0 ].Num(); ++Idx )
		{
			int32 Index = INDEX_NONE;;
			FindTextureId( Textures[ 0 ][ Idx ], IconData[ 0 ], Index );
			UE_LOG( LogTemp, Warning, TEXT("Index: %d, IconData[ 0 ].Num(%d)"), Index, IconData[ 0 ].Num() )

			FIconData Data;
			Data.Texture = Textures[ 0 ][ Idx ].Texture;
			Data.IconName = Textures[ 0 ][ Idx ].IconName;
			Data.IconType = Textures[ 0 ][ Idx ].IconType;
			Data.ID = Textures[ 0 ][ Idx ].ID;

			if( Index == INDEX_NONE )
			{
				int32 NewId = IconData[ 0 ].Add( Data );
				if( IconData[ 0 ][ NewId ].ID == INDEX_NONE )
				{
					IconData[ 0 ][ NewId ].ID = NewId;
				}
			}
			else
			{
				IconData[ 0 ][ Index ].IconName = Data.IconName;
				IconData[ 0 ][ Index ].IconType = Data.IconType;
			}
		}

		TArray<TArray<FIconData>> EndIconData;
		EndIconData.SetNum( IconData.Num() );

		// Revalidate all Icons
		for( int32 RootIdx = 0; RootIdx < IconData.Num(); ++RootIdx )
		{
			for( int32 ChildIndex = 0; ChildIndex < IconData[ RootIdx ].Num(); ++ChildIndex )
			{
				TSoftObjectPtr<UObject> TestObj = IconData[ RootIdx ][ ChildIndex ].Texture;
				if( ( !TestObj.IsNull() || TestObj.IsPending() || TestObj.IsValid() ) && IconData[ RootIdx ][ ChildIndex ].ID >= 2000 )
				{
					EndIconData[ RootIdx ].Add( IconData[ RootIdx ][ ChildIndex ] );
				}
			}
		}

		NewLib->mIconData				= EndIconData[ 0 ];
	}
}

void UKEUBuildingLib::GetIconsForSchematic( TSubclassOf<UFGSchematic> InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray< FIconDataInformation >& Textures, int32& IndexCounter, TArray< FIconData > CurIconData )
{
	if( IsValid( InClass ) )
	{
		for (FItemAmount Cost : UFGSchematic::GetCost( InClass ) )
		{
			GetIconsForItemDesc( Cost.ItemClass, ScannedItemDesc, Textures, IndexCounter, CurIconData );
		}
		
		for (UFGUnlock* Unlock : UFGSchematic::GetUnlocks( InClass ) )
		{
			if( UFGUnlockSchematic* SchematicUnlock = Cast< UFGUnlockSchematic >( Unlock ) )
			{
				for (TSubclassOf<UFGSchematic> SchematicsToUnlock : SchematicUnlock->GetSchematicsToUnlock() )
				{
					GetIconsForSchematic( SchematicsToUnlock, ScannedItemDesc, Textures, IndexCounter, CurIconData );
				}
			}
			else if( UFGUnlockRecipe* RecipeUnlock = Cast< UFGUnlockRecipe >( Unlock ) )
			{
				for (TSubclassOf<UFGRecipe> RecipesToUnlock : RecipeUnlock->GetRecipesToUnlock() )
				{
					GetIconsForRecipe( RecipesToUnlock, ScannedItemDesc, Textures, IndexCounter, CurIconData );
				}
			}
			/*else if( UFGUnlockScannableObject* ScannableObject = Cast< UFGUnlockScannableObject >( Unlock ) )
			{
				for (FScannableObjectData ScannableObjectsToUnlock : ScannableObject->GetScannableObjectsToUnlock() )
				{
					GetIconsForItemDesc( ScannableObjectsToUnlock.ItemDescriptor, ScannedItemDesc, Textures, IndexCounter, CurIconData );
				}
			}*/
			else if( UFGUnlockScannableResource* ScannableResource = Cast< UFGUnlockScannableResource >( Unlock ) )
			{
				for (FScannableResourcePair ResourcesToAddToScanner : ScannableResource->GetResourcesToAddToScanner() )
				{
					GetIconsForItemDesc( ResourcesToAddToScanner.ResourceDescriptor, ScannedItemDesc, Textures, IndexCounter, CurIconData );
				}
			}
			else if( UUKLUnlockCleanerItem* CleanerItem = Cast< UUKLUnlockCleanerItem >( Unlock ) )
			{
				GetIconsForRecipe( CleanerItem->mFakeRecipe, ScannedItemDesc, Textures, IndexCounter, CurIconData );
				TSubclassOf< UKLCleanerDescriptor > Desc = TSubclassOf< UKLCleanerDescriptor >{ CleanerItem->mCleanerDescriptor };
				if( IsValid( Desc ) )
				{
					GetIconsForItemDesc( UKLCleanerDescriptor::GetInFluid( Desc ), ScannedItemDesc, Textures, IndexCounter, CurIconData );
					GetIconsForItemDesc( UKLCleanerDescriptor::GetOutFluid( Desc ), ScannedItemDesc, Textures, IndexCounter, CurIconData );
					for (FCleanerInfo Bypass : UKLCleanerDescriptor::GetBypass( Desc ) )
					{
						GetIconsForItemDesc( Bypass.mProduceItem, ScannedItemDesc, Textures, IndexCounter, CurIconData );
					}
					FCleanerInfo CInfo = UKLCleanerDescriptor::GetCleanerItemInfo( Desc );
					if( IsValid( CInfo.mProduceItem ) )
					{
						GetIconsForItemDesc( CInfo.mProduceItem, ScannedItemDesc, Textures, IndexCounter, CurIconData );
					}
				}
			}
		}

		/*
		FIconDataInformation Information;
		Information.Texture = UFGSchematic::GetItemIcon( InClass ).GetResourceObject();
		Information.IconName = UFGSchematic::mDisplayName
		*/
	}
}

void UKEUBuildingLib::GetIconsForItemDesc( TSubclassOf<UFGItemDescriptor> InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray<FIconDataInformation>& Textures, int32& IndexCounter, TArray< FIconData > CurIconData )
{
	if(
		IsValid( InClass ) &&
		!InClass->IsChildOf( UFGAnyUndefinedDescriptor::StaticClass() )&&
		!InClass->IsChildOf( UFGOverflowDescriptor::StaticClass() )&&
		!InClass->IsChildOf( UFGNoneDescriptor::StaticClass() ) &&
		!ScannedItemDesc.Contains( InClass )
	)
	{
		FIconDataInformation Information;
		Information.Texture = UFGItemDescriptor::GetBigIcon( InClass );
		Information.ID = IndexCounter;
		Information.IconName = UFGItemDescriptor::GetItemName( InClass );
		Information.IconType = EIconType::ESIT_Part;
		
		if( InClass->IsChildOf( UFGEquipmentDescriptor::StaticClass() ) )
		{
			Information.IconType = EIconType::ESIT_Equipment;
		}
		else if( InClass->IsChildOf( UFGBuildDescriptor::StaticClass() ) )
		{
			Information.IconType = EIconType::ESIT_Building;
		}

		int32 Index = INDEX_NONE;
		FindTextureId( Information, CurIconData, Index );
		if( Index == INDEX_NONE )
		{
			ScannedItemDesc.Add( InClass );
			Textures.Add( Information );
			IndexCounter++;
		}
		else
		{
			for (FIconDataInformation& Texture : Textures )
			{
				if( Texture.Texture == Information.Texture )
				{
					Texture.IconName = Information.IconName;
					Texture.IconType = Information.IconType;
					break;
				}
			}
		}
	}
}

void UKEUBuildingLib::GetIconsForRecipe( TSubclassOf<UFGRecipe> InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray< FIconDataInformation >& Textures, int32& IndexCounter, TArray< FIconData > CurIconData )
{
	if( IsValid( InClass ) )
	{
		TArray< FItemAmount > Amounts = UFGRecipe::GetIngredients( InClass );
		Amounts.Append( UFGRecipe::GetProducts( InClass, true ) );
		
		for (FItemAmount Amount : Amounts )
		{
			GetIconsForItemDesc( Amount.ItemClass, ScannedItemDesc, Textures, IndexCounter, CurIconData );
		}
	}
}

int32 UKEUBuildingLib::FindTextureId( FIconDataInformation DataInformation, TArray<FIconData> IconData )
{
	int32 Dummy;
	return FindTextureId( DataInformation, IconData, Dummy );
}

int32 UKEUBuildingLib::FindTextureId( FIconDataInformation DataInformation, TArray<FIconData> IconData, int32& Index )
{
	for( int32 Idx = 0; Idx < IconData.Num(); ++Idx )
	{
		FIconData Data = IconData[ Idx ];
		if( Data.Texture.IsPending() || !Data.Texture.IsNull() || Data.Texture.IsValid() )
		{
			if( Data.Texture.LoadSynchronous() == DataInformation.Texture )
			{
				Index = Idx;
				return Data.ID;
			}
		}
	}
	return INDEX_NONE;
}
