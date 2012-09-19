{foreach from=$dirWalk item=file key=key}
{if is_array($file)}
{include file="make_fileRecurse.tpl" dirWalk=$file}
{elseif dontCompile($file->path, $projOutput)}
{else}
{$file->path} \
{/if}
{/foreach}
