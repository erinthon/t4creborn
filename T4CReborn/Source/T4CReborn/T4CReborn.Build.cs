using UnrealBuildTool;

public class T4CReborn : ModuleRules
{
	public T4CReborn(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// O código está organizado em subpastas (Core/Player/Attributes) sem
		// split Public/Private; expõe a raiz do módulo para includes do tipo
		// "Core/T4CGameMode.h" resolverem.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"NetCore",
			"UMG",
			"AIModule",
			// Gameplay Ability System (Fase 4): ASC, AttributeSet, GameplayEffects,
			// GameplayAbilities, GameplayTags e GameplayTasks.
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			// Persistência: cliente HTTP + JSON para falar com o serviço externo.
			"HTTP",
			"Json",
			"JsonUtilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
	}
}
