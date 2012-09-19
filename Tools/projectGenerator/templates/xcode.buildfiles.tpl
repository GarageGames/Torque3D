[if is_array($dirWalk)]
[foreach from=$dirWalk item=dir key=key]
[include file="xcode.buildfiles.tpl" dirWalk=$dir dirName=$key parentDir=$dirName]
[/foreach]
[elseif strcasecmp( pathinfo( $dirWalk->name, PATHINFO_EXTENSION ), "h" ) != 0]
      B[$dirWalk->hash] = /* [$dirWalk->name] in [$parentDir] */ {isa = PBXBuildFile; fileRef = F[$dirWalk->hash] /* [$dirWalk->name] */; };
[/if]