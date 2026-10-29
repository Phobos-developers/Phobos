@echo off
rem Prints a ref pointing at the current HEAD, preferring a tag. Used when `git symbolic-ref HEAD`
rem fails, i.e. on detached HEAD checkouts such as CI pull-request merges and tag builds.
rem The ref name is the third space-separated field of git for-each-ref's default output, and each
rem loop prints only the first match then exits. It is parsed here in a batch file because
rem MSBuild's Exec task would otherwise interpret a %(refname) format string as item metadata and
rem mangle it into `(refname)`.
for /f "tokens=3" %%r in ('git for-each-ref --points-at HEAD refs/tags 2^>nul') do @echo(%%r&goto :eof
for /f "tokens=3" %%r in ('git for-each-ref --points-at HEAD 2^>nul') do @echo(%%r&goto :eof
