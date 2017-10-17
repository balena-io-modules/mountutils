#!/bin/bash

PLATFORM=$(node -p process.platform)

if [[ $GITHUB_TOKEN ]]; then
  npm run prebuild -- --arch x64 -u "$GITHUB_TOKEN"
  if [[ $PLATFORM == "linux" ]]; then
    npm run prebuild -- --arch x86 -u "$GITHUB_TOKEN"
  fi
fi
