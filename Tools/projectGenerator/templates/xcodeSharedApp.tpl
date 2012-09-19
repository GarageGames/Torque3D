[assign var="dirWalk" value=$fileArray]
// !$*UTF8*$!
{
	archiveVersion = 1;
	classes = {
	};
	objectVersion = 42;
	objects = {

/* Begin PBXContainerItemProxy section */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|cat:"proxy_file"|uid]
[assign var=libuid2 value=$def|cat:"proxy"|uid]
[assign var=libuid3 value=$def|cat:"remote"|uid]
[assign var=libuid4 value=$def|uid]
[assign var=libuid5 value=$def|cat:"target"|uid]
		F[$libuid] /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = C[$libuid4] /* [$def].xcodeproj */;
			proxyType = 2;
			remoteGlobalIDString = F[$libuid3];
			remoteInfo = "[$def]";
		};
		E[$libuid2] /* PBXContainerItemProxy */ = {
			isa = PBXContainerItemProxy;
			containerPortal = C[$libuid4] /* [$def].xcodeproj */;
			proxyType = 1;
			remoteGlobalIDString = D[$libuid5] /* [$def] */;
			remoteInfo = "[$def]";
		};
[/foreach]
/* End PBXContainerItemProxy section */


/* Begin PBXBuildFile section */
      FAE3EDA00EEE3A0F0024DCA3 /* resizeNWSE.png in Resources */ = {isa = PBXBuildFile; fileRef = FAE3ED9D0EEE3A0F0024DCA3 /* resizeNWSE.png */; };
		FAE3EDA10EEE3A0F0024DCA3 /* resizeall.png in Resources */ = {isa = PBXBuildFile; fileRef = FAE3ED9E0EEE3A0F0024DCA3 /* resizeall.png */; };
		FAE3EDA20EEE3A0F0024DCA3 /* resizeNESW.png in Resources */ = {isa = PBXBuildFile; fileRef = FAE3ED9F0EEE3A0F0024DCA3 /* resizeNESW.png */; };
		D21C29DF0CE933AB00670EED /* mainMenu.nib in Resources */ = {isa = PBXBuildFile; fileRef = D21C29DE0CE933AB00670EED /* mainMenu.nib */; };
		D297FD360C05187F00C14A16 /* OpenAL.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D297FD350C05187F00C14A16 /* OpenAL.framework */; };
		D297FD360C05187F00C14A16 /* OpenAL.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D297FD350C05187F00C14A16 /* OpenAL.framework */; };
		bf_copy_openal_framework /* OpenAL.framework in Copy Frameworks */ = {isa = PBXBuildFile; fileRef = D297FD350C05187F00C14A16 /* OpenAL.framework */; };
		D29CADE70C88C2C900BBF312 /* AGL.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D29CADE30C88C2C900BBF312 /* AGL.framework */; };
		D29CADE80C88C2C900BBF312 /* Carbon.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D29CADE40C88C2C900BBF312 /* Carbon.framework */; };
		D29CADE90C88C2C900BBF312 /* Cocoa.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D29CADE50C88C2C900BBF312 /* Cocoa.framework */; };
		D29CADEA0C88C2C900BBF312 /* OpenGL.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = D29CADE60C88C2C900BBF312 /* OpenGL.framework */; };
		D2559B3B0C921A2B0003B62A /* torqueDemo.icns in Resources */ = {isa = PBXBuildFile; fileRef = D2559B3A0C921A2B0003B62A /* torqueDemo.icns */; };

   /* Begin PBXBuildFiles included by project generator */
[include file="xcode.buildfiles.tpl" dirWalk=$dirWalk]
   /* End PBXBuildFiles included by project generator */
   
   /* Begin PBXBuildFiles for built lib dependencies */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
   F[$libuid] /* lib[$def].a in Frameworks */ = {isa = PBXBuildFile; fileRef = D[$libuid] /* lib[$def].a */; };
[/foreach]
   /* End PBXBuildFiles for built lib dependencies */
/* End PBXBuildFile section */

/* Begin PBXBuildRule section */
		D297FC2D0C04FB9A00C14A16 /* PBXBuildRule */ = {
			isa = PBXBuildRule;
			compilerSpec = com.apple.compilers.nasm;
			filePatterns = "*.asm";
			fileType = pattern.proxy;
			isEditable = 1;
			outputFiles = (
			);
		};
/* End PBXBuildRule section */

/* Begin PBXFileReference section */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
      C[$libuid] /* [$def].xcodeproj */ = {isa = PBXFileReference; lastKnownFileType = "wrapper.pb-project"; name = "[$def].xcodeproj"; path = "[$def].xcodeproj"; sourceTree = SOURCE_ROOT; };
[/foreach]
[foreach key=def item=type from=$projTypes]
[assign var=libuid value=$def|uid]
[if $type eq 1]
[else]
      D[$libuid] /* lib[$def].a */ = {isa = PBXFileReference; lastKnownFileType = archive.ar; name = lib[$def].a; path = ../../../../engine/lib/builtLibs/lib[$def].a; sourceTree = SOURCE_ROOT; };
[/if]
[/foreach]

		D26834AB0C02C77B0020EE4F /* [$projName].app */ = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = "[$projName].app"; sourceTree = BUILT_PRODUCTS_DIR; };
		D21C29DE0CE933AB00670EED /* mainMenu.nib */ = {isa = PBXFileReference; lastKnownFileType = wrapper.nib; name = mainMenu.nib; path = ../../../../engine/source/platformMac/menus/mainMenu.nib; sourceTree = SOURCE_ROOT; };
		D26834AE0C02C77B0020EE4F /* Info.plist */ = {isa = PBXFileReference; lastKnownFileType = text.xml; path = "Info.plist"; sourceTree = "<group>"; };
		D2559B3A0C921A2B0003B62A /* torqueDemo.icns */ = {isa = PBXFileReference; lastKnownFileType = image.icns; path = torqueDemo.icns; sourceTree = "<group>"; };
		D297FD350C05187F00C14A16 /* OpenAL.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = OpenAL.framework; path = ../../../../engine/lib/openal/macosx/OpenAL.framework; sourceTree = SOURCE_ROOT; };
		D29CADE30C88C2C900BBF312 /* AGL.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = AGL.framework; path = /System/Library/Frameworks/AGL.framework; sourceTree = "<absolute>"; };
		D29CADE40C88C2C900BBF312 /* Carbon.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Carbon.framework; path = /System/Library/Frameworks/Carbon.framework; sourceTree = "<absolute>"; };
		D29CADE50C88C2C900BBF312 /* Cocoa.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Cocoa.framework; path = /System/Library/Frameworks/Cocoa.framework; sourceTree = "<absolute>"; };
		D29CADE60C88C2C900BBF312 /* OpenGL.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = OpenGL.framework; path = /System/Library/Frameworks/OpenGL.framework; sourceTree = "<absolute>"; };
                FAE3ED9D0EEE3A0F0024DCA3 /* resizeNWSE.png */ = {isa = PBXFileReference; lastKnownFileType = image.png; name = resizeNWSE.png; path = ../../../../engine/source/platformMac/cursors/resizeNWSE.png; sourceTree = SOURCE_ROOT; };
		FAE3ED9E0EEE3A0F0024DCA3 /* resizeall.png */ = {isa = PBXFileReference; lastKnownFileType = image.png; name = resizeall.png; path = ../../../../engine/source/platformMac/cursors/resizeall.png; sourceTree = SOURCE_ROOT; };
		FAE3ED9F0EEE3A0F0024DCA3 /* resizeNESW.png */ = {isa = PBXFileReference; lastKnownFileType = image.png; name = resizeNESW.png; path = ../../../../engine/source/platformMac/cursors/resizeNESW.png; sourceTree = SOURCE_ROOT; };

   /* Begin PBXFileReferences included by project generator */
      [include file="xcode.filerefs.tpl" dirWalk=$dirWalk ]
   /* End PBXFileReferences included by project generator */
   
/* End PBXFileReference section */

/* Begin PBXFrameworksBuildPhase section */
		D26834A90C02C77B0020EE4F /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				D297FD360C05187F00C14A16 /* OpenAL.framework in Frameworks */,
				D29CADE70C88C2C900BBF312 /* AGL.framework in Frameworks */,
				D29CADE80C88C2C900BBF312 /* Carbon.framework in Frameworks */,
				D29CADE90C88C2C900BBF312 /* Cocoa.framework in Frameworks */,
				D29CADEA0C88C2C900BBF312 /* OpenGL.framework in Frameworks */,
            
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
            F[$libuid] /* lib[$def].a */,
[/foreach]
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */


/* Begin PBXCopyFilesBuildPhase section */
		AFFFF000000000000000FFFF /* Copy Frameworks */ = {
			isa = PBXCopyFilesBuildPhase;
			buildActionMask = 2147483647;
			dstPath = "";
			dstSubfolderSpec = 10;
			files = (

                                 AAAAA00000000000000000AA /* OpenAL.framework  */,

[foreach key=def item=type from=$projTypes]
[assign var=libuid value=$def|cat:"target"|uid]
[if $type eq 1]
				[$libuid] /* libtgea.dylib in Copy Frameworks */,
[/if]
[/foreach]
			);
			name = "Copy Frameworks";
			runOnlyForDeploymentPostprocessing = 0;
		};

/* End PBXCopyFilesBuildPhase section */



/* Begin PBXGroup section */
		AFFFF0000000000000000000 = {
			isa = PBXGroup;
			children = (
				A00000000000000000000000 /* Code */,
            AEEEE0000000000000000000 /* Dependencies */,
				D297FD340C05186400C14A16 /* Frameworks */,
				D26834AC0C02C77B0020EE4F /* Products */,
				D26834AE0C02C77B0020EE4F /* Info.plist */,
            D2559B3A0C921A2B0003B62A /* torqueDemo.icns */,
            D21C29DE0CE933AB00670EED /* mainMenu.nib */,
            FAE3ED9D0EEE3A0F0024DCA3 /* resizeNWSE.png */,
				FAE3ED9E0EEE3A0F0024DCA3 /* resizeall.png */,
				FAE3ED9F0EEE3A0F0024DCA3 /* resizeNESW.png */,
			);
			sourceTree = "<group>";
		};

/* Begin Generated Dependencies Group */
		AEEEE0000000000000000000 /* Dependencies */ = {
			isa = PBXGroup;
			children = (
            ADDDD0000000000000000000 /* builtLibs */,
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
            C[$libuid], /* [$def].xcodeproj */
[/foreach]
			);
			name = Dependencies;
			sourceTree = "<group>";
		};
      ADDDD0000000000000000000 /* builtLibs */ = {
			isa = PBXGroup;
			children = (
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
           D[$libuid] /* lib[$def].a */,
[/foreach]         
			);
			name = builtLibs;
			path = ../../../../engine/lib/builtLibs;
			sourceTree = SOURCE_ROOT;
		};
/* End Generated Dependencies Group */

/* Begin Products Groups for each Dependency */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
[assign var=libuid2 value=$def|cat:"refproxy_file"|uid]
      B[$libuid] /* Products */ = {
			isa = PBXGroup;
			children = (
				C[$libuid2] /* [$def].a */,
			);
			name = Products;
			sourceTree = "<group>";
		};
[/foreach]
/* End Products Groups for each Dependency */


		D26834AC0C02C77B0020EE4F /* Products */ = {
			isa = PBXGroup;
			children = (
				D26834AB0C02C77B0020EE4F /* [$projName].app */,
			);
			name = Products;
			sourceTree = "<group>";
		};
		D297FD340C05186400C14A16 /* Frameworks */ = {
			isa = PBXGroup;
			children = (
				D29CAE9C0C88C2D200BBF312 /* System */,
				D29CAEAF0C88C2DE00BBF312 /* Local */,
			);
			name = Frameworks;
			sourceTree = "<group>";
		};
		D29CAE9C0C88C2D200BBF312 /* System */ = {
			isa = PBXGroup;
			children = (
				D29CADE30C88C2C900BBF312 /* AGL.framework */,
				D29CADE40C88C2C900BBF312 /* Carbon.framework */,
				D29CADE50C88C2C900BBF312 /* Cocoa.framework */,
				D29CADE60C88C2C900BBF312 /* OpenGL.framework */,
			);
			name = System;
			sourceTree = "<group>";
		};
		D29CAEAF0C88C2DE00BBF312 /* Local */ = {
			isa = PBXGroup;
			children = (
				D297FD350C05187F00C14A16 /* OpenAL.framework */,
			);
			name = Local;
			sourceTree = "<group>";
		};


   /* Begin PBXGroups from project generator */
[include file="xcode.groups.tpl" dirWalk=$dirWalk recurse="no" groupPath="paxorr" groupName="Code" groupHash="00000000000000000000000"]
   /* End PBXGroups from project generator */

/* End PBXGroup section */

/* Begin PBXNativeTarget section */
		D26834AA0C02C77B0020EE4F /* [$projName] */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = D26834AF0C02C77B0020EE4F /* Build configuration list for PBXNativeTarget [$projName] */;
			buildPhases = (
				D26834A70C02C77B0020EE4F /* Resources */,
				D26834A80C02C77B0020EE4F /* Sources */,
				D26834A90C02C77B0020EE4F /* Frameworks */,
                                AFFFF000000000000000FFFF /* Copy Frameworks */,
			);
			buildRules = (
				D297FC2D0C04FB9A00C14A16 /* PBXBuildRule */,
			);
			dependencies = (
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|cat:"dep"|uid]
            D[$libuid] /* PBXTargetDependency */,
[/foreach]
			);
			name = "[$projName]";
			productName = "[$projName]";
			productReference = D26834AB0C02C77B0020EE4F /* [$projName].app */;
			productType = "com.apple.product-type.application";
		};
/* End PBXNativeTarget section */

/* Begin PBXProject section */
		D268347E0C02C5750020EE4F /* Project object */ = {
			isa = PBXProject;
			buildConfigurationList = D268347F0C02C5750020EE4F /* Build configuration list for PBXProject [$projName] */;
			compatibilityVersion = "Xcode 3.2";
			hasScannedForEncodings = 0;
			mainGroup = AFFFF0000000000000000000;
			productRefGroup = D26834AC0C02C77B0020EE4F /* Products */;
			projectDirPath = "";
			targets = (
				D26834AA0C02C77B0020EE4F /* [$projName] */,
			);
         projectRoot = "";
         projectReferences = (
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
				{
					ProductGroup = B[$libuid] /* Products */;
					ProjectRef = C[$libuid] /* [$def].xcodeproj */;
				},
[/foreach]
			);
		};
/* End PBXProject section */

/* Begin PBXReferenceProxy section */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|cat:"proxy_file"|uid]
[assign var=libuid2 value=$def|cat:"refproxy_file"|uid]
      C[$libuid2] /* lib[$def].a */ = {
			isa = PBXReferenceProxy;
			fileType = archive.ar;
			path = "lib[$def].a";
			remoteRef = F[$libuid] /* PBXContainerItemProxy */;
			sourceTree = BUILT_PRODUCTS_DIR;
      };
[/foreach]   
/* End PBXReferenceProxy section */

/* Begin PBXTargetDependency section */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|cat:"dep"|uid]
[assign var=libuid2 value=$def|cat:"proxy"|uid]
		D[$libuid] /* PBXTargetDependency */ = {
			isa = PBXTargetDependency;
			name = "[$def]";
			targetProxy = E[$libuid2] /* PBXContainerItemProxy */;
		};
[/foreach]
/* End PBXTargetDependency section */

/* Begin PBXResourcesBuildPhase section */
		D26834A70C02C77B0020EE4F /* Resources */ = {
			isa = PBXResourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
            D2559B3B0C921A2B0003B62A /* torqueDemo.icns in Resources */,
            D21C29DF0CE933AB00670EED /* mainMenu.nib in Resources */,
            FAE3EDA00EEE3A0F0024DCA3 /* resizeNWSE.png in Resources */,
				FAE3EDA10EEE3A0F0024DCA3 /* resizeall.png in Resources */,
				FAE3EDA20EEE3A0F0024DCA3 /* resizeNESW.png in Resources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXResourcesBuildPhase section */

/* Begin PBXSourcesBuildPhase section */
		D26834A80C02C77B0020EE4F /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
[include file="xcode.list_buildfiles.tpl" dirWalk=$dirWalk]
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXSourcesBuildPhase section */

/* Begin XCBuildConfiguration section */
		D26834800C02C5750020EE4F /* Debug */ = {
			isa = XCBuildConfiguration;
         /* Project level debug config */
			buildSettings = {
				ARCHS = "$(ARCHS_STANDARD_32_BIT)";
				CONFIGURATION_BUILD_DIR = ../../[$gameFolder]/;
				COPY_PHASE_STRIP = NO;
				DEBUG_INFORMATION_FORMAT = dwarf;
				GCC_ENABLE_SSE3_EXTENSIONS = YES;
				OTHER_LDFLAGS_i386 = "-framework Accelerate";
				OTHER_LDFLAGS_ppc = "-framework vecLib";
				SDKROOT = /Developer/SDKs/MacOSX10.5.sdk;
            GCC_VERSION = 4.2;
				TORQUE_BUILT_LIBS_DIR = ../../../../engine/lib/builtLibs/;
			};
			name = Debug;
		};
		D26834810C02C5750020EE4F /* Release */ = {
			isa = XCBuildConfiguration;
         /* Project level release config */
			buildSettings = {
				ARCHS = "$(ARCHS_STANDARD_32_BIT)";
				CONFIGURATION_BUILD_DIR = ../../[$gameFolder]/;
				COPY_PHASE_STRIP = YES;
				DEBUG_INFORMATION_FORMAT = dwarf;
				GCC_ENABLE_SSE3_EXTENSIONS = YES;
				OTHER_LDFLAGS_i386 = "-framework Accelerate";
				OTHER_LDFLAGS_ppc = "-framework vecLib";
				SDKROOT = /Developer/SDKs/MacOSX10.5.sdk;
            GCC_VERSION = 4.2;
				TORQUE_BUILT_LIBS_DIR = ../../../../engine/lib/builtLibs/;
			};
			name = Release;
		};
		D26834B00C02C77B0020EE4F /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = 
         {
				ALWAYS_SEARCH_USER_PATHS = NO;
				COPY_PHASE_STRIP = NO;
				DEBUG_INFORMATION_FORMAT = dwarf;
				FRAMEWORK_SEARCH_PATHS = (
					"$(inherited)",
					../../../../engine/lib/openal/macosx,
					../../../../engine/lib/xiph/macosx,
				);
				GCC_DYNAMIC_NO_PIC = NO;
                GCC_ENABLE_CPP_EXCEPTIONS = YES;
				GCC_ENABLE_FIX_AND_CONTINUE = NO;
				GCC_GENERATE_DEBUGGING_SYMBOLS = YES;
				GCC_MODEL_TUNING = G5;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_PREPROCESSOR_DEFINITIONS = (
					"$(TORQUE_DEBUG_DEFINE)",
               __MACOSX__,
					WIN32_LEAN_AND_MEAN,
					NOMINMAX,
					TORQUE_MULTITHREAD,
					TORQUE_DISABLE_MEMORY_MANAGER,
					TORQUE_UNICODE,
[foreach item=def from=$projDefines]
               "[$def]",
[/foreach]
            );
				GCC_TREAT_NONCONFORMANT_CODE_ERRORS_AS_WARNINGS = YES;
				GCC_WARN_INHIBIT_ALL_WARNINGS = YES;
				HEADER_SEARCH_PATHS = ( 
[foreach item=def from=$projIncludes]
               "[$def]",
[/foreach]
				);
				INFOPLIST_FILE = "Info.plist";
				LIBRARY_SEARCH_PATHS = "$(TORQUE_BUILT_LIBS_DIR)";
            OTHER_LDFLAGS = "-lz";
				PREBINDING = NO;
				PRODUCT_NAME = "[$projName]_DEBUG";
				TORQUE_DEBUG_DEFINE = TORQUE_DEBUG;
				WARNING_CFLAGS = "-Wall -Wno-sign-compare -Wpointer-arith";
				WRAPPER_EXTENSION = app;
				ZERO_LINK = NO;
			};
			name = Debug;
		};
		D26834B10C02C77B0020EE4F /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				COPY_PHASE_STRIP = YES;
				DEBUG_INFORMATION_FORMAT = dwarf;
				FRAMEWORK_SEARCH_PATHS = (
					"$(inherited)",
					../../../../engine/lib/openal/macosx,
					../../../../engine/lib/xiph/macosx,
				);
                GCC_ENABLE_CPP_EXCEPTIONS = YES;
				GCC_ENABLE_FIX_AND_CONTINUE = NO;
				GCC_GENERATE_DEBUGGING_SYMBOLS = YES;
				GCC_MODEL_TUNING = G5;
				GCC_OPTIMIZATION_LEVEL = s;
            GCC_PREPROCESSOR_DEFINITIONS = (
					"$(TORQUE_DEBUG_DEFINE)",
               __MACOSX__,
					WIN32_LEAN_AND_MEAN,
					NOMINMAX,
					TORQUE_MULTITHREAD,
					TORQUE_DISABLE_MEMORY_MANAGER,
					TORQUE_UNICODE,
[foreach item=def from=$projDefines]
               "[$def]",
[/foreach]
            );
				GCC_TREAT_NONCONFORMANT_CODE_ERRORS_AS_WARNINGS = YES;
				GCC_WARN_INHIBIT_ALL_WARNINGS = YES;
				HEADER_SEARCH_PATHS = (
[foreach item=def from=$projIncludes]
               "[$def]",
[/foreach]
				);
				INFOPLIST_FILE = "Info.plist";
				LIBRARY_SEARCH_PATHS = "$(TORQUE_BUILT_LIBS_DIR)";
				OTHER_LDFLAGS = "-lz";
				PREBINDING = NO;
				PRODUCT_NAME = "[$projName]";
				TORQUE_DEBUG_DEFINE = "";
				WARNING_CFLAGS = "-Wall -Wno-sign-compare -Wpointer-arith";
				WRAPPER_EXTENSION = app;
				ZERO_LINK = NO;
			};
			name = Release;
		};
/* End XCBuildConfiguration section */

/* Begin XCConfigurationList section */
		D268347F0C02C5750020EE4F /* Build configuration list for PBXProject [$projName] */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				D26834800C02C5750020EE4F /* Debug */,
				D26834810C02C5750020EE4F /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
		D26834AF0C02C77B0020EE4F /* Build configuration list for PBXNativeTarget [$projName] */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				D26834B00C02C77B0020EE4F /* Debug */,
				D26834B10C02C77B0020EE4F /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
/* End XCConfigurationList section */
	};
	rootObject = D268347E0C02C5750020EE4F /* Project object */;
}
