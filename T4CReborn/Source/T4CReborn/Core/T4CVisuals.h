#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

/**
 * Utilitário de cor: aplica uma cor sólida a um MeshComponent criando um
 * MaterialInstanceDynamic do material gerado /Game/Materials/M_T4C_Colored
 * (ver Scripts/gen_materials.py). É o jeito confiável de colorir primitivas —
 * o BasicShapeMaterial da engine não responde a SetVectorParameterValueOnMaterials.
 * No-op gracioso se o material ainda não foi gerado.
 */
namespace T4CVisuals
{
	inline void ApplyBodyColor(UMeshComponent* Mesh, UObject* Outer, const FLinearColor& Color)
	{
		if (!Mesh)
		{
			return;
		}
		UMaterialInterface* Base = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Game/Materials/M_T4C_Colored.M_T4C_Colored"));
		if (!Base)
		{
			return; // material não gerado ainda
		}
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Outer))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
			Mesh->SetMaterial(0, MID);
		}
	}
}
