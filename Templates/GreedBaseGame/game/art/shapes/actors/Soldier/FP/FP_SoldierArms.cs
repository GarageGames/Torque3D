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

singleton TSShapeConstructor(FP_SoldierArmsDAE)
{
   baseShape = "./FP_SoldierArms.DAE";
   loadLights = "0";
};

function FP_SoldierArmsDAE::onLoad(%this)
{
   // BEGIN: General rifle Sequences
   // Extracted from Lurker
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE run", "Rifle_run");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE sprint", "Rifle_sprint");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE idle", "Rifle_idle");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE idle_fidget1", "Rifle_idle_fidget1");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE fire", "Rifle_fire");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE reload", "Rifle_reload");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE fire_alt", "Rifle_fire_alt");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE switch_out", "Rifle_switch_out");
   %this.addSequence("art/shapes/weapons/Lurker/FP_Lurker.DAE switch_in", "Rifle_switch_in");
   
   %this.setSequenceCyclic("Rifle_idle_fidget1", "0");
   %this.setSequenceCyclic("Rifle_fire", "0");
   %this.setSequenceCyclic("Rifle_reload", "0");
   %this.setSequenceCyclic("Rifle_fire_alt", "0");
   %this.setSequenceCyclic("Rifle_switch_out", "0");
   %this.setSequenceCyclic("Rifle_switch_in", "0");
   // END: General rifle Sequences

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

   // BEGIN: ProxMine Sequences
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE run", "ProxMine_run");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE run2sprint", "ProxMine_run2sprint");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE sprint", "ProxMine_sprint");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE sprint2run", "ProxMine_sprint2run");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE idle", "ProxMine_idle");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE fire", "ProxMine_fire");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE fire_release", "ProxMine_fire_release");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE switch_out", "ProxMine_switch_out");
   %this.addSequence("art/shapes/weapons/ProxMine/FP_ProxMine.DAE switch_in", "ProxMine_switch_in");
   
   %this.setSequenceCyclic("ProxMine_fire", "0");
   %this.setSequenceCyclic("ProxMine_fire_release", "0");
   %this.setSequenceCyclic("ProxMine_switch_out", "0");
   %this.setSequenceCyclic("ProxMine_switch_in", "0");
   %this.setSequenceCyclic("ProxMine_run2sprint", "0");
   %this.setSequenceCyclic("ProxMine_sprint2run", "0");
   // END: ProxMine Sequences

   // BEGIN: Turret Sequences
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE run", "Turret_run");
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE sprint", "Turret_sprint");
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE idle", "Turret_idle");
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE fire", "Turret_fire");
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE switch_out", "Turret_switch_out");
   %this.addSequence("art/shapes/weapons/Turret/FP_Turret.DAE switch_in", "Turret_switch_in");
   
   %this.setSequenceCyclic("Turret_fire", "0");
   %this.setSequenceCyclic("Turret_switch_out", "0");
   %this.setSequenceCyclic("Turret_switch_in", "0");
   // END: Turret Sequences
}
