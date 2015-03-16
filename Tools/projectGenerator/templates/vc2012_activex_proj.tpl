<?xml version="1.0" encoding="utf-8"?>
<!-- ActiveX Project Template -->
<Project DefaultTargets="Build" ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{$GUID}</ProjectGuid>
    <Keyword>AtlProj</Keyword>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <UseOfAtl>Static</UseOfAtl>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>DynamicLibrary</ConfigurationType>
    <UseOfAtl>Static</UseOfAtl>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Label="ExtensionSettings">
  </ImportGroup>
  <ImportGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="PropertySheets">
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
    <IgnoreImportLibrary Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</IgnoreImportLibrary>
    <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">true</LinkIncremental>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">{$projOutName}</TargetName>
    <OutDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projectOffset}../../{$gameFolder}/</OutDir>
    <IntDir Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projectOffset}../Link/VC2010.$(Configuration).$(PlatformName)/$(ProjectName)/</IntDir>
    <IgnoreImportLibrary Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">true</IgnoreImportLibrary>
    <LinkIncremental Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">false</LinkIncremental>
    <TargetName Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">{$projOutName}</TargetName>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
    <Midl>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <MkTypLibCompatible>false</MkTypLibCompatible>
      <TargetEnvironment>Win32</TargetEnvironment>
      <GenerateStublessProxies>true</GenerateStublessProxies>
      <TypeLibraryName>$(IntDir)IEWebGamePlugin.tlb</TypeLibraryName>
      <HeaderFileName>..\..\..\web\source\activex\IEWebGamePlugin_i.h</HeaderFileName>
      <DllDataFileName>
      </DllDataFileName>
      <InterfaceIdentifierFileName>..\..\..\web\source\activex\IEWebGamePlugin_i.c</InterfaceIdentifierFileName>
      <ProxyFileName>..\..\..\web\source\activex\IEWebGamePlugin_p.c</ProxyFileName>
      <ValidateAllParameters>true</ValidateAllParameters>
    </Midl>
    <ClCompile>
      <Optimization>Disabled</Optimization>
      <PreprocessorDefinitions>WIN32;_WINDOWS;_DEBUG;_USRDLL;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <MinimalRebuild>true</MinimalRebuild>
      <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
      <RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>
      <PrecompiledHeader>Use</PrecompiledHeader>
      <WarningLevel>Level3</WarningLevel>
      <DebugInformationFormat>EditAndContinue</DebugInformationFormat>
    </ClCompile>
    <ResourceCompile>
      <PreprocessorDefinitions>_DEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Culture>0x0409</Culture>
      <AdditionalIncludeDirectories>$(IntDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ResourceCompile>
    <Link>
      <PerUserRedirection>true</PerUserRedirection>
      <RegisterOutput>true</RegisterOutput>
      <ModuleDefinitionFile>..\..\..\web\source\activex\IEWebGamePlugin.def</ModuleDefinitionFile>
      <GenerateDebugInformation>true</GenerateDebugInformation>
      <SubSystem>Windows</SubSystem>
      <TargetMachine>MachineX86</TargetMachine>
      <OutputFile>$(OutDir){$projOutName}.dll</OutputFile>
      <ProgramDatabaseFile>$(IntDir)$(ProjectName).pdb</ProgramDatabaseFile>
      <AdditionalDependencies>version.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <Midl>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <MkTypLibCompatible>false</MkTypLibCompatible>
      <TargetEnvironment>Win32</TargetEnvironment>
      <GenerateStublessProxies>true</GenerateStublessProxies>
      <TypeLibraryName>$(IntDir)IEWebGamePlugin.tlb</TypeLibraryName>
      <HeaderFileName>..\..\..\web\source\activex\IEWebGamePlugin_i.h</HeaderFileName>
      <DllDataFileName>
      </DllDataFileName>
      <InterfaceIdentifierFileName>..\..\..\web\source\activex\IEWebGamePlugin_i.c</InterfaceIdentifierFileName>
      <ProxyFileName>..\..\..\web\source\activex\IEWebGamePlugin_p.c</ProxyFileName>
      <ValidateAllParameters>true</ValidateAllParameters>
    </Midl>
    <ClCompile>
      <Optimization>MaxSpeed</Optimization>
      <PreprocessorDefinitions>WIN32;_WINDOWS;NDEBUG;_USRDLL;_CRT_SECURE_NO_WARNINGS;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <RuntimeLibrary>MultiThreaded</RuntimeLibrary>
      <PrecompiledHeader>Use</PrecompiledHeader>
      <WarningLevel>Level3</WarningLevel>
      <DebugInformationFormat>
      </DebugInformationFormat>
    </ClCompile>
    <ResourceCompile>
      <PreprocessorDefinitions>NDEBUG;%(PreprocessorDefinitions)</PreprocessorDefinitions>
      <Culture>0x0409</Culture>
      <AdditionalIncludeDirectories>$(IntDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    </ResourceCompile>
    <Link>
      <PerUserRedirection>true</PerUserRedirection>
      <RegisterOutput>true</RegisterOutput>
      <ModuleDefinitionFile>..\..\..\web\source\activex\IEWebGamePlugin.def</ModuleDefinitionFile>
      <GenerateDebugInformation>false</GenerateDebugInformation>
      <SubSystem>Windows</SubSystem>
      <OptimizeReferences>true</OptimizeReferences>
      <EnableCOMDATFolding>true</EnableCOMDATFolding>
      <TargetMachine>MachineX86</TargetMachine>
      <OutputFile>$(OutDir){$projOutName}.dll</OutputFile>
      <AdditionalDependencies>version.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <ProgramDatabaseFile>$(IntDir)$(ProjectName).pdb</ProgramDatabaseFile>
      <ProfileGuidedDatabase>$(IntDir)$(ProjectName).pgd</ProfileGuidedDatabase>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="..\..\..\web\source\activex\dllmain.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
      </PrecompiledHeader>
      <CompileAsManaged Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">false</CompileAsManaged>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
      </PrecompiledHeader>
      <CompileAsManaged Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">false</CompileAsManaged>
    </ClCompile>
    <ClCompile Include="..\..\..\web\source\activex\IEWebGameCtrl.cpp" />
    <ClCompile Include="..\..\..\web\source\activex\IEWebGamePlugin.cpp" />
    <ClCompile Include="..\..\..\web\source\activex\IEWebGameWindow.cpp" />
    <ClCompile Include="..\..\..\web\source\common\webCommon.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
      </PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
      </PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\..\..\web\source\activex\stdafx.cpp">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Create</PrecompiledHeader>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Create</PrecompiledHeader>
    </ClCompile>
    <ClCompile Include="..\..\..\web\source\activex\IEWebGamePlugin_i.c">
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
      </PrecompiledHeader>
      <CompileAsManaged Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">false</CompileAsManaged>
      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
      </PrecompiledHeader>
      <CompileAsManaged Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">false</CompileAsManaged>
    </ClCompile>
  </ItemGroup>
  <ItemGroup>
    <None Include="..\..\..\web\source\activex\IEWebGamePlugin.def" />
    <None Include="..\..\..\web\source\activex\IEWebGameCtrl.bmp" />
    <None Include="..\..\..\web\source\activex\IEWebGameCtrl.rgs" />
    <None Include="..\..\..\web\source\activex\IEWebGamePlugin.rgs" />
  </ItemGroup>
  <ItemGroup>
    <Midl Include="..\..\..\web\source\activex\IEWebGamePlugin.idl" />
  </ItemGroup>
  <ItemGroup>
    <ClInclude Include="..\..\..\web\source\activex\dllmain.h" />
    <ClInclude Include="..\..\..\web\source\activex\IEWebGameCtrl.h" />
    <ClInclude Include="..\..\..\web\source\activex\IEWebGameWindow.h" />
    <ClInclude Include="..\..\..\web\source\activex\Resource.h" />
    <ClInclude Include="..\..\..\web\source\activex\stdafx.h" />
    <ClInclude Include="..\..\..\web\source\activex\targetver.h" />
    <ClInclude Include="..\..\..\web\source\common\webConfig.h" />
    <ClInclude Include="..\..\..\web\source\common\webCommon.h" />
    <ClInclude Include="..\..\..\web\source\activex\IEWebGamePlugin_i.h" />
  </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="..\..\..\web\source\activex\IEWebGamePlugin.rc" />
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
