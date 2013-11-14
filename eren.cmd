@echo off
if %1¨==¨ gosub usage
if %1==/a gosub appendfname
for %%a in (%1) do exifren %%a
quit
:appendfname
for %%a in (%2) do exifren /a %%a
quit
:usage
echo Eren [/a] wildcard.jpg
echo /a = append filename after date/time

