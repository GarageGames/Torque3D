Torque 3D v3.5 - PhysX 3.3 Basic Plugin
==========================

This is a basic PhysX 3.3 plugin that does not contain any added features like cloth,particles and CCD. This plugin provides no more features than the bullet plugin and can therefore be used as a drop in alternative. It does not modify any files outside of the physx3 folder. A far more advanced physx3 plugin with CCD, cloth and particles can be found on this repository under the physx3 branch.

Setting up PhysX 3.3 using the Torque 3D Project Manager
------------------------------------------
 - You can find a pre compiled binary of the Torque3D Project Manager that supports PhysX 3.3 here: http://www.narivtech.com/downloads/T3DProjectManager-2-1-devel.zip and source code here: https://github.com/rextimmy/Torque3D-ProjectManager/tree/development
 - For the Project Manager to find PhysX 3.3 SDK you have two options 1)Create an environment variable called TORQUE_PHYSX3_PATH and have that pointing to the location you installed the SDK 2)Place the SDK into a folder called "Program Files"/NVIDIA Corporation/NVIDIA PhysX SDK/v3.3.0_win
 - Simply choose PhysX 3.3 physics from the modules list in the project manager and everything should be automatically taken care of.

Setting up PhysX 3.3 manually
------------------------------------------

 - You will need the latest SDK from NVIDIA. This requires signing up for their developer program. If you don't already have access to their developer site then sign up now as access is not immediate.
 - Set up a standard Torque3D project, don't include any PhysX or Bullet, just regular Torque Physics in project manager options (if you're using it)
 - Generate Projects and open the source code in Visual Studio ( or the IDE of your choice )
 - In the solution explorer in the DLL for your project you should find Source Files -> Engine -> T3D -> physics
 - Add a new filter "physx3" and then right click on it and add existing item
 - Add all the files found under Engine\Source\T3D\physics\physx3\
 - Now you need to add the PhysX SDK. 
 - Under the properties for the DLL project, under Linker -> Additional Library Directories add the lib\win32 directory for the PhysX 3.3 SDK. For example, mine is in: C:\Program Files (x86)\NVIDIA Corporation\NVIDIA PhysX SDK\v3.3.0_win\Lib\win32
 - In the same window under C/C++ you should see Additional Include Directories, you need to add the Include directory for the PhysX 3.3 SDK. For example, mine is in: C:\Program Files %28x86%29\NVIDIA Corporation\NVIDIA PhysX SDK\v3.3.0_win\Include
 - You should now be able to compile now without any issues.

The following libraries will also be needed:
 
Release , Debug

 - PhysX3_x86.lib,PhysX3CHECKED_x86.lib
 - PhysX3Common_x86.lib,PhysX3CommonCHECKED_x86.lib
 - PhysX3Extensions.lib,PhysX3ExtensionsCHECKED.lib
 - PhysX3Cooking_x86.lib,PhysX3CookingCHECKED_x86.lib
 - PxTask.lib,PxTaskCHECKED.lib
 - PhysX3CharacterKinematic_x86.lib,PhysX3CharacterKinematicCHECKED_x86.lib
 - PhysXVisualDebuggerSDK.lib, PhysXVisualDebuggerSDKCHECKED.lib
 - PhysXProfileSDK.lib, PhysXProfileSDKCHECKED.lib

With debug build feel free to change CHECKED to DEBUG if you prefer but it will still require the CHECKED dll's though.
 
Running a project
------------------------------------------

 - To run a release project you will need the following from the SDK bin folder:
   1. PhysX3_x86.dll
   2. PhysX3CharacterKinematic_x86.dll
   3. PhysX3Common_x86.dll
   4. PhysX3Cooking_x86.dll
   
 - To run a debug project you will need the following from the SDK bin folder:
   1. PhysX3CHECKED_x86.dll
   2. nvToolsExt32_1.dll
   3. PhysX3CookingCHECKED_x86.dll
   4. PhysX3CommonCHECKED_x86.dll
   5. PhysX3CharacterKinematicCHECKED_x86.dll
 
Place these files along side the exe and this should get you up and running.
