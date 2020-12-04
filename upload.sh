#!/bin/bash
set -e

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

aws s3 cp MICROBIT.hex s3://edge-impulse-cdn-prod/demos/microbit-live.hex
aws cloudfront create-invalidation --distribution-id E2K84RSILM6W49 --paths "/demos/*"

echo "Uploaded. Now available at https://cdn.edgeimpulse.com/demos/microbit-live.hex"
