@echo off
setlocal

REM
REM Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
REM
REM Distributed under the Boost Software License, Version 1.0. (See accompanying
REM file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
REM

if "%~1"=="" (
    echo No playbook supplied, using default playbook
    set "PLAYBOOK=local-playbook.yml"
) else (
    set "PLAYBOOK=%~1"
)

echo Building documentation with Antora...
echo Installing npm dependencies...
call npm ci

echo Building docs in custom dir...
set "PATH=%CD%\node_modules\.bin;%PATH%"
call npx antora --clean --fetch "%PLAYBOOK%"
echo Done
