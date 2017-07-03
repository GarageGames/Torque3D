enableWinConsole(true);
exec("core/scripts/client/scriptDoc.cs");
writeOutFunctions();
writeOutClasses();
dumpEngineDocs("scriptModules.txt");
quit();
