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

singleton TSShapeConstructor(FP_LurkerDAE)
{
   baseShape = "./FP_Lurker.DAE";
   lodType = "TrailingNumber";
   loadLights = "0";
};

function FP_LurkerDAE::onLoad(%this)
{
   %this.renameSequence("ambient", "timeline");
   %this.setSequenceCyclic("timeline", "0");

   %this.addSequence("timeline", "root", "120", "121", "1", "0");
   %this.addSequence("timeline", "idle_fidget1", "210", "390", "1", "0");
   %this.addSequence("timeline", "reload", "460", "550", "1", "0");
   %this.addSequence("timeline", "switch_out", "660", "670", "1", "0");
   %this.addSequence("timeline", "switch_in", "680", "690", "1", "0");
   %this.addSequence("timeline", "fire_alt", "580", "625", "1", "0");
   %this.addSequence("timeline", "fire", "420", "428", "1", "0");
   %this.addSequence("timeline", "Run", "10", "39", "1", "0");
   %this.addSequence("timeline", "sprint", "70", "89", "1", "0");
   %this.addSequence("timeline", "idle", "120", "179", "1", "0");

   %this.setSequenceCyclic("root", "1");
   %this.setSequenceCyclic("idle_fidget1", "0");
   %this.setSequenceCyclic("reload", "0");
   %this.setSequenceCyclic("switch_out", "0");
   %this.setSequenceCyclic("switch_in", "0");
   %this.setSequenceCyclic("fire_alt", "0");
   %this.setSequenceCyclic("fire", "0");
   %this.setSequenceCyclic("Run", "1");
   %this.setSequenceCyclic("sprint", "1");
   %this.setSequenceCyclic("idle", "1");
}
