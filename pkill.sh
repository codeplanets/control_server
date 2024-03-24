#!/bin/bash
# pid=$(echo ${var} | awk '{print $2}')
# echo process id: ${pid}

echo "Shutdown"
pkill -2 -f controlserver
sleep 5
pkill -9 -f controlserver
echo "Bye"
