<?xml version="1.0" encoding="utf-8"?>
<!-- Shared Template -->
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Optimized Debug|Win32">
      <Configuration>Optimized Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
   <RootNamespace>{$projName}</RootNamespace>
    <ProjectGuid>{$GUID}</ProjectGuid>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseOfMfc>false</UseOfMfc>
	<PlatformToolset>v110</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseOfMfc>false</UseOfMfc>
	<PlatformToolset>v110</PlatformToolset>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseOfMfc>false</UseOfMfc>
	<PlatformToolset>v110</PlatformToolset>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="PropertySheets">
    <Import Project="Torque.Cpp.$(Platform).user.props" Condition="exists('Torque.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'" Label="PropertySheets">
    <Import Project="Torque.Cpp.$(Platform).user.props" Condition="exists('Torque.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="PropertySheets">
    <Import Project="Torque.Cpp.$(Platform).user.props" Condition="exists('Torque.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup>
    <_ProjectFileVersion>10.0.30319.1</_ProjectFileVersion>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">{$projectOffset}../../{$gameFolder}/</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName)/$(ProjectName)/</IntDir>
    <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</LinkIncremental>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">{$projOutName}_DEBUG</TargetName>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'">{$projectOffset}../../{$gameFolder}/</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'">{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName)/$(ProjectName)/</IntDir>
    <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'">false</LinkIncremental>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'">{$projOutName}_OPTIMIZEDDEBUG</TargetName>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projectOffset}../../{$gameFolder}/</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName)/$(ProjectName)/</IntDir>
    <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">false</LinkIncremental>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projOutName}</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Midl>
      <TypeLibraryName>$(IntDir)Torque 3D.tlb</TypeLibraryName>
      <HeaderFileName>
      </HeaderFileName>
    </Midl>
    <ClCompile>
      <AdditionalOptions>/MP4 %(AdditionalOptions)</AdditionalOptions>
      <Optimization>Disabled</Optimization>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>{foreach item=def from=$projDefines}{$def};{/foreach}TORQUE_DEBUG;TORQUE_DEBUG_GUARD;D3D_DEBUG_INFO;TORQUE_NET_STATS;UNICODE;_CRT_SECURE_NO_DEPRECATE;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ExceptionHandling>Sync</ExceptionHandling>
      <BasicRuntimeChecks>Default</BasicRuntimeChecks>
      <StringPooling>true</StringPooling>
      <RuntimeLibrary>{if $projRuntimeDebug == 1}MultiThreadedDebug{else}MultiThreadedDebugDLL{/if}</RuntimeLibrary>
      <BufferSecurityCheck>false</BufferSecurityCheck>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>
      <RuntimeTypeInfo>true</RuntimeTypeInfo>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <AssemblerListingLocation>$(IntDir)</AssemblerListingLocation>
      <WarningLevel>Level4</WarningLevel>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <DebugInformationFormat>EditAndContinue</DebugInformationFormat>
      <CompileAs>Default</CompileAs>
      <DisableSpecificWarnings>{foreach item=def from=$projDisabledWarnings}{$def};{/foreach}4018;4100;4121;4127;4130;4244;4245;4389;4511;4512;4800;%(DisableSpecificWarnings)</DisableSpecificWarnings>
    </ClCompile>
    <ResourceCompile>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Culture>0x0409</Culture>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ResourceCompile>
    <Link>
      <AdditionalDependencies>{foreach item=def from=$projLibsDebug}{$def};{/foreach}%(AdditionalDependencies)</AdditionalDependencies>
      <OutputFile>$(OutDir){$projOutName}_DEBUG.exe</OutputFile>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <AdditionalLibraryDirectories>{foreach item=def from=$projLibDirs}{$def};{/foreach}{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <IgnoreSpecificDefaultLibraries>LIBC;LIBCD;{foreach item=def from=$projLibsIgnore}{$def};{/foreach}%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <ProgramDatabaseFile>$(IntDir)$(ProjectName).pdb</ProgramDatabaseFile>
      <SubSystem>{if $projSubSystem == 1}Console{else}Windows{/if}</SubSystem>
      <TargetMachine>MachineX86</TargetMachine>
      <RandomizedBaseAddress>false</RandomizedBaseAddress>
      <DataExecutionPrevention>
      </DataExecutionPrevention>
      <ProfileGuidedDatabase>$(IntDir)$(ProjectName).pgd</ProfileGuidedDatabase>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|Win32'">
    <Midl>
      <TypeLibraryName>$(IntDir)Torque 3D.tlb</TypeLibraryName>
      <HeaderFileName>
      </HeaderFileName>
    </Midl>
    <ClCompile>
      <AdditionalOptions>/MP4 %(AdditionalOptions)</AdditionalOptions>
      <Optimization>Full</Optimization>
      <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>{foreach item=def from=$projDefines}{$def};{/foreach}TORQUE_DEBUG;TORQUE_DEBUG_GUARD;D3D_DEBUG_INFO;TORQUE_NET_STATS;UNICODE;_CRT_SECURE_NO_DEPRECATE;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ExceptionHandling>Sync</ExceptionHandling>
      <BasicRuntimeChecks>Default</BasicRuntimeChecks>
      <StringPooling>true</StringPooling>
      <RuntimeLibrary>{if $projRuntimeDebug == 1}MultiThreadedDebug{else}MultiThreadedDebugDLL{/if}</RuntimeLibrary>
      <BufferSecurityCheck>false</BufferSecurityCheck>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>
      <RuntimeTypeInfo>true</RuntimeTypeInfo>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <AssemblerListingLocation>$(IntDir)</AssemblerListingLocation>
      <WarningLevel>Level4</WarningLevel>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <CompileAs>Default</CompileAs>
      <DisableSpecificWarnings>{foreach item=def from=$projDisabledWarnings}{$def};{/foreach}4018;4100;4121;4127;4130;4244;4245;4389;4511;4512;4800;%(DisableSpecificWarnings)</DisableSpecificWarnings>
    </ClCompile>
    <ResourceCompile>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Culture>0x0409</Culture>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ResourceCompile>
    <Link>
      <AdditionalDependencies>{foreach item=def from=$projLibsDebug}{$def};{/foreach}%(AdditionalDependencies)</AdditionalDependencies>
      <OutputFile>$(OutDir){$projOutName}_OPTIMIZEDDEBUG.exe</OutputFile>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <AdditionalLibraryDirectories>{foreach item=def from=$projLibDirs}{$def};{/foreach}{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <IgnoreSpecificDefaultLibraries>LIBC;LIBCD;{foreach item=def from=$projLibsIgnore}{$def};{/foreach}%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <ProgramDatabaseFile>$(IntDir)$(ProjectName).pdb</ProgramDatabaseFile>
      <SubSystem>{if $projSubSystem == 1}Console{else}Windows{/if}</SubSystem>
      <TargetMachine>MachineX86</TargetMachine>
      <RandomizedBaseAddress>false</RandomizedBaseAddress>
      <DataExecutionPrevention>
      </DataExecutionPrevention>
      <ProfileGuidedDatabase>$(IntDir)$(ProjectName).pgd</ProfileGuidedDatabase>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Midl>
      <TypeLibraryName>$(IntDir)Torque 3D.tlb</TypeLibraryName>
      <HeaderFileName>
      </HeaderFileName>
    </Midl>
    <ClCompile>
      <AdditionalOptions>/MP4 %(AdditionalOptions)</AdditionalOptions>
      <Optimization>Full</Optimization>
      <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
      <IntrinsicFunctions>true</IntrinsicFunctions>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
      <PreprocessorDefinitions>{foreach item=def from=$projDefines}{$def};{/foreach}UNICODE;_CRT_SECURE_NO_DEPRECATE;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <ExceptionHandling>Sync</ExceptionHandling>
      <BasicRuntimeChecks>Default</BasicRuntimeChecks>
      <StringPooling>true</StringPooling>
      <RuntimeLibrary>{if $projRuntimeRelease == 0}MultiThreaded{else}MultiThreadedDLL{/if}</RuntimeLibrary>
      <BufferSecurityCheck>false</BufferSecurityCheck>
      <FunctionLevelLinking>true</FunctionLevelLinking>
      <TreatWChar_tAsBuiltInType>false</TreatWChar_tAsBuiltInType>
      <RuntimeTypeInfo>true</RuntimeTypeInfo>
      <PrecompiledHeader>
      </PrecompiledHeader>
      <AssemblerListingLocation>$(IntDir)</AssemblerListingLocation>
      <WarningLevel>Level4</WarningLevel>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
      <CompileAs>Default</CompileAs>
      <DisableSpecificWarnings>{foreach item=def from=$projDisabledWarnings}{$def};{/foreach}4018;4100;4121;4127;4130;4244;4245;4389;4511;4512;4800;%(DisableSpecificWarnings)</DisableSpecificWarnings>
    </ClCompile>
    <ResourceCompile>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Culture>0x0409</Culture>
      <AdditionalIncludeDirectories>{foreach item=def from=$projIncludes}{$def};{/foreach}%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ResourceCompile>
    <Link>
      <AdditionalDependencies>{foreach item=def from=$projLibs}{$def};{/foreach}%(AdditionalDependencies)</AdditionalDependencies>
      <OutputFile>$(OutDir){$projOutName}.exe</OutputFile>
      <SuppressStartupBanner>true</SuppressStartupBanner>
      <AdditionalLibraryDirectories>{foreach item=def from=$projLibDirs}{$def};{/foreach}{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <IgnoreSpecificDefaultLibraries>LIBC;LIBCD;{foreach item=def from=$projLibsIgnore}{$def};{/foreach}%(IgnoreSpecificDefaultLibraries)</IgnoreSpecificDefaultLibraries>
      <GenerateDebugInformation>false</GenerateDebugInformation>
      <ProgramDatabaseFile>$(IntDir)$(ProjectName).pdb</ProgramDatabaseFile>
      <SubSystem>{if $projSubSystem == 1}Console{else}Windows{/if}</SubSystem>
      <TargetMachine>MachineX86</TargetMachine>
      <RandomizedBaseAddress>false</RandomizedBaseAddress>
      <DataExecutionPrevention>
      </DataExecutionPrevention>
      <ProfileGuidedDatabase>$(IntDir)$(ProjectName).pgd</ProfileGuidedDatabase>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ResourceCompile Include="Torque.rc" />
  </ItemGroup>
  <ItemGroup>
{assign var="dirWalk" value=$fileArray}
{include file="vc2010_fileRecurse.tpl" dirWalk=$dirWalk depth=1 dirPath=$projOutput->base_dir}
  </ItemGroup>
  <ItemGroup>
  {foreach item=dep from=$projDepend}
    <ProjectReference Include="{$projectDepends[$dep]->name}.vcxproj">
      <Project>{$projectDepends[$dep]->guid}</Project>
      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
    </ProjectReference>
   {/foreach}
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Label="ExtensionTargets">
  </ImportGroup>
</Project>
