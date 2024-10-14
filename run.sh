#!/bin/bash

# Compile client.c
echo "Compiling client.c..."
gcc utils.c client.c -o client -Wall -Wextra
if [ $? -ne 0 ]; then
    echo "Compilation of client.c failed."
    exit 1
fi

# Compile server.c
echo "Compiling server.c..."
gcc utils.c server.c -o server -Wall -Wextra
if [ $? -ne 0 ]; then
    echo "Compilation of server.c failed."
    exit 1
fi

echo "Compilation successful!"
