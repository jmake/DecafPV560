// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MyProject5 : ModuleRules
{
	public MyProject5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			//"SpicyWrapper", 
			"SpicyLibrary"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		ExternalStatic(); 
		ExternalDynamic(); 

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}


    private void ExternalDynamic()
    {
        string ExternalLibsPath = Path.Combine(ModuleDirectory, "../../ExternalDynamic");

        // - Add the folder with the external headers
        PublicIncludePaths.Add(Path.Combine(ExternalLibsPath, "."));

        // - Add the import library (.lib)
        PublicAdditionalLibraries.Add(Path.Combine(ExternalLibsPath, ".", "ExternalLibrary.lib"));

        // - Copy to the output (Binaries/Win64) folder (editor-time!!)
        string Source = Path.Combine(ExternalLibsPath, ".", "ExternalLibrary.dll");
        string Dest = Path.Combine(ModuleDirectory, "../../Binaries/Win64", "ExternalLibrary.dll");
		File.Copy(Source, Dest, true); 

		// - Mainly for packaged builds, not editor-time
        // Tell UE to delay-load the DLL at runtime
        //PublicDelayLoadDLLs.Add("ExternalLibrary.dll");

        // Ensure the DLL is copied to the output (Binaries/Win64) folder
        //RuntimeDependencies.Add("$(ProjectDir)/ExternalLibs/ExternalLibrary.dll");
    }


    private void ExternalStatic()
    {
        string ExternalLibsPath = Path.Combine(ModuleDirectory, "../../ExternalStatic");

        // - Add the folder with the external headers
        PublicIncludePaths.Add(Path.Combine(ExternalLibsPath, "."));

    	// Add the static library (.lib)
    	PublicAdditionalLibraries.Add(Path.Combine(ExternalLibsPath, "StaticLibrary.lib"));
	} 

}
