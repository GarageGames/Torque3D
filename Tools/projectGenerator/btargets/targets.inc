<?php
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

///////////////////////////// VS9 Xenon /////////////////////////////
   
$c = BuildTarget::add(	'VS9 Xenon',							// Name		         			
                        'buildFiles/VisualStudio 2008 (XDK)',	// Solution output directory
                        'projects/',	                        // Project output directory (relative to solution)
                        '../../source/',				        // Base directory		         	
                        'vc2k8_360_proj.tpl',				    // App Project Template
                        'vc2k8_360_proj.tpl',				    // Shared App Project Template
                        'vc2k8_360_lib_proj.tpl',			    // Lib Project Template	
                        'vc2k8_360_dll_proj.tpl',			    // Shared Lib Project Template	
                        '',			                            // ActiveX Project Template	(not supported on 360)
                        '.vcproj' );					        // File ext to write the project to

// Solution template and extension
$c->setSolutionInfo( 'vc2k8_360_sln.tpl', '', '.sln' );

$c->setFileExtensions( 'c', 'cc', 'h', 'cpp', 'inl', 'm', 'mm' );

$c->setPlatforms( "360" );
         
$c->setDontCompilePatterns( "#/platformPOSIX.*#", "#/platformMac.*#", "#/platformWin32.*#", "#\.mac\.#", "#\.win32\.#", "#/mac/#", "#/win32/#", "#\.mm$#", "#\.m$#" );

///////////////////////////// VS9 PS3 /////////////////////////////
   
$c = BuildTarget::add(	'VS9 PS3',							// Name		         			
                        'buildFiles/VisualStudio 2008 (PS3)',	// Solution output directory
                        'projects/',	                        // Project output directory (relative to solution)
                        '../../source/',				        // Base directory		         	
                        'vc2k8_ps3_proj.tpl',				    // App Project Template
                        'vc2k8_ps3_proj.tpl',				    // Shared App Project Template
                        'vc2k8_ps3_lib_proj.tpl',			    // Lib Project Template	
                        'vc2k8_ps3_dll_proj.tpl',			    // Shared Lib Project Template	
                        '',			                            // ActiveX Project Template	(not supported on ps3)
                        '.vcproj' );					        // File ext to write the project to

// Solution template and extension
$c->setSolutionInfo( 'vc2k8_ps3_sln.tpl', '', '.sln' );

$c->setFileExtensions( 'c', 'cc', 'h', 'cpp', 'inl', 'm', 'mm' );

$c->setPlatforms( "ps3" );
         
$c->setDontCompilePatterns( "#/platformXbox.*#", "#/platformWin32.*#", "#\.mac\.#", "#\.win\.#", "#\.win32\.#", "#/mac/#", "#/win32/#", "#\.mm$#", "#\.m$#" );


///////////////////////////// VS9 /////////////////////////////

$c = BuildTarget::add(	'VS9',							   // Name
                        'buildFiles/VisualStudio 2008',	   // Solution output directory
                        'projects/',	                   // Project output directory (relative to solution)
                        '../../source/',				   // Base directory         
                        'vc2k8_proj.tpl',				   // App Project Template
                        'vc2k8_shared.tpl',				   // Shared App Project Template
                        'vc2k8_lib_proj.tpl',			   // Lib Project Template
                        'vc2k8_dll_proj.tpl',			   // Shared Lib Project Template
                        'vc2k8_activex_proj.tpl',		   // ActiveX Control	
                        '.vcproj' );				       // File ext to write the project to
                  
// Solution template and extension
$c->setSolutionInfo( 'vc2k8_sln.tpl', 'vc2k8_proj_user.tpl', '.sln' );

$c->setFileExtensions( 'c', 'cc', 'h', 'cpp', 'inl', 'asm', 'rc', 'm', 'mm', 'cs' );

$c->setPlatforms( "win", "win32" );

$c->setDotNetInfo('vc2k8_csproj.tpl');

$c->setDontCompilePatterns( "#/platformPOSIX.*#", "#/platformMac.*#", "#\.mac\.#", "#/mac/#", "#\.mm$#", "#\.m$#" );


///////////////////////////// VS2010 /////////////////////////////

$c = BuildTarget::add(	'VS2010',							   // Name
                        'buildFiles/VisualStudio 2010',	   // Solution output directory
                        'projects/',	                   // Project output directory (relative to solution)
                        '../../source/',				   // Base directory         
                        'vc2010_proj.tpl',				   // App Project Template
                        'vc2010_shared.tpl',				   // Shared App Project Template
                        'vc2010_lib_proj.tpl',			   // Lib Project Template
                        'vc2010_dll_proj.tpl',			   // Shared Lib Project Template
                        'vc2010_activex_proj.tpl',		   // ActiveX Control	
                        '.vcxproj' );				       // File ext to write the project to
                  
// Solution template and extension
$c->setSolutionInfo( 'vc2010_sln.tpl', '', '.sln', 'vc2010_filter.tpl' );

$c->setFileExtensions( 'c', 'cc', 'h', 'cpp', 'inl', 'asm', 'rc', 'm', 'mm', 'cs' );
$c->setSourceFileExtensions( 'c', 'cc', 'cpp' );

$c->setPlatforms( "win", "win32" );

$c->setDotNetInfo('vc2010_csproj.tpl');

$c->setDontCompilePatterns( "#/platformPOSIX.*#", "#/platformMac.*#", "#\.mac\.#", "#/mac/#", "#\.mm$#", "#\.m$#" );

/////////////////////////// XCode /////////////////////////////

$c = BuildTarget::add( 'Xcode',						         // Name
                      'buildFiles/Xcode',		            // Solution output directory
                      '',	                              // Project output directory (relative to solution)
                      '../../source/',				         // Base directory
                      'xcode.tpl',				            // App Project Template
                      'xcodeSharedApp.tpl',				   // Shared App Project Template
                      'xcodeLib.tpl',				         // Lib Project Template
                      'xcodeSharedLib.tpl',			      // Shared Lib Project Template
                      'xcodeSafariPlugin.tpl',			   // Safari Plugin
                      '.xcodeproj/project.pbxproj' );    // File ext to write the project to

$c->setDelimiters( '[', ']' ); 						// What delimeters does this template use?
         
$c->setFileExtensions( 'c', 'cc', 'h', 'cpp', 'inl', 'm', 'mm' );

$c->setPlatforms( "mac" );
       
$c->setDontCompilePatterns( "*\.h*", "*win32*", "*\.win\.*", "#/platformWin32/#", "/D3D.*/", "*dsound*" );


///////////////////////////// Make //////////////////////////////

$c = BuildTarget::add( 'Make',                // Name
                       'buildFiles/Make',     // Solution Output directory
		       '',                    // Project Output directory (relative to solution)
		       '../../source/',       // Base directory
		       'makeApp.tpl',         // App Project Template
		       'makeAppShared.tpl',   // Shared App Project Template
		       'makeLib.tpl',         // Lib Project Template
		       'makeSo.tpl',          // Shared Lib Project Template
		       '',                    // NP Plugin
		       '' );                  // File ext to write the project to

$c->setSolutionInfo( 'makeSolution.tpl', '', '' );

$c->setFileExtensions( 'c', 'cc', 'cpp', 'asm' );

$c->setPlatforms( "linux" );

$c->setDontCompilePatterns( "*\.h*", "*win32*", "*\.win\.*", "/D3D.*/", "*dsound*", "#/mac/#", "#\.mm$#", "#\.m$#", "*\.dedicated\.*" );


///////////////////////////// Make Dedicated ////////////////////

$c = BuildTarget::add( 'Make Dedicated',      // Name
                       'buildFiles/Make_Ded', // Output directory
		       '',                    // Project Output directory (relative to solution)
		       '../../source/',       // Base directory
		       'makeApp.tpl',         // App Project Template
		       'makeAppShared.tpl',   // Shared App Project Template
		       'makeLib.tpl',         // Lib Project Template
		       'makeSo.tpl',          // Shared Lib Project Template
		       '',                    // NP Plugin
		       '' );                  // File ext to write the project to

$c->setSolutionInfo( 'makeSolution.tpl', '', '' );

$c->setFileExtensions( 'c', 'cc', 'cpp', 'asm' );

$c->setPlatforms( "linux_dedicated" );

$c->setDontCompilePatterns( "*\.h*", "*win32*", "*\.win\.*", "/D3D.*/", "#/gl/#", "#/mac/#", "#\.mm$#", "#\.m$#", "*\.client\.*" );


///////////////////////////// Build /////////////////////////////

// 'buildManifest_'.$name.'_'.Generator::$platform.'.txt',

$c = BuildTarget::add( 'build',						// Name         
                  'buildFiles',         			// Solution output directory
                  'projects/',	                  // Project output directory (relative to solution)
                  '../source/',         			// Base directory
                  'buildManifest.tpl',         	// App Project Template
                  'buildManifest.tpl',         	// Shared Project Template
                  '',         				   // Lib Project Template
                  '',         				   // Shared Lib Project Template
                  '',         				   // ActiveX Project Template
                  '.txt' );
         
$c->setFileExtensions( 'c', 'cc', 'cpp', 'asm', 'm', 'mm' );

// NO PLATFORMS - SO ALL OR NONE?
$c->setPlatforms("");

////////////////////

function generateAppConfigOutputs( $name )
{
    return BuildTarget::getInstances();
}

function generateLibConfigOutputs( $name )
{
    return BuildTarget::getInstances();
}

?>
