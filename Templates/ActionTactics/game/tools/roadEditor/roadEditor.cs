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

singleton GuiControlProfile( RoadEditorProfile )
{
   canKeyFocus = true;
   opaque = true;
   fillColor = "192 192 192 192";
   category = "Editor";
};

singleton GuiControlProfile (GuiSimpleBorderProfile)
{
   opaque = false;   
   border = 1;   
   category = "Editor";
};

singleton GuiCursor(RoadEditorMoveCursor)
{
   hotSpot = "4 4";
   renderOffset = "0 0";
   bitmapName = "~/gui/images/macCursor";
   category = "Editor";
};  

singleton GuiCursor( RoadEditorMoveNodeCursor )
{
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./Cursors/outline/drag_node_outline";
   category = "Editor";
};

singleton GuiCursor( RoadEditorAddNodeCursor )
{
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./Cursors/outline/add_to_end_outline";
   category = "Editor";
};

singleton GuiCursor( RoadEditorInsertNodeCursor )
{
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./Cursors/outline/insert_in_middle_outline";
   category = "Editor";
};

singleton GuiCursor( RoadEditorResizeNodeCursor )
{
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./Cursors/outline/widen_path_outline";
   category = "Editor";
};
