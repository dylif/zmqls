#!/bin/bash

FILE=sample.json

echo "starting zmqls server..."
echo "using $FILE"
cd zmqls/build/server
./server --help
echo "waiting 3 seconds..."
sleep 3
./server "$FILE"