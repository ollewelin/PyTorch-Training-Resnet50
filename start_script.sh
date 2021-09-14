#!/bin/bash

cd ~/jetson-inference

docker/run.sh --volume /:/olle_test1 -r "bash ../olle_test1/home/jetson/jetson-inference/examples/my-recognition/start.sh"
