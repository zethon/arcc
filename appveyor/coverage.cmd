OpenCppCoverage.exe --export_type cobertura:coverage.xml --modules "*.exe" --sources %APPVEYOR_BUILD_FOLDER% --excluded_sources "%APPVEYOR_BUILD_FOLDER%\build\*" --excluded_sources "%APPVEYOR_BUILD_FOLDER%\Simple-Web-Server\*" --excluded_sources "%APPVEYOR_BUILD_FOLDER%\rang\*" -- ctest -C %CONFIGURATION% -V
codecov -f coverage.xml --root %APPVEYOR_BUILD_FOLDER%
