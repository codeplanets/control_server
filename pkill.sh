#!/bin/bash

echo "Shutdown"
pkill -2 -f controlserver
sleep 5
pkill -9 -f controlserver
./initsem
echo "Bye"
