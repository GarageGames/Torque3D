# make bin bundle
pushd $env:gamedir
if ($env:appveyor_repo_tag_name -eq "dev"){
    echo "The last commit was $env:appveyor_repo_commit when compiling this binary." > VERSION
}
$env:bundle_name="Torque3D-${env:appveyor_repo_tag_name}-Win-${env:platform}.zip"
7z a $env:bundle_name *
mv $env:bundle_name $env:rootdir
popd

# mv doxygen output
mkdir -p doc\$env:appveyor_repo_tag_name
mv $env:toolsdir\doxygen\output\* doc\$env:appveyor_repo_tag_name
rm doc\.gitignore
rm doc\$env:appveyor_repo_tag_name\.gitignore
