#!/bin/bash
set -e

export DOCKER_BUILDKIT=1

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

TEMPDIR=/tmp/build
mkdir $TEMPDIR

# clean out existing model folders
rm -r /app/source/model-parameters/
rm -r /app/source/tflite-model/

# copy the input files
cp -r /home/input/* $TEMPDIR
rm -rf $TEMPDIR/edge-impulse-sdk
cp -r $TEMPDIR/* /app/firmware/

echo "Building firmware..."

cd /app
python build.py

echo "Building firmware OK"
echo ""

echo "Copying artefacts..."

cp /app/MICROBIT.hex /home/output/MICROBIT.hex

echo "Copying artefacts OK"
