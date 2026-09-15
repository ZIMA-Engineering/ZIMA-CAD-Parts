param(
    [string]$Version,
    [switch]$Custom,
    [switch]$Check
)
$ErrorActionPreference = 'Stop'
try {
    $root = $PSScriptRoot
    if (-not $Version) {
        $settings = @{}
        foreach ($line in Get-Content -LiteralPath (Join-Path $root 'launcher.ini')) {
            if ($line -match '^\s*(windows|windows_custom)\s*=\s*([^;#]+?)\s*$') {
                $settings[$matches[1]] = $matches[2]
            }
        }
        $Version = $settings['windows']
        $Custom = $settings['windows_custom'] -eq 'true'
    }
    if ($Custom) {
        if ($Version -notmatch '^[A-Za-z0-9][A-Za-z0-9._-]*$') { throw 'Invalid custom build name.' }
        $base = Join-Path $root 'custom/windows'
    } else {
        if ($Version -notmatch '^\d{10}$') { throw 'Invalid release version.' }
        $base = Join-Path $root 'windows'
    }
    $directory = Join-Path $base $Version
    $exe = Join-Path $directory 'ZIMA-CAD-Parts.exe'
    if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Build not found: $exe" }
    if (-not $Custom) {
        $manifest = Get-Content -Raw -LiteralPath (Join-Path $directory 'version.json') | ConvertFrom-Json
        if ($manifest.version -ne $Version -or $manifest.platform -ne 'windows-x64') {
            throw 'Release manifest does not match the selected build.'
        }
    }
    if ($Check) { Write-Output $exe; exit 0 }
    # A developer's Qt environment must not inject another kit into the package.
    foreach ($name in @('QT_PLUGIN_PATH','QT_QPA_PLATFORM_PLUGIN_PATH','QML_IMPORT_PATH','QML2_IMPORT_PATH','QTWEBENGINEPROCESS_PATH','QTWEBENGINE_RESOURCES_PATH','QTWEBENGINE_LOCALES_PATH')) {
        [Environment]::SetEnvironmentVariable($name, $null, 'Process')
    }
    $occtResources = @{
        CSF_ShadersDirectory = 'Shaders'; CSF_SHMessage = 'SHMessage';
        CSF_XSMessage = 'XSMessage'; CSF_STEPDefaults = 'XSTEPResource';
        CSF_IGESDefaults = 'XSTEPResource'; CSF_PluginDefaults = 'StdResource';
        CSF_StandardDefaults = 'StdResource'; CSF_XCAFDefaults = 'StdResource';
        CSF_MDTVTexturesDirectory = 'Textures'; CSF_XmlOcafResource = 'XmlOcafResource'
    }
    foreach ($name in $occtResources.Keys) {
        $path = Join-Path $directory ("occt/" + $occtResources[$name])
        if (Test-Path -LiteralPath $path -PathType Container) {
            [Environment]::SetEnvironmentVariable($name, $path, 'Process')
        }
    }
    $env:PATH = "$directory;$env:SystemRoot\System32;$env:SystemRoot"
    Start-Process -FilePath $exe -WorkingDirectory $directory | Out-Null
} catch {
    Write-Error $_
    exit 1
}
