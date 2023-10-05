#! /bin/bash
mkdir build
chmod +x autoBuild.sh
cmake . build
cmake --build build
