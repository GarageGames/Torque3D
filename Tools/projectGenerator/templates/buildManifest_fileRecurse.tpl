{*                                   *}
{*      Is this a dir or an item?    *}
{*                                   *}
{if is_array($dirWalk)}
{*                                   *}
{*      Iterate over children        *}
{*                                   *}
{foreach from=$dirWalk item=dir key=key}
{include file="buildManifest_fileRecurse.tpl" dirWalk=$dir dirName=$key dirPath="$dirPath$dirName/" depth=$depth+1}
{/foreach}
{else}
{*                                   *}
{*      Output an item               *}
{*                                   *}
{$dirWalk->path}
{/if}