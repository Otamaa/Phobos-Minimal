@if not defined _echo echo off

rem Builds Phobos docs with Sphinx.

rem Ensure we're in correct directory.
cd /D "%~dp0"
cd ..\docs

sphinx-build -b gettext ./ ./locale
sphinx-intl update -p ./locale -l zh_CN