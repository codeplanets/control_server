#!/bin/bash

var=$(ps -ef | grep 'controlserver')
echo process info: ${var}

pid=$(echo ${var} | awk '{print $2}')
echo process id: ${pid}

kill -2 ${pid}
