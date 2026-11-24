param([string]$Salida = 'build/salinas.exe')
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $compilerCommand = Get-Command g++ -ErrorAction SilentlyContinue
    $compilerPath = if ($compilerCommand) { $compilerCommand.Source } else { 'C:\Program Files\CodeBlocks\MinGW\bin\g++.exe' }
    if (-not (Test-Path -LiteralPath $compilerPath)) { throw 'No se encontro g++. Instala un compilador compatible con C++17.' }
    $env:PATH = (Split-Path -Parent $compilerPath) + [IO.Path]::PathSeparator + $env:PATH
    New-Item -ItemType Directory -Force -Path build | Out-Null
    & $compilerPath '-std=c++17' '-D_WIN32_WINNT=0x0A00' '-Wall' '-Wextra' '-O0' '-g' '-static' '-Ithird_party' 'backend/main.cpp' '-o' $Salida '-lws2_32' '-lcrypt32'
    if ($LASTEXITCODE -ne 0) { throw 'La compilacion fallo.' }
    Write-Host 'Compilacion correcta. Ejecuta .\iniciar.ps1'
} finally { Pop-Location }



