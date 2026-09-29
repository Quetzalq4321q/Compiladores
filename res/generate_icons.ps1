Add-Type -AssemblyName System.Drawing

$outputDir = $PSScriptRoot
if (-not $outputDir) { $outputDir = "." }

$pngPath = Join-Path $outputDir "compiler.png"
$icoPath = Join-Path $outputDir "compiler.ico"

function Draw-CompilerIcon([int]$size) {
    $bmp = New-Object System.Drawing.Bitmap $size, $size
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic

    $scale = $size / 256.0

    # 1. Fondo Squircle
    $bgColor = [System.Drawing.Color]::FromArgb(15, 23, 42)
    $borderColor = [System.Drawing.Color]::FromArgb(56, 189, 248)
    
    $radius = [int](44 * $scale)
    $diameter = $radius * 2
    $margin = [int](12 * $scale)
    $rectW = $size - ($margin * 2)
    $rectH = $size - ($margin * 2)

    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $path.AddArc($margin, $margin, $diameter, $diameter, 180, 90)
    $path.AddArc($margin + $rectW - $diameter, $margin, $diameter, $diameter, 270, 90)
    $path.AddArc($margin + $rectW - $diameter, $margin + $rectH - $diameter, $diameter, $diameter, 0, 90)
    $path.AddArc($margin, $margin + $rectH - $diameter, $diameter, $diameter, 90, 90)
    $path.CloseFigure()

    $brushBg = New-Object System.Drawing.SolidBrush($bgColor)
    $g.FillPath($brushBg, $path)

    $penBorder = New-Object System.Drawing.Pen($borderColor, [Math]::Max(1.0, 3.5 * $scale))
    $g.DrawPath($penBorder, $path)

    # 2. Engranaje central sutil
    $gearPen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(70, 56, 189, 248), [Math]::Max(1.0, 2.0 * $scale))
    $gearBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(35, 14, 165, 233))
    $centerX = $size / 2.0
    $centerY = $size / 2.0
    $gearR = 48.0 * $scale
    $g.FillEllipse($gearBrush, ($centerX - $gearR), ($centerY - $gearR), ($gearR * 2), ($gearR * 2))
    $g.DrawEllipse($gearPen, ($centerX - $gearR), ($centerY - $gearR), ($gearR * 2), ($gearR * 2))

    $centerHoleR = 24.0 * $scale
    $g.FillEllipse($brushBg, ($centerX - $centerHoleR), ($centerY - $centerHoleR), ($centerHoleR * 2), ($centerHoleR * 2))
    $g.DrawEllipse($gearPen, ($centerX - $centerHoleR), ($centerY - $centerHoleR), ($centerHoleR * 2), ($centerHoleR * 2))

    # 3. Brackets de Codigo y Slash < / >
    # Bracket Izquierdo < (Verde)
    $penLeft = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(16, 185, 129), [Math]::Max(2.0, 14.0 * $scale))
    $penLeft.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $penLeft.EndCap   = [System.Drawing.Drawing2D.LineCap]::Round
    $penLeft.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round

    $pL1 = New-Object System.Drawing.PointF((86.0 * $scale), (94.0 * $scale))
    $pL2 = New-Object System.Drawing.PointF((46.0 * $scale), (128.0 * $scale))
    $pL3 = New-Object System.Drawing.PointF((86.0 * $scale), (162.0 * $scale))
    $g.DrawLines($penLeft, @($pL1, $pL2, $pL3))

    # Barra Central / (Cian)
    $penSlash = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(56, 189, 248), [Math]::Max(2.0, 13.0 * $scale))
    $penSlash.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $penSlash.EndCap   = [System.Drawing.Drawing2D.LineCap]::Round
    $g.DrawLine($penSlash, (142.0 * $scale), (84.0 * $scale), (114.0 * $scale), (172.0 * $scale))

    # Bracket Derecho > (Ambar / Dorado)
    $penRight = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(245, 158, 11), [Math]::Max(2.0, 14.0 * $scale))
    $penRight.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $penRight.EndCap   = [System.Drawing.Drawing2D.LineCap]::Round
    $penRight.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Round

    $pR1 = New-Object System.Drawing.PointF((170.0 * $scale), (94.0 * $scale))
    $pR2 = New-Object System.Drawing.PointF((210.0 * $scale), (128.0 * $scale))
    $pR3 = New-Object System.Drawing.PointF((170.0 * $scale), (162.0 * $scale))
    $g.DrawLines($penRight, @($pR1, $pR2, $pR3))

    # 4. Texto LP en la parte inferior si la resolucion lo permite
    if ($size -ge 48) {
        $badgeBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(30, 41, 59))
        $badgePen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(56, 189, 248), [Math]::Max(1.0, 1.5 * $scale))
        $bX = 94.0 * $scale
        $bY = 196.0 * $scale
        $bW = 68.0 * $scale
        $bH = 26.0 * $scale
        $g.FillRectangle($badgeBrush, $bX, $bY, $bW, $bH)
        $g.DrawRectangle($badgePen, $bX, $bY, $bW, $bH)

        $font = New-Object System.Drawing.Font("Consolas", [Math]::Max(6.0, 12.0 * $scale), [System.Drawing.FontStyle]::Bold)
        $textBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(56, 189, 248))
        $sf = New-Object System.Drawing.StringFormat
        $sf.Alignment = [System.Drawing.StringAlignment]::Center
        $sf.LineAlignment = [System.Drawing.StringAlignment]::Center
        $rectF = New-Object System.Drawing.RectangleF($bX, $bY, $bW, $bH)
        $g.DrawString("LP", $font, $textBrush, $rectF, $sf)
    }

    $g.Dispose()
    return $bmp
}

Write-Host "Generando icono PNG a 256x256..."
$bmp256 = Draw-CompilerIcon 256
$bmp256.Save($pngPath, [System.Drawing.Imaging.ImageFormat]::Png)
Write-Host "PNG generado: $pngPath"

Write-Host "Generando icono Windows ICO a 32x32 y 48x48..."
$bmpIco = Draw-CompilerIcon 48
$hIcon = $bmpIco.GetHicon()
$icon = [System.Drawing.Icon]::FromHandle($hIcon)
$fs = [System.IO.File]::Create($icoPath)
$icon.Save($fs)
$fs.Close()
$icon.Dispose()
$bmpIco.Dispose()
$bmp256.Dispose()

Write-Host "ICO generado: $icoPath"
