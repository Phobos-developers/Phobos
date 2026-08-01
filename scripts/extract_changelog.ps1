param(
    [Parameter(Mandatory = $true)]
    [string]$Tag,

    [Parameter(Mandatory = $true)]
    [string]$WhatsNewPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath
)

$version = $Tag.TrimStart('v')

if (-not (Test-Path $WhatsNewPath)) {
    Write-Error "File not found: $WhatsNewPath"
    exit 1
}

$content = (Get-Content -Path $WhatsNewPath -Raw -Encoding utf8) -replace "`r`n", "`n"

# ---- Parse the document into a heading tree ----
$lines = $content -split "`n"
$headings = @()
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^(#{1,6})\s+(.+)$') {
        $headings += @{
            Level = $matches[1].Length
            Text  = $matches[2]
            Line  = $i
        }
    }
}

# ---- Find ## Changelog and its ### children ----
$changelogNode = $headings | Where-Object { $_.Level -eq 2 -and $_.Text -eq 'Changelog' } | Select-Object -First 1
if (-not $changelogNode) {
    Write-Error "Could not find '## Changelog' section in $WhatsNewPath"
    exit 1
}

# Next heading at level <= 2 marks the end of ## Changelog
$changelogEndNode = $headings | Where-Object { $_.Level -le 2 -and $_.Line -gt $changelogNode.Line } | Select-Object -First 1
$changelogEnd = if ($changelogEndNode) { $changelogEndNode.Line } else { $lines.Count }

# All ### headings within ## Changelog
$versionNodes = $headings | Where-Object {
    $_.Level -eq 3 -and $_.Line -gt $changelogNode.Line -and $_.Line -lt $changelogEnd
}

if ($versionNodes.Count -eq 0) {
    Write-Error "No version sections found within '## Changelog'"
    exit 1
}

# ---- Extract the version string from a ### heading ----
function Get-VersionText($headingText) {
    if ($headingText -match '^Version\s') {
        return $headingText  # "Version TBD (develop branch nightly builds)"
    }
    # Take the first space-delimited token, e.g. "0.4.0.3"
    return ($headingText -split '\s')[0]
}

# ---- Match version ----
$matchedNode = $null

# 1) Exact version match
$matchedNode = $versionNodes | Where-Object { (Get-VersionText $_.Text) -eq $version } | Select-Object -First 1

# 2) Drop the pre-release suffix (pre-releases share the section of the version they lead up to,
#    e.g. 0.5-beta1 -> 0.5), then strip trailing numeric components until found (0.5.0.1 -> 0.5.0 -> 0.5)
if (-not $matchedNode) {
    $tryVersion = $version -replace '-.*$', ''
    while ($true) {
        $matchedNode = $versionNodes | Where-Object { (Get-VersionText $_.Text) -eq $tryVersion } | Select-Object -First 1
        if ($matchedNode) { break }
        $stripped = $tryVersion -replace '\.[0-9]+$', ''
        if ($stripped -eq $tryVersion) { break }
        $tryVersion = $stripped
    }
}

# 3) Fall back to Version TBD
if (-not $matchedNode) {
    $matchedNode = $versionNodes | Where-Object { (Get-VersionText $_.Text) -match '^Version TBD' } | Select-Object -First 1
}

if (-not $matchedNode) {
    Write-Error "Could not find changelog section for version '$version' or 'Version TBD'"
    exit 1
}

# ---- Extract content between this ### and the next ### (or end of ## Changelog) ----
$sectionStart = $matchedNode.Line + 1  # skip the heading line itself
$nextNode = $versionNodes | Where-Object { $_.Line -gt $matchedNode.Line } | Select-Object -First 1
$sectionEnd = if ($nextNode) { $nextNode.Line } else { $changelogEnd }

$sectionLines = $lines[$sectionStart..($sectionEnd - 1)]
$sectionContent = ($sectionLines -join "`n").Trim()

# ---- Handle outer ```{dropdown} ... ``` wrapper ----
$dropdownPattern = '(?ms)^```\{dropdown\}[^\n]*\n(.+?)^```$'
$dropdownMatch = [regex]::Match($sectionContent, $dropdownPattern)
if ($dropdownMatch.Success) {
    $sectionContent = $dropdownMatch.Groups[1].Value.Trim()
}

# Remove :open: Sphinx directive
$sectionContent = $sectionContent -replace '(?m)^:open:\s*\n', ''
$sectionContent = $sectionContent.Trim()

if (-not $sectionContent) {
    Write-Error "Extracted changelog content is empty for version '$version'"
    exit 1
}

$sectionContent | Out-File -FilePath $OutputPath -Encoding utf8

Write-Host "Extracted changelog for version '$version' to '$OutputPath'"
