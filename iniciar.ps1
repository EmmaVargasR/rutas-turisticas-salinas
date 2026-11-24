param([ValidateRange(1,65535)][int]$Puerto = 8080, [string]$Archivo = 'data/mapa.txt')
$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $executable = Join-Path $PSScriptRoot 'build/salinas.exe'
    $needsBuild = -not (Test-Path -LiteralPath $executable)
    if (-not $needsBuild) {
        $buildTime = (Get-Item -LiteralPath $executable).LastWriteTimeUtc
        $needsBuild = @(Get-ChildItem -Path backend,third_party -File -Recurse | Where-Object { $_.LastWriteTimeUtc -gt $buildTime }).Count -gt 0
    }
    if ($needsBuild) { & .\compilar.ps1 }
    # Una copia descargada de GitHub comienza con el circuito aproximado acordado.
    # Nunca reemplazar un mapa que el usuario ya haya modificado.
    if ($Archivo -eq 'data/mapa.txt' -and -not (Test-Path -LiteralPath $Archivo)) {
        $initialMap = Join-Path $PSScriptRoot 'data/mapa-inicial.txt'
        if (Test-Path -LiteralPath $initialMap) { Copy-Item -LiteralPath $initialMap -Destination $Archivo }
    }
    & $executable $Puerto $Archivo
    if ($LASTEXITCODE -ne 0) { throw 'El servidor no pudo iniciarse o termino con un error.' }
} finally { Pop-Location }

