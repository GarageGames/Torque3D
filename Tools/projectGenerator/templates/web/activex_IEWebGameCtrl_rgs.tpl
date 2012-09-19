HKCR
{
	__WEBGAME_PLUGINNAME__.__WEBGAME_CTRLNAME__.1 = s '__WEBGAME_CTRLNAME__ Class'
	{
		CLSID = s '{__WEBGAMELIB_UUID__}'
	}
	__WEBGAME_PLUGINNAME__.__WEBGAME_CTRLNAME__ = s '__WEBGAME_CTRLNAME__ Class'
	{
		CLSID = s '{__WEBGAMELIB_UUID__}'
		CurVer = s '__WEBGAME_PLUGINNAME__.__WEBGAME_CTRLNAME__.1'
	}
	NoRemove CLSID
	{
		ForceRemove {__WEBGAMELIB_UUID__} = s '__WEBGAME_CTRLNAME__ Class'
		{
			ProgID = s '__WEBGAME_PLUGINNAME__.__WEBGAME_CTRLNAME__.1'
			VersionIndependentProgID = s '__WEBGAME_PLUGINNAME__.__WEBGAME_CTRLNAME__'
			ForceRemove 'Programmable'
			InprocServer32 = s '%MODULE%'
			{
				val ThreadingModel = s 'Apartment'
			}
			val AppID = s '%APPID%'
			ForceRemove 'Control'
			ForceRemove 'ToolboxBitmap32' = s '%MODULE%, 102'
			'MiscStatus' = s '0'
			{
			    '1' = s '%OLEMISC%'
			}
			'TypeLib' = s '{__WEBGAME_UUID__}'
			'Version' = s '1.0'
		}
	}
}
