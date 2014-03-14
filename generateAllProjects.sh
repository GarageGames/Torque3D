#!/bin/bash

for project in $(cat allProjects.txt | sed s/\\\\/\\//g)
do
if [ -e $project/generateProjects.sh ]; then
    echo
    echo
    echo === $project =======
    echo
    pushd .
    cd $project
    ./generateProjects.sh
    popd
else
    echo "Project '$project' specified in allProjects.txt does not exists."
fi
done
