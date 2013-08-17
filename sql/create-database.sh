#!/bin/bash

command -v sqlite3 &> /dev/null
if [ $? -eq 1 ]; then
  echo "sqlite3 is required."
fi

if [ $# -ne 2 ]; then
  echo "usage: $0 <path-to-database> <schema>"
  exit 1
fi

if [ -f $1 ]; then
  echo "error: file $1 already exists"
  exit 1
fi

sqlite3 "$1" < $2
