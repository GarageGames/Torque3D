{assign var="dirWalk" value=$fileArray}
{include file="buildManifest_fileRecurse.tpl" dirWalk=$dirWalk depth=2 dirPath=$projOutput->base_dir}