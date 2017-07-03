Set-PSDebug -Trace 1

function exec([scriptblock]$cmd, [string]$errorMessage = "Error executing command: " + $cmd){
    & $cmd
    if ($LastExitCode -ne 0) {
        throw $errorMessage
    }
}

if ($env:appveyor_repo_branch -eq "development"){
    $env:appveyor_repo_tag="true"
    $env:appveyor_repo_tag_name="dev"
}

if ($env:appveyor_repo_tag -and
    ($env:appveyor_build_worker_image -eq "Visual Studio 2015"))
{
    if($env:bin_deployable_conf){
        $env:bin_deploy_condition="true"
    }
    if($env:doxydoc_deployable_conf -and
       ($env:platform -eq "i386")){
        $env:doxydoc_deploy_condition="true"
    }
}

if($env:doxydoc_deploy_condition -eq "true"){
    exec { choco install -y doxygen.portable graphviz }
}

$env:rootdir=$env:appveyor_build_folder
$env:gamedir="$env:rootdir\My Projects\Torque3D\game"
$env:toolsdir="$env:rootdir\Tools"
