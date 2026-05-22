import { mkdir, readFile, rm, stat, writeFile } from 'node:fs/promises'
import { basename, dirname, extname, join, posix, relative, resolve } from 'node:path'
import type { Plugin, ViteDevServer } from 'vite'
import gettextParser from 'gettext-parser'
import { isErrnoException, readDirSafe } from './shared/fs.ts'
import { docsDir, normalizePath } from './shared/paths.ts'
import type { PhobosLocalePage, PhobosSourcePage } from './shared/pages.ts'
import { getPhobosRootAssetRelPath } from './vitepress-root-pages-plugin.ts'

const localeRootDir = resolve(docsDir, 'locale')
const localeMessagesDirName = 'LC_MESSAGES'
const generatedRootRelPath = 'vitepress/generated/locales'
const generatedRootDir = resolve(docsDir, generatedRootRelPath)
const includeFuzzy = process.env.PHOBOS_PO_INCLUDE_FUZZY !== '0'
const ignoredDocsMarkdownDirs = new Set(['.artifacts', '.vitepress', '_static', 'locale', 'node_modules', 'vitepress'])

type SourcePage = PhobosSourcePage
type LocalePage = PhobosLocalePage

type PoLocaleOptions = {
  prepareSources?: () => Promise<SourcePage[]> | SourcePage[]
  sourcePages?: SourcePage[]
}

async function fileExists(path: string): Promise<boolean> {
  try {
    const fileStat = await stat(path)
    return fileStat.isFile()
  } catch (error) {
    if (isErrnoException(error) && error.code === 'ENOENT') {
      return false
    }

    throw error
  }
}

async function collectPoFiles(dir: string, baseDir = dir): Promise<string[]> {
  const entries = await readDirSafe(dir, { withFileTypes: true })
  const poFiles: string[] = []

  for (const entry of entries) {
    const entryPath = resolve(dir, entry.name)

    if (entry.isDirectory()) {
      poFiles.push(...(await collectPoFiles(entryPath, baseDir)))
      continue
    }

    if (entry.isFile() && extname(entry.name) === '.po') {
      poFiles.push(normalizePath(relative(baseDir, entryPath)))
    }
  }

  return poFiles
}

async function collectDocsMarkdownFiles(dir = docsDir, baseDir = docsDir): Promise<SourcePage[]> {
  const entries = await readDirSafe(dir, { withFileTypes: true })
  const pages: SourcePage[] = []

  for (const entry of entries) {
    const entryPath = resolve(dir, entry.name)

    if (entry.isDirectory()) {
      if (dir === docsDir && ignoredDocsMarkdownDirs.has(entry.name)) {
        continue
      }

      pages.push(...(await collectDocsMarkdownFiles(entryPath, baseDir)))
      continue
    }

    if (entry.isFile() && extname(entry.name) === '.md') {
      pages.push({
        page: normalizePath(relative(baseDir, entryPath)),
        sourcePath: entryPath,
      })
    }
  }

  return pages
}

function getSourcePageCandidatesForPoRelPath(poRelPath: string): string[] {
  const dir = dirname(poRelPath)
  const name = basename(poRelPath, '.po')

  if (name !== 'index') {
    return [normalizePath(join(dir === '.' ? '' : dir, `${name}.md`))]
  }

  const parentDir = dir === '.' ? '' : dir
  return [normalizePath(join(parentDir, 'README.md')), normalizePath(join(parentDir, 'index.md'))]
}

function getGeneratedRelPath(locale: string, page: string): string {
  return `${generatedRootRelPath}/${locale}/${page}`
}

function getLocalizedOutputPath(locale: string, page: string): string {
  if (basename(page) === 'README.md') {
    const pageDir = dirname(page)
    const localeDir = pageDir === '.' ? locale : `${locale}/${normalizePath(pageDir)}`
    return `${localeDir}/index.md`
  }

  return `${locale}/${page}`
}

function getSourcePageMap(sourcePages: SourcePage[]): Map<string, SourcePage> {
  return new Map(sourcePages.map(page => [page.page, page]))
}

async function collectMarkdownSourcePages(sourcePages: SourcePage[]): Promise<SourcePage[]> {
  const pages = new Map(sourcePages.map(page => [page.page, page]))

  for (const page of await collectDocsMarkdownFiles()) {
    if (pages.has(page.page)) {
      continue
    }

    pages.set(page.page, page)
  }

  return [...pages.values()]
}

async function resolveSourcePageForPo(
  locale: string,
  poRelPath: string,
  sourcePageMap: Map<string, SourcePage>,
): Promise<SourcePage | null> {
  for (const page of getSourcePageCandidatesForPoRelPath(poRelPath)) {
    const generatedSource = sourcePageMap.get(page)
    if (generatedSource) {
      return {
        page,
        sourcePath: generatedSource.sourcePath,
        originalSourcePath: generatedSource.originalSourcePath || generatedSource.sourcePath,
      }
    }

    const sourcePath = resolve(docsDir, page)

    if (await fileExists(sourcePath)) {
      return { page, sourcePath, originalSourcePath: sourcePath }
    }
  }

  const candidates = getSourcePageCandidatesForPoRelPath(poRelPath).join(' or ')
  console.warn(`Skipping ${locale}/${poRelPath}: source page ${candidates} was not found.`)
  return null
}

export async function discoverPhobosPoLocalePages(options: PoLocaleOptions = {}): Promise<LocalePage[]> {
  const sourcePageMap = getSourcePageMap(options.sourcePages || [])
  const localeDirs = await readDirSafe(localeRootDir, { withFileTypes: true })
  const discoveredPages = []

  for (const localeDir of localeDirs) {
    if (!localeDir.isDirectory()) {
      continue
    }

    const locale = localeDir.name
    const localeMessagesDirPath = resolve(localeRootDir, locale, localeMessagesDirName)
    const poFiles = await collectPoFiles(localeMessagesDirPath)

    for (const poRelPath of poFiles) {
      const source = await resolveSourcePageForPo(locale, poRelPath, sourcePageMap)
      if (!source) {
        continue
      }

      discoveredPages.push({
        locale,
        page: source.page,
        sourcePath: source.sourcePath,
        originalSourcePath: source.originalSourcePath,
        poPath: resolve(localeMessagesDirPath, poRelPath),
        targetPath: resolve(docsDir, getGeneratedRelPath(locale, source.page)),
        generatedRelPath: getGeneratedRelPath(locale, source.page),
        outputPath: getLocalizedOutputPath(locale, source.page),
      })
    }
  }

  return discoveredPages.sort((a, b) => {
    const localeOrder = a.locale.localeCompare(b.locale)
    return localeOrder || a.page.localeCompare(b.page)
  })
}

function parsePo(buffer: Buffer): Map<string, string> {
  const result = new Map<string, string>()
  const parsed = gettextParser.po.parse(buffer)
  const table = parsed.translations[''] || {}

  for (const entry of Object.values(table)) {
    const msgid = entry.msgid || ''
    const msgstr = Array.isArray(entry.msgstr) ? entry.msgstr[0] || '' : ''
    const fuzzy = Boolean(entry.comments?.flag?.includes('fuzzy'))

    if (!msgid || !msgstr) {
      continue
    }
    if (!includeFuzzy && fuzzy) {
      continue
    }
    if (!result.has(msgid)) {
      result.set(msgid, msgstr)
    }
  }

  return result
}

async function readLocaleIndexTranslations(locale: string): Promise<Map<string, string>> {
  const indexPoPath = resolve(localeRootDir, locale, localeMessagesDirName, 'index.po')

  if (!(await fileExists(indexPoPath))) {
    return new Map()
  }

  return parsePo(await readFile(indexPoPath))
}

const localeIndexTranslationsCache = new Map<string, Promise<Map<string, string>>>()

export async function readPhobosLocaleIndexTranslations(locale: string): Promise<Map<string, string>> {
  if (!localeIndexTranslationsCache.has(locale)) {
    localeIndexTranslationsCache.set(locale, readLocaleIndexTranslations(locale))
  }

  return localeIndexTranslationsCache.get(locale) ?? new Map()
}

function getMarkdownTitle(content: string): string | null {
  const title = content.match(/^#\s+(.+?)\s*#*\s*$/m)
  return title?.[1]?.trim() || null
}

async function getLocalizedDocLinkAliases(locale: string, sourcePages: SourcePage[]): Promise<Map<string, string>> {
  const translations = await readLocaleIndexTranslations(locale)
  const aliases = new Map()

  if (!translations.size) {
    return aliases
  }

  for (const sourcePage of await collectMarkdownSourcePages(sourcePages)) {
    const title = getMarkdownTitle(await readFile(sourcePage.sourcePath, 'utf8'))
    const translatedTitle = title ? translations.get(title) : null

    if (!translatedTitle || translatedTitle === title) {
      continue
    }

    const sourcePagePath = normalizePath(sourcePage.page)
    const sourcePageDir = posix.dirname(sourcePagePath)
    const translatedFileName = `${translatedTitle}.md`

    aliases.set(translatedFileName, posix.basename(sourcePagePath))

    if (sourcePageDir !== '.') {
      aliases.set(`${sourcePageDir}/${translatedFileName}`, sourcePagePath)
    }
  }

  return aliases
}

async function getLocalizedDocLinkAliasesByLocale(
  localePages: LocalePage[],
  sourcePages: SourcePage[],
): Promise<Map<string, Map<string, string>>> {
  const aliasesByLocale = new Map<string, Map<string, string>>()

  for (const locale of new Set(localePages.map(page => page.locale))) {
    aliasesByLocale.set(locale, await getLocalizedDocLinkAliases(locale, sourcePages))
  }

  return aliasesByLocale
}

function isUnsafeStandaloneToken(msgid: string): boolean {
  return /^[A-Za-z][A-Za-z0-9_-]*$/.test(msgid)
}

function getLineEnding(content: string): '\n' | '\r\n' {
  return content.includes('\r\n') ? '\r\n' : '\n'
}

function normalizeLineEndings(content: string): string {
  return content.replace(/\r\n?/g, '\n')
}

function restoreLineEndings(content: string, lineEnding: '\n' | '\r\n'): string {
  return lineEnding === '\n' ? content : content.replace(/\n/g, lineEnding)
}

function getRelativeGeneratedLink(fromGeneratedRelPath: string, targetGeneratedRelPath: string): string {
  const relativePath = posix.relative(posix.dirname(fromGeneratedRelPath), targetGeneratedRelPath)
  return relativePath.startsWith('.') ? relativePath : `./${relativePath}`
}

function normalizeLocalizedDocLinks(
  content: string,
  page: LocalePage,
  aliases: Map<string, string> = new Map(),
): string {
  const { generatedRelPath } = page
  const rootAssets = new Set(['logo.png', 'logo-mono.png'])

  return content.replace(/\]\(([^)]+)\)/g, (full: string, target: string) => {
    const assetMatch = target.match(/^([^#\s]+)(#[^)]+)?$/)
    if (!assetMatch) {
      return full
    }

    const rawPath = assetMatch[1]
    const hash = assetMatch[2] || ''

    if (/^(?:https?:|\/|#)/i.test(rawPath)) {
      return full
    }

    const rootAssetName = basename(rawPath)
    if (rootAssets.has(rootAssetName)) {
      const assetLink = getRelativeGeneratedLink(generatedRelPath, getPhobosRootAssetRelPath(rootAssetName))
      return `](${assetLink}${hash})`
    }

    if (rawPath.startsWith('_static/')) {
      return `](/${rawPath.slice('_static/'.length)}${hash})`
    }

    if (!rawPath.endsWith('.md')) {
      return full
    }

    const mapped = aliases.get(rawPath)
    if (!mapped) {
      return full
    }

    return `](${mapped}${hash})`
  })
}

function translateMarkdown(content: string, translations: Map<string, string>): string {
  const lineEnding = getLineEnding(content)
  let output = normalizeLineEndings(content)
  const normalizedTranslations = new Map<string, string>()

  for (const [msgid, msgstr] of translations.entries()) {
    const normalizedMsgid = normalizeLineEndings(msgid)
    const normalizedMsgstr = normalizeLineEndings(msgstr)

    if (!normalizedTranslations.has(normalizedMsgid)) {
      normalizedTranslations.set(normalizedMsgid, normalizedMsgstr)
    }
  }

  const entries = [...normalizedTranslations.entries()]
    .filter(([msgid, msgstr]) => msgid && msgstr && msgid !== msgstr && !isUnsafeStandaloneToken(msgid))
    .sort((a, b) => b[0].length - a[0].length)

  for (const [msgid, msgstr] of entries) {
    if (output.includes(msgid)) {
      output = output.split(msgid).join(msgstr)
    }
  }

  return restoreLineEndings(output, lineEnding)
}

export async function getPhobosPoLocaleRewrites(
  localePages: LocalePage[] | null = null,
  options: PoLocaleOptions = {},
) {
  const pages = localePages || (await discoverPhobosPoLocalePages(options))
  return Object.fromEntries(pages.map(page => [page.generatedRelPath, page.outputPath]))
}

async function prepareSourcePages(options: PoLocaleOptions): Promise<SourcePage[]> {
  const preparedSourcePages = options.prepareSources ? await options.prepareSources() : null

  return options.sourcePages || preparedSourcePages || []
}

export async function generatePhobosPoLocalePages(options: PoLocaleOptions = {}): Promise<LocalePage[]> {
  const sourcePages = await prepareSourcePages(options)
  const localePages = await discoverPhobosPoLocalePages({ ...options, sourcePages })
  const linkAliasesByLocale = await getLocalizedDocLinkAliasesByLocale(localePages, sourcePages)

  await rm(generatedRootDir, { recursive: true, force: true })

  for (const page of localePages) {
    await mkdir(dirname(page.targetPath), { recursive: true })

    const [sourceText, poBuffer] = await Promise.all([readFile(page.sourcePath, 'utf8'), readFile(page.poPath)])
    const translatedText = normalizeLocalizedDocLinks(
      translateMarkdown(sourceText, parsePo(poBuffer)),
      page,
      linkAliasesByLocale.get(page.locale),
    )
    await writeFile(page.targetPath, translatedText)
  }

  return localePages
}

function shouldRegenerateForPath(changedPath: string, sourcePages: SourcePage[]): boolean {
  const normalizedPath = normalizePath(resolve(changedPath))
  const normalizedLocaleRootDir = `${normalizePath(localeRootDir)}/`
  const normalizedDocsDir = normalizePath(docsDir)

  if (normalizedPath.startsWith(normalizedLocaleRootDir) && normalizedPath.endsWith('.po')) {
    return true
  }

  for (const sourcePage of sourcePages) {
    const watchedSourcePath = sourcePage.originalSourcePath || sourcePage.sourcePath
    if (normalizedPath === normalizePath(watchedSourcePath)) {
      return true
    }
  }

  return dirname(normalizedPath) === normalizedDocsDir && normalizedPath.endsWith('.md')
}

export function phobosPoLocalePlugin(options: PoLocaleOptions = {}): Plugin {
  let pendingGeneration: Promise<LocalePage[]> | null = null
  const regenerate = async () => {
    pendingGeneration ??= generatePhobosPoLocalePages(options).finally(() => {
      pendingGeneration = null
    })

    await pendingGeneration
  }

  return {
    name: 'phobos-po-locale-pages',
    enforce: 'pre',
    async buildStart() {
      await regenerate()
    },
    configureServer(server: ViteDevServer) {
      const sourcePaths = (options.sourcePages || []).map(page => page.originalSourcePath || page.sourcePath)
      server.watcher.add([localeRootDir, docsDir, ...sourcePaths])

      const handleFilesystemChange = async (changedPath: string) => {
        if (!shouldRegenerateForPath(changedPath, options.sourcePages || [])) {
          return
        }

        await regenerate()
        server.ws.send({ type: 'full-reload' })
      }

      server.watcher.on('add', handleFilesystemChange)
      server.watcher.on('change', handleFilesystemChange)
      server.watcher.on('unlink', handleFilesystemChange)
    },
  }
}
