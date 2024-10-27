#!/bin/bash
# Script to stop and restart the application

# Kill the existing process
pkill -f imagenet4

# Wait for a few seconds to ensure the process is terminated
sleep 5

# Start the application
cd ~/jetson-inference/examples/my-recognition
./imagenet4 --model=./model_46.onnx --input_blob=input --output_blob=output --labels=./labels.txt csi://0
