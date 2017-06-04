{*                                   *}
{*      Is this a dir or an item?    *}
{*                                   *}
{if is_array($dirWalk)}

   {*                                   *}
   {*      Iterate over children        *}
   {*                                   *}
   {foreach from=$dirWalk item=dir key=key}
   {include file="vc2010_fileRecurse.tpl" dirWalk=$dir dirName=$key dirPath="$dirPath$dirName/" depth=$depth+1}
   {/foreach}
   
{else}

   {*                                   *}
   {*      Output an item               *}
   {*                                   *}
   {capture assign="itemOut"}
   {* we don't compile some files. *}
   {if dontCompile($dirWalk->path, $projOutput)}
      <ClCompile Include=
      "{$dirWalk->path|replace:'//':'/'|replace:'/':'\\'}">
            <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">true</ExcludedFromBuild>
            <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|x64'">true</ExcludedFromBuild>
            <ExcludedFromBuild Condition="'$(Configuration)|$(Platform)'=='Release|x64'">true</ExcludedFromBuild>
      </ClCompile>
   {else}
      {if substr($dirWalk->path, -4, 4) == ".asm"}
         <CustomBuild Include="{$dirWalk->path|replace:'//':'/'|replace:'/':'\\'}">
            <Command Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">"{$binDir|replace:'//':'/'|replace:'/':'\\'}nasm\nasm.exe" -f x64 "%(FullPath)" -o "$(IntDir)%(Filename).obj"</Command>
            <Outputs Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(IntDir)%(Filename).obj;%(Outputs)</Outputs>
            <Command Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|x64'">"{$binDir|replace:'//':'/'|replace:'/':'\\'}nasm\nasm.exe" -f x64 "%(FullPath)" -o "$(IntDir)%(Filename).obj"</Command>
            <Outputs Condition="'$(Configuration)|$(Platform)'=='Optimized Debug|x64'">$(IntDir)%(Filename).obj;%(Outputs)</Outputs>
            <Command Condition="'$(Configuration)|$(Platform)'=='Release|x64'">"{$binDir|replace:'//':'/'|replace:'/':'\\'}nasm\nasm.exe" -f x64 "%(FullPath)" -o "$(IntDir)%(Filename).obj"</Command>
            <Outputs Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(IntDir)%(Filename).obj;%(Outputs)</Outputs>
         </CustomBuild>
      {elseif $projOutput->isSourceFile( $dirWalk->path ) }
         <ClCompile Include="{$dirWalk->path|replace:'//':'/'|replace:'/':'\\'}" />      
      {elseif $projOutput->isResourceFile( $dirWalk->path ) }
         <ResourceCompile Include="{$dirWalk->path|replace:'//':'/'|replace:'/':'\\'}" />      
      {else}
         <ClInclude Include="{$dirWalk->path|replace:'//':'/'|replace:'/':'\\'}" />      
      {/if}{* if path == "*.asm" *}   
   {/if}{* if dontCompile() *}
   {/capture}
   {$itemOut|indent:$depth:"\t"}
   
{/if}