import { readFile, stat, writeFile } from 'node:fs/promises'
import { dirname, relative } from 'node:path'
import { rewriteCssForOffline } from './offline-css.ts'
import { rewriteHtmlForOffline } from './offline-html.ts'
import { mapRelPathToOfflineRelPath, rewriteHtmlForFilesLayout } from './offline-layout.ts'
import {
  moveOfflineFilesUnderPayloadDir,
  removeBundledVitePressJsFiles,
  removeEmptyDirs,
  removeInlinedFontFiles,
  removeStaticHtmlPageCopies,
  renameOfflineEntryFile,
} from './offline-payload.ts'
import { buildOfflineVitePressRuntime } from './offline-runtime.ts'
import { collectFiles } from './shared/fs.ts'
import { normalizeUrlPath } from './shared/paths.ts'
import { offlineFilesDir, outputDir } from './shared/offline.ts'

const outputStats = await stat(outputDir).catch(() => null)
if (!outputStats || !outputStats.isDirectory()) {
  throw new Error('Offline VitePress output not found. Run "npm run build:offline" first.')
}

const bundledPageModules = await buildOfflineVitePressRuntime()

const htmlFiles = await collectFiles(outputDir, entry => entry.name.endsWith('.html'))
const cssFiles = await collectFiles(outputDir, entry => entry.name.endsWith('.css'))
const htmlPageRelPaths = new Set(htmlFiles.map(htmlPath => normalizeUrlPath(relative(outputDir, htmlPath))))

for (const htmlPath of htmlFiles) {
  const relPath = relative(outputDir, htmlPath)
  const normalizedRelPath = normalizeUrlPath(relPath)
  const finalRelPath = mapRelPathToOfflineRelPath(normalizedRelPath)
  const rootDepth = dirname(relPath) === '.' ? 0 : dirname(relPath).split('/').length
  const source = await readFile(htmlPath, 'utf8')
  const offlineHtml = rewriteHtmlForOffline(source, rootDepth)
  const rewritten = rewriteHtmlForFilesLayout(offlineHtml, normalizedRelPath, finalRelPath, htmlPageRelPaths)
  await writeFile(htmlPath, rewritten)
}

for (const cssPath of cssFiles) {
  const relPath = relative(outputDir, cssPath)
  const normalizedRelPath = normalizeUrlPath(relPath)
  const source = await readFile(cssPath, 'utf8')
  const rewritten = await rewriteCssForOffline(source, normalizedRelPath, htmlPageRelPaths)
  await writeFile(cssPath, rewritten)
}

const removedFontFiles = await removeInlinedFontFiles()
const removedBundledJsFiles = await removeBundledVitePressJsFiles()
const removedHtmlPageCopies = await removeStaticHtmlPageCopies()
const removedEmptyDirs = await removeEmptyDirs()
await renameOfflineEntryFile()
const movedRootEntries = await moveOfflineFilesUnderPayloadDir()

console.log(`Offline HTML export ready: ${outputDir}`)
console.log(`Processed ${htmlFiles.length} HTML files.`)
console.log(`Processed ${cssFiles.length} CSS files.`)
console.log(`Bundled ${bundledPageModules} VitePress page modules into the offline runtime.`)
console.log(`Removed ${removedFontFiles} inlined font files from the offline payload.`)
console.log(`Removed ${removedBundledJsFiles} bundled VitePress JS source files from the offline payload.`)
console.log(`Removed ${removedHtmlPageCopies} static HTML page copies from the offline payload.`)
console.log(`Removed ${removedEmptyDirs} empty directories from the offline payload.`)
console.log(`Moved ${movedRootEntries} root entries to ${offlineFilesDir}.`)
