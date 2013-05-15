Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 2012
{foreach name=projects item=project from=$projects}
{if $project->projectFileExt eq ".csproj"}
Project("{literal}{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}{/literal}") = "{$project->name}", "projects/{$project->name}{$project_ext}", "{$project->guid}"
{else}
Project("{$solution_guid}") = "{$project->name}", "projects/{$project->name}{$project_ext}", "{$project->guid}"
{/if}
EndProject
{/foreach}
{foreach key=pname item=v from=$projectExtRefs}
Project("{$v[2]}") = "{$pname}", "{$v[0]}", "{$v[1]}"
EndProject
{/foreach}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Optimized Debug|Win32 = Optimized Debug|Win32
		Release|Win32 = Release|Win32
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
{foreach name=projects item=project from=$projects}
		{$project->guid}.Debug|Win32.ActiveCfg = Debug|Win32
		{$project->guid}.Debug|Win32.Build.0 = Debug|Win32
		{$project->guid}.Optimized Debug|Win32.ActiveCfg = Optimized Debug|Win32
		{$project->guid}.Optimized Debug|Win32.Build.0 = Optimized Debug|Win32
		{$project->guid}.Release|Win32.ActiveCfg = Release|Win32
		{$project->guid}.Release|Win32.Build.0 = Release|Win32
{/foreach}
{foreach key=pname item=v from=$projectExtRefs}
		{$v[1]}.Debug|Win32.ActiveCfg = Debug|Win32
		{$v[1]}.Debug|Win32.Build.0 = Debug|Win32
		{$v[1]}.Optimized Debug|Win32.ActiveCfg = Optimized Debug|Win32
		{$v[1]}.Optimized Debug|Win32.Build.0 = Optimized Debug|Win32
		{$v[1]}.Release|Win32.ActiveCfg = Release|Win32
		{$v[1]}.Release|Win32.Build.0 = Release|Win32
{/foreach}
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
