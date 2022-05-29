$PSDefaultParameterValues['Stop-Process:ErrorAction'] = 'SilentlyContinue'
$PSDefaultParameterValues['*:Encoding'] = 'utf8'

$tsl_check = [Net.ServicePointManager]::SecurityProtocol
if (!($tsl_check -match '^tls12$' )) {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
}

Write-Host "*****************"
Write-Host "Developer: " -NoNewline
Write-Host "@R3nzTheCodeGOD" -ForegroundColor DarkYellow
Write-Host "*****************"`n

$SpotifyDirectory = "$env:APPDATA\Spotify"
$SpotifyExecutable = "$SpotifyDirectory\Spotify.exe"
$Podcasts_off = $false

Stop-Process -Name Spotify
Stop-Process -Name SpotifyWebHelper

if ($PSVersionTable.PSVersion.Major -ge 7) {
    Import-Module Appx -UseWindowsPowerShell
}

$win_os = (get-itemproperty -Path "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion" -Name ProductName).ProductName
$win11 = $win_os -match "\windows 11\b"
$win10 = $win_os -match "\windows 10\b"
$win8_1 = $win_os -match "\windows 8.1\b"
$win8 = $win_os -match "\windows 8\b"

if ($win11 -or $win10 -or $win8_1 -or $win8) {
    if (Get-AppxPackage -Name SpotifyAB.SpotifyMusic) {
        Write-Host "Unsupported Microsoft Store version of Spotify detected."`n
        $ch = Read-Host -Prompt "Do you want to uninstall Spotify Windows Store version? (Y/N)"
        if ($ch -eq 'y') {
            Write-Host "Spotify Uninstalling..."`n
            Get-AppxPackage -Name SpotifyAB.SpotifyMusic | Remove-AppxPackage
        } else {
            Write-Host 'Exiting...'`n
            Pause 
            exit
        }
    }
}

Push-Location -LiteralPath $env:TEMP
try {
    New-Item -Type Directory -Name "SpotifyInternal-$(Get-Date -UFormat '%Y-%m-%d_%H-%M-%S')" `
    | Convert-Path `
    | Set-Location
} catch {
    Write-Output ''
    Pause
    exit
}

Write-Host "Downloading the latest version of SpotifyInternal..."`n

$webClient = New-Object -TypeName System.Net.WebClient
try {
    $webClient.DownloadFile('https://github.com/R3nzTheCodeGOD/Spotify-Internal/releases/latest/download/SpotifyInternal.zip', "$PWD\SpotifyInternal.zip")
} catch {
    Write-Output ''
    Start-Sleep
}

Expand-Archive -Force -LiteralPath "$PWD\SpotifyInternal.zip" -DestinationPath $PWD
Remove-Item -LiteralPath "$PWD\SpotifyInternal.zip"

$spotifyInstalled = (Test-Path -LiteralPath $SpotifyExecutable)
if (-not $spotifyInstalled) {
    
    try {
        $webClient.DownloadFile('https://download.scdn.co/SpotifySetup.exe', "$PWD\SpotifySetup.exe")
    } catch {
        Write-Output ''
        Pause
        exit
    }
    mkdir $SpotifyDirectory | Out-Null

    $version_client_check = (get-item $PWD\SpotifySetup.exe).VersionInfo.ProductVersion
    $version_client = $version_client_check -split '.\w\w\w\w\w\w\w\w\w'
   
    Write-Host "Downloading and installing Spotify" -NoNewline
    Write-Host  $version_client -ForegroundColor Green
    Write-Host "Please wait..."`n
    
    Start-Process -FilePath $PWD\SpotifySetup.exe; wait-process -name SpotifySetup

    Stop-Process -Name Spotify
    Stop-Process -Name SpotifyWebHelper
    Stop-Process -Name SpotifyFullSetup

    $ErrorActionPreference = 'SilentlyContinue'

    if (test-path "$env:LOCALAPPDATA\Microsoft\Windows\Temporary Internet Files\") {
        get-childitem -path "$env:LOCALAPPDATA\Microsoft\Windows\Temporary Internet Files\" -Recurse -Force -Filter  "SpotifyFullSetup*" | remove-item  -Force
    }
    if (test-path $env:LOCALAPPDATA\Microsoft\Windows\INetCache\) {
        get-childitem -path "$env:LOCALAPPDATA\Microsoft\Windows\INetCache\" -Recurse -Force -Filter  "SpotifyFullSetup*" | remove-item  -Force
    }
}

if (!(test-path $SpotifyDirectory/_chrome_elf.dll)) {
    Move-Item $SpotifyDirectory\chrome_elf.dll $SpotifyDirectory\_chrome_elf.dll
}

Write-Host 'Spotify Patching...'`n
$patchFiles = "$PWD\chrome_elf.dll"
Copy-Item -LiteralPath $patchFiles -Destination "$SpotifyDirectory"

$tempDirectory = $PWD
Pop-Location

Start-Sleep -Milliseconds 200
Remove-Item -Recurse -LiteralPath $tempDirectory 

do {
    $ch = Read-Host -Prompt "Do you want to turn off podcasts? (Y/N)"
    Write-Host ""
    if (!($ch -eq 'n' -or $ch -eq 'y')) {
    
        Write-Host "Error, wrong value," -ForegroundColor Red -NoNewline
        Write-Host "again (Y/N)" -NoNewline
        Start-Sleep -Milliseconds 1000
        Write-Host "3" -NoNewline
        Start-Sleep -Milliseconds 1000
        Write-Host ".2" -NoNewline
        Start-Sleep -Milliseconds 1000
        Write-Host ".1"
        Start-Sleep -Milliseconds 1000     
        Clear-Host
    }
}
while ($ch -notmatch '^y$|^n$')
if ($ch -eq 'y') { $Podcasts_off = $true }

$xpui_spa_patch = "$env:APPDATA\Spotify\Apps\xpui.spa"
$xpui_js_patch = "$env:APPDATA\Spotify\Apps\xpui\xpui.js"

If (Test-Path $xpui_js_patch) {
    Write-Host "Files to be patched found"`n 

    $xpui_js = Get-Content $xpui_js_patch -Raw
    
    If (!($xpui_js -match 'Patched by R3nzTheCodeGOD')) {
        Copy-Item $xpui_js_patch "$xpui_js_patch.bak"
        
        $new_js = $xpui_js `
            -replace 'adsEnabled:!0', 'adsEnabled:!1' `
            -replace "allSponsorships", "" `
            -replace '(return|.=.=>)"free"===(.+?)(return|.=.=>)"premium"===', '$1"premium"===$2$3"free"===' `
            -replace '(Show "Made For You" entry point in the left sidebar.,default:)(!1)', '$1!0' `
            -replace '(Enable the new Search with chips experience",default:)(!1)', '$1!0' `
            -replace '(Enable Liked Songs section on Artist page",default:)(!1)', '$1!0' `
            -replace '(Enable block users feature in clientX",default:)(!1)', '$1!0' `
            -replace '(Enables quicksilver in-app messaging modal",default:)(!0)', '$1!1' `
            -replace '(With this enabled, clients will check whether tracks have lyrics available",default:)(!1)', '$1!0' `
            -replace '(Enables new playlist creation flow in Web Player and DesktopX",default:)(!1)', '$1!0' `
            -replace '(Enable Enhance Playlist UI and functionality for end-users",default:)(!1)', '$1!0' `
            -replace '(Enable a condensed disography shelf on artist pages",default:)(!1)', '$1!0' `
            -replace '(Enable the new fullscreen lyrics page",default:)(!1)', '$1!0' `
            -replace '(Enable Playlist Permissions flows for Prod",default:)(!1)', '$1!0' `
            -replace '(Enable Enhance Liked Songs UI and functionality",default:)(!1)', '$1!0'
        if ($Podcasts_off) {
            $new_js = $new_js `
                -replace '(return this\.queryParameters=(.),)', '$2.types=["album","playlist","artist","station"];$1' `
                -replace ',this[.]enableShows=[a-z]', ""
        }

        Set-Content -Path $xpui_js_patch -Force -Value $new_js
        add-content -Path $xpui_js_patch -Value '// Patched by R3nzTheCodeGOD' -passthru | Out-Null
        $contentjs = [System.IO.File]::ReadAllText($xpui_js_patch)
        $contentjs = $contentjs.Trim()
        [System.IO.File]::WriteAllText($xpui_js_patch, $contentjs)
    } else {
        Write-Host "Spotify is already patched"`n 
    }
}

If (Test-Path $xpui_spa_patch) {
    Add-Type -Assembly 'System.IO.Compression.FileSystem'

    $zip = [System.IO.Compression.ZipFile]::Open($xpui_spa_patch, 'update')
    $entry = $zip.GetEntry('xpui.js')
    $reader = New-Object System.IO.StreamReader($entry.Open())
    $patched_by_spotx = $reader.ReadToEnd()
    $reader.Close()
 

    If (!($patched_by_spotx -match 'Patched by R3nzTheCodeGOD')) {
        $zip.Dispose()
        Copy-Item $xpui_spa_patch $env:APPDATA\Spotify\Apps\xpui.bak 

        Add-Type -Assembly 'System.IO.Compression.FileSystem'
        $zip = [System.IO.Compression.ZipFile]::Open($xpui_spa_patch, 'update')
    
        $entry_xpui = $zip.GetEntry('xpui.js')

        $reader = New-Object System.IO.StreamReader($entry_xpui.Open())
        $xpuiContents = $reader.ReadToEnd()
        $reader.Close()

        $xpuiContents = $xpuiContents `
            -replace 'adsEnabled:!0', 'adsEnabled:!1' `
            -replace "allSponsorships", "" `
            -replace '(return|.=.=>)"free"===(.+?)(return|.=.=>)"premium"===', '$1"premium"===$2$3"free"===' `
            -replace '(Show "Made For You" entry point in the left sidebar.,default:)(!1)', '$1!0' `
            -replace '(Enable the new Search with chips experience",default:)(!1)', '$1!0' `
            -replace '(Enable Liked Songs section on Artist page",default:)(!1)', '$1!0' `
            -replace '(Enable block users feature in clientX",default:)(!1)', '$1!0' `
            -replace '(Enables quicksilver in-app messaging modal",default:)(!0)', '$1!1' `
            -replace '(With this enabled, clients will check whether tracks have lyrics available",default:)(!1)', '$1!0' `
            -replace '(Enables new playlist creation flow in Web Player and DesktopX",default:)(!1)', '$1!0' `
            -replace '(Enable Enhance Playlist UI and functionality for end-users",default:)(!1)', '$1!0' `
            -replace '(Enable a condensed disography shelf on artist pages",default:)(!1)', '$1!0' `
            -replace '(Enable the new fullscreen lyrics page",default:)(!1)', '$1!0' `
            -replace '(Enable Playlist Permissions flows for Prod",default:)(!1)', '$1!0' `
            -replace '(Enable Enhance Liked Songs UI and functionality",default:)(!1)', '$1!0'
        if ($Podcasts_off) {
            $xpuiContents = $xpuiContents `
                -replace '(return this\.queryParameters=(.),)', '$2.types=["album","playlist","artist","station"];$1' -replace ',this[.]enableShows=[a-z]', ""
        }

        $writer = New-Object System.IO.StreamWriter($entry_xpui.Open())
        $writer.BaseStream.SetLength(0)
        $writer.Write($xpuiContents)
        $writer.Write([System.Environment]::NewLine + '// Patched by R3nzTheCodeGOD')
        $writer.Close()

        $entry_vendor_xpui = $zip.GetEntry('vendor~xpui.js')

        $reader = New-Object System.IO.StreamReader($entry_vendor_xpui.Open())
        $xpuiContents_vendor = $reader.ReadToEnd()
        $reader.Close()

        $xpuiContents_vendor = $xpuiContents_vendor `
            -replace "prototype\.bindClient=function\(\w+\)\{", '${0}return;'

        $writer = New-Object System.IO.StreamWriter($entry_vendor_xpui.Open())
        $writer.BaseStream.SetLength(0)
        $writer.Write($xpuiContents_vendor)
        $writer.Close()

        $zip.Entries | Where-Object FullName -like '*.css' | ForEach-Object {
            $readercss = New-Object System.IO.StreamReader($_.Open())
            $xpuiContents_css = $readercss.ReadToEnd()
            $readercss.Close()

            $xpuiContents_css = $xpuiContents_css `
                -replace "}\[dir=ltr\]\s?([.a-zA-Z\d[_]+?,\[dir=ltr\])", '}[dir=str] $1' `
                -replace "}\[dir=ltr\]\s?", "} " `
                -replace "html\[dir=ltr\]", "html" `
                -replace ",\s?\[dir=rtl\].+?(\{.+?\})", '$1' `
                -replace "[\w\-\.]+\[dir=rtl\].+?\{.+?\}", "" `
                -replace "\}\[lang=ar\].+?\{.+?\}", "}" `
                -replace "\}\[dir=rtl\].+?\{.+?\}", "}" `
                -replace "\}html\[dir=rtl\].+?\{.+?\}", "}" `
                -replace "\}html\[lang=ar\].+?\{.+?\}", "}" `
                -replace "\[lang=ar\].+?\{.+?\}", "" `
                -replace "html\[dir=rtl\].+?\{.+?\}", "" `
                -replace "html\[lang=ar\].+?\{.+?\}", "" `
                -replace "\[dir=rtl\].+?\{.+?\}", "" `
                -replace "\[dir=str\]", "[dir=ltr]" `
                -replace "[/]\*([^*]|[\r\n]|(\*([^/]|[\r\n])))*\*[/]", "" `
                -replace "[/][/]#\s.*", "" `
                -replace "\r?\n(?!\(1|\d)", ""
            
            $writer = New-Object System.IO.StreamWriter($_.Open())
            $writer.BaseStream.SetLength(0)
            $writer.Write($xpuiContents_css)
            $writer.Close()
        }
        $zip.Dispose()
    } else {
        $zip.Dispose()
        Write-Host "Spotify is already patched"`n
    }
}

$ErrorActionPreference = 'SilentlyContinue' 

if (Test-Path "$env:USERPROFILE\Desktop") {  
    $desktop_folder = "$env:USERPROFILE\Desktop"
}

$regedit_desktop_folder = Get-ItemProperty -Path "Registry::HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders\"
$regedit_desktop = $regedit_desktop_folder.'{754AC886-DF64-4CBA-86B5-F7FBF4FBCEF5}'
 
if (!(Test-Path "$env:USERPROFILE\Desktop")) {
    $desktop_folder = $regedit_desktop
}

$ErrorActionPreference = 'SilentlyContinue' 

If (!(Test-Path $env:USERPROFILE\Desktop\Spotify.lnk)) {
    $source = "$env:APPDATA\Spotify\Spotify.exe"
    $target = "$desktop_folder\Spotify.lnk"
    $WorkingDir = "$env:APPDATA\Spotify"
    $WshShell = New-Object -comObject WScript.Shell
    $Shortcut = $WshShell.CreateShortcut($target)
    $Shortcut.WorkingDirectory = $WorkingDir
    $Shortcut.TargetPath = $source
    $Shortcut.Save()      
}

Write-Host "Installation Complete"`n -ForegroundColor Green
exit
