$PixVersion="1.0.220810001"
$DStorageVersion="1.1.0"

function ProcessPackage {
    param (
        $cmd,
        $pkgName, $pkgVer, 
        $projName
    )

    if($cmd -eq "Install") {
        $ret = Find-Package $pkgName -AllVersions
        $foundInfo = $ret.Versions | where OriginalVersion -eq $pkgVer
        if($null -eq $foundInfo) {
            Write-Error("{0} {1} not found!" -f $pkgName, $pkgVer)
            return
        }
        $fullCmd = "Install-Package {0} -Version {1} -Project {2}" -f $pkgName, $pkgVer, $projName
        Invoke-Expression $fullCmd
    }
    elseif($cmd -eq "Reset") {
        $fullCmd = "Update-Package {0} -ProjectName {1} -Reinstall" -f $pkgName, $projName
        Invoke-Expression $fullCmd
    }
}

if($args.Count -ne 1) {
    Write-Error("Only accept 1 parameter")
    return
}

ProcessPackage $args[0] WinPixEventRuntime $PixVersion BasicGeometryFrameGraph
ProcessPackage $args[0] WinPixEventRuntime $PixVersion HelloDX12
ProcessPackage $args[0] WinPixEventRuntime $PixVersion HelloFrameGraph
ProcessPackage $args[0] WinPixEventRuntime $PixVersion HelloPyQT
ProcessPackage $args[0] WinPixEventRuntime $PixVersion MSAA
ProcessPackage $args[0] WinPixEventRuntime $PixVersion OffScreenRenderingDemo
ProcessPackage $args[0] WinPixEventRuntime $PixVersion TexLoadingTest
ProcessPackage $args[0] WinPixEventRuntime $PixVersion forwardDX12

ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion BasicGeometryFrameGraph
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion HelloDX12
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion HelloFrameGraph
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion HelloPyQT
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion MSAA
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion OffScreenRenderingDemo
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion TexLoadingTest
ProcessPackage $args[0] Microsoft.Direct3D.DirectStorage $DStorageVersion forwardDX12
