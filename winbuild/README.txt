Tools needed
------------
1. Windows 10
2. Visual Studio C++ 2017
3. Windows 10 SDK 

Installing dependencies
-----------------------

1. PThreads
-----------
- download latest bin ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.zip
- extract to some folder
x86 version:
	- copy Pre-build.2/lib/x86/pthreadVC2.lib to winbuild\dist\lib\x86\ folder
x64 version:
	- copy Pre-build.2/lib/x64/pthreadVC2.lib to winbuild\dist\lib\x64\ folder

2. Install AMD APP SDK (OpenCL), latest version
-----------------------------------------------
- go to https://developer.amd.com/amd-accelerated-parallel-processing-app-sdk/ and download appropriate version (x86/x64) and install
- copy C:\Program Files (x86)\AMD APP SDK\3.0\lib\x86\OpenCL.lib to winbuild/dist/lib/x86/
- copy C:\Program Files (x86)\AMD APP SDK\3.0\bin\x86\OpenCL.dll to winbuild/dist/bin/x86/
- copy C:\Program Files (x86)\AMD APP SDK\3.0\lib\x86_64\OpenCL.lib to winbuild/dist/lib/x64/
- copy C:\Program Files (x86)\AMD APP SDK\3.0\bin\x86_64\OpenCL.dll to winbuild/dist/bin/x64/
- copy C:\Program Files (x86)\AMD APP SDK\3.0\include\CL\* winbuild/dist/include/CL/

3. Download AMD Display Library (ADL) SDK, latest version
-----------------------------------------------
- go to https://developer.amd.com/display-library-adl-sdk/
- extract to some folder
- copy include/* to ADL_SDK


4. PDCurses
-----------
- download source http://sourceforge.net/projects/pdcurses/files/pdcurses/3.4/pdcurs34.zip/download and extract it somewhere
- copy curses.h to winbuild\dist\include\
x86 version:
	- open Visual Studio Command Prompt (x86)
	- go to win32 folder
	- execute: nmake -f vcwin32.mak WIDE=1 UTF8=1 pdcurses.lib DLL=Y
	- copy newly created pdcurses.lib to winbuild\dist\lib\x86\ folder
x64 version:
- open Visual Studio Command Prompt (x64)
	- go to win32 folder
	- edit vcwin32.mak end change line:
		cvtres /MACHINE:IX86 /NOLOGO /OUT:pdcurses.obj pdcurses.res
		to
		cvtres /MACHINE:X64 /NOLOGO /OUT:pdcurses.obj pdcurses.res
	- execute: nmake -f vcwin32.mak WIDE=1 UTF8=1 pdcurses.lib DLL=Y
	- copy newly created pdcurses.lib to winbuild\dist\lib\x64\ folder


5. OpenSSL (needed for Curl)
----------------------------
- go to http://slproweb.com/products/Win32OpenSSL.html and download 1.02o full installer x86 and/or x64 (not light version)
- install to default location (e.g C:\OpenSSL-Win32 or C:\OpenSSL-Win64) and select bin/ folder when asked
- install Visual C++ (x86/x64) Redistributables if needed

6. Curl
-------
- go to http://curl.haxx.se/download.html and download latest source (>=7.39.0) and extract it somewhere
- replace original curl winbuild\MakefileBuild.vc with provided winbuild\MakefileBuild.vc (corrected paths and static library names for VC)

x86 version:
- open Visual Studio Command Prompt (x86)
	- go to winbuild folder and execute:
		nmake -f Makefile.vc mode=static VC=13 WITH_DEVEL=C:\OpenSSL-Win32 WITH_SSL=static ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no GEN_PDB=no DEBUG=no MACHINE=x86
	- copy builds\libcurl-vc13-x86-release-static-ssl-static\lib\libcurl_a.lib to winbuild\dist\lib\x86
	- copy builds\libcurl-vc13-x86-release-static-ssl-static\include\* winbuild\dist\include\

x64 version:
- open Visual Studio Command Prompt (x64)
	- go to winbuild folder and execute:
		nmake -f Makefile.vc mode=static VC=13 WITH_DEVEL=C:\OpenSSL-Win64 WITH_SSL=static ENABLE_SSPI=no ENABLE_IPV6=no ENABLE_IDN=no GEN_PDB=no DEBUG=no MACHINE=x64
	- copy builds\libcurl-vc13-x64-release-static-ssl-static\lib\libcurl_a.lib to winbuild\dist\lib\x64
	- copy builds\libcurl-vc13-x64-release-static-ssl-static\include\* winbuild\dist\include\

7. Jansson
----------
If using git run commands below from sgminer/ folder:

  git submodule init
  git submodule update
  
or clone/extract Jansson source from https://github.com/akheron/jansson to submodules/jansson folder.
