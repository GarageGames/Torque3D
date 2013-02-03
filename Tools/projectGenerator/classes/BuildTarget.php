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

class BuildTarget
{
    private static $instances = array(); 	

    public $name;
    public $output_dir;
    public $project_dir;
    public $project_rel_path;
    public $base_dir;
    public $template_app;
    public $template_shared_app;
    public $template_lib;
    public $template_shared_lib;
    public $template_activex;
    public $template_sln;
    public $template_user;
    public $template_csproj;
    public $solution_ext;
    public $output_ext;
    public $file_exts;
    public $reject_patternss;
    public $dont_compile_patterns;
    public $source_file_exts;
    public $platforms;
    public $ldelim;
    public $rdelim;
    
    
    static function add( $name, $out, $project, $base, $template_app, $template_shared_app, $template_lib, $template_shared_lib, $template_activex, $out_ext )
    {
        $c = new BuildTarget( $name, $out, $project, $base, $template_app, $template_shared_app, $template_lib, $template_shared_lib, $template_activex, $out_ext );
        
        self::$instances[ $name ] = $c;
        
        return $c;
    }
    
    static function getInstances()
    {
        return self::$instances;
    }
    
    function BuildTarget( $name, $out, $project, $base, $template_app, $template_shared_app, $template_lib, $template_shared_lib, $template_activex, $out_ext )
    {
        $this->name 	  	   = $name;
        $this->output_dir 	= $out;
        $this->project_dir   = $project;
        $this->output_ext	   = $out_ext;
        $this->base_dir	  	= $base;
        $this->template_app	= $template_app;
        $this->template_shared_app	= $template_shared_app;
        $this->template_lib	= $template_lib;
        $this->template_shared_lib	= $template_shared_lib;
        $this->template_activex = $template_activex;
        $this->template_csproj = "";
        $this->template_user = "";
        
        // The template for a filters file used by VS2010.
        $this->template_filter = "";

        $p = explode( '/', $project );
        $o = array();

      for( $i = 0; $i < sizeof( $p ); $i++ )
      {
         // Skip meaningless . or empty terms.
         if( $p[ $i ] == '' || $p[ $i ] == '.' )
            continue;

         array_push( $o, '..' );
      }

      $this->project_rel_path = implode( '/', $o );
      
      if (strlen($this->project_rel_path) > 0)
         $this->project_rel_path = $this->project_rel_path . "/";
    }	

    function setDelimiters( $l, $r )
    {
        $this->ldelim = $l;
        $this->rdelim = $r;
    }

    function setSolutionInfo( $template_sln, $template_user, $output_ext, $template_filter = "" )
    {
        $this->template_sln = $template_sln;
        $this->solution_ext = $output_ext;
        $this->template_user = $template_user;
        $this->template_filter = $template_filter;
    }
    
    function setDotNetInfo($template_csprog)
    {
        $this->template_csproj = $template_csprog;
    }

    function setFileExtensions()
    {
        $args  = func_get_args();
        $count = func_num_args();
    
        $this->file_exts = $args;
    }
    
    function setSourceFileExtensions()
    {
        $args  = func_get_args();
        $count = func_num_args();
    
        $this->source_file_exts = $args;
    }

    function setRejectPatterns()
    {
        $args = func_get_args();
        
        $this->reject_patterns = $args;
    }
    
    function setDontCompilePatterns()
    {
        $args = func_get_args();
        
        $this->dont_compile_patterns = $args;
    }
    
    function setPlatforms()
    {
        $args = func_get_args();
        
        $this->platforms = $args;
    }
    
    function supportsPlatform( $platform )
    {
        if( isset( $this->platforms ) )
            foreach( $this->platforms as $rule )
                if( strcmp( $rule, $platform ) == 0 )
                    return true;
                    
        return false;					
    }
    
    function ruleReject( $file )
    {
        if( isset( $this->reject_patterns ) )		
            foreach( $this->reject_patterns as $rule )			
                if( preg_match( $rule, $file ) ) 
                    return true;
            
        return false;
    }
    
    function allowedFileExt( $file )
    {
        foreach( $this->file_exts as $ext ) 
        {
            $ext 			= ".{$ext}";
            $extLen 		= strlen( $ext );
            $possibleMatch	= substr( $file, -$extLen, $extLen );

            if( $possibleMatch == $ext )
                return true; 
        }
        
        return false;
    }

    function isSourceFile( $file )
    {
        foreach( $this->source_file_exts as $ext ) 
        {
            $ext 			= ".{$ext}";
            $extLen 		= strlen( $ext );
            $possibleMatch	= substr( $file, -$extLen, $extLen );
            if( $possibleMatch == $ext )
                return true; 
        }
		
        return false;
    }

    function isResourceFile( $file )
    {
        $ext 			= ".rc";
        $extLen 		= strlen( $ext );
        $possibleMatch	= substr( $file, -$extLen, $extLen );
        if( $possibleMatch == $ext )
            return true; 

        return false;
    }
}

?>
