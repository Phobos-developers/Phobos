import { copyFile, mkdir, readFile, rm, writeFile } from 'node:fs/promises'
import { dirname, resolve } from 'node:path'
import type { Plugin, ViteDevServer } from 'vite'
import { docsDir, normalizePath, rootDir } from './shared/paths.ts'
import type { PhobosGeneratedPage } from './shared/pages.ts'
export type { PhobosGeneratedPage } from './shared/pages.ts'

type RootSourcePage = {
  page: string
  outputPath: string
  sourcePath: string
  transform: (content: string) => string
}

type RootGeneratedSourcePage = RootSourcePage & {
  generatedRelPath: string
  targetPath: string
}

type RootAsset = {
  fileName: string
  sourcePath: string
}

const generatedRootRelPath = 'vitepress/generated/root'
const generatedRootDir = resolve(docsDir, generatedRootRelPath)
const repositoryDocsUrl = 'https://github.com/Phobos-developers/Phobos/tree/develop/docs/'
let pendingRootPagesGeneration: Promise<PhobosGeneratedPage[]> | null = null

function normalizeReadmeForVitePress(content: string): string {
  let output = content

  // Remove all markdown badges from docs README.
  output = output.replace(/^\[!\[[^\]]*]\([^)]*\)]\([^)]*\)\s*$/gm, '')
  output = output.replace(/\s*\[!\[[^\]]*]\([^)]*\)]\([^)]*\)\s*/g, ' ')

  // Render GitHub-style warning quotes as VitePress warning containers.
  output = output.replace(/^> (?:\*\*Warning\*\*|\[!WARNING])\r?\n((?:> .+\r?\n)+)/m, (_match, body: string) => {
    const warningText = body
      .split(/\r?\n/)
      .filter(Boolean)
      .map((line: string) => line.replace(/^> ?/, ''))
      .join('\n')

    return `::: warning\n${warningText}\n:::\n`
  })

  output = output.replace(/\[Official docs source\]\(docs\/?\)/u, `[Official docs source](${repositoryDocsUrl})`)
  output = output.replace(/\]\(docs\/?\)/gu, `](${repositoryDocsUrl})`)
  output = output.replace(/\]\((logo(?:-mono)?\.png)(#[^)]+)?\)/gu, (_match, path: string, hash = '') => {
    return `](${path}${hash})`
  })

  // Clean up empty lines left after badge removal.
  output = output.replace(/[ \t]+$/gm, '')
  output = output.replace(/(?:\r?\n){3,}/g, '\n\n')

  return `${output.trim()}\n`
}

function normalizeLicenseForVitePress(content: string): string {
  return `# License\n\n${content.trim()}\n`
}

const rootPages: RootSourcePage[] = [
  {
    page: 'README.md',
    outputPath: 'index.md',
    sourcePath: resolve(rootDir, 'README.md'),
    transform: normalizeReadmeForVitePress,
  },
  {
    page: 'CREDITS.md',
    outputPath: 'CREDITS.md',
    sourcePath: resolve(rootDir, 'CREDITS.md'),
    transform: (content: string) => `${content.trim()}\n`,
  },
  {
    page: 'License.md',
    outputPath: 'License.md',
    sourcePath: resolve(rootDir, 'LICENSE.md'),
    transform: normalizeLicenseForVitePress,
  },
]

function getGeneratedRelPath(page: string): string {
  return `${generatedRootRelPath}/${page}`
}

const rootAssets: RootAsset[] = [
  { fileName: 'logo.png', sourcePath: resolve(rootDir, 'logo.png') },
  { fileName: 'logo-mono.png', sourcePath: resolve(rootDir, 'logo-mono.png') },
]

export function getPhobosRootAssetRelPath(fileName: string): string {
  return getGeneratedRelPath(fileName)
}

function getRootPagesWithGeneratedPaths(): RootGeneratedSourcePage[] {
  return rootPages.map(page => ({
    ...page,
    generatedRelPath: getGeneratedRelPath(page.page),
    targetPath: resolve(docsDir, getGeneratedRelPath(page.page)),
  }))
}

export function getPhobosRootPageRewrites(
  pages: PhobosGeneratedPage[] | RootGeneratedSourcePage[] = getRootPagesWithGeneratedPaths(),
): Record<string, string> {
  return Object.fromEntries(pages.map(page => [page.generatedRelPath, page.outputPath]))
}

async function generateRootPagesOnce(): Promise<PhobosGeneratedPage[]> {
  const pages = getRootPagesWithGeneratedPaths()

  await rm(generatedRootDir, { recursive: true, force: true })

  for (const page of pages) {
    await mkdir(dirname(page.targetPath), { recursive: true })
    const sourceText = await readFile(page.sourcePath, 'utf8')
    await writeFile(page.targetPath, page.transform(sourceText))
  }

  for (const asset of rootAssets) {
    const targetPath = resolve(generatedRootDir, asset.fileName)
    await mkdir(dirname(targetPath), { recursive: true })
    await copyFile(asset.sourcePath, targetPath)
  }

  return pages.map(page => ({
    page: page.page,
    sourcePath: page.targetPath,
    originalSourcePath: page.sourcePath,
    generatedRelPath: page.generatedRelPath,
    outputPath: page.outputPath,
  }))
}

export async function generatePhobosRootPages(): Promise<PhobosGeneratedPage[]> {
  pendingRootPagesGeneration ??= generateRootPagesOnce().finally(() => {
    pendingRootPagesGeneration = null
  })

  return pendingRootPagesGeneration
}

export function phobosRootPagesPlugin(): Plugin {
  const sourcePaths = [...rootPages.map(page => page.sourcePath), ...rootAssets.map(asset => asset.sourcePath)]
  const normalizedSourcePaths = new Set(sourcePaths.map(normalizePath))
  const regenerate = async () => {
    await generatePhobosRootPages()
  }

  return {
    name: 'phobos-root-pages',
    enforce: 'pre',
    async buildStart() {
      await regenerate()
    },
    configureServer(server: ViteDevServer) {
      server.watcher.add(sourcePaths)
      server.watcher.on('change', async (changedPath: string) => {
        if (!normalizedSourcePaths.has(normalizePath(resolve(changedPath)))) {
          return
        }

        await regenerate()
        server.ws.send({ type: 'full-reload' })
      })
    },
  }
}
