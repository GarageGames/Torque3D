<?php

// Written for GarageGames project generator Xcode files

// Map a file path to a unique 23-digit number [unique for the life of the PHP execution]
function smarty_modifier_uid($string)
{
    static $uid = 1;
    static $map = array();
    
    if ( !array_key_exists( $string, $map ) )
        $map[$string] = sprintf( "%023X", $uid++ );
        
   return $map[$string]; 
}

?>
