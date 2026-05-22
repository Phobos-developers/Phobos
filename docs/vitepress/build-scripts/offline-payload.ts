import { mkdir, readdir, rename, rm } from 'node:fs/promises'
import { posix, relative, resolve } from 'node:path'
import { collectFiles } from './shared/fs.ts'
import { normalizeUrlPath } from './shared/paths.ts'
import {
  flattenedPayloadDirs,
  offlineEntryFileName,
  offlineFilesDir,
  offlineFilesDirName,
  outputDir,
} from './shared/offline.ts'

export async function moveOfflineFilesUnderPayloadDir(): Promise<number> {
  const entries = await readdir(outputDir, { withFileTypes: true })
  const moves: Array<{ from: string; to: string }> = []
  const targetPaths = new Set<string>()

  await mkdir(offlineFilesDir, { recursive: true })

  for (const entry of entries) {
    if (entry.name === 'index.html' || entry.name === offlineEntryFileName || entry.name === offlineFilesDirName) {
      continue
    }

    const entryPath = resolve(outputDir, entry.name)

    if (entry.isDirectory() && flattenedPayloadDirs.has(entry.name)) {
      const nestedFiles = await collectFiles(entryPath, () => true)
      for (const nestedFile of nestedFiles) {
        moves.push({
          from: nestedFile,
          to: resolve(offlineFilesDir, posix.basename(normalizeUrlPath(nestedFile))),
        })
      }
      continue
    }

    moves.push({
      from: entryPath,
      to: resolve(offlineFilesDir, entry.name),
    })
  }

  for (const { to } of moves) {
    const normalizedTo = normalizeUrlPath(to)
    if (targetPaths.has(normalizedTo)) {
      throw new Error(`Offline payload flattening target conflict: ${normalizedTo}`)
    }
    targetPaths.add(normalizedTo)
  }

  for (const { from, to } of moves) {
    await rename(from, to)
  }

  for (const dirName of flattenedPayloadDirs) {
    await rm(resolve(outputDir, dirName), { recursive: true, force: true })
  }

  return moves.length
}

export async function removeInlinedFontFiles(): Promise<number> {
  const fontFiles = await collectFiles(outputDir, entry => /\.(?:woff2?|ttf|otf)$/iu.test(entry.name))

  for (const fontFile of fontFiles) {
    await rm(fontFile, { force: true })
  }

  return fontFiles.length
}

export async function removeBundledVitePressJsFiles(): Promise<number> {
  const assetsDir = resolve(outputDir, 'assets')
  const jsFiles = await collectFiles(assetsDir, entry => entry.name.endsWith('.js')).catch(() => [])

  for (const jsFile of jsFiles) {
    await rm(jsFile, { force: true })
  }

  return jsFiles.length
}

export async function removeStaticHtmlPageCopies(): Promise<number> {
  const htmlFiles = await collectFiles(outputDir, entry => entry.name.endsWith('.html'))
  const pageCopies = htmlFiles.filter(htmlFile => normalizeUrlPath(relative(outputDir, htmlFile)) !== 'index.html')

  for (const pageCopy of pageCopies) {
    await rm(pageCopy, { force: true })
  }

  await rm(resolve(outputDir, 'zh_CN'), { recursive: true, force: true })

  return pageCopies.length
}

export async function renameOfflineEntryFile(): Promise<void> {
  const sourcePath = resolve(outputDir, 'index.html')
  const targetPath = resolve(outputDir, offlineEntryFileName)

  await rename(sourcePath, targetPath)
}

export async function removeEmptyDirs(dir = outputDir): Promise<number> {
  const entries = await readdir(dir, { withFileTypes: true }).catch(() => [])
  let removed = 0

  for (const entry of entries) {
    if (!entry.isDirectory()) {
      continue
    }

    removed += await removeEmptyDirs(resolve(dir, entry.name))
  }

  const remainingEntries = await readdir(dir).catch(() => [])
  if (remainingEntries.length === 0 && dir !== outputDir) {
    await rm(dir, { recursive: true, force: true })
    removed += 1
  }

  return removed
}
