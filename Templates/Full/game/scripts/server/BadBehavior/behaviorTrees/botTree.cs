//--- OBJECT WRITE BEGIN ---
new Root(BotTree) {
   canSave = "1";
   canSaveDynamicFields = "1";

   new Parallel() {
      returnPolicy = "REQUIRE_ALL";
      canSave = "1";
      canSaveDynamicFields = "1";

      new Ticker() {
         frequencyMs = "1000";
         canSave = "1";
         canSaveDynamicFields = "1";

         new Loop() {
            numLoops = "0";
            terminationPolicy = "ON_FAILURE";
            canSave = "1";
            canSaveDynamicFields = "1";

            new ScriptedBehavior() {
               preconditionMode = "ONCE";
               internalName = "look for enemy";
               class = "pickTargetTask";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
         };
      };
      new Loop() {
         numLoops = "0";
         terminationPolicy = "ON_FAILURE";
         canSave = "1";
         canSaveDynamicFields = "1";

         new ActiveSelector() {
            recheckFrequency = "1000";
            canSave = "1";
            canSaveDynamicFields = "1";

            new SubTree() {
               subTreeName = "getHealthTree";
               internalName = "get health";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
            new SubTree() {
               subTreeName = "combatTree";
               internalName = "combat";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
            new SubTree() {
               subTreeName = "PatrolTree";
               internalName = "patrol";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
            new SubTree() {
               subTreeName = "WanderTree";
               internalName = "wander";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
         };
      };
   };
};
//--- OBJECT WRITE END ---
