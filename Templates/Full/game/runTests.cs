setLogMode(2);
$Con::LogBufferEnabled = false;
$Testing::CheckMemoryLeaks = false;
runAllUnitTests();
quit();
