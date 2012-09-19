[assign var="dirWalk" value=$fileArray]
// !$*UTF8*$!
{
	archiveVersion = 1;
	classes = {
	};
	objectVersion = 42;
	objects = {
[assign var=libuid value=$projName|cat:"remote"|uid]
[assign var=libuid2 value=$projName|cat:"target"|uid]

/* Begin PBXBuildFile section */

   /* Begin PBXBuildFiles included by project generator */
[include file="xcode.buildfiles.tpl" dirWalk=$dirWalk]
   /* End PBXBuildFiles included by project generator */

/* End PBXBuildFile section */

/* Begin PBXFileReference section */
	  F[$libuid] /* lib[$projName].a */ = {isa = PBXFileReference; explicitFileType = archive.ar; includeInIndex = 0; path = lib[$projName].a; sourceTree = BUILT_PRODUCTS_DIR; };

   /* Begin PBXFileReferences included by project generator */
	  [include file="xcode.filerefs.tpl" dirWalk=$dirWalk ]
   /* End PBXFileReferences included by project generator */


/* End PBXFileReference section */

/* Begin PBXFrameworksBuildPhase section */
	  D27393730CDABF9C006111D4 /* Frameworks */ = {
		 isa = PBXFrameworksBuildPhase;
		 buildActionMask = 2147483647;
		 files = (
		 );
		 runOnlyForDeploymentPostprocessing = 0;
	  };
/* End PBXFrameworksBuildPhase section */

/* Begin PBXGroup section */
	  D268347C0C02C5750020EE4F = {
		 isa = PBXGroup;
		 children = (
			A00000000000000000000000 /* Code */,
			D26834AC0C02C77B0020EE4F /* Products */,
		 );
		 sourceTree = "<group>";
	  };
	  D26834AC0C02C77B0020EE4F /* Products */ = {
		 isa = PBXGroup;
		 children = (
			F[$libuid] /* lib[$projName].a */,
		 );
		 name = Products;
		 sourceTree = SOURCE_ROOT;
	  };
	  
   /* Begin PBXGroups from project generator */
[include file="xcode.groups.tpl" dirWalk=$dirWalk recurse="no" groupPath="paxorr" groupName="Code" groupHash="00000000000000000000000"]
   /* End PBXGroups from project generator */
   
/* End PBXGroup section */

/* Begin PBXNativeTarget section */
	  D[$libuid2] /* [$projName] */ = {
		 isa = PBXNativeTarget;
		 buildConfigurationList = D27393740CDABF9C006111D4 /* Build configuration list for PBXNativeTarget "[$projName]" */;
		 buildPhases = (
			D27393300CDABF9C006111D4 /* Sources */,
		 );
		 buildRules = (
		 );
		 dependencies = (
		 );
		 name = [$projName];
		 productName = lib[$projName];
		 productReference = F[$libuid] /* lib[$projName].a */;
		 productType = "com.apple.product-type.library.static";
	  };
/* End PBXNativeTarget section */

/* Begin PBXProject section */
	  D268347E0C02C5750020EE4F /* Project object */ = {
		 isa = PBXProject;
		 buildConfigurationList = D268347F0C02C5750020EE4F /* Build configuration list for PBXProject "[$projName]" */;
		 compatibilityVersion = "Xcode 3.2";
		 hasScannedForEncodings = 0;
		 mainGroup = D268347C0C02C5750020EE4F;
		 productRefGroup = D26834AC0C02C77B0020EE4F /* Products */;
		 projectDirPath = "";
		 projectRoot = ../../../..;
		 targets = (
			D[$libuid2] /* [$projName] */,
		 );
	  };
/* End PBXProject section */

/* Begin PBXSourcesBuildPhase section */
		D27393300CDABF9C006111D4 /* Sources */ = {
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
			buildSettings = {
				ARCHS = "$(ARCHS_STANDARD_32_BIT)";
				CONFIGURATION_BUILD_DIR = "$(TORQUE_BUILT_LIBS_DIR)";
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
			buildSettings = {
				ARCHS = "$(ARCHS_STANDARD_32_BIT)";
				CONFIGURATION_BUILD_DIR = ../../assets/;
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
		D27393750CDABF9C006111D4 /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				COPY_PHASE_STRIP = NO;
				DEAD_CODE_STRIPPING = YES;
				GCC_DYNAMIC_NO_PIC = NO;
				GCC_ENABLE_CPP_EXCEPTIONS = NO;
				GCC_ENABLE_FIX_AND_CONTINUE = NO;
				GCC_GENERATE_DEBUGGING_SYMBOLS = YES;
				GCC_MODEL_TUNING = G5;
				GCC_OPTIMIZATION_LEVEL = 0;
				GCC_PREPROCESSOR_DEFINITIONS = (
			   __MACOSX__,
[foreach item=def from=$projDefines]
			   "[$def]",
[/foreach]
			);
				GENERATE_MASTER_OBJECT_FILE = YES;
				HEADER_SEARCH_PATHS = (
[foreach item=def from=$projIncludes]
				"[$def]",
[/foreach]
				);
				INSTALL_PATH = /usr/local/lib;
				PREBINDING = NO;
				PRODUCT_NAME = [$projName];
				ZERO_LINK = NO;
			};
			name = Debug;
		};
		D27393760CDABF9C006111D4 /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				ALWAYS_SEARCH_USER_PATHS = NO;
				CONFIGURATION_BUILD_DIR = "$(TORQUE_BUILT_LIBS_DIR)";
				COPY_PHASE_STRIP = YES;
				DEAD_CODE_STRIPPING = YES;
				DEBUG_INFORMATION_FORMAT = dwarf;
				GCC_DYNAMIC_NO_PIC = NO;
				GCC_ENABLE_CPP_EXCEPTIONS = NO;
				GCC_ENABLE_FIX_AND_CONTINUE = NO;
				GCC_GENERATE_DEBUGGING_SYMBOLS = NO;
				GCC_MODEL_TUNING = G5;
				GCC_OPTIMIZATION_LEVEL = s;
				GCC_PREPROCESSOR_DEFINITIONS = (
			   __MACOSX__,
[foreach item=def from=$projDefines]
			   "[$def]",
[/foreach]
				);
				GENERATE_MASTER_OBJECT_FILE = YES;
				HEADER_SEARCH_PATHS = (
[foreach item=def from=$projIncludes]
				"[$def]",
[/foreach]
				);
				INSTALL_PATH = /usr/local/lib;
				PREBINDING = NO;
				PRODUCT_NAME = [$projName];
				ZERO_LINK = NO;
			};
			name = Release;
		};
/* End XCBuildConfiguration section */

/* Begin XCConfigurationList section */
		D268347F0C02C5750020EE4F /* Build configuration list for PBXProject "LibTemplate" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				D26834800C02C5750020EE4F /* Debug */,
				D26834810C02C5750020EE4F /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
		D27393740CDABF9C006111D4 /* Build configuration list for PBXNativeTarget "[$projName]" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				D27393750CDABF9C006111D4 /* Debug */,
				D27393760CDABF9C006111D4 /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};
/* End XCConfigurationList section */
	};
	rootObject = D268347E0C02C5750020EE4F /* Project object */;
}
