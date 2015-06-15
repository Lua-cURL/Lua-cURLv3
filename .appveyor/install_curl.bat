SETLOCAL

set PLAT_NAME=Win32
set CURL_PLAT=Win32
set CURL_URL=http://curl.haxx.se/download
set CURL_CFG=DLL Release - DLL Windows SSPI

if /I "%platform%" == "x64" (
  set PLAT_NAME=Win64
  set CURL_PLAT=x64
)

cd %APPVEYOR_BUILD_FOLDER%

echo =========================================
echo External library path: %LR_EXTERNAL%
echo =========================================

if not exist %LR_EXTERNAL%\libcurl.dll (
  @echo Download %CURL_URL%/curl-%CURL_VER%.zip ...

  appveyor DownloadFile %CURL_URL%/curl-%CURL_VER%.zip
  7z x curl-%CURL_VER%.zip
  cd curl-%CURL_VER%

  @echo Build curl %CURL_CFG% / %PLAT_NAME% ...

  if exist projects\Windows\VC12\curl.sln (
    msbuild projects\Windows\VC12\curl.sln /p:Configuration="%CURL_CFG%" /p:Platform=%CURL_PLAT%
  ) else (
    msbuild projects\Windows\VC12\curl-all.sln /p:Configuration="%CURL_CFG%" /p:Platform=%CURL_PLAT%
  )

  @echo Build curl done

  if not exist %LR_EXTERNAL%\include\curl mkdir %LR_EXTERNAL%\include\curl
  copy "include\curl\*.h" %LR_EXTERNAL%\include\curl
  copy "build\%PLAT_NAME%\VC12\%CURL_CFG%\libcurl.lib" %LR_EXTERNAL%\lib\libcurl.lib
  copy "build\%PLAT_NAME%\VC12\%CURL_CFG%\libcurl.dll" %LR_EXTERNAL%\libcurl.dll
)

if not exist %LR_EXTERNAL%\libcurl.dll (
  exit /B 1
)

appveyor PushArtifact %LR_EXTERNAL%\libcurl.dll -DeploymentName ext-deps

cd %APPVEYOR_BUILD_FOLDER%
