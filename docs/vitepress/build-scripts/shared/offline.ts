import { resolve } from 'node:path'
import { docsDir } from './paths.ts'

export const offlineOutputDirName = 'offline-doc'
export const offlineEntryFileName = 'offline-doc.htm'
export const offlineFilesDirName = 'offline-doc'
export const offlineRuntimeFileName = 'vitepress-runtime.js'
export const offlineRuntimeRelPath = offlineRuntimeFileName
export const flattenedPayloadDirs = new Set(['assets', 'images', 'offline-assets'])

export const artifactsDistDir = resolve(docsDir, '.artifacts', 'dist')
export const outputDir = resolve(docsDir, '.artifacts', offlineOutputDirName)
export const offlineFilesDir = resolve(outputDir, offlineFilesDirName)
