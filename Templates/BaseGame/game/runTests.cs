setLogMode(2);
$Con::LogBufferEnabled = false;
$Testing::CheckMemoryLeaks = false;
runAllUnitTests("-*.Stress*");
quit();
