param([string]$Bfc = "$PSScriptRoot\..\build\Debug\bfc.exe")

$ErrorActionPreference = "Stop"

if (-not (Get-Command ml64 -ErrorAction SilentlyContinue)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $root = & $vswhere -latest -property installationPath
    cmd /c "`"$root\VC\Auxiliary\Build\vcvars64.bat`" >nul && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') { Set-Item "env:$($Matches[1])" $Matches[2] }
    }
}

$work = Join-Path ([IO.Path]::GetTempPath()) "bfc_e2e"
New-Item -ItemType Directory -Force $work | Out-Null
$failures = 0

function Test-Program($name, $source, $stdin, $expectedOut, $expectedExit, $flags) {
    $bf = Join-Path $work "$name.bf"
    $exe = Join-Path $work "$name.exe"
    [IO.File]::WriteAllText($bf, $source)
    & $Bfc $bf -o $exe @flags | Out-Null
    if ($LASTEXITCODE -ne 0) { Write-Host "FAIL $name (compile)"; $script:failures++; return }
    $actual = ($stdin | & $exe) -join "`n"
    if ($actual -ne $expectedOut -or $LASTEXITCODE -ne $expectedExit) {
        Write-Host "FAIL $name (out='$actual' exit=$LASTEXITCODE)"; $script:failures++
    } else { Write-Host "ok   $name" }
}

$hello = Get-Content -Raw "$PSScriptRoot\..\examples\hello.bf"
foreach ($mode in @(@("opt", @()), @("noopt", @("--no-opt")))) {
    $n, $f = $mode
    Test-Program "hello_$n" $hello "" "Hello World!" 0 $f
    Test-Program "mul_$n" "++++++++[>++++++++<-]>+." "" "A" 0 $f
    Test-Program "cat_$n" ",[.,]" "xyz" "xyz" 0 $f
    Test-Program "oob_$n" "<" "" "" 1 $f
}

exit $failures
