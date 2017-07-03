# from https://github.com/FeodorFitsner/selenium-tests/blob/master/take-screenshot.ps1 Thanks!

[Reflection.Assembly]::LoadWithPartialName("System.Drawing")
function screenshot([Drawing.Rectangle]$bounds, $path) {
    $bmp = New-Object Drawing.Bitmap $bounds.width, $bounds.height
    $graphics = [Drawing.Graphics]::FromImage($bmp)

    $graphics.CopyFromScreen($bounds.Location, [Drawing.Point]::Empty, $bounds.size)

    $bmp.Save($path)

    $graphics.Dispose()
    $bmp.Dispose()
}

#NUC bounds
$bounds = [Drawing.Rectangle]::FromLTRB(0, 0, 1280, 800)

#remote display bounds
#$bounds = [Drawing.Rectangle]::FromLTRB(0, 0, 1280, 800)

$PC_name=$(Get-WmiObject Win32_Computersystem).name

$dateandtime = Get-Date -Format yyyy-MM-dd-hh-mm-ss

$path_pcname= "$env:appveyor_build_folder\" + $PC_name + "_screenshot_" + "$dateandtime"+ ".png"

screenshot $bounds $path_pcname
