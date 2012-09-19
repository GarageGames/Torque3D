HKCR
{
	NoRemove AppID
	{
		'%APPID%' = s '__WEBGAME_PLUGINNAME__'
		'__WEBGAME_PLUGINNAME__.DLL'
		{
			val AppID = s '%APPID%'
		}
	}
}
