//-----------------------------------------------------------------------------
// Copyright (c) 2014 Guy Allard
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

$BotMatchTickFrequency = 250;

// start a bot match
function botMatch(%numBots)
{
   // Avoid having lots of dead bodies lying around.
   $CorpseTimeoutValue = 2000;

   // script object to attach the BT to
   if(!isObject(BotMatch))
      new ScriptObject(botMatch);
   
   // number of bots that will be spawned
   botMatch.numBots = %numBots;
   
   // set the behavior tree
   botMatch.setBehavior(botMatchTree, $BotMatchTickFrequency);
}


// cancel the match
function cancelBotmatch()
{
   // post the signal to the behavior tree
   if(isObject(botMatch))
      botMatch.behaviorTree.postSignal("onBotmatchCancel");
}

