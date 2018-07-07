Set-PSDebug -Trace 1

switch -wildcard ($env:appveyor_build_worker_image)
{ 
    "*2013" { $env:generator="Visual Studio 12 2013" }
    "*2015" { $env:generator="Visual Studio 14 2015" }
    "*2017" { $env:generator="Visual Studio 15 2017" }
    default { throw "Unkown Worker Image!" }
}
if ($env:platform -eq "amd64"){
    $env:generator=$env:generator + " Win64"
}

mkdir -p "My Projects\Torque3D\buildFiles\CMake"
pushd "My Projects\Torque3D\buildFiles\CMake"
cmake ..\..\..\..\ -G $env:generator -DTORQUE_APP_NAME=Torque3D $env:additional_cmake_args
exec { iex "cmake --build . --target install $env:additional_build_args" }
popd

if ($env:doxydoc_deploy_condition -eq "true"){
    pushd "Tools\doxygen\"
    . .\generateInput.ps1
    doxygen engineReference.cfg
    doxygen scriptReference.cfg
    popd
}

powershell -file $env:toolsdir\appveyor\takeScreenshot.ps1
