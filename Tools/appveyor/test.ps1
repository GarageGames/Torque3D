pushd $env:gamedir
.\Torque3*.exe runTests.cs
start-sleep -seconds 120
powershell -file $env:toolsdir\appveyor\takeScreenshot.ps1
mv console.log unitTests.log
push-appveyorArtifact unitTests.log 
popd
