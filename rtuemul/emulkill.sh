#!/bin/bash

echo "Shutdown"
pkill -2 -f rtuemul
sleep 5
pkill -9 -f rtuemul
echo "Bye"
