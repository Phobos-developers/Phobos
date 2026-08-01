<#
.SYNOPSIS
    Extracts the release notes for a tag out of the What's New document.

.DESCRIPTION
    Whats-New.md is laid out as "type of change -> version": each top level section
    (`## Breaking changes`, `## Changelog`) holds one subsection per version. A release
    is about a single version, so this script parses the document into a heading tree,
    picks every subsection matching the released version wherever it sits in the tree,
    and re-emits them as "version -> type of change" - the version is the release
    itself, so the type of change becomes the top level heading.

    Markup that only means something to Sphinx is normalized on the way out, so the
    result can be used as a release body as is: `{dropdown}` directives are unwrapped,
    admonitions become GitHub alerts, directive options are dropped and relative
    documentation links are resolved against -LinkBase.
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$Tag,

    [Parameter(Mandatory = $true)]
    [string]$WhatsNewPath,

    [Parameter(Mandatory = $true)]
    [string]$OutputPath,

    # Base URL to resolve relative documentation links against, e.g.
    # https://github.com/Phobos-developers/Phobos/blob/v0.5/docs
    # When empty the links are emitted unchanged, which only works within the docs themselves.
    [string]$LinkBase = ''
)

$ErrorActionPreference = 'Stop'

# MyST admonitions that map onto GitHub alerts. Anything else is unwrapped silently.
$AlertKinds = @{
    'note'       = 'NOTE'
    'seealso'    = 'NOTE'
    'admonition' = 'NOTE'
    'tip'        = 'TIP'
    'hint'       = 'TIP'
    'important'  = 'IMPORTANT'
    'attention'  = 'IMPORTANT'
    'warning'    = 'WARNING'
    'caution'    = 'CAUTION'
    'danger'     = 'CAUTION'
    'error'      = 'CAUTION'
}

$linkPrefix = $LinkBase.TrimEnd('/')

function New-Section([int]$Level, [string]$Title, $Parent) {
    [pscustomobject]@{
        Level    = $Level
        Title    = $Title
        Parent   = $Parent
        Lines    = [System.Collections.Generic.List[string]]::new()
        Children = [System.Collections.Generic.List[object]]::new()
    }
}

# Returns the fence descriptor for a line that opens or closes a fenced block, else $null.
function Get-Fence([string]$Line) {
    if ($Line -match '^ {0,3}(?<fence>`{3,}|~{3,})[ \t]*(?<info>.*?)[ \t]*$') {
        return [pscustomobject]@{
            Char = $Matches.fence.Substring(0, 1)
            Len  = $Matches.fence.Length
            Info = $Matches.info
        }
    }
    return $null
}

# A fence closes another one when it uses the same character, is at least as long
# and carries no info string.
function Test-FenceCloses($Fence, $Opener) {
    return ($Fence.Char -eq $Opener.Char) -and ($Fence.Len -ge $Opener.Len) -and ($Fence.Info -eq '')
}

# Relative links into the docs are meaningless outside of them, so anchor them to -LinkBase.
function Convert-Link([string]$Text) {
    if (-not $linkPrefix) { return $Text }

    $evaluator = [System.Text.RegularExpressions.MatchEvaluator] {
        param($m)
        '](' + $linkPrefix + '/' + $m.Groups['path'].Value + $m.Groups['frag'].Value + ')'
    }
    return [regex]::Replace($Text, '\]\((?!\w+:|[#/])(?<path>[^)\s#]+\.md)(?<frag>#[^)\s]*)?\)', $evaluator)
}

# ---- Parse the document into a tree of sections carrying their own contents ----

function Read-SectionTree([string[]]$Lines) {
    $root = New-Section 0 '' $null
    $current = $root

    # Fenced code blocks are opaque - their contents are never markdown.
    # MyST directive fences (```{dropdown} etc.) are transparent - they wrap markdown,
    # headings included, so they only affect how the contents are emitted.
    $code = $null
    $directives = [System.Collections.Generic.List[object]]::new()
    $inOptions = $false

    foreach ($line in $Lines) {

        if ($code) {
            $current.Lines.Add($line)
            $fence = Get-Fence $line
            if ($fence -and (Test-FenceCloses $fence $code)) { $code = $null }
            continue
        }

        $fence = Get-Fence $line
        if ($fence) {
            # Innermost first: a bare fence closes the directive it is nested in.
            if ($directives.Count -gt 0 -and (Test-FenceCloses $fence $directives[$directives.Count - 1])) {
                $directives.RemoveAt($directives.Count - 1)
                $inOptions = $false
                continue
            }

            if ($fence.Info.StartsWith('{')) {
                $kind = ($fence.Info -replace '^\{([^}]*)\}.*$', '$1').Trim().ToLowerInvariant()
                $alert = $null
                if ($AlertKinds.ContainsKey($kind)) {
                    $alert = $AlertKinds[$kind]
                    $current.Lines.Add('')
                    $current.Lines.Add("> [!$alert]")
                }
                $directives.Add([pscustomobject]@{ Char = $fence.Char; Len = $fence.Len; Kind = $kind; Alert = $alert })
                $inOptions = $true
                continue
            }

            $code = $fence
            $current.Lines.Add($line)
            continue
        }

        # `:open:` and friends configure the directive, they are not contents.
        if ($inOptions) {
            if ($line -match '^\s*:[A-Za-z][A-Za-z0-9_-]*:') { continue }
            $inOptions = $false
        }

        if ($line -match '^(?<hashes>#{1,6})\s+(?<title>.*?)\s*#*\s*$') {
            $level = $Matches.hashes.Length
            while ($current.Level -ge $level) { $current = $current.Parent }

            $node = New-Section $level (Convert-Link $Matches.title) $current
            $current.Children.Add($node)
            $current = $node
            continue
        }

        $text = Convert-Link $line

        # Contents of an admonition have to be quoted for the alert to hold them.
        $alert = $null
        for ($i = $directives.Count - 1; $i -ge 0; $i--) {
            if ($directives[$i].Alert) { $alert = $directives[$i].Alert; break }
        }
        if ($alert) { $text = if ($text.Trim() -eq '') { '>' } else { "> $text" } }

        $current.Lines.Add($text)
    }

    return $root
}

function Get-AllSections($Node) {
    foreach ($child in $Node.Children) {
        $child
        Get-AllSections $child
    }
}

# The version a section documents, or $null when it documents no particular version.
function Get-SectionVersion([string]$Title) {
    if ($Title -match '^\s*Version\s+TBD\b') { return 'TBD' }
    if ($Title -match '^\s*[vV]?(?<version>\d+(\.\d+)*(-[0-9A-Za-z.]+)?)\b') { return $Matches.version }
    return $null
}

# ---- Render a matched section as "type of change -> contents" ----

function Add-SectionContents($Node, [int]$Shift, $Sink) {
    foreach ($line in $Node.Lines) { $Sink.Add($line) }

    foreach ($child in $Node.Children) {
        $level = [Math]::Min(6, [Math]::Max(1, $child.Level + $Shift))
        $Sink.Add('')
        $Sink.Add(('#' * $level) + ' ' + $child.Title)
        Add-SectionContents $child $Shift $Sink
    }
}

# The path from the top level section down to the matched one names the type of change.
function Get-SectionKind($Node) {
    $chain = @()
    $parent = $Node.Parent
    while ($parent -and $parent.Level -ge 2) {
        $chain = , $parent.Title + $chain
        $parent = $parent.Parent
    }
    return ($chain -join ' / ')
}

# ---- Main ----

if (-not (Test-Path -LiteralPath $WhatsNewPath)) {
    throw "File not found: $WhatsNewPath"
}

$content = (Get-Content -LiteralPath $WhatsNewPath -Raw -Encoding utf8) -replace "`r`n", "`n"
$root = Read-SectionTree ($content -split "`n")
$sections = @(Get-AllSections $root)

$version = $Tag -replace '^[vV]', ''
$baseVersion = $version -replace '-.*$', ''

$matched = @($sections | Where-Object { (Get-SectionVersion $_.Title) -eq $version })

# Only a pre-release may document itself under another name: it shares the sections of the
# version it leads up to (0.5-beta1 -> 0.5), which are still called "Version TBD" until the
# release branch is cut. A stable or patch release documenting nothing is an oversight, and
# publishing whatever else is at hand - the unreleased develop notes above all - would hide it.
if (-not $matched -and $baseVersion -ne $version) {
    $matched = @($sections | Where-Object { (Get-SectionVersion $_.Title) -eq $baseVersion })
    if ($matched) { Write-Host "No sections for '$version', using the ones for '$baseVersion'" }

    if (-not $matched) {
        $matched = @($sections | Where-Object { (Get-SectionVersion $_.Title) -eq 'TBD' })
        if ($matched) { Write-Warning "No sections for '$version', falling back to 'Version TBD' - rename them to '$baseVersion' in $WhatsNewPath" }
    }
}

if (-not $matched) {
    $known = @($sections | ForEach-Object { Get-SectionVersion $_.Title } | Where-Object { $_ } | Select-Object -Unique) -join ', '
    throw "No sections for version '$version' in $WhatsNewPath. Documented versions: $known"
}

$out = [System.Collections.Generic.List[string]]::new()
foreach ($section in $matched) {
    $kind = Get-SectionKind $section
    if ($kind) {
        $out.Add('')
        $out.Add("## $kind")
    }
    # The matched section's own children start right below the type of change heading.
    Add-SectionContents $section (2 - $section.Level) $out
}

$text = ($out -join "`n") -replace '(?m)[ \t]+$', ''
$text = ($text -replace "\n{3,}", "`n`n").Trim()

if (-not $text) {
    throw "Extracted release notes for version '$version' are empty"
}

$outPath = if ([System.IO.Path]::IsPathRooted($OutputPath)) { $OutputPath } else { Join-Path (Get-Location).Path $OutputPath }
[System.IO.File]::WriteAllText($outPath, $text + "`n", (New-Object System.Text.UTF8Encoding($false)))

$kinds = @($matched | ForEach-Object { Get-SectionKind $_ }) -join ', '
Write-Host "Extracted release notes for version '$version' ($kinds) to '$OutputPath'"
