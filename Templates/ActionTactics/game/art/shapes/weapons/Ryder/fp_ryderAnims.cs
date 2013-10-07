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

   // BEGIN: General pistol Sequences
   // Extracted from Ryder
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE run", "Pistol_run");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE sprint", "Pistol_sprint");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE idle", "Pistol_idle");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE fire", "Pistol_fire");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE reload", "Pistol_reload");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE switch_out", "Pistol_switch_out");
   %this.addSequence("art/shapes/weapons/Ryder/FP_Ryder.DAE switch_in", "Pistol_switch_in");
   
   %this.setSequenceCyclic("Pistol_fire", "0");
   %this.setSequenceCyclic("Pistol_reload", "0");
   %this.setSequenceCyclic("Pistol_switch_out", "0");
   %this.setSequenceCyclic("Pistol_switch_in", "0");
   // END: General pistol Sequences