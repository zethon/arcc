echo "Installing OpenCppCoverage"

REM "choco install opencppcoverage" suddenly broke on AppVeyor, so let's 
REM manually grab the installer and install install

REM choco install opencppcoverage
wget 'https://github.com/OpenCppCoverage/OpenCppCoverage/releases/download/release-0.9.8.0/OpenCppCoverageSetup-x64-0.9.6.1.exe' -OutFile coverage_setup.exe
coverage_setup.exe /VERYSILENT
set path=C:\Program Files\OpenCppCoverage;%PATH%

echo "Installing CodeCov"
choco install codecov
set path=C:\ProgramData\chocolatey\lib\codecov\tools;%PATH%

