import { readFile } from 'node:fs/promises'
import { posix, resolve } from 'node:path'
import { mapRelPathToOfflineRelPath, rewriteLocalUrlForFilesLayout } from './offline-layout.ts'
import { normalizeUrlPath } from './shared/paths.ts'
import { outputDir } from './shared/offline.ts'

const cssFontDataUrlCache = new Map<string, string>()

function getFontMimeType(fontPath: string): string {
  if (/\.woff2(?:[?#].*)?$/iu.test(fontPath)) {
    return 'font/woff2'
  }

  if (/\.woff(?:[?#].*)?$/iu.test(fontPath)) {
    return 'font/woff'
  }

  if (/\.ttf(?:[?#].*)?$/iu.test(fontPath)) {
    return 'font/ttf'
  }

  if (/\.otf(?:[?#].*)?$/iu.test(fontPath)) {
    return 'font/otf'
  }

  return 'application/octet-stream'
}

function resolveCssAssetPath(url: string, cssRelPath: string): string | null {
  if (!url || /^(?:data:|[a-z][a-z0-9+.-]*:|\/\/)/iu.test(url)) {
    return null
  }

  const [pathOnly] = url.split(/[?#]/u)
  const decodedPath = decodeURIComponent(pathOnly)

  if (decodedPath.startsWith('/')) {
    return resolve(outputDir, decodedPath.replace(/^\/+/u, ''))
  }

  const cssDir = posix.dirname(normalizeUrlPath(cssRelPath))
  const normalizedCssDir = cssDir === '.' ? '' : cssDir

  return resolve(outputDir, normalizeUrlPath(posix.join(normalizedCssDir, decodedPath)))
}

async function getCssFontDataUrl(url: string, cssRelPath: string): Promise<string | null> {
  const fontPath = resolveCssAssetPath(url, cssRelPath)
  if (!fontPath) {
    return null
  }

  const cached = cssFontDataUrlCache.get(fontPath)
  if (cached) {
    return cached
  }

  const font = await readFile(fontPath)
  const dataUrl = `data:${getFontMimeType(fontPath)};base64,${font.toString('base64')}`
  cssFontDataUrlCache.set(fontPath, dataUrl)

  return dataUrl
}

async function inlineCssFontUrls(css: string, cssRelPath: string): Promise<string> {
  let output = ''
  let lastIndex = 0
  const urlPattern = /url\((["']?)([^"')]+)\1\)/giu

  for (const match of css.matchAll(urlPattern)) {
    const [raw, , url] = match

    if (!/\.(?:woff2?|ttf|otf)(?:[?#]|$)/iu.test(url)) {
      continue
    }

    const dataUrl = await getCssFontDataUrl(url, cssRelPath)
    if (!dataUrl) {
      continue
    }

    output += css.slice(lastIndex, match.index)
    output += `url("${dataUrl}")`
    lastIndex = match.index + raw.length
  }

  return lastIndex === 0 ? css : output + css.slice(lastIndex)
}

export async function rewriteCssForOffline(
  css: string,
  cssRelPath: string,
  htmlPageRelPaths: Set<string>,
): Promise<string> {
  const cssWithInlineFonts = await inlineCssFontUrls(css, cssRelPath)
  const rewrittenUrls = cssWithInlineFonts.replace(
    /url\((["']?)\/(?!\/)([^"')]+)\1\)/gu,
    (_match, quote: string, path: string) => {
      const finalCssPath = mapRelPathToOfflineRelPath(cssRelPath)
      const localUrl = rewriteLocalUrlForFilesLayout(`/${path}`, cssRelPath, finalCssPath, 'src', htmlPageRelPaths)
      return `url(${quote}${localUrl}${quote})`
    },
  )

  // In offline docs we prefer visual stability over early paint, so avoid
  // font swap flashes on each full-page navigation.
  return rewrittenUrls.replace(/font-display:\s*swap\b/giu, 'font-display:block')
}
