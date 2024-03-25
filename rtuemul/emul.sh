#!/bin/bash

for i in {9000001..9000200}
do
    nohup ./rtuemul $i > /dev/null &
    sleep 0.5
done
