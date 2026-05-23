import { mkdir, readdir, readFile, rm, writeFile } from 'node:fs/promises'
import { posix, relative, resolve } from 'node:path'
import type { build as esbuildBuild } from 'esbuild'
import { collectFiles } from './shared/fs.ts'
import { normalizeUrlPath } from './shared/paths.ts'
import { offlineFilesDirName, offlineRuntimeFileName, outputDir } from './shared/offline.ts'

function toJsString(value: string): string {
  return JSON.stringify(value)
}

function toRelativeImportPath(fromDir: string, targetPath: string): string {
  const relativePath = posix.relative(normalizeUrlPath(fromDir), normalizeUrlPath(targetPath))
  return relativePath.startsWith('.') ? relativePath : `./${relativePath}`
}

function getOfflinePageLoaderSource(pageModuleRelPaths: string[], fromRelDir: string): string {
  const imports: string[] = []
  const entries: string[] = []

  pageModuleRelPaths.forEach((relPath, index) => {
    const binding = `__vp_offline_page_${index}`
    const importPath = toRelativeImportPath(fromRelDir, relPath)
    imports.push(`import * as ${binding} from ${toJsString(importPath)};`)
    entries.push(`[${toJsString(`/${relPath}`)}, () => Promise.resolve(${binding})]`)
  })

  return `${imports.join('\n')}
const __vpOfflinePageModules = new Map([${entries.join(',')}]);
globalThis.__DOCS_OFFLINE_LOAD_PAGE__ = path => {
  const key = String(path || '').replace(/^.*\\/assets\\//u, '/assets/');
  const load = __vpOfflinePageModules.get(key);
  if (!load) {
    console.error('Offline VitePress page module was not found:', path);
    return Promise.resolve(null);
  }
  return load();
};
`
}

async function collectPageModuleRelPaths(): Promise<string[]> {
  const assetsDir = resolve(outputDir, 'assets')
  const entries = await readdir(assetsDir, { withFileTypes: true })
  return entries
    .filter(entry => entry.isFile() && /\.md\.[^.]+(?:\.lean)?\.js$/u.test(entry.name))
    .map(entry => normalizeUrlPath(posix.join('assets', entry.name)))
}

async function getLocalSearchChunkPath(): Promise<string | null> {
  const chunksDir = resolve(outputDir, 'assets', 'chunks')
  const entries = await readdir(chunksDir, { withFileTypes: true })
  const searchChunk = entries.find(entry => entry.isFile() && /^VPLocalSearchBox\..*\.js$/u.test(entry.name))
  return searchChunk ? resolve(chunksDir, searchChunk.name) : null
}

async function collectJsFiles(dir: string): Promise<string[]> {
  return collectFiles(dir, entry => entry.name.endsWith('.js'))
}

async function rewriteJsAssetUrlsForOfflineRuntime(): Promise<void> {
  const jsFiles = await collectJsFiles(resolve(outputDir, 'assets'))

  for (const modulePath of jsFiles) {
    const source = await readFile(modulePath, 'utf8')
    const rewritten = source
      .replace(/(["'])\/(?:images|assets)\//gu, `$1./${offlineFilesDirName}/`)
      .replace(/\/images\//gu, `./${offlineFilesDirName}/`)

    if (rewritten !== source) {
      await writeFile(modulePath, rewritten)
    }
  }
}

async function assertFileIncludes(filePath: string, snippets: string[], description: string): Promise<void> {
  const source = await readFile(filePath, 'utf8')
  const missingSnippet = snippets.find(snippet => !source.includes(snippet))

  if (missingSnippet) {
    throw new Error(
      `${description} is not offline-ready. Run docs build with DOCS_VITEPRESS_OFFLINE=1 before exporting.`,
    )
  }
}

async function assertOfflineVitePressRuntime(appEntryPath: string, frameworkPath: string): Promise<void> {
  await assertFileIncludes(
    appEntryPath,
    ['__DOCS_OFFLINE_LOAD_PAGE__', '__DOCS_VITEPRESS_DATA__', '__DOCS_VITEPRESS_ROUTER__'],
    'VitePress app bundle',
  )
  await assertFileIncludes(frameworkPath, [`./${offlineFilesDirName}/`, '#/'], 'VitePress framework bundle')

  const localSearchChunkPath = await getLocalSearchChunkPath()
  if (localSearchChunkPath) {
    await assertFileIncludes(localSearchChunkPath, ['__DOCS_OFFLINE_LOAD_PAGE__'], 'VitePress local search bundle')
  }
}

async function writeOfflineRuntimeEntry(appEntryPath: string, pageModuleRelPaths: string[]): Promise<string> {
  const appEntryRelPath = normalizeUrlPath(relative(outputDir, appEntryPath))
  const runtimeEntryPath = resolve(outputDir, 'offline-runtime-entry.js')
  const runtimeEntrySource = `${getOfflinePageLoaderSource(pageModuleRelPaths, '')}
import(${toJsString(toRelativeImportPath('', appEntryRelPath))});
`

  await writeFile(runtimeEntryPath, runtimeEntrySource)
  return runtimeEntryPath
}

export async function buildOfflineVitePressRuntime(): Promise<number> {
  const assetsDir = resolve(outputDir, 'assets')
  const chunksDir = resolve(assetsDir, 'chunks')
  const assetEntries = await readdir(assetsDir, { withFileTypes: true })
  const chunkEntries = await readdir(chunksDir, { withFileTypes: true })
  const appEntry = assetEntries.find(entry => entry.isFile() && /^app\..*\.js$/u.test(entry.name))
  const frameworkChunk = chunkEntries.find(entry => entry.isFile() && /^framework\..*\.js$/u.test(entry.name))

  if (!appEntry || !frameworkChunk) {
    throw new Error('Could not find VitePress runtime entry/chunks in offline output.')
  }

  const pageModuleRelPaths = await collectPageModuleRelPaths()
  const appEntryPath = resolve(assetsDir, appEntry.name)
  const frameworkPath = resolve(chunksDir, frameworkChunk.name)
  const runtimeEntryPath = await writeOfflineRuntimeEntry(appEntryPath, pageModuleRelPaths)

  await assertOfflineVitePressRuntime(appEntryPath, frameworkPath)
  await rewriteJsAssetUrlsForOfflineRuntime()

  await mkdir(outputDir, { recursive: true })
  const { build } = (await import('esbuild')) as { build: typeof esbuildBuild }
  await build({
    entryPoints: [runtimeEntryPath],
    bundle: true,
    format: 'iife',
    target: ['es2020'],
    minify: true,
    legalComments: 'none',
    outfile: resolve(outputDir, offlineRuntimeFileName),
    logLevel: 'silent',
    write: true,
  })
  await rm(runtimeEntryPath, { force: true })

  return pageModuleRelPaths.length
}
