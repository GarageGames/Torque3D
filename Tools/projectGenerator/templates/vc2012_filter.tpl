<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
   <ItemGroup>
      <Filter Include="Source Files">
         <UniqueIdentifier>{gen_uuid}</UniqueIdentifier>
         <Extensions>cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;cc</Extensions>
      </Filter>
{foreach item=folder from=$Folders}
      <Filter Include="Source Files\{$folder}">
         <UniqueIdentifier>{gen_uuid}</UniqueIdentifier>
      </Filter>
{/foreach}
   </ItemGroup>
  <ItemGroup>
    <ResourceCompile Include="Torque.rc">
      <Filter>Source Files</Filter>
    </ResourceCompile>
  </ItemGroup>
   <ItemGroup>
{foreach item=dir key=path from=$SrcFiles}
      <ClCompile Include="{$path}">
         <Filter>Source Files\{$dir}</Filter>
      </ClCompile>
{/foreach}
   </ItemGroup>
   <ItemGroup>
{foreach item=dir key=path from=$IncFiles}
      <ClInclude Include="{$path}">
         <Filter>Source Files\{$dir}</Filter>
      </ClInclude>
{/foreach}
   </ItemGroup>
   <ItemGroup>
{foreach item=dir key=path from=$OtherFiles}
      <CustomBuild Include="{$path}">
         <Filter>Source Files\{$dir}</Filter>
      </CustomBuild>
{/foreach}
   </ItemGroup>
</Project>