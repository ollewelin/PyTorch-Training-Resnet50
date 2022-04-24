#!/bin/bash

~/jetson-inference/examples/my-recognition/imagenet3 --model=./model_31.onnx --input_blob=input --output_blob=output --labels=./labels.txt csi://0 rtp://192.168.1.242:1234


