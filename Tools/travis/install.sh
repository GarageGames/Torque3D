#!/bin/bash
set -e

docker_compose () {
    docker-compose -f Tools/travis/dockerCompose.yml $@
}
export -f docker_compose

docker_exec () {
    docker exec --user root travis_${OS_ID}_1 bash -c \
           "cd Torque3D/; TRAVIS_OS_NAME=$TRAVIS_OS_NAME OS_ID=$OS_ID $@"
}
export -f docker_exec

if [[ $TRAVIS_BRANCH = development ]]; then export TRAVIS_TAG=dev; fi

if [[ ($OS_ID = "ubuntu-latest-lts" || $TRAVIS_OS_NAME = osx ) && \
          $TRAVIS_TAG && \
          $DEPLOYABLE_CONF = true ]]; then
    DEPLOY_CONDITION="true"
fi

if [[ $TRAVIS_OS_NAME = linux ]]; then
    docker_compose up -d $OS_ID
    docker_exec ./Tools/travis/installDependencies.sh

elif [[ $TRAVIS_OS_NAME = osx ]]; then
    ./Tools/travis/installDependencies.sh
fi
