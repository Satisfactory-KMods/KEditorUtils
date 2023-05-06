#pragma once
#include "Buildable/KPCLProducerBase.h"

#include "FGIconLibrary.h"
#include "Desc/KLCleanerDescriptor.h"
#include "Unlocks/FGUnlockRecipe.h"

#include "KEUBuildingLib.generated.h"

USTRUCT( )
struct FIconDataInformation
{
	GENERATED_BODY()
	
	UPROPERTY( EditDefaultsOnly, Category = "Icon Data" )
	UObject* Texture;

	UPROPERTY( EditDefaultsOnly, Category = "Icon Data" )
	FText IconName;

	UPROPERTY( VisibleDefaultsOnly, Category = "Icon Data" )
	EIconType IconType;

	UPROPERTY( VisibleDefaultsOnly, Category = "Icon Data" )
	int32 ID;
};

UCLASS(BlueprintType, MinimalAPI)
class UKEUBuildingLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void SetDefaultStateColors( AFGBuildable* Buildable, TArray< FLinearColor > Colors, TArray< ENewProductionState > States );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void MakeStaticMeshesToInstances( AFGBuildable* Buildable, TEnumAsByte<EComponentMobility::Type> Mobility  );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void EditInstanceMobility( AFGBuildable* Buildable, TEnumAsByte<EComponentMobility::Type> Mobility  );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void ClassIsDirty( UClass* Class );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void ObjectIsDirty( UObject* Object );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void DebugInstanceHandleArray( AFGBuildable* Buildable, bool ShouldClear );
		
	UFUNCTION( BlueprintCallable, meta=( DevelopmentOnly ) )
	static void GenerateModdedIconDataBase( TSubclassOf<UFGIconLibrary> DefaulticonLibraryClass, TSubclassOf< UFGIconLibrary > iconLibraryClass, TArray< TSubclassOf< UFGSchematic > > SchematicsToScans, TArray< TSubclassOf< UFGItemDescriptor > > ForcedItemDesc );
		
	static void GetIconsForSchematic( TSubclassOf< UFGSchematic > InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray< FIconDataInformation >& Textures, int32& IndexCounter, TArray< FIconData > CurIconData );
	static void GetIconsForRecipe( TSubclassOf< UFGRecipe > InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray< FIconDataInformation >& Textures, int32& IndexCounter, TArray< FIconData > CurIconData );
	static void GetIconsForItemDesc( TSubclassOf< UFGItemDescriptor > InClass, TArray<TSubclassOf<UFGItemDescriptor>>& ScannedItemDesc, TArray< FIconDataInformation >& Textures, int32& IndexCounter, TArray< FIconData > CurIconData );
	static int32 FindTextureId( FIconDataInformation DataInformation, TArray< FIconData > IconData );
	static int32 FindTextureId( FIconDataInformation DataInformation, TArray< FIconData > IconData, int32& Index );
};
