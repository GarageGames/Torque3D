//--- OBJECT WRITE BEGIN ---
new Root(getHealthTree) {
   canSave = "1";
   canSaveDynamicFields = "1";

   new Sequence() {
      canSave = "1";
      canSaveDynamicFields = "1";

      new ScriptEval() {
         behaviorScript = "if (%obj.getDamagePercent() > 0.75) return SUCCESS;";
         defaultReturnStatus = "FAILURE";
         internalName = "need health?";
         canSave = "1";
         canSaveDynamicFields = "1";
      };
      new ScriptedBehavior() {
         preconditionMode = "ONCE";
         internalName = "look for health";
         class = "findHealthTask";
         canSave = "1";
         canSaveDynamicFields = "1";
      };
      new ScriptedBehavior() {
         preconditionMode = "TICK";
         internalName = "get health";
         class = "getHealthTask";
         canSave = "1";
         canSaveDynamicFields = "1";
      };
   };
};
//--- OBJECT WRITE END ---
