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

singleton TSShapeConstructor(FP_TurretDAE)
{
   baseShape = "./FP_Turret.DAE";
   neverImport = "EnvironmentAmbientLight	null";
   loadLights = "0";
   lodType = "TrailingNumber";
};

function FP_TurretDAE::onLoad(%this)
{
   %this.renameSequence("ambient", "timeline");
   %this.addSequence("timeline", "root", "120", "121", "1", "0");
   %this.addSequence("timeline", "Run", "160", "189", "1", "0");
   %this.addSequence("timeline", "sprint", "160", "189", "1", "0");
   %this.addSequence("timeline", "idle", "70", "129", "1", "0");
   %this.addSequence("timeline", "fire", "230", "250", "1", "0");
   %this.setSequenceCyclic("fire", "0");
   %this.addSequence("timeline", "reload", "52", "53", "1", "0");
   %this.setSequenceCyclic("reload", "0");
   %this.addSequence("timeline", "switch_out", "280", "295", "1", "0");
   %this.setSequenceCyclic("switch_out", "0");
   %this.addSequence("timeline", "switch_in", "10", "30", "1", "0");
   %this.setSequenceCyclic("switch_in", "0");
   %this.addSequence("timeline", "soldier_sprint", "160", "189", "1", "0");
   %this.addSequence("timeline", "soldier_reload", "52", "53", "1", "0");
   %this.setSequenceCyclic("soldier_reload", "0");
   %this.addSequence("timeline", "soldier_run", "160", "189", "1", "0");
   %this.addSequence("timeline", "soldier_idle", "70", "129", "1", "0");
   %this.addSequence("timeline", "soldier_fire", "230", "250", "1", "0");
   %this.setSequenceCyclic("soldier_fire", "0");
   %this.addSequence("timeline", "soldier_switch_out", "280", "295", "1", "0");
   %this.setSequenceCyclic("soldier_switch_out", "0");
   %this.addSequence("timeline", "soldier_switch_in", "10", "30", "1", "0");
   %this.setSequenceCyclic("soldier_switch_in", "0");
}
