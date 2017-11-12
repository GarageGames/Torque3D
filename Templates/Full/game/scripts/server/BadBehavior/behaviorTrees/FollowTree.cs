//--- OBJECT WRITE BEGIN ---
new Root(FollowTree) {
   canSave = "1";
   canSaveDynamicFields = "1";

   new Sequence() {
      canSave = "1";
      canSaveDynamicFields = "1";

      new ScriptEval() {
         behaviorScript = "return isObject(%obj.followObject) ? SUCCESS : FAILURE;\n";
         defaultReturnStatus = "SUCCESS";
         internalName = "has object to follow?";
         canSave = "1";
         canSaveDynamicFields = "1";
      };
      
      new FollowBehaviorAction() {
            preconditionMode = "TICK";
            internalName = "follow the object";
            canSave = "1";
            canSaveDynamicFields = "1";
      };
   };
};
//--- OBJECT WRITE END ---
