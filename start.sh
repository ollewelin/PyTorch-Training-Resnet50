#!/bin/bash

cd ../olle_test1/home/jetson/jetson-inference/examples/my-recognition

./imagenet3 --model=./model_28.onnx --input_blob=input --output_blob=output --labels=./labels.txt csi://0
