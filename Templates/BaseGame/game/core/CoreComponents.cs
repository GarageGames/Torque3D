
function CoreComponentsModule::onCreate(%this)
{
   %classList = enumerateConsoleClasses( "Component" );

   foreach$( %componentClass in %classList )
   {
      echo("Native Component of type: " @ %componentClass);
   }  
}