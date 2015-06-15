@setlocal

@for /f "delims=" %%A in ('luarocks pack %1') do @set "rock_name=%%A"

@set rock_name=%rock_name:Packed: =%

@echo make rock file as artifact: %rock_name%

@appveyor PushArtifact %rock_name% -DeploymentName %2

@endlocal

