//--- OBJECT WRITE BEGIN ---
new Root(botMatchTree) {
   canSave = "1";
   canSaveDynamicFields = "1";

   new Selector() {
      canSave = "1";
      canSaveDynamicFields = "1";

      new Sequence() {
         internalName = "run the match";
         canSave = "1";
         canSaveDynamicFields = "1";

         new ScriptEval() {
            behaviorScript = "if(isObject(%obj.botGroup))\n   %obj.botGroup.delete();\n\n%obj.botGroup = new SimGroup();\n%obj.countdown = 5;\n%obj.numBotsToSpawn = %obj.numBots;";
            defaultReturnStatus = "SUCCESS";
            internalName = "init";
            canSave = "1";
            canSaveDynamicFields = "1";
         };
         new Sequence() {
            internalName = "countdown";
            canSave = "1";
            canSaveDynamicFields = "1";

            new Ticker() {
               frequencyMs = "1000";
               canSave = "1";
               canSaveDynamicFields = "1";

               new Loop() {
                  numLoops = "5";
                  terminationPolicy = "ON_FAILURE";
                  canSave = "1";
                  canSaveDynamicFields = "1";

                  new ScriptEval() {
                     behaviorScript = "centerPrintAll(\"BotMatch in\" SPC %obj.countdown, 2);\n%obj.countdown --;";
                     defaultReturnStatus = "SUCCESS";
                     internalName = "countdown message";
                     canSave = "1";
                     canSaveDynamicFields = "1";
                  };
               };
            };
            new ScriptEval() {
               behaviorScript = "centerPrintAll(\"GO!\",1);";
               defaultReturnStatus = "SUCCESS";
               internalName = "lets go";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
         };
         new SucceedAlways() {
            canSave = "1";
            canSaveDynamicFields = "1";

            new Loop() {
               numLoops = "0";
               terminationPolicy = "ON_FAILURE";
               canSave = "1";
               canSaveDynamicFields = "1";

               new Sequence() {
                  internalName = "spawn bots";
                  canSave = "1";
                  canSaveDynamicFields = "1";

                  new ScriptEval() {
                     behaviorScript = "// pick a marker to spawn at\n%spawnpoint = PatrolPath.getRandom();\n\n// create the bot\n%bot = BadBot::spawn(\"\", %spawnpoint);\n\n// set its behavior\n%bot.setbehavior(BotTree, $BotTickFrequency);\n\n// add it to the botgroup\n%obj.botGroup.add(%bot);\n\n// keep track of the current bot\n%obj.currentBot = %bot;\n\n// decrement the number of bots left to spawn\n%obj.numBotsToSpawn --;";
                     defaultReturnStatus = "SUCCESS";
                     internalName = "spawn one bot";
                     canSave = "1";
                     canSaveDynamicFields = "1";
                  };
                  new RandomSelector() {
                     internalName = "pick a weapon";
                     canSave = "1";
                     canSaveDynamicFields = "1";

                     new ScriptEval() {
                        behaviorScript = "%obj.currentBot.use(Ryder);";
                        defaultReturnStatus = "SUCCESS";
                        internalName = "Ryder";
                        canSave = "1";
                        canSaveDynamicFields = "1";
                     };
                     new ScriptEval() {
                        behaviorScript = "%obj.currentBot.use(Lurker);";
                        defaultReturnStatus = "SUCCESS";
                        internalName = "Lurker";
                        canSave = "1";
                        canSaveDynamicFields = "1";
                     };
                  };
                  new ScriptEval() {
                     behaviorScript = "if(%obj.numBotsToSpawn == 0) return FAILURE;";
                     defaultReturnStatus = "SUCCESS";
                     internalName = "check if more bots to spawn";
                     canSave = "1";
                     canSaveDynamicFields = "1";
                  };
               };
            };
         };
         new FailAlways() {
            canSave = "1";
            canSaveDynamicFields = "1";

            new WaitForSignal() {
               signalName = "onBotmatchCancel";
               timeoutMs = "0";
               internalName = "stop on cancel signal";
               canSave = "1";
               canSaveDynamicFields = "1";
            };
         };
      };
      new Sequence() {
         internalName = "end the match";
         canSave = "1";
         canSaveDynamicFields = "1";

         new ScriptEval() {
            behaviorScript = "%obj.botGroup.delete();";
            defaultReturnStatus = "SUCCESS";
            internalName = "remove bots";
            canSave = "1";
            canSaveDynamicFields = "1";
         };
         new ScriptEval() {
            behaviorScript = "%obj.behaviorTree.schedule(10, stop);";
            defaultReturnStatus = "SUCCESS";
            internalName = "stop";
            canSave = "1";
            canSaveDynamicFields = "1";
         };
      };
   };
};
//--- OBJECT WRITE END ---
