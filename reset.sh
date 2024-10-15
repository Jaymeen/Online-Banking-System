#!/bin/bash

DIRECTORY_PATH=/home/jarvis/Desktop/Banking-System/data

# Check if the provided path is a directory
if [ ! -d "$DIRECTORY_PATH" ]; then
    echo "Error: $DIRECTORY_PATH is not a valid directory."
    exit 1
fi

# Remove all files and directories inside the specified path
rm -rf "$DIRECTORY_PATH"/*
rm "client"
rm "extra"
rm "server"

echo "All files and directories in $DIRECTORY_PATH have been removed."

gcc extra.c -o extra
./extra

echo "All required files have been re-created."