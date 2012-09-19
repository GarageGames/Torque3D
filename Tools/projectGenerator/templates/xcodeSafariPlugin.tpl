// !$*UTF8*$!
{
	archiveVersion = 1;
	classes = {
	};
	objectVersion = 44;
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
		4315B01B0F3A517D002F4B9F /* npWebGamePlugin.mm in Sources */ = {isa = PBXBuildFile; fileRef = 4315B01A0F3A517D002F4B9F /* npWebGamePlugin.mm */; };
		43E38C570FE438E300E9AAA9 /* npPlugin.cpp in Sources */ = {isa = PBXBuildFile; fileRef = 43E38C570FD438E300E9AAA9 /* npPlugin.cpp */; };
		43E38C5A0FE4393700E9AAA9 /* webCommon.cpp in Sources */ = {isa = PBXBuildFile; fileRef = 43E38C5A0FD4393700E9AAA9 /*  webCommon.cpp */; };
		4315B02C0F3A5261002F4B9F /* Cocoa.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = 4315B02B0F3A5261002F4B9F /* Cocoa.framework */; };
		8D01CCCA0486CAD60068D4B7 /* InfoPlist.strings in Resources */ = {isa = PBXBuildFile; fileRef = 089C167DFE841241C02AAC07 /* InfoPlist.strings */; };
		8D01CCCE0486CAD60068D4B7 /* Carbon.framework in Frameworks */ = {isa = PBXBuildFile; fileRef = 08EA7FFBFE8413EDC02AAC07 /* Carbon.framework */; };

   /* Begin PBXBuildFiles for built lib dependencies */
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
   F[$libuid] /* lib[$def].a in Frameworks */ = {isa = PBXBuildFile; fileRef = D[$libuid] /* lib[$def].a */; };
[/foreach]
   /* End PBXBuildFiles for built lib dependencies */
/* End PBXBuildFile section */

/* Begin PBXFileReference section */

[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
      C[$libuid] /* [$def].xcodeproj */ = {isa = PBXFileReference; lastKnownFileType = "wrapper.pb-project"; name = "[$def].xcodeproj"; path = "[$def].xcodeproj"; sourceTree = SOURCE_ROOT; };
[/foreach]

		089C167EFE841241C02AAC07 /* English */ = {isa = PBXFileReference; fileEncoding = 10; lastKnownFileType = text.plist.strings; name = English; path = ../../web/source/npplugin/mac/English.lproj/InfoPlist.strings; sourceTree = "<group>"; };
		08EA7FFBFE8413EDC02AAC07 /* Carbon.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Carbon.framework; path = /System/Library/Frameworks/Carbon.framework; sourceTree = "<absolute>"; };
		32BAE0B30371A71500C91783 /* WebGamePlugin_Prefix.pch */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = WebGamePlugin_Prefix.pch; path = ../../web/source/npplugin/mac/WebGamePlugin_Prefix.pch; sourceTree = "<group>"; };
		43E38C570FD438E300E9AAA9 /* npPlugin.cpp */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.cpp.objcpp; name = npPlugin.cpp; path = ../../web/source/npplugin/npPlugin.cpp; sourceTree = SOURCE_ROOT; };
		43E38C590BC4393700E9AAA9 /* webConfig.h */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = webConfig.h; path = ../../web/source/common/webConfig.h; sourceTree = SOURCE_ROOT; };
		43E38C590FD4393700E9AAA9 /* webCommon.h */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = webCommon.h; path = ../../web/source/common/webCommon.h; sourceTree = SOURCE_ROOT; };
		43E38C5A0FD4393700E9AAA9 /* webCommon.cpp */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.cpp.objcpp; name = webCommon.cpp; path = ../../web/source/common/webCommon.cpp; sourceTree = SOURCE_ROOT; };		
                4315B01A0F3A517D002F4B9F /* npWebGamePlugin.mm */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.cpp.objcpp; name = npWebGamePlugin.mm; path = ../../web/source/npplugin/mac/npWebGamePlugin.mm; sourceTree = "<group>"; };
		4315B01A0F3A627D002F4B9F /* npWebGamePlugin.h */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = npWebGamePlugin.h; path = ../../web/source/npplugin/mac/npWebGamePlugin.h; sourceTree = "<group>"; };
		4315B02B0F3A5261002F4B9F /* Cocoa.framework */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = Cocoa.framework; path = /System/Library/Frameworks/Cocoa.framework; sourceTree = "<absolute>"; };
		8D01CCD10486CAD60068D4B7 /* Info.plist */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = text.plist; name = Info.plist; path = ../../web/source/npplugin/mac/Info.plist; sourceTree = "<group>"; };
		8D01CCD20486CAD60068D4B7 /* WebGamePlugin.bundle */ = {isa = PBXFileReference; explicitFileType = wrapper.cfbundle; includeInIndex = 0; path = "[$projName].bundle"; sourceTree = BUILT_PRODUCTS_DIR; };
/* End PBXFileReference section */

/* Begin PBXFrameworksBuildPhase section */
		8D01CCCD0486CAD60068D4B7 /* Frameworks */ = {
			isa = PBXFrameworksBuildPhase;
			buildActionMask = 2147483647;
			files = (
				8D01CCCE0486CAD60068D4B7 /* Carbon.framework in Frameworks */,
				4315B02C0F3A5261002F4B9F /* Cocoa.framework in Frameworks */,
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|uid]
            F[$libuid] /* lib[$def].a */,
[/foreach]

			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXFrameworksBuildPhase section */

/* Begin PBXGroup section */
		089C166AFE841209C02AAC07 /* WebGamePlugin */ = {
			isa = PBXGroup;
			children = (
				08FB77ADFE841716C02AAC07 /* Source */,
				AEEEE0000000000000000000 /* Dependencies */,
				089C167CFE841241C02AAC07 /* Resources */,
				089C1671FE841209C02AAC07 /* External Frameworks and Libraries */,
				19C28FB4FE9D528D11CA2CBB /* Products */,
			);
			name = "[$projName]";
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

		089C1671FE841209C02AAC07 /* External Frameworks and Libraries */ = {
			isa = PBXGroup;
			children = (
				4315B02B0F3A5261002F4B9F /* Cocoa.framework */,
				08EA7FFBFE8413EDC02AAC07 /* Carbon.framework */,
			);
			name = "External Frameworks and Libraries";
			sourceTree = "<group>";
		};
		089C167CFE841241C02AAC07 /* Resources */ = {
			isa = PBXGroup;
			children = (
				8D01CCD10486CAD60068D4B7 /* Info.plist */,
				089C167DFE841241C02AAC07 /* InfoPlist.strings */,
			);
			name = Resources;
			sourceTree = "<group>";
		};
		08FB77ADFE841716C02AAC07 /* Source */ = {
			isa = PBXGroup;
			children = (
				4315B01A0F3A517D002F4B9F /* npWebGamePlugin.mm */,
				4315B01A0F3A627D002F4B9F /* npWebGamePlugin.h */,
				43E38C570FD438E300E9AAA9 /* npPlugin.cpp */,
				43E38C590BC4393700E9AAA9 /* webConfig.h */, 
				43E38C590FD4393700E9AAA9 /* webCommon.h */,
				43E38C5A0FD4393700E9AAA9 /* webCommon.cpp */, 	
				32BAE0B30371A71500C91783 /* WebGamePlugin_Prefix.pch */,
			);
			name = Source;
			sourceTree = "<group>";
		};
		19C28FB4FE9D528D11CA2CBB /* Products */ = {
			isa = PBXGroup;
			children = (
				8D01CCD20486CAD60068D4B7 /* WebGamePlugin.bundle */,
			);
			name = Products;
			sourceTree = "<group>";
		};
/* End PBXGroup section */

/* Begin PBXNativeTarget section */
		8D01CCC60486CAD60068D4B7 /* WebGamePlugin */ = {
			isa = PBXNativeTarget;
			buildConfigurationList = 4FADC23308B4156C00ABE55E /* Build configuration list for PBXNativeTarget "WebGamePlugin" */;
			buildPhases = (
				8D01CCC90486CAD60068D4B7 /* Resources */,
				8D01CCCB0486CAD60068D4B7 /* Sources */,
				8D01CCCD0486CAD60068D4B7 /* Frameworks */,
			);
			buildRules = (
			);
			dependencies = (
[foreach item=def from=$projDepend]
[assign var=libuid value=$def|cat:"dep"|uid]
            D[$libuid] /* PBXTargetDependency */,
[/foreach]

			);
			name = "[$projName]";
			productInstallPath = "$(HOME)/Library/Bundles";
			productName = "[$projName]";
			productReference = 8D01CCD20486CAD60068D4B7 /* WebGamePlugin.bundle */;
			productType = "com.apple.product-type.bundle";
		};
/* End PBXNativeTarget section */

/* Begin PBXProject section */
		089C1669FE841209C02AAC07 /* Project object */ = {
			isa = PBXProject;
			buildConfigurationList = 4FADC23708B4156C00ABE55E /* Build configuration list for PBXProject "WebGamePlugin" */;
			compatibilityVersion = "Xcode 3.2";
			hasScannedForEncodings = 1;
			mainGroup = 089C166AFE841209C02AAC07 /* WebGamePlugin */;
			projectDirPath = "";
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

			targets = (
				8D01CCC60486CAD60068D4B7 /* WebGamePlugin */,
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
		8D01CCC90486CAD60068D4B7 /* Resources */ = {
			isa = PBXResourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				8D01CCCA0486CAD60068D4B7 /* InfoPlist.strings in Resources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXResourcesBuildPhase section */

/* Begin PBXSourcesBuildPhase section */
		8D01CCCB0486CAD60068D4B7 /* Sources */ = {
			isa = PBXSourcesBuildPhase;
			buildActionMask = 2147483647;
			files = (
				4315B01B0F3A517D002F4B9F /* npWebGamePlugin.mm in Sources */,
				43E38C570FE438E300E9AAA9 /* npPlugin.cpp in Sources */,
				43E38C5A0FE4393700E9AAA9 /* webCommon.cpp in Sources */,
			);
			runOnlyForDeploymentPostprocessing = 0;
		};
/* End PBXSourcesBuildPhase section */

/* Begin PBXVariantGroup section */
		089C167DFE841241C02AAC07 /* InfoPlist.strings */ = {
			isa = PBXVariantGroup;
			children = (
				089C167EFE841241C02AAC07 /* English */,
			);
			name = InfoPlist.strings;
			sourceTree = "<group>";
		};
/* End PBXVariantGroup section */

/* Begin XCBuildConfiguration section */
		4FADC23408B4156C00ABE55E /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				COPY_PHASE_STRIP = NO;
				GCC_DYNAMIC_NO_PIC = NO;
				GCC_ENABLE_FIX_AND_CONTINUE = YES;
				GCC_MODEL_TUNING = G5;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_PRECOMPILE_PREFIX_HEADER = YES;
				GCC_PREFIX_HEADER = ../../web/source/npplugin/mac/WebGamePlugin_Prefix.pch;
				INFOPLIST_FILE = ../../web/source/npplugin/mac/Info.plist;
				INSTALL_PATH = "$(HOME)/Library/Internet Plug-Ins";
				PRODUCT_NAME = "[$projName]";
				WRAPPER_EXTENSION = bundle;
				ZERO_LINK = YES;
                                GCC_PREPROCESSOR_DEFINITIONS = ( "DEBUG=1" );
			};
			name = Debug;
		};
		4FADC23508B4156C00ABE55E /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ARCHS = "$(ARCHS_STANDARD_32_BIT)";
				CONFIGURATION_BUILD_DIR = "$HOME/Library/Internet Plug-Ins";
				DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
				GCC_MODEL_TUNING = G5;
				GCC_PRECOMPILE_PREFIX_HEADER = YES;
				GCC_PREFIX_HEADER = ../../web/source/npplugin/mac/WebGamePlugin_Prefix.pch;
				INFOPLIST_FILE = ../../web/source/npplugin/mac/Info.plist;
				INSTALL_PATH = "$HOME/Library/Internet Plug-Ins";
				PRODUCT_NAME = "[$projName]";
				WRAPPER_EXTENSION = bundle;
			};
			name = Release;
		};
		4FADC23808B4156C00ABE55E /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				CONFIGURATION_BUILD_DIR = "$HOME/Library/Internet Plug-Ins";
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNUSED_VARIABLE = YES;
				PREBINDING = NO;
            GCC_VERSION = 4.2;
				SDKROOT = "$(DEVELOPER_SDK_DIR)/MacOSX10.5.sdk";
			};
			name = Debug;
		};
		4FADC23908B4156C00ABE55E /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				CONFIGURATION_BUILD_DIR = "$HOME/Library/Internet Plug-Ins";
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
				GCC_WARN_UNUSED_VARIABLE = YES;
				PREBINDING = NO;
            GCC_VERSION = 4.2;
				SDKROOT = "$(DEVELOPER_SDK_DIR)/MacOSX10.5.sdk";
			};
			name = Release;
		};
/* End XCBuildConfiguration section */

/* Begin XCConfigurationList section */
		4FADC23308B4156C00ABE55E /* Build configuration list for PBXNativeTarget "WebGamePlugin" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				4FADC23408B4156C00ABE55E /* Debug */,
				4FADC23508B4156C00ABE55E /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
		4FADC23708B4156C00ABE55E /* Build configuration list for PBXProject "WebGamePlugin" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				4FADC23808B4156C00ABE55E /* Debug */,
				4FADC23908B4156C00ABE55E /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
/* End XCConfigurationList section */
	};
	rootObject = 089C1669FE841209C02AAC07 /* Project object */;
}
