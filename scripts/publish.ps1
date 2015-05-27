param(
    [string]$outdir = "Publish",
    [string]$bindir = "Release"
)

$binary_list = @(
    "SDL2.dll",
    "FreeImage.dll",
    "AntTweakBar.dll"
)

$asset_folder = "assets"

Write-Output ("Publishing " + $bindir + " build into " + $outdir)

# Copy binary files to output directory
foreach($binary in $binary_list)
{
    Copy-Item -Path $binary -Destination (Join-Path $outdir $binary) -Recurse
}

Copy-Item -Path (Join-Path $bindir "Pathtracer.exe") -Destination (Join-Path $outdir "Pathtracer.exe") -Recurse

# Copy the asset directory to output
Copy-Item -Path $asset_folder -Destination (Join-Path $outdir $asset_folder) -Recurse