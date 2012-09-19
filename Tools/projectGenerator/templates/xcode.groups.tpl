[*                                  *]
[* emit a PBXGroup for this folder  *]
[*                                  *]
[if is_array($dirWalk)]
		A[$groupHash] /* [$groupName] */ = {
			isa = PBXGroup;
			children = (
[foreach from=$dirWalk item=child key=key]
[if is_array($child)]
[assign var='childpath' value=$groupPath|cat:"/"|cat:$key]
[assign var=childhash value=$childpath|uid]
            A[$childhash] /* [$key] -- [$childpath] */,
[else]
            F[$child->hash] /* [$child->name] */,
[/if]
[/foreach]
			);
			name = [$groupName];
			sourceTree = "<group>"; /* [$groupPath] */
         };
         
[/if]
[*                                              *]
[* now we emit a group for every child folder   *]
[*                                              *]
[foreach from=$dirWalk item=child key=key]
[if is_array($child)]
[assign var='childpath' value=$groupPath|cat:"/"|cat:$key]
[assign var='childhash' value=$childpath|uid]
[include file="xcode.groups.tpl" dirWalk=$child groupPath=$childpath groupName=$key groupHash=$childhash]
[/if]
[/foreach]
[*----------------------------------------------------------------------------*]
[* groupPath does not correctly correspond to the file system, and so it is   *]
[* only used for getting the groupHash.                                       *]
[*----------------------------------------------------------------------------*]
