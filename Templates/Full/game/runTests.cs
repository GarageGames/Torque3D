new GuiControlProfile(GuiDefaultProfile);
new GuiControlProfile(GuiToolTipProfile);
new GuiCanvas(Canvas);
function onLightManagerActivate() {}
function onLightManagerDeactivate() {}
Canvas.setWindowTitle("Torque 3D Unit Tests");
new RenderPassManager(DiffuseRenderPassManager);
setLightManager("Basic Lighting");
setLogMode(2);
$Con::LogBufferEnabled = false;
$Testing::checkMemoryLeaks = false;
runAllUnitTests();
quit();
