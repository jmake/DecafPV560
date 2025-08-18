// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MyProject5 : ModuleRules
{
	public MyProject5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //PublicDefinitions.Add("HAVE_ZLIB"); //  error C4101: 'n2read': unreferenced local variable
		//PublicDefinitions.Add("HAVE_UNISTD_H");

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", 
			//"SpicyWrapper", 
			"SpicyLibrary", 
			"ProceduralMeshComponent", 
			"AdvancedWidgets", // RadialSlider
			"SlateCore", // FSlateBrush, 
//			"UnrealEd", "AssetTools", "AssetRegistry", 
		});

//if (Target.Type == TargetType.Editor){
//    PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "AssetTools", "AssetRegistry" });
//}

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		VTKStaticLibs(); 
		VTKHeaders(); 
		ExternalStatic(); 
		ExternalDynamic(); 

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}


    void ExternalDynamic()
    {
        string ExternalLibsPath = Path.Combine(ModuleDirectory, "../../ExternalDynamic");

        // - Add the folder with the external headers
        PublicIncludePaths.Add(Path.Combine(ExternalLibsPath, "."));

        // - Add the import library (.lib)
        PublicAdditionalLibraries.Add(Path.Combine(ExternalLibsPath, "Release", "DynamicLibrary.lib"));

        // - Copy to the output (Binaries/Win64) folder (editor-time!!)
        string Source = Path.Combine(ExternalLibsPath, "Release", "DynamicLibrary.dll");
        string Dest = Path.Combine(ModuleDirectory, "../../Binaries/Win64", "DynamicLibrary.dll");

		// IMPORTANT : DYNAMIC LIBRARIES MUST BE COPIED TO 'Binaries/Win64' !!!!! 

		//if (File.Exists(Dest)) File.Delete(Dest);
		//File.Copy(Source, Dest, true); // not working!!?? :/ 

		// - Mainly for packaged builds, not editor-time
        // Tell UE to delay-load the DLL at runtime
        //PublicDelayLoadDLLs.Add("ExternalLibrary.dll");

        // Ensure the DLL is copied to the output (Binaries/Win64) folder
        //RuntimeDependencies.Add("$(ProjectDir)/ExternalLibs/ExternalLibrary.dll");
    }


    void ExternalStatic()
    {
        string ExternalLibsPath = Path.Combine(ModuleDirectory, "../../ExternalStatic/");

        // - Add the folder with the external headers
        PublicIncludePaths.Add(Path.Combine(ExternalLibsPath, "."));

    	// Add the static library (.lib)
    	PublicAdditionalLibraries.Add(Path.Combine(ExternalLibsPath, "Release", "StaticLibrary.lib"));
	} 


	void VTKHeaders() 
	{
		/*
		SEE : 
			Paths must be included in 'UnrealEditor-MyProject5.lib.rsp' which shold be located in 
			'Intermediate\Build\Win64\x64\UnrealEditor\Development\MyProject5\MyProject5.Shared.rsp'

			Check 'INCLUDES' in 'build.ninja'
		*/

		PublicIncludePaths.Add("F:/z2025_1/Dicom/zlib/Execs/include"); //  Cannot open include file: 'zlib.h'

        string ModuleDir = Path.Combine(ModuleDirectory, "../../SpicyPlugin/");

		string IncludeFilePath = Path.Combine(ModuleDir, "ParaViewIncludeDirs.txt");

		if (File.Exists(IncludeFilePath))
		{
			string[] rawPaths = File.ReadAllText(IncludeFilePath).Split(';');
			foreach (string path in rawPaths)
			{
				string trimmed = path.Trim();
				if (!string.IsNullOrEmpty(trimmed) && Directory.Exists(trimmed))
				{
					PublicIncludePaths.Add(trimmed);
				}
			}
		}
		else
		{
			System.Console.WriteLine("WARNING: ParaViewIncludeDirs.txt not found.");
		}
	}


	void VTKStaticLibs()
	{
		/*
		SEE : 
			- Check 'LINK_LIBRARIES' in 'build.ninja', 'libs_1.txt' has been created from the information found there

			- Libraries must be included in 'UnrealEditor-MyProject5.dll.rsp' which shold be located in 
			  'Intermediate\Build\Win64\x64\UnrealEditor\Development\MyProject5'

			- dumpbin.exe vtkCommonCore-pv5.6.lib shows if a given library es static or dynamic 
			  The presence of '.xdata' and '.data' indicates that this library should be static.
		*/

		string ModuleDir = Path.Combine(ModuleDirectory, "../../SpicyPlugin/");
		string[] libLists = Directory.GetFiles(ModuleDir, "libs_*.txt");

		if (libLists.Length == 0)
		{
			System.Console.WriteLine("WARNING: No extracted_libs_*.txt files found.");
			return;
		}

		foreach (string listFile in libLists)
		{
			System.Console.WriteLine( $"[listFile] :'{listFile}'" );

			string[] libs = File.ReadAllLines(listFile);
			foreach (string lib in libs)
			{
				string trimmed = lib.Trim();
				if (!string.IsNullOrEmpty(trimmed) && File.Exists(trimmed))
				{
					System.Console.WriteLine( $"[lib] :'{lib}'" );
					PublicAdditionalLibraries.Add(trimmed);
				}
			}
		}
	}
}