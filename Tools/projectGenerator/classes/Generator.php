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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Static Generator class
///  
require_once( "FileUtil.php" );
require_once( "Solution.php" );
require_once( "Project.php" );
require_once( "BuildTarget.php" );
require_once( "Torque3D.php");
require_once( "WindowsRegistry.php");
require_once( "WebPlugin.php");
require_once( "ActiveXWebPlugin.php");
require_once( "NPWebPlugin.php");
require_once( "SafariWebPlugin.php");


class T3D_Generator
{
    public static $app_name;
    public static $paths            = array();
    public static $prefs            = array();
    public static $config_projects  = array(); 
    public static $app_lib_includes = array(); // An accumulative list of includes that are needed by libs and subsequently needed by the application
    public static $platform         = 'win32';
    public static $solutions        = array();
    public static $libGuard         = array(); // protect against libraries included multiple times across modules
    public static $absPath          = NULL;
    public static $gameProjectName  = NULL;
    public static $toolBuild        = true;
    public static $watermarkBuild   = false;
    public static $purchaseScnBuild = false;
    public static $demoBuild        = false;
    public static $objectLimitBuild = false;
    public static $timeOutBuild     = false;
    public static $useDLLRuntime    = false;
    
    private static $solution_cur;
    private static $project_cur;
    private static $module_cur;
    
    static function init( $torqueRoot )
    {
        // If the torque root is absolute then store and use it.
        if ( realpath( $torqueRoot ) == $torqueRoot ||
             realpath( $torqueRoot ) == str_replace("/", "\\", $torqueRoot) )
        {
            self::$absPath = str_replace( "\\", "/", $torqueRoot );
            $torqueRoot = self::$absPath;
        }
       
        if ( self::$absPath )
        {
           self::$paths[ 'engineLib' ] = '../../../Engine/lib/';
           self::$paths[ 'engineSrc' ] = '../../../Engine/source/';
           self::$paths[ 'engineBin' ] = '../../../Engine/bin/';
        }
        else
        {
           self::$paths[ 'engineLib' ] = $torqueRoot . '/../Engine/lib/';
           self::$paths[ 'engineSrc' ] = $torqueRoot . '/../Engine/source/';
           self::$paths[ 'engineBin' ] = $torqueRoot . '/../Engine/bin/';
        }

        self::$paths[ 'modules' ]   = $torqueRoot . '/Tools/projectGenerator/modules/';
        self::$paths[ 'libs' ]      = $torqueRoot . '/Tools/projectGenerator/libs/';
        self::$platform             = 'win32';
    }
    
    static function setGameProjectName($name)
    {
       self::$gameProjectName = $name;
    }

    static function getGameProjectName()
    {
       return self::$gameProjectName;
    }

    static function setToolBuild($tb)
    {
       self::$toolBuild = $tb;
    }

    static function getToolBuild()
    {
       return self::$toolBuild;
    }

    static function setWatermarkBuild($wb)
    {
       self::$watermarkBuild = $wb;
    }

    static function getWatermarkBuild()
    {
       return self::$watermarkBuild;
    }

    static function setPurchaseScreenBuild($psb)
    {
       self::$purchaseScnBuild = $psb;
    }

    static function getPurchaseScreenBuild()
    {
       return self::$purchaseScnBuild;
    }
    
    static function setDemoBuild($db)
    {
       self::$demoBuild = $db;
    }

    static function getDemoBuild()
    {
       return self::$demoBuild;
    }

    static function setObjectLimitBuild($olb)
    {
       self::$objectLimitBuild = $olb;
    }

    static function getObjectLimitBuild()
    {
       return self::$objectLimitBuild;
    }

    static function setTimeOutBuild($tob)
    {
       self::$timeOutBuild = $tob;
    }

    static function getTimeOutBuild()
    {
       return self::$timeOutBuild;
    }
	
    static function isApp()
    {
        return self::$project_cur->isApp();
    }


    static function getGeneratorLibsPath()
    {
        return self::$paths[ 'libs' ];
    }
    
    static function getGeneratorModulesPath()
    {
        return self::$paths['modules'];
    }
    
    static function getEngineSrcDir()
    {
        return self::$paths['engineSrc'];
    }
    
    static function getLibSrcDir()
    {
        return self::$paths['engineLib'];
    }
    
    static function getEngineBinDir()
    {
        return self::$paths['engineBin'];
    }
    
    static function addSrcDirRecursive($basePath, $dirPath)
    {
      $ignore = array( '.', '..', '.svn', '_svn', 'CVS' ); 
      $absPath = realpath($argv[1])."/buildFiles/";
      
      array_push( self::$project_cur->dir_list, $basePath.'/'.$dirPath );

      $dirHandle = opendir( $absPath.$basePath.'/'.$dirPath); 
    
      while( $file = readdir( $dirHandle ) )
      { 
         if( !in_array( $file, $ignore ) ) 
         {
            if( is_dir( $absPath.$basePath.'/'.$dirPath.'/'.$file ) )
               self::addSrcDirRecursive( $basePath, "$dirPath/$file"); 
         }
      }
     
      closedir( $dirHandle ); 
 
    }
    
    static function addSrcDir( $dir, $recurse = false )
    {   
        if (!$recurse)
          array_push( self::$project_cur->dir_list, $dir );
        else
        {
          self::addSrcDirRecursive($dir, "");
        }
    }
    
    static function addSrcFile( $file )
    {
        array_push( self::$project_cur->dir_list, $file );
    }
    
    static function addIncludePath( $path )
    {
        array_push( self::$project_cur->includes, $path );
    }
    
    static function addProjectDefine( $d, $v )
    {
        if (!$v)
            array_push( self::$project_cur->defines, $d );
        else
            array_push( self::$project_cur->defines, $d."=".$v );
    }
    
    static function isDefined( $d )
    {
        foreach( self::$project_cur->defines as $v )
        {
            if( $v === $d )
                return true;
            else if( strpos( $v, $d . "=" ) === 0 )
                return true;
        }
        return false;
    }
    
    static function disableProjectWarning( $warning )
    {
        array_push( self::$project_cur->disabledWarnings, $warning );
    }
    
    static function addProjectDefines( $args_array )
    {
        self::$project_cur->defines = array_merge( self::$project_cur->defines, $args_array );
    }
    
    static function addProjectLibDir( $dir )
    {
        array_push( self::$project_cur->lib_dirs, $dir );
    }
    
    static function addProjectLibInput( $lib, $libDebug = null )
    {
        array_push( self::$project_cur->libs, $lib );
		array_push( self::$project_cur->libsDebug, $libDebug != null ? $libDebug : $lib );
    }
	
	static function addProjectIgnoreDefaultLib( $lib )
	{
		array_push( self::$project_cur->libsIgnore, $lib );
	}
    
    static function includeLib( $lib )
    {
        foreach( self::$libGuard as $libName )
            if( $libName == $lib )
                return;
                
        array_push( self::$libGuard, $lib );
        
        // if currently in a project, delay the include
        if (T3D_Generator::inProjectConfig())
        {
            array_push( self::$project_cur->lib_includes, $lib );
            return;
        }

        // otherwise include it immediately
        require( T3D_Generator::getGeneratorLibsPath() . $lib . '.conf' );                
    }
        
    static function addProjectDependency( $pd )
    {
        array_push( self::$project_cur->dependencies, $pd );
    }
    
    static function removeProjectDependency( $pd )
    {
        foreach (self::$project_cur->dependencies as $key => $value)
        {
            if ($value == $pd)
            {
                unset(self::$project_cur->dependencies[$key]);
            }
        }
        
        self::$project_cur->dependencies = array_values(self::$project_cur->dependencies);        
    }
    

    static function addProjectReference( $refName, $version = "")
    {
        self::$project_cur->addReference( $refName, $version );
    }
    
    static function setProjectGUID( $guid )
    {
        self::$project_cur->guid = $guid;
    }
    
    static function setProjectModuleDefinitionFile ( $mdef )
    {
        self::$project_cur->moduleDefinitionFile = $mdef;
    }
    
    static function copyFileToProject( $sourcePath, $projectDestPath )
    {
        // Create the array to hold the source and destination
        $paths = array();
        array_push( $paths, $sourcePath );
        array_push( $paths, $projectDestPath );
        
        // Add to the project
        array_push( self::$project_cur->fileCopyPaths, $paths );
    }
    
    static function beginModule( $name )
    {
        if( !self::$module_cur )
            self::$module_cur = $name;
        else
            echo( "T3D_Generator::beginModule() - already in module!" );   
    }
    
    static function endModule()
    {
        if( self::$module_cur )
            self::$module_cur = null;
        else
            trigger_error( "T3D_Generator::endModule() - no active module!", E_USER_ERROR );
    }
    
    static function inProjectConfig()
    {
        return self::$project_cur != null;
    }
    
    static function setProjectSubSystem( $subSystem )
    {
        self::$project_cur->setSubSystem( $subSystem );
    }
    
    static function beginProjectConfig( $name, $type, $guid, $game_dir, $output_name )
    {
        if( !self::$project_cur )
        {
            echo( "   - begin project: " . $name . "=" . $guid . "\n" );
            
            self::$project_cur = new Project( $name, $type, $guid, $game_dir, $output_name );
            
            self::$config_projects[ $name ] = self::$project_cur;
        }
        else
            trigger_error( "T3D_Generator::beginProjectConfig() - a project is already open!", E_USER_ERROR );
    }
    
    static function endProjectConfig( $type )
    {
        //echo( "PT: " .$type. ":".self::$project_cur->type . "\n");
    
        if( self::$project_cur )
        {
            if( self::$project_cur->type == $type )   
            {
                echo( "      - end project " . self::$project_cur->name . "\n" );    

                // Set project outputs
                self::$project_cur->outputs = BuildTarget::getInstances();

                // Allow project to optimize and validate state, etc    
                self::$project_cur->validate();
                   
                // Bit of flummery from original code, not sure what it is supposed to do -- neo
                if( $type == Project::$TYPE_LIB )
                {
                    // Merge lib includes to global lib include array
                    self::$app_lib_includes = array_merge( self::$app_lib_includes, self::$project_cur->includes );
                }
                else if ( $type == Project::$TYPE_SHARED_LIB )
                {
                    // Merge lib includes into app include list
                    self::$app_lib_includes = array_merge( self::$app_lib_includes, self::$project_cur->includes );
                }
                else if ( $type == Project::$TYPE_ACTIVEX )
                {
                    // Merge lib includes into app include list
                    self::$app_lib_includes = array_merge( self::$app_lib_includes, self::$project_cur->includes );
                }
                else if ( $type == Project::$TYPE_SAFARI )
                {
                    // Merge lib includes into app include list
                    self::$app_lib_includes = array_merge( self::$app_lib_includes, self::$project_cur->includes );
                }
                else if ( $type == Project::$TYPE_SHARED_APP )
                {
                    // Merge lib includes into app include list
                    self::$app_lib_includes = array_merge( self::$app_lib_includes, self::$project_cur->includes );
                }
                else if ( $type == Project::$TYPE_APP )
                {
                    // Merge lib includes into app include list
                    self::$project_cur->addIncludes( self::$app_lib_includes );
                }

                // Clear out sucker
                $p = self::$project_cur;
                self::$project_cur = null;
                
                // Now include any libraries included in the modules
                foreach( $p->lib_includes as $libName )
                    require( T3D_Generator::getGeneratorLibsPath() . $libName . '.conf' );
                
            }
            else
                trigger_error( "T3D_Generator::endProjectConfig() - closing type mismatch!", E_USER_ERROR );
        }
        else
            trigger_error( "T3D_Generator::endProjectConfig() - no currently open project!", E_USER_ERROR );	 	   
    }

    static function beginActiveXConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_ACTIVEX, $guid, $game_dir, $output_name );
        
        // Handle ActiveX specific setup (including ATL template processing, etc)
        $activex = new ActiveXWebPlugin();
        $activex->process(self::$project_cur);
    }

    static function endActiveXConfig()
    {
        self::endProjectConfig( Project::$TYPE_ACTIVEX );
    }

    static function beginSafariConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_SAFARI, $guid, $game_dir, $output_name );
        
        // Handle Safari specific setup 
        $safari = new SafariWebPlugin();
        $safari->process(self::$project_cur);
    }

    static function endSafariConfig()
    {
        self::endProjectConfig( Project::$TYPE_SAFARI );
    }

    
    static function beginNPPluginConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_SHARED_LIB, $guid, $game_dir, $output_name );
        self::$project_cur->setUniformOutputFile();

        // Handle NP specific setup (resource template processing, etc)
        $NP = new NPWebPlugin();
        $NP->process(self::$project_cur);
    }

    static function endNPPluginConfig()
    {
        self::endProjectConfig( Project::$TYPE_SHARED_LIB );
    }


    static function beginSharedLibConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_SHARED_LIB, $guid, $game_dir, $output_name );
    }

    static function endSharedLibConfig()
    {
        self::endProjectConfig( Project::$TYPE_SHARED_LIB );
    }

    static function beginCSProjectConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_CSPROJECT, $guid, $game_dir, $output_name );
    }

    static function endCSProjectConfig()
    {
        self::endProjectConfig( Project::$TYPE_CSPROJECT );
    }


    static function beginLibConfig( $lib_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {  
        self::beginProjectConfig( $lib_name, Project::$TYPE_LIB, $guid, $game_dir, $output_name );
    }
    
    static function endLibConfig()
    {
        self::endProjectConfig( Project::$TYPE_LIB );
    }
	
    static function beginSharedAppConfig( $app_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {
        self::beginProjectConfig( $app_name, Project::$TYPE_SHARED_APP, $guid, $game_dir, $output_name );
    }
    
    static function endSharedAppConfig()
    {
        self::endProjectConfig( Project::$TYPE_SHARED_APP);   
    }

    
    static function beginAppConfig( $app_name, $guid = '', $game_dir = 'game', $output_name = '' )
    {
        self::beginProjectConfig( $app_name, Project::$TYPE_APP, $guid, $game_dir, $output_name );
    }
    
    static function endAppConfig()
    {
        self::endProjectConfig( Project::$TYPE_APP );   
    }
    
    static function lookupProjectByName( $pname )
    {
        foreach( self::$config_projects as $projName => $proj )
            if( $projName == $pname )
                return $proj;
                
        return null;
    }

    // We encapsulate the meat of the project generator in generateProjects()
    // so that more complex .config files can generate more than one run of projects
    // without having to run a second instance of php
    static function generateProjects( $tpl ) 
    {
        // Alright, for each project scan and generate the file list.
        $projectFiles    = array ();
        $rootPhpBuildDir = getcwd();
        
        foreach( self::$config_projects as $projName => $proj ) 
        {
            echo( "   - Processing project '$projName'...\n" );

            $proj->generate( $tpl, self::$platform, $rootPhpBuildDir );
        }
    
        echo( "PROJECTS DONE\n\n" );
    
        chdir( $rootPhpBuildDir );
    }
    
    /////////////////////// SOLUTIONS /////////////////////////
    
    static function beginSolution( $name, $guid )
    {
        if( !self::$solution_cur )
        {
            self::$solution_cur       = new Solution( $name, $guid );
            self::$solutions[ $name ] = self::$solution_cur;
        }
        else
           trigger_error( "T3D_Generator::beginSolution() - tried to start $name but already in the ".self::$solution_cur->name." solution!", E_USER_ERROR );
    }
    
    static function addSolutionProjectRef( $pname )
    {
        if( isset( self::$solution_cur ) )	
            self::$solution_cur->addProjectRef( $pname );
        else
            trigger_error( "T3D_Generator::addSolutionProjectRef(): no such project - " . $pname . "\n", E_USER_ERROR );
    }
    
    static function addSolutionProjectRefExt( $pname, $ppath, $pguid )
    {
        if( isset( self::$solution_cur ) )	
            self::$solution_cur->addSolutionProjectRefExt( $pname, $ppath, $pguid );
        else
            trigger_error( "T3D_Generator::addSolutionProjectRefExt(): no such project - " . $pname . "\n", E_USER_ERROR );
    }
    
    static function endSolution()
    {
        if( isset( self::$solution_cur ) )
        {
            self::$solution_cur->setOutputs( BuildTarget::getInstances() );    
    
            self::$solution_cur = null;
        }    
        else
            trigger_error( "T3D_Generator::endSolution(): no active solution!\n", E_USER_ERROR );
    }
    
    static function generateSolutions( $tpl ) 
    {
        echo( "GENERATING SOLUTIONS\n\n" );
    
        $rootPhpBuildDir = getcwd();
    
        // Process all solutions	
        foreach( self::$solutions as $sname => $solution )
        {
            echo( "   - Generating solution: " . $sname . "\n" );			    
            
            chdir( $rootPhpBuildDir );
            
            $solution->generate( $tpl, self::$platform, $rootPhpBuildDir );
        }
        
        chdir( $rootPhpBuildDir );
    }
    
    static function setDLLRuntime( $val )
    {
        self::$useDLLRuntime = $val;
    }
} 
?>
