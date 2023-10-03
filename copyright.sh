#!/usr/bin/env bash
for file in $(find . -type f -name \*.cpp); do
  echo "/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/" > copyright-file.txt;
  echo "" >> copyright-file.txt;
  cat $file >> copyright-file.txt;
  mv copyright-file.txt $file;
done
