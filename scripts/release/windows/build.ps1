# Build và đóng gói bộ cài Windows x64 của CrossDesk Postcard trong thư mục build riêng.
# Script này được build_windows.sh chép sang máy Windows rồi chạy qua SSH.
# Tương thích Windows PowerShell 5.1.
param(
  [Parameter(Mandatory = $true)] [string]$Root,
  [Parameter(Mandatory = $true)] [string]$RepoUrl,
  [Parameter(Mandatory = $true)] [string]$Commit,
  [Parameter(Mandatory = $true)] [string]$Version,
  [Parameter(Mandatory = $true)] [string]$PublicKey
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
Set-StrictMode -Version Latest

$ForkUrl = "https://github.com/thuongtin/crossdesk"
$CudaPackage = "cuda 12.6.3"
$InetcUrl = "https://github.com/DigitalMediaServer/NSIS-INetC-plugin/releases/download/v1.0.5.7/InetC.zip"
$InetcSha256 = "b01077e56ebb19c005b45d40f837958ca6a92f51a5a937dc1bb497c7c7f2aa93"

function Write-Step {
  param([string]$Message)
  Write-Host "[windows $(Get-Date -Format HH:mm:ss)] $Message"
}

# Chạy lệnh native và dừng khi mã thoát khác 0. Hạ ErrorActionPreference trong
# phạm vi hàm để PowerShell 5.1 không coi dòng stderr của git là lỗi.
function Invoke-Native {
  param([string]$File, [string[]]$Arguments)
  $ErrorActionPreference = "Continue"
  & $File @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw "$File $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
  }
}

function Test-Native {
  param([string]$File, [string[]]$Arguments)
  $ErrorActionPreference = "Continue"
  & $File @Arguments 2>&1 | Out-Null
  return ($LASTEXITCODE -eq 0)
}

if ($RepoUrl -ne $ForkUrl) {
  throw "Refusing to build from $RepoUrl; only $ForkUrl is allowed"
}
if ($Version -notmatch '^v[0-9]+\.[0-9]+\.[0-9]+-[0-9]+-[0-9A-Za-z]+$') {
  throw "Invalid version: $Version"
}
if ($PublicKey -notmatch '^[0-9a-f]{64}$') {
  throw "Invalid update public key"
}

$src = Join-Path $Root "src"
$tools = Join-Path $Root "tools"
$out = Join-Path $Root "out\$Version"
New-Item -ItemType Directory -Force -Path $tools | Out-Null

# Bản clone riêng: chỉ clone từ fork, checkout đúng commit rồi xoá sạch file thừa.
Write-Step "Preparing $src at $Commit"
if (!(Test-Path (Join-Path $src ".git"))) {
  Invoke-Native git @("clone", "--no-checkout", $RepoUrl, $src)
}
$origin = (& git -C $src remote get-url origin).Trim()
if ($origin -ne $RepoUrl -and $origin -ne "$RepoUrl.git") {
  throw "Build clone $src points at $origin, expected $RepoUrl"
}
Invoke-Native git @("-C", $src, "fetch", "--force", "--prune", "origin", "+refs/heads/*:refs/remotes/origin/*")
if (!(Test-Native git @("-C", $src, "cat-file", "-e", "$Commit^{commit}"))) {
  Invoke-Native git @("-C", $src, "fetch", "origin", $Commit)
}
Invoke-Native git @("-C", $src, "checkout", "--force", "--detach", $Commit)
Invoke-Native git @("-C", $src, "submodule", "sync", "--recursive")
Invoke-Native git @("-C", $src, "submodule", "update", "--init", "--recursive", "--force")
Invoke-Native git @("-C", $src, "clean", "-ffdxq")
Invoke-Native git @("-C", $src, "submodule", "foreach", "--quiet", "--recursive", "git clean -ffdxq")
$head = (& git -C $src rev-parse HEAD).Trim()
if ($head -ne $Commit) {
  throw "Checkout of $Commit failed, HEAD is $head"
}
$status = (& git -C $src status --porcelain --ignore-submodules=none | Out-String).Trim()
if ($status -ne "") {
  throw "Build clone is not clean after checkout: $status"
}

# Plugin NSIS nằm trong thư mục tools thay vì Program Files, nạp qua NSISCONFDIR
# nên không cần quyền admin và không sửa bản NSIS đang cài trên máy.
Write-Step "Preparing NSIS plugins"
$pluginDir = Join-Path $tools "nsis-plugins\x86-unicode"
New-Item -ItemType Directory -Force -Path $pluginDir | Out-Null
$inetcDll = Join-Path $pluginDir "INetC.dll"
if (!(Test-Path $inetcDll)) {
  $zip = Join-Path $tools "InetC-1.0.5.7.zip"
  Invoke-WebRequest -UseBasicParsing -Uri $InetcUrl -OutFile $zip
  $actual = (Get-FileHash -Algorithm SHA256 -Path $zip).Hash.ToLowerInvariant()
  if ($actual -ne $InetcSha256) {
    Remove-Item -Force $zip
    throw "INetC download checksum mismatch: $actual"
  }
  $extractDir = Join-Path $tools "InetC"
  Expand-Archive -Path $zip -DestinationPath $extractDir -Force
  Copy-Item (Join-Path $extractDir "x86-unicode\INetC.dll") $inetcDll -Force
}
Copy-Item (Join-Path $src "apps\desktop\scripts\windows\nsProcess.dll") $pluginDir -Force
$nsisConfDir = Join-Path $tools "nsisconf"
New-Item -ItemType Directory -Force -Path $nsisConfDir | Out-Null
Set-Content -Path (Join-Path $nsisConfDir "nsisconf.nsh") -Encoding ASCII `
  -Value "!addplugindir /x86-unicode `"$pluginDir`""
$env:NSISCONFDIR = $nsisConfDir

# CUDA lấy từ gói xmake giống CI upstream; dự án probe chỉ dùng để hỏi installdir.
Write-Step "Resolving $CudaPackage"
$env:XMAKE_THEME = "plain"
$probe = Join-Path $tools "cuda-probe"
New-Item -ItemType Directory -Force -Path $probe | Out-Null
Set-Content -Path (Join-Path $probe "xmake.lua") -Encoding ASCII -Value @"
add_requires("$CudaPackage")
target("cuda_probe")
    set_kind("phony")
    add_packages("cuda")
"@
Push-Location $probe
try {
  Invoke-Native xmake @("require", "-y", $CudaPackage)
  $ErrorActionPreference = "Continue"
  $info = & xmake require --info $CudaPackage 2>&1 | Out-String
  $ErrorActionPreference = "Stop"
} finally {
  Pop-Location
}
$match = [regex]::Match($info, 'installdir:\s*(\S.*)')
if (!$match.Success) {
  throw "Cannot resolve the CUDA install directory from xmake"
}
$env:CUDA_PATH = $match.Groups[1].Value.Trim()
Remove-Item Env:XMAKE_THEME
Write-Step "CUDA_PATH=$($env:CUDA_PATH)"

# Phiên bản nhúng trong app không có tiền tố v, giống bản fork đã phát hành;
# tên bộ cài vẫn giữ tiền tố v theo tag.
$AppVersion = $Version.Substring(1)
Write-Step "Building CrossDesk $AppVersion for Windows x64"
$env:CROSSDESK_UPDATE_PUBLIC_KEY = $PublicKey
Push-Location $src
try {
  Invoke-Native xmake @("f", "-y", "-p", "windows", "-a", "x64", "-m", "release",
    "--CROSSDESK_VERSION=$AppVersion", "--CROSSDESK_UPDATE_PUBLIC_KEY=$PublicKey", "--USE_CUDA=true")
  Invoke-Native xmake @("b", "-vy", "crossdesk")
} finally {
  Pop-Location
}

Write-Step "Packaging installer"
Invoke-Native powershell @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File",
  (Join-Path $src "apps\desktop\scripts\windows\pkg_x64.ps1"), $Version)

$installerName = "crossdesk-win-x64-$Version.exe"
$installer = Join-Path $src "apps\desktop\scripts\windows\$installerName"
if (!(Test-Path $installer)) {
  throw "Missing installer: $installer"
}

# Thư mục out chứa bộ cài và bản sao các file thực thi gốc để chạy fork-conformance gate.
if (Test-Path $out) {
  Remove-Item -Recurse -Force $out
}
$gate = Join-Path $out "gate"
New-Item -ItemType Directory -Force -Path $gate | Out-Null
Copy-Item $installer $out
$buildDir = Join-Path $src "build\windows\x64\release"
Get-ChildItem -Path $buildDir -File | Where-Object { $_.Extension -in ".exe", ".dll" } |
  ForEach-Object { Copy-Item $_.FullName $gate }
if (!(Test-Path (Join-Path $gate "crossdesk.exe"))) {
  throw "Missing crossdesk.exe in $buildDir"
}

$signature = Get-AuthenticodeSignature (Join-Path $out $installerName)
Set-Content -Path (Join-Path $out "windows-signing.txt") -Encoding ASCII `
  -Value "authenticode=$($signature.Status)"
$hash = (Get-FileHash -Algorithm SHA256 -Path (Join-Path $out $installerName)).Hash.ToLowerInvariant()
Write-Step "Installer ready: $installerName sha256=$hash authenticode=$($signature.Status)"
