Set-PSDebug -Trace 1

pushd $env:gamedir
cp $env:toolsdir\doxygen\dumpScriptDoc.cs .
& ".\Torque3*.exe" dumpScriptDoc.cs
start-sleep -seconds 60
mv scriptClasses.txt $env:toolsdir\doxygen\input  
mv scriptFunctions.txt $env:toolsdir\doxygen\input
mv scriptModules.txt $env:toolsdir\doxygen\input
popd
