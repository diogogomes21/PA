#!/bin/bash
for dir in ~/Downloads/*/*/; do (cd "$dir" && echo ==== &&echo $dir && make clean); done
