<?php
/*
*	b3rtWindowsRegistry 1.0
*	===========
*	Class for reading from and writing to the Windows registry
*
*	Copyright (c) 2008, H. Poort
*	All rights reserved.
*
*	Redistribution and use in source and binary forms, with or without
*	modification, are permitted provided that the following conditions are met:
*	  * 	Redistributions of source code must retain the above copyright
*		notice, this list of conditions and the following disclaimer.
*	  *	Redistributions in binary form must reproduce the above copyright
*		notice, this list of conditions and the following disclaimer in the
*		documentation and/or other materials provided with the distribution.
*	  *	The name the copyright holder may not be used to endorse or promote
*		products derived from this software without specific prior written
*		permission.
*
*	THIS SOFTWARE IS PROVIDED BY H. Poort "AS IS" AND ANY
*	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*	DISCLAIMED. IN NO EVENT SHALL H. Poort BE LIABLE FOR ANY
*	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

class WindowsRegistry 
{
	private $RegistryObject;

	const HKEY_CLASSES_ROOT = 0x80000000;
	const HKEY_CURRENT_USER = 0x80000001;
	const HKEY_LOCAL_MACHINE = 0x80000002;
	const HKEY_USERS = 0x80000003;
	const HKEY_CURRENT_CONFIG = 0x80000005;

	const REG_NONE = 0;
	const REG_SZ = 1;
	const REG_EXPAND_SZ = 2;
	const REG_BINARY = 3;
	const REG_DWORD = 4;
	const REG_MULTI_SZ = 7;

	public function __construct()
	{
		/*$this->WbemLocator = new COM('WbemScripting.SWbemLocator');
		$this->WbemServices = $this->WbemLocator->ConnectServer('.', '/root/default');
		$this->WbemServices->Security_->ImpersonationLevel = 0x3;
		$this->RegObject = $this->WbemServices->Get('StdRegProv');*/

		// Get the WMI StdRegProv class
		try
		{
			$this->RegistryObject = new COM('WINMGMTS:{impersonationLevel=impersonate}!//./root/default:StdRegProv');
		}
		catch (Exception $ex)
		{
			$this->RegistryObject = NULL;
			throw new Exception('Could not get a connection to the registry.');
		}
	}

	public function __destruct()
	{
		unset($this->RegistryObject);
	}

	public function ReadValue($keyPath, $valueName, $asString = FALSE)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return NULL;

		$valueType = $this->GetValueType($hKeyId, $subKey, $valueName);
		if (-1 == $valueType)
			return NULL;

		switch ($valueType)
		{
			case self::REG_NONE: // No support for REG_NONE
				return NULL;
			case self::REG_SZ:
				return $this->GetStringValue($hKeyId, $subKey, $valueName);
			case self::REG_EXPAND_SZ:
				return $this->GetExpandedStringValue($hKeyId, $subKey, $valueName);
			case self::REG_BINARY:
				return $this->GetBinaryValue($hKeyId, $subKey, $valueName, $asString);
			case self::REG_DWORD:
				return $this->GetDWORDValue($hKeyId, $subKey, $valueName, $asString);
			case self::REG_MULTI_SZ:
				return $this->GetMultiStringValue($hKeyId, $subKey, $valueName, $asString);
		}
	}

	public function WriteValue($keyPath, $valueName, $valueContents, $forceType = -1)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		if (!$this->CreateKey($keyPath))
			return FALSE;

		if (-1 != $forceType)
			$valueType = $forceType;
		else
			$valueType = $this->GetValueType($hKeyId, $subKey, $valueName);

		if (self::REG_NONE == $valueType) // No support for REG_NONE
			return FALSE;

		if (-1 == $valueType) // valueType unknown
		{
			if (is_string($valueContents))
				$valueType = self::REG_SZ;
			else if (is_int($valueContents))
				$valueType = self::REG_DWORD;
			else if (is_array($valueContents) && count($valueContents))
			{
				if (is_string($valueContents[0]))
					$valueType = self::REG_MULTI_SZ;
				else if (is_int($valueContents[0]))
					$valueType = self::REG_BINARY;
			}
		}
		if (-1 == $valueType) // valueType still unknown, leave
			return FALSE;

		$result = -1;

		if ((self::REG_SZ == $valueType) || (self::REG_EXPAND_SZ == $valueType))
		{
			//if (!is_string($valueContents))
			//	$valueContents = strval($valueContents);
				
			if (self::REG_SZ == $valueType)
				$result = $this->RegistryObject->SetStringValue($hKeyId, $subKey, $valueName, $valueContents);
			else
				$result = $this->RegistryObject->SetExpandedStringValue($hKeyId, $subKey, $valueName, $valueContents);
		}
		else if (self::REG_DWORD == $valueType)
		{
			//if (!is_int($valueContents))
			//	$valueContents = intval($valueContents);

			$result = $this->RegistryObject->SetDWORDValue($hKeyId, $subKey, $valueName, $valueContents);
		}
		else if (self::REG_MULTI_SZ == $valueType)
		{
			if (!is_array($valueContents) || !is_string($valueContents[0]))
				return FALSE;

			$result = $this->RegistryObject->SetMultiStringValue($hKeyId, $subKey, $valueName, $valueContents);
		}
		else if (self::REG_BINARY == $valueType)
		{
			if (!is_array($valueContents) || !is_int($valueContents[0]))
				return FALSE;

			$result = $this->RegistryObject->SetBinaryValue($hKeyId, $subKey, $valueName, $valueContents);
		}

		return (0 == $result);
	}

	public function DeleteValue($keyPath, $valueName)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		return ($this->RegistryObject->DeleteValue($hKeyId, $subKey, $valueName) == 0);
	}

	public function GetValueNames($keyPath, $includeTypes = FALSE)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		$valueList = array();
		if (!$this->EnumValues($hKeyId, $subKey, $valueList))
			return NULL;

		if (!$includeTypes)
		{
			$valueNames = array();
			for ($i = 0, $cnt = count($valueList); $i < $cnt; $i++)
				$valueNames[] = $valueList[$i][0];
			return $valueNames;
		}
		else
			return $valueList;
	}

	public function CreateKey($keyPath)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		return ($this->RegistryObject->CreateKey($hKeyId, $subKey) == 0);
	}

	public function DeleteKey($keyPath, $deleteSubkeys = FALSE)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		if (!$deleteSubkeys)
			return ($this->RegistryObject->DeleteKey($hKeyId, $subKey) == 0);
		else
		{
			if (!function_exists('deleteSubKeysRecursive'))
			{
				function deleteSubKeysRecursive(&$thisRef, $hKeyId, $subKey)
				{
					$mapHkeysToString = array(
						WindowsRegistry::HKEY_CLASSES_ROOT => 'HKEY_CLASSES_ROOT',
						WindowsRegistry::HKEY_CURRENT_USER => 'HKEY_CURRENT_USER',
						WindowsRegistry::HKEY_LOCAL_MACHINE => 'HKEY_LOCAL_MACHINE',
						WindowsRegistry::HKEY_USERS => 'HKEY_USERS',
						WindowsRegistry::HKEY_CURRENT_CONFIG => 'HKEY_CURRENT_CONFIG');

					if (!isset($mapHkeysToString[$hKeyId]))
						return FALSE;

					$subKeys = $thisRef->GetSubKeys($mapHkeysToString[$hKeyId] . '\\' . $subKey);
					if ($subKeys)
					{
						for ($i = 0, $cnt = count($subKeys); $i < $cnt; $i++)
						{
							if (!deleteSubKeysRecursive($thisRef, $hKeyId, $subKey . '\\' . $subKeys[$i]))
								return FALSE;
						}
					}

					return ($thisRef->DeleteKey($mapHkeysToString[$hKeyId] . '\\' . $subKey));
				}
			}

			return deleteSubKeysRecursive($this, $hKeyId, $subKey);
		}
	}

	public function GetSubKeys($keyPath)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		$keyList = array();
		if (!$this->EnumKeys($hKeyId, $subKey, $keyList))
			return NULL;

		return $keyList;
	}

	public function KeyExists($keyPath)
	{
		$hKeyId = 0;
		$subKey = '';
		if (!$this->SplitKeyPath($keyPath, $hKeyId, $subKey))
			return FALSE;

		return ($this->RegistryObject->EnumValues($hKeyId, $subKey, new VARIANT(), new VARIANT()) == 0);
	}

	// PRIVATE FUNCTIONS
	// ================================
	private function SplitKeyPath($keyPath, &$hKeyIdResult, &$subKeyResult)
	{
		$hKeyIdResult = 0;
		$subKeyResult = 'foo';

		$splitPath = explode('\\', $keyPath, 2);

		if (FALSE === $splitPath)
			return FALSE;
		else if (count($splitPath) == 1)
			$splitPath[1] = '';

		$subKeyResult = $splitPath[1];

		switch ($splitPath[0])
		{
			case 'HKEY_CLASSES_ROOT':
				$hKeyIdResult = self::HKEY_CLASSES_ROOT;
				break;
			case 'HKEY_CURRENT_USER':
				$hKeyIdResult = self::HKEY_CURRENT_USER;
				break;
			case 'HKEY_LOCAL_MACHINE':
				$hKeyIdResult = self::HKEY_LOCAL_MACHINE;
				break;
			case 'HKEY_USERS':
				$hKeyIdResult = self::HKEY_USERS;
				break;
			case 'HKEY_CURRENT_CONFIG':
				$hKeyIdResult = self::HKEY_CURRENT_CONFIG;
				break;
			default:
				return FALSE;
		}

		return TRUE;
	}

	private function EnumKeys($hKeyId, $subKey, &$keyList)
	{
		$keyNames = new VARIANT();
		if ($this->RegistryObject->EnumKey($hKeyId, $subKey, $keyNames) != 0)
			return FALSE;

		$keyList = array();

		if (variant_get_type($keyNames) == (VT_VARIANT | VT_ARRAY))
		{
			for ($i = 0, $cnt = count($keyNames); $i < $cnt; $i++)
				$keyList[] = strval($keyNames[$i]);
		}

		return TRUE;
	}

	private function EnumValues($hKeyId, $subKey, &$valueList)
	{
		$valueNames = new VARIANT();
		$valueTypes = new VARIANT();

		if ($this->RegistryObject->EnumValues($hKeyId, $subKey, $valueNames, $valueTypes) != 0)
			return FALSE;

		$valueList = array();

		if (variant_get_type($valueNames) == (VT_VARIANT | VT_ARRAY))
		{
			for ($i = 0, $cnt = count($valueNames); $i < $cnt; $i++)
				$valueList[] = array(strval($valueNames[$i]), intval($valueTypes[$i]));
		}
		else // Handle a bug in StdRegProv's EnumValues (http://groups.google.com/group/microsoft.public.win32.programmer.wmi/browse_thread/thread/d74c0ca865887e6b)
		{
			if ($this->GetStringValue($hKeyId, $subKey, '') != NULL)
				$valueList[] = array('', self::REG_SZ);
			else if ($this->GetDWORDValue($hKeyId, $subKey, '') != NULL)
				$valueList[] = array('', self::REG_DWORD);
			else if ($this->GetExpandedStringValue($hKeyId, $subKey, '') != NULL)
				$valueList[] = array('', self::REG_EXPAND_SZ);
			else if ($this->GetBinaryValue($hKeyId, $subKey, '') != NULL)
				$valueList[] = array('', self::REG_BINARY);
			else if ($this->GetMultiStringValue($hKeyId, $subKey, '') != NULL)
				$valueList[] = array('', self::REG_MULTI_SZ);
		}

		return TRUE;
	}

	private function GetValueType($hKeyId, $subKey, $valueName)
	{
		$valueList = array();
		if (!$this->EnumValues($hKeyId, $subKey, $valueList))
			return -1;

		for ($i = 0, $cnt = count($valueList); $i < $cnt; $i++)
		{
			if ($valueList[$i][0] == $valueName)
				return $valueList[$i][1];
		}

		return -1;
	}

	private function GetStringValue($hKeyId, $subKey, $valueName)
	{
		$stringValue = new VARIANT();
		return (($this->RegistryObject->GetStringValue($hKeyId, $subKey, $valueName, $stringValue) == 0) ?
			strval($stringValue) : NULL);
	}

	private function GetExpandedStringValue($hKeyId, $subKey, $valueName)
	{
		$expandStringValue = new VARIANT();
		return ((0 == $this->RegistryObject->GetExpandedStringValue($hKeyId, $subKey, $valueName, $expandStringValue)) ?
			strval($expandStringValue) : NULL);
	}

	private function GetBinaryValue($hKeyId, $subKey, $valueName, $asString = FALSE)
	{
		$binaryValue = new VARIANT();
		if ($this->RegistryObject->GetBinaryValue($hKeyId, $subKey, $valueName, $binaryValue) != 0)
			return NULL;

		if ($asString)
		{
			$result = '';
			for ($i = 0, $cnt = count($binaryValue); $i < $cnt; $i++)
				$result .= dechex($binaryValue[$i]) . ((($cnt - 1) != $i) ? ' ' : '');
		}
		else
		{
			$result = array();
			for ($i = 0, $cnt = count($binaryValue); $i < $cnt; $i++)
				$result .= intval($binaryValue[$i]);
		}

		return $result;
	}

	private function GetDWORDValue($hKeyId, $subKey, $valueName, $asString = FALSE)
	{
		$dwordValue = new VARIANT();
		return (($this->RegistryObject->GetDWORDValue($hKeyId, $subKey, $valueName, $dwordValue) == 0) ?
			($asString ? strval($dwordValue) : intval($dwordValue)) : NULL);
	}

	private function GetMultiStringValue($hKeyId, $subKey, $valueName, $asString = FALSE)
	{
		$multiStringValue = new VARIANT();
		if ($this->RegistryObject->GetMultiStringValue($hKeyId, $subKey, $valueName, $multiStringValue) != 0)
			return NULL;

		if ($asString)
		{
			$result = '';
			for ($i = 0, $cnt = count($multiStringValue); $i < $cnt; $i++)
				$result .= strval($multiStringValue[$i]) . ((($cnt - 1) != $i) ? "\n" : '');
		}
		else
		{
			$result = array();
			for ($i = 0, $cnt = count($multiStringValue); $i < $cnt; $i++)
				$result[] = strval($multiStringValue[$i]);
		}

		return $result;
	}
}
?>