{if is_array($dirWalk)}
{foreach from=$dirWalk item=dir key=key}
{include file="vc2k8_fileRecurseCSharp.tpl" dirWalk=$dir dirName=$key dirPath="$dirPath$dirName/" depth=$depth+1}
{/foreach}
{else}
<Compile Include="{$dirWalk->path|replace:'/':'\\'}" >
   <Link>{$dirWalk->path|replace:'../':''|replace:'/':'\\'}</Link>
</Compile>
{/if}