$ProfilerGraphCtrl::refreshRate = 32;
// Profiles
new GuiControlProfile (ProfilerGraphCtrlProfile)
{
   modal = false;
   opaque = false;
   canKeyFocus = false;
};

new GuiControlProfile (ProfilerGraphCtrlKeyContainerProfile)
{
   border = true;
   opaque = true;
   fillColor = "100 100 100 200";
};
new GuiControlProfile (ProfilerGraphCtrlGraphFrameRateProfile)
{
   border = false;
   fontColor = "255 255 255";
};
new GuiControlProfile (ProfilerGraphCtrlPolyCountProfile)
{
   border = false;
   fontColor = "255 0 0";
};
new GuiControlProfile (ProfilerGraphCtrlDrawCountProfile)
{
   border = false;
   fontColor = "0 255 0";
};
new GuiControlProfile (ProfilerGraphCtrlRTChangesProfile)
{
   border = false;
   fontColor = "0 0 255";
};
new GuiControlProfile (ProfilerGraphCtrlLatencyProfilerProfile)
{
   border = false;
   fontColor = "0 255 255";
};
new GuiControlProfile (ProfilerGraphCtrlPacketLossProfilerProfile)
{
   border = false;
   fontColor = "0 0 0";
};

function toggleProfilerGraphCtrl()
{
    if(!$ProfilerGraphCtrl::isInitialized)
    {
        ProfilerGraphCtrl::updateStats();
        $ProfilerGraphCtrl::isInitialized = true;
    }

    if(!Canvas.isMember(ProfilerGraphCtrlGui))
    {
        Canvas.add(ProfilerGraphCtrlGui);
    }
    else
      Canvas.remove(ProfilerGraphCtrlGui);
}

function ProfilerGraphCtrl::updateStats()
{
  $ProfilerGraphCtrlThread = ProfilerGraphCtrl.schedule($ProfilerGraphCtrl::refreshRate, "updateStats");

  if(!$Stats::netGhostUpdatesProfiler)
     return;

  if(isobject(ProfilerGraphCtrl))
  {
    GhostsActiveProfiler.setText("Frame Rate: " @ $fps::real);
    ProfilerGraphCtrl.addDatum(1,$fps::real);
    
    GhostUpdatesProfiler.setText("Poly Count: " @ $GFXDeviceStatistics::polyCount);
    ProfilerGraphCtrl.addDatum(2,$GFXDeviceStatistics::polyCount);
    
    BitsSentProfiler.setText("Draw Calls: " @ $GFXDeviceStatistics::drawCalls);
    ProfilerGraphCtrl.addDatum(3,$GFXDeviceStatistics::drawCalls);
    
    BitsReceivedProfiler.setText("Render Target Changes: " @ $GFXDeviceStatistics::renderTargetChanges);
    ProfilerGraphCtrl.addDatum(4,$GFXDeviceStatistics::renderTargetChanges);
    
    ProfilerGraphCtrl.matchScale(2,3);

    //LatencyProfiler.setText("LatencyProfiler: " @ ServerConnection.getPing());
    //ProfilerGraphCtrl.addDatum(5,ServerConnection.getPacketLossProfiler());
    
    //PacketLossProfiler.setText("Packet Loss: " @ ServerConnection.getPacketLossProfiler());
  }
}

function ProfilerGraphCtrl::toggleKey()
{
  if(!GhostsActiveProfiler.visible)
  {
    GhostsActiveProfiler.visible = 1;
    GhostUpdatesProfiler.visible = 1;
    BitsSentProfiler.visible = 1;
    BitsReceivedProfiler.visible = 1;
    LatencyProfiler.visible = 1;
    PacketLossProfiler.visible = 1;
  }
  else
  {
    GhostsActiveProfiler.visible = 0;
    GhostUpdatesProfiler.visible = 0;
    BitsSentProfiler.visible = 0;
    BitsReceivedProfiler.visible = 0;
    LatencyProfiler.visible = 0;
    PacketLossProfiler.visible = 0;
  }
}
