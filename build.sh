#!/bin/bash
set -e

export DOCKER_BUILDKIT=1

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

echo "Synchronizing files..."

rsync --recursive --ignore-times --checksum --delete --exclude ".git" --exclude "libraries" $SCRIPTPATH/ /app

echo "Synchronizing files OK"

echo "Building firmware..."

cd /app
python build.py

echo "Building firmware OK"

echo "Copying files..."

rm -rf $SCRIPTPATH/build
cp -r build/ $SCRIPTPATH/build
cp MICROBIT.* $SCRIPTPATH

echo "Copying files OK"
