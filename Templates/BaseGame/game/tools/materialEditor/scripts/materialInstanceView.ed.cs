function MaterialInstanceFilter::onGainFirstResponder( %this )
{
   %this.selectAllText();
}

function MaterialInstanceFilter::onReturn( %this )
{
   %text = %this.getText();
   if( %text $= "" )
   {
      %this.reset();
      MaterialInstanceViewTree.clearFilterText();
   }
   else
   {
      MaterialInstanceViewTree.setFilterText(%text);
   }
   
   MaterialInstanceViewTree.buildVisibleTree(true);
}

function MaterialInstanceFilterBtn::onClick(%this)
{
   MaterialInstanceFilter.reset();
   MaterialInstanceViewTree.clearFilterText();
   MaterialInstanceViewTree.buildVisibleTree(true);
}