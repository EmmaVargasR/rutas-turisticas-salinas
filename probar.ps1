$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
$previousPath = $env:PATH
try {
    $compilerCommand = Get-Command g++ -ErrorAction SilentlyContinue
    $compilerPath = if ($compilerCommand) { $compilerCommand.Source } else { 'C:\Program Files\CodeBlocks\MinGW\bin\g++.exe' }
    if (-not (Test-Path -LiteralPath $compilerPath)) { throw 'No se encontro un compilador C++17.' }
    $env:PATH = (Split-Path -Parent $compilerPath) + [IO.Path]::PathSeparator + $env:PATH
    New-Item -ItemType Directory -Force -Path build | Out-Null
    foreach ($suite in @('estructuras', 'grafo', 'rutas', 'persistencia')) {
        $sourcePath = Join-Path 'tests' ($suite + '.cpp')
        $executablePath = Join-Path 'build' ('pruebas-' + $suite + '.exe')
        & $compilerPath '-std=c++17' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-O0' '-static' '-Ibackend' $sourcePath '-o' $executablePath
        if ($LASTEXITCODE -ne 0) { throw "No se pudieron compilar las pruebas de $suite." }
        & (Join-Path $PSScriptRoot $executablePath)
        if ($LASTEXITCODE -ne 0) { throw "Las pruebas de $suite detectaron un error." }
    }
} finally {
    $env:PATH = $previousPath
    Pop-Location
}


