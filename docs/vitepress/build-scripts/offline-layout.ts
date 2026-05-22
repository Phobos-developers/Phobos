import { posix } from 'node:path'
import { normalizeUrlPath } from './shared/paths.ts'
import { flattenedPayloadDirs, offlineEntryFileName, offlineFilesDirName } from './shared/offline.ts'

export function mapRelPathToOfflineRelPath(relPath: string): string {
  const normalizedRelPath = normalizeUrlPath(relPath)
  const [firstSegment, ...rest] = normalizedRelPath.split('/')

  if (flattenedPayloadDirs.has(firstSegment) && rest.length > 0) {
    return normalizeUrlPath(posix.join(offlineFilesDirName, posix.basename(normalizedRelPath)))
  }

  return normalizedRelPath === 'index.html'
    ? offlineEntryFileName
    : normalizeUrlPath(posix.join(offlineFilesDirName, normalizedRelPath))
}

function resolveOriginalRelPath(sourceRelPath: string, targetPath: string): string {
  const sourceDir = posix.dirname(normalizeUrlPath(sourceRelPath))
  const normalizedSourceDir = sourceDir === '.' ? '' : sourceDir

  if (targetPath.startsWith('/')) {
    return normalizeUrlPath(targetPath.replace(/^\/+/u, ''))
  }

  return normalizeUrlPath(posix.join(normalizedSourceDir, targetPath))
}

function toRelativeUrl(fromRelPath: string, targetRelPath: string): string {
  const fromDir = posix.dirname(normalizeUrlPath(fromRelPath))
  const normalizedFromDir = fromDir === '.' ? '' : fromDir
  const relativePath =
    posix.relative(normalizedFromDir, normalizeUrlPath(targetRelPath)) || posix.basename(targetRelPath)

  return relativePath.startsWith('.') ? relativePath : `./${relativePath}`
}

function resolveHtmlRouteTargetPath(originalTargetPath: string, htmlPageRelPaths: Set<string>): string | null {
  if (htmlPageRelPaths.has(originalTargetPath)) {
    return originalTargetPath
  }

  if (originalTargetPath.endsWith('/')) {
    const indexTargetPath = normalizeUrlPath(posix.join(originalTargetPath, 'index.html'))
    return htmlPageRelPaths.has(indexTargetPath) ? indexTargetPath : null
  }

  if (!/\.[^/]+$/u.test(originalTargetPath)) {
    const htmlTargetPath = `${originalTargetPath}.html`
    return htmlPageRelPaths.has(htmlTargetPath) ? htmlTargetPath : null
  }

  return null
}

export function rewriteLocalUrlForFilesLayout(
  url: string,
  originalRelPath: string,
  finalRelPath: string,
  attr: string,
  htmlPageRelPaths: Set<string>,
): string {
  if (!url || /^(?:#|[a-z][a-z0-9+.-]*:|\/\/)/iu.test(url)) {
    return url
  }

  const [pathAndQuery, hash = ''] = url.split('#')
  const [pathname, query = ''] = pathAndQuery.split('?')
  if (!pathname) {
    return url
  }

  const originalTargetPath = resolveOriginalRelPath(originalRelPath, pathname)
  const htmlTargetPath = attr === 'href' ? resolveHtmlRouteTargetPath(originalTargetPath, htmlPageRelPaths) : null
  if (htmlTargetPath) {
    const routePath = `/${htmlTargetPath}`.replace(/\/index\.html$/u, '/').replace(/\.html$/u, '')
    const rootIndexUrl = toRelativeUrl(finalRelPath, offlineEntryFileName)
    const queryPart = query ? `?${query}` : ''
    const hashPart = hash ? `#${hash}` : ''

    return `${rootIndexUrl}#${routePath}${queryPart}${hashPart}`
  }

  const finalTargetPath = mapRelPathToOfflineRelPath(originalTargetPath)
  const relativeUrl = toRelativeUrl(finalRelPath, finalTargetPath)
  const queryPart = query ? `?${query}` : ''
  const hashPart = hash ? `#${hash}` : ''

  return `${relativeUrl}${queryPart}${hashPart}`
}

export function rewriteHtmlForFilesLayout(
  html: string,
  originalRelPath: string,
  finalRelPath: string,
  htmlPageRelPaths: Set<string>,
): string {
  return html
    .replace(/\b(href|src)=(")([^"]*)\2/gu, (_match, attr: string, quote: string, url: string) => {
      const rewritten = rewriteLocalUrlForFilesLayout(url, originalRelPath, finalRelPath, attr, htmlPageRelPaths)
      return `${attr}=${quote}${rewritten}${quote}`
    })
    .replace(/\b(href|src)=(')([^']*)\2/gu, (_match, attr: string, quote: string, url: string) => {
      const rewritten = rewriteLocalUrlForFilesLayout(url, originalRelPath, finalRelPath, attr, htmlPageRelPaths)
      return `${attr}=${quote}${rewritten}${quote}`
    })
}
