Add-Type -AssemblyName System.Windows.Forms

# Older PS versions have a bug with non-ShowHelp dialog
$showHelp = ($PSVersionTable.PSVersion.Major -le 2)

[System.Windows.Forms.MessageBox]::Show(
    'This utility allows you to automatically replace some of the outdated INI entries ' +
    'made using an older version of Phobos when upgrading to a newer version and also ' +
    'apply different kinds of processing "scripts" (while also backing them up) to your ' +
    'INI files. Internally it is simply a wrapper around GNU sed utility.' +
    [environment]::NewLine + [environment]::NewLine +
    'Select a sed script file on the next dialog screen, then select the ' +
    'files to apply the chosen script on a dialog screen after that.',
    'Migration Utility', 'OK',
    [System.Windows.Forms.MessageBoxIcon]::None)


$scriptDialog = New-Object Windows.Forms.OpenFileDialog -Property @{
        InitialDirectory = Get-Location;
        ShowHelp = $showHelp;
        Multiselect = $false;
        Title = "Select a sed script file...";
        Filter = "Stream Editor Script Files (*.sed)|*.sed|All Files (*.*)|*.*";
    }

[void]$scriptDialog.ShowDialog()

Write-Output $scriptDialog.FileName
if (!$scriptDialog.FileName)
{
    Exit
}

$iniDialog = New-Object Windows.Forms.OpenFileDialog -Property @{
        InitialDirectory = Get-Location;
        ShowHelp = $showHelp;
        Multiselect = $true;
        RestoreDirectory = $true;
        Title = "Select files to process...";
        Filter = "INI/Map Files (*.ini, *.map, *.mpr, *.yrm)|*.ini;*.map;*.mpr;*.yrm|All Files (*.*)|*.*";
    }

[void]$iniDialog.ShowDialog()

Write-Output $iniDialog.FileNames
if (!$iniDialog.FileNames)
{
    Exit
}

try
{
    $cmdOutput = (& .\sed.exe --regexp-extended --in-place=.backup -f $scriptDialog.FileName $iniDialog.FileNames 2>&1)
    if ($cmdOutput)
    {
        throw $cmdOutput
    }

    [System.Windows.Forms.MessageBox]::Show(
        'The script was applied successfully. The files were backed up. ' +
        [environment]::NewLine + [environment]::NewLine +
        'Please manually review the output files and search for any FIXME comments ' +
        'for the entries that could not be processed manually.',
        'Success', 'OK',
        [System.Windows.Forms.MessageBoxIcon]::Information)
}
catch
{
    [System.Windows.Forms.MessageBox]::Show(
        'The following errors happened during the execution:' +
        [environment]::NewLine + [environment]::NewLine +
        $_,
        'Errors detected', 'OK',
        [System.Windows.Forms.MessageBoxIcon]::Error)
}

