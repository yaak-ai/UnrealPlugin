

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UltraleapTrackingEditor : ModuleRules
	{

		public UltraleapTrackingEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"UltraleapTrackingEditor/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add other public dependencies that you statically link with here ...
					"Engine",
					"Core",
					"CoreUObject",
					"UltraleapTracking",
					"PropertyEditor",
					"Slate",
					"SlateCore",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"BlueprintGraph",
					"AnimGraph",
					"AnimGraphRuntime",
					"BodyState"
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}