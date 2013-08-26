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


//
// Hello, hello...
//
echo( "Welcome to projectGenerator 0.9\n" );
echo( "Copyright (c) 2012 GarageGames, LLC\n" );
echo( "Released under a MIT license.\n" );
echo( "\n" );

//
//	Configure error logging
//
error_reporting( E_ALL & ~E_NOTICE );

//
// Requires
//
require_once( "classes/BuildTarget.php" );
require_once( "btargets/targets.inc" );
require_once( "classes/Generator.php" );
require_once( "projectGenUtils.inc" );
require_once( "smarty/Smarty.class.php" );

//
// Initialization
//
$g_cwd = realpath( dirname( $_SERVER[ 'PHP_SELF' ] ) );

echo( "   CWD = " . $g_cwd . "\n" );

//
// Load, instantiate, and configure the smarty template engine.
//
echo( "   - Loading Smarty...\n" );

$tpl 				= new Smarty();
$tpl->template_dir	= $g_cwd . "/templates";
$tpl->compile_dir	= $g_cwd . "/templates_c";

$tpl->clear_all_cache();

// By default we assume that the root of the Torque SDK
// is located two folders up from the CWD.  That is unless
// another path is passed in the command line.
$torqueRoot = "../..";
if ( $argc >= 3 )
    $torqueRoot = str_replace( "\\", "/", $argv[2] );

// Kick off the generator
T3D_Generator::init( $torqueRoot );

// Ready to read our config file.
echo( "   - Loading config file " . realpath($argv[1])."\n" );

require( $argv[ 1 ] );

// Generate all projects
T3D_Generator::generateProjects( $tpl );

// Now the solutions (if any)
$tpl->clear_all_cache();

T3D_Generator::generateSolutions( $tpl );

// finally write out the sample.html for web deployment (if any)
WebPlugin::writeSampleHtml();

exit;
?>
