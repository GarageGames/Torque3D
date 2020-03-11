$ProfilerGraph::refreshRate = 32;
// Profiles
new GuiControlProfile (ProfilerGraphProfile)
{
   modal = false;
   opaque = false;
   canKeyFocus = false;
};

new GuiControlProfile (ProfilerGraphKeyContainerProfile)
{
   border = true;
   opaque = true;
   fillColor = "100 100 100 200";
};
new GuiControlProfile (ProfilerGraphGraphFrameRateProfile)
{
   border = false;
   fontColor = "255 255 255";
};
new GuiControlProfile (ProfilerGraphPolyCountProfile)
{
   border = false;
   fontColor = "255 0 0";
};
new GuiControlProfile (ProfilerGraphDrawCountProfile)
{
   border = false;
   fontColor = "0 255 0";
};
new GuiControlProfile (ProfilerGraphRTChangesProfile)
{
   border = false;
   fontColor = "0 0 255";
};
new GuiControlProfile (ProfilerGraphLatencyProfile)
{
   border = false;
   fontColor = "0 255 255";
};
new GuiControlProfile (ProfilerGraphPacketLossProfile)
{
   border = false;
   fontColor = "0 0 0";
};

function toggleProfilerGraph()
{
    if(!$ProfilerGraph::isInitialized)
    {
        ProfilerGraph::updateStats();
        $ProfilerGraph::isInitialized = true;
    }

    if(!Canvas.isMember(ProfilerGraphGui))
    {
        Canvas.add(ProfilerGraphGui);
    }
    else
      Canvas.remove(ProfilerGraphGui);
}

function ProfilerGraph::updateStats()
{
  $ProfilerGraphThread = ProfilerGraph.schedule($ProfilerGraph::refreshRate, "updateStats");

  if(!$Stats::netGhostUpdates)
     return;

  if(isobject(ProfilerGraph))
  {
    GhostsActive.setText("Frame Rate: " @ $fps::real);
    ProfilerGraph.addDatum(1,$fps::real);
    
    GhostUpdates.setText("Poly Count: " @ $GFXDeviceStatistics::polyCount);
    ProfilerGraph.addDatum(2,$GFXDeviceStatistics::polyCount);
    
    BitsSent.setText("Draw Calls: " @ $GFXDeviceStatistics::drawCalls);
    ProfilerGraph.addDatum(3,$GFXDeviceStatistics::drawCalls);
    
    BitsReceived.setText("Render Target Changes: " @ $GFXDeviceStatistics::renderTargetChanges);
    ProfilerGraph.addDatum(4,$GFXDeviceStatistics::renderTargetChanges);
    
    ProfilerGraph.matchScale(2,3);

    //Latency.setText("Latency: " @ ServerConnection.getPing());
    //ProfilerGraph.addDatum(5,ServerConnection.getPacketLoss());
    
    //PacketLoss.setText("Packet Loss: " @ ServerConnection.getPacketLoss());
  }
}

function ProfilerGraph::toggleKey()
{
  if(!GhostsActive.visible)
  {
    GhostsActive.visible = 1;
    GhostUpdates.visible = 1;
    BitsSent.visible = 1;
    BitsReceived.visible = 1;
    Latency.visible = 1;
    PacketLoss.visible = 1;
  }
  else
  {
    GhostsActive.visible = 0;
    GhostUpdates.visible = 0;
    BitsSent.visible = 0;
    BitsReceived.visible = 0;
    Latency.visible = 0;
    PacketLoss.visible = 0;
  }
}
