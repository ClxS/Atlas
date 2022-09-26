@echo off
SET VSVersion=2022
SET VSFlavour=Community
SET Platform=Win64
set OPENSSL_DIR=%~dp0..\src\ThirdParty\openssl
set OPENSSL_PREBUILT_DIR=%~dp0..\prebuilt\openssl\%PLATFORM%
set OPENSSL_PREBUILT_DIR_LIB=%~dp0..\prebuilt\openssl\%PLATFORM%\lib
set OPENSSL_PREBUILT_DIR_INCLUDE=%~dp0..\prebuilt\openssl\%PLATFORM%\include\openssl
set OPENSSL_CONFIG_ARGS=no-shared no-afalgeng no-ssl3 no-idea no-shared no-hw no-engine no-threads no-dso no-weak-ssl-ciphers no-rc2 no-rc4 no-rc5 no-md2 no-md4 no-async no-threads no-capieng no-dso no-dynamic-engine -DOPENSSL_USE_IPV6=0

call "C:\Program Files\Microsoft Visual Studio\%VSVersion%\%VSFlavour%\VC\Auxiliary\Build\vcvarsall.bat" x64
set Path=%Path%;%LOCALAPPDATA%\bin\NASM
pushd %OPENSSL_DIR%

echo Configuring OpenSSL Build (Debug)
perl Configure VC-WIN64A %OPENSSL_CONFIG_ARGS% --debug

echo Compiling (Debug)
nmake

echo Copying Artifacts
IF NOT EXIST %OPENSSL_PREBUILT_DIR_LIB% ( mkdir %OPENSSL_PREBUILT_DIR_LIB% )
IF NOT EXIST %OPENSSL_PREBUILT_DIR_INCLUDE% ( mkdir %OPENSSL_PREBUILT_DIR_INCLUDE% )

ROBOCOPY "%OPENSSL_DIR%" "%OPENSSL_PREBUILT_DIR_LIB%\debug" *.lib
ROBOCOPY "%OPENSSL_DIR%" "%OPENSSL_PREBUILT_DIR_LIB%\debug" ossl_static.pdb
ROBOCOPY "%OPENSSL_DIR%\include\openssl" "%OPENSSL_PREBUILT_DIR_INCLUDE%" opensslconf.h
del "%OPENSSL_DIR%\include\openssl\opensslconf.h"
del "%OPENSSL_DIR%\libssl.lib"
del "%OPENSSL_DIR%\libcrypto.lib"
del "%OPENSSL_DIR%\ossl_static.pdb"

echo Configuring OpenSSL Build (Release)
perl Configure VC-WIN64A %OPENSSL_CONFIG_ARGS% --release

echo Compiling (Release)
nmake

ROBOCOPY "%OPENSSL_DIR%" "%OPENSSL_PREBUILT_DIR_LIB%\release" *.lib
ROBOCOPY "%OPENSSL_DIR%" "%OPENSSL_PREBUILT_DIR_LIB%\release" ossl_static.pdb
del "%OPENSSL_DIR%\include\openssl\opensslconf.h"
del "%OPENSSL_DIR%\libssl.lib"
del "%OPENSSL_DIR%\libcrypto.lib"
del "%OPENSSL_DIR%\ossl_static.pdb"

popd