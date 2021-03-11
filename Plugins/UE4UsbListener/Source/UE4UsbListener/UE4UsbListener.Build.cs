// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UE4UsbListener : ModuleRules
{
	public UE4UsbListener(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);

		// Adding your definition here, will add a global Preprocessor value for cpp
		//PublicDefinitions.Add("_WIN32_WINNT=0x0600");

		//ADD_DEFINITIONS("-D_WIN32_WINNT=0x0600");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4146");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4305");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4355");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996");
		//set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819");
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
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
