@echo off

if %APPVEYOR_REPO_BRANCH% == master (
  if %GITHUB_TOKEN% neq "" (
    npm run prebuild -- --arch x64 -u %GITHUB_TOKEN%
    npm run prebuild -- --arch x86 -u %GITHUB_TOKEN%
  )
)
