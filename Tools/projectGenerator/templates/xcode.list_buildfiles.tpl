[if is_array($dirWalk)]
[foreach from=$dirWalk item=dir key=key]
[include file="xcode.list_buildfiles.tpl" dirWalk=$dir dirName=$key parentDir=$dirName]
[/foreach]
[else]
[if !dontCompile($dirWalk->path, $projOutput)]
      B[$dirWalk->hash] /* [$dirWalk->name] in [$parentDir] */,
[/if]
[/if]