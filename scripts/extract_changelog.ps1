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

$changelogIdx = $content.IndexOf("## Changelog")
if ($changelogIdx -lt 0) {
    Write-Error "Could not find '## Changelog' section in $WhatsNewPath"
    exit 1
}

$afterChangelog = $content.Substring($changelogIdx + "## Changelog".Length + 1)

$nextSectionIdx = $afterChangelog.IndexOf("`n## ")
$changelogBody = if ($nextSectionIdx -ge 0) { $afterChangelog.Substring(0, $nextSectionIdx) } else { $afterChangelog }

# Try to find the section: exact version -> parent versions -> Version TBD
$sectionStart = -1

# 1) Exact version match
$sectionStart = $changelogBody.IndexOf("### $version`n")

# 2) Strip trailing numeric components until found
if ($sectionStart -lt 0) {
    $tryVersion = $version -replace '\.[0-9]+$', ''
    while ($tryVersion -ne $version) {
        $sectionStart = $changelogBody.IndexOf("### $tryVersion`n")
        if ($sectionStart -ge 0) { break }
        $version = $tryVersion
        $tryVersion = $version -replace '\.[0-9]+$', ''
    }
    $version = $Tag.TrimStart('v')  # restore original
}

# 3) Fall back to Version TBD
if ($sectionStart -lt 0) {
    $sectionStart = $changelogBody.IndexOf("### Version TBD")
}

if ($sectionStart -lt 0) {
    Write-Error "Could not find changelog section for version '$version' or 'Version TBD'"
    exit 1
}

$sectionEnd = $changelogBody.IndexOf("`n### ", $sectionStart + 5)
if ($sectionEnd -ge 0) {
    $sectionContent = $changelogBody.Substring($sectionStart, $sectionEnd - $sectionStart)
} else {
    $sectionContent = $changelogBody.Substring($sectionStart)
}

$firstNewline = $sectionContent.IndexOf("`n")
if ($firstNewline -ge 0) {
    $sectionContent = $sectionContent.Substring($firstNewline + 1)
} else {
    $sectionContent = ""
}

$sectionContent = $sectionContent.Trim()

$dropdownPattern = '(?ms)^```\{dropdown\}[^\n]*\n(.+?)^```'
$dropdownMatch = [regex]::Match($sectionContent, $dropdownPattern)

if ($dropdownMatch.Success) {
    $sectionContent = $dropdownMatch.Groups[1].Value.Trim()
}

$sectionContent = $sectionContent -replace '(?m)^:open:\s*\n', ''
$sectionContent = $sectionContent.Trim()

if (-not $sectionContent) {
    Write-Error "Extracted changelog content is empty for version '$version'"
    exit 1
}

$sectionContent | Out-File -FilePath $OutputPath -Encoding utf8

Write-Host "Extracted changelog for version '$version' to '$OutputPath'"
