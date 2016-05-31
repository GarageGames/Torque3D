//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
function execGameObjects()
{
   //find all GameObjectAssets
   %assetQuery = new AssetQuery();
   if(!AssetDatabase.findAssetType(%assetQuery, "GameObjectAsset"))
      return; //if we didn't find ANY, just exit
      
   %count = %assetQuery.getCount();
      
	for(%i=0; %i < %count; %i++)
	{
	   %assetId = %assetQuery.getAsset(%i);
      
      %gameObjectAsset = AssetDatabase.acquireAsset(%assetId);
      
      if(isFile(%gameObjectAsset.scriptFilePath))
         exec(%gameObjectAsset.scriptFilePath);
	}
}

function findGameObject(%name)
{
   //find all GameObjectAssets
   %assetQuery = new AssetQuery();
   if(!AssetDatabase.findAssetType(%assetQuery, "GameObjectAsset"))
      return 0; //if we didn't find ANY, just exit
      
   %count = %assetQuery.getCount();
      
	for(%i=0; %i < %count; %i++)
	{
	   %assetId = %assetQuery.getAsset(%i);
      
      %gameObjectAsset = AssetDatabase.acquireAsset(%assetId);
      
      if(%gameObjectAsset.gameObjectName $= %name)
		{
		   if(isFile(%gameObjectAsset.TAMLFilePath))
         {
            return %gameObjectAsset;
         }
		}
	}
		
	return 0;
}

function spawnGameObject(%name, %addToMissionGroup)
{
	if(%addToMissionGroup $= "")
		%addToMissionGroup = true;
		
   %gameObjectAsset = findGameObject(%name);
   
   if(isObject(%gameObjectAsset))
   {
      %newSGOObject = TamlRead(%gameObjectAsset.TAMLFilePath);
            
      if(%addToMissionGroup == true) //save instance when saving level
         MissionGroup.add(%newSGOObject);
      else // clear instance on level exit
         MissionCleanup.add(%newSGOObject);
         
      return %newSGOObject;
   }
		
	return 0;
}

function saveGameObject(%name, %tamlPath, %scriptPath)
{
	%gameObjectAsset = findGameObject(%name);
   
   //find if it already exists. If it does, we'll update it, if it does not, we'll  make a new asset
   if(isObject(%gameObjectAsset))
   {
      %assetID = %gameObjectAsset.getAssetId();
      
      %gameObjectAsset.TAMLFilePath = %tamlPath;
      %gameObjectAsset.scriptFilePath = %scriptPath;
      
      TAMLWrite(%gameObjectAsset, AssetDatabase.getAssetFilePath(%assetID));
      AssetDatabase.refreshAsset(%assetID);
   }
   else
   {
      //Doesn't exist, so make a new one
      %gameObjectAsset = new GameObjectAsset()
      {
         assetName = %name @ "Asset";
         gameObjectName = %name;
         TAMLFilePath = %tamlPath;
         scriptFilePath = %scriptPath;
      };
      
      //Save it alongside the taml file
      %path = filePath(%tamlPath);
      
      TAMLWrite(%gameObjectAsset, %path @ "/" @ %name @ ".asset.taml");
      AssetDatabase.refreshAllAssets(true);
   }
}