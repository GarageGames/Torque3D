[if is_array($dirWalk)]
[foreach from=$dirWalk item=dir key=key]
[include file="xcode.filerefs.tpl" dirWalk=$dir dirName=$key parentDir=$dirName]
[/foreach]
[else]
      F[$dirWalk->hash] = /* [$dirWalk->name] */ {isa = PBXFileReference; sourceTree = SOURCE_ROOT; name = [$dirWalk->name]; path = [$dirWalk->path]; };
[/if]