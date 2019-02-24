//--- OBJECT WRITE BEGIN ---
new Root(CombatTree) {
   canSave = "1";
   canSaveDynamicFields = "1";

   new Sequence() {
      canSave = "1";
      canSaveDynamicFields = "1";

      new ScriptEval() {
         behaviorScript = "if (isObject(%obj.targetObject) && %obj.targetObject.isEnabled()) return SUCCESS;";
         defaultReturnStatus = "FAILURE";
         canSave = "1";
         canSaveDynamicFields = "1";
      };
      new Parallel() {
         returnPolicy = "REQUIRE_ALL";
         canSave = "1";
         canSaveDynamicFields = "1";

         new Loop() {
            numLoops = "0";
            terminationPolicy = "ON_FAILURE";
            canSave = "1";
            canSaveDynamicFields = "1";

            new Sequence() {
               canSave = "1";
               canSaveDynamicFields = "1";

               new ScriptedBehavior() {
                  preconditionMode = "ONCE";
                  internalName = "combatMoveTask";
                  class = "combatMoveTask";
                  canSave = "1";
                  canSaveDynamicFields = "1";
               };
               new RandomWait() {
                  waitMinMs = "500";
                  waitMaxMs = "1000";
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

            new Ticker() {
               frequencyMs = "500";
               canSave = "1";
               canSaveDynamicFields = "1";

               new ScriptedBehavior() {
                  preconditionMode = "TICK";
                  internalName = "aim";
                  class = "aimAtTargetTask";
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

            new SucceedAlways() {
               canSave = "1";
               canSaveDynamicFields = "1";

               new Sequence() {
                  canSave = "1";
                  canSaveDynamicFields = "1";

                  new ScriptedBehavior() {
                     preconditionMode = "ONCE";
                     internalName = "fire";
                     class = "shootAtTargetTask";
                     canSave = "1";
                     canSaveDynamicFields = "1";
                  };
                  new RandomWait() {
                     waitMinMs = "500";
                     waitMaxMs = "1000";
                     canSave = "1";
                     canSaveDynamicFields = "1";
                  };
               };
            };
         };
      };
   };
};
//--- OBJECT WRITE END ---
