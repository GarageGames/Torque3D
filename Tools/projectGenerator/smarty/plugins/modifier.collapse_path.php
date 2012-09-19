<?php
// Turn foo/../bar/../baz/ into baz/
function smarty_modifier_collapse_path($p)
{
    $p=explode('/', $p);
    $o=array();
    for ($i=0; $i<sizeof($p); $i++)
    {
        // Skip meaningless . or empty terms.
        if (''==$p[$i] || '.'==$p[$i])
           continue;

        // Consider if we can pop something off the list.
        if ('..'==$p[$i] && $i>0 && '..'!=$o[sizeof($o)-1])
        {
            array_pop($o);
            continue;
        }
        array_push($o, $p[$i]);
    }
    return implode('/', $o);
}

?>
