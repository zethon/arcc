echo "Installing OpenCppCoverage"
choco install opencppcoverage
set path=C:\Program Files\OpenCppCoverage;%PATH%
choco install codecov
set path=C:\ProgramData\chocolatey\lib\codecov\tools;%PATH%