param([string]$Editor = 'C:\Program Files\Unity\Hub\Editor\6000.6.4f1\Editor\Unity.exe')
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Editor)) { throw 'No se encontró Unity. Indica su ubicación con -Editor.' }
$taskUnityProject = Join-Path $PSScriptRoot 'unity/SalinasRecorrido'
$taskUnityLog = Join-Path $PSScriptRoot 'build/unity-preparacion.log'
New-Item -ItemType Directory -Force -Path (Join-Path $PSScriptRoot 'build') | Out-Null
$taskUnityArguments = @('-batchmode', '-nographics', '-quit', '-projectPath', ('"' + $taskUnityProject + '"'),
    '-executeMethod', 'PrepararRecorrido.CrearEscena', '-logFile', ('"' + $taskUnityLog + '"'))
$taskUnityProcess = Start-Process -FilePath $Editor -ArgumentList $taskUnityArguments -WindowStyle Hidden -PassThru
# Esperar solo al editor: sus servicios auxiliares pueden seguir abiertos al salir.
$taskUnityProcess.WaitForExit()
$taskUnityProcess.Refresh()
if ($taskUnityProcess.ExitCode -ne 0) {
    if ((Test-Path -LiteralPath $taskUnityLog) -and (Select-String -LiteralPath $taskUnityLog -SimpleMatch 'No valid Unity Editor license found' -Quiet)) {
        throw 'Unity necesita una licencia activa. Abre Unity Hub, inicia sesión y revisa Settings > Licenses. Después vuelve a ejecutar este script.'
    }
    throw "No se pudo preparar Unity (código $($taskUnityProcess.ExitCode)). Revisa build/unity-preparacion.log."
}
if (-not (Select-String -LiteralPath $taskUnityLog -SimpleMatch 'SALINAS_OK:' -Quiet)) {
    throw 'Unity terminó sin confirmar la creación y validación de la escena. Revisa build/unity-preparacion.log.'
}
Write-Host 'Escena inicial preparada y muestra JSON validada. Abre Unity Hub y añade unity/SalinasRecorrido.'
