import { execFile } from 'node:child_process'
import { relative } from 'node:path'
import { promisify } from 'node:util'
import { isErrnoException } from './shared/fs.ts'
import { normalizePath, rootDir } from './shared/paths.ts'
import type { GeneratedPage, LocalePage } from './shared/pages.ts'

const execFileAsync = promisify(execFile)
const gitTimestampCache = new Map<string, number | null>()

type LastUpdatedTransformOptions = {
  rootPages?: GeneratedPage[]
  localePages?: LocalePage[]
}

function getRepositoryRelativePath(filePath: string): string {
  return normalizePath(relative(rootDir, filePath))
}

async function getGitTimestampMs(filePath: string): Promise<number | null> {
  const repositoryRelativePath = getRepositoryRelativePath(filePath)

  if (gitTimestampCache.has(repositoryRelativePath)) {
    return gitTimestampCache.get(repositoryRelativePath) ?? null
  }

  let timestamp = null

  try {
    const { stdout } = await execFileAsync('git', ['log', '-1', '--format=%ct', '--', repositoryRelativePath], {
      cwd: rootDir,
    })
    const seconds = Number(stdout.trim())

    if (Number.isFinite(seconds) && seconds > 0) {
      timestamp = seconds * 1000
    }
  } catch (error) {
    if (!isErrnoException(error) || error.code !== 'ENOENT') {
      throw error
    }
  }

  gitTimestampCache.set(repositoryRelativePath, timestamp)
  return timestamp
}

async function getMaxGitTimestampMs(filePaths: string[]): Promise<number | null> {
  const timestamps = await Promise.all([...new Set(filePaths)].map(getGitTimestampMs))
  const validTimestamps = timestamps.filter((timestamp): timestamp is number => Number.isFinite(timestamp))

  return validTimestamps.length ? Math.max(...validTimestamps) : null
}

export function createLastUpdatedTransform(options: LastUpdatedTransformOptions = {}) {
  const sourcePathsByGeneratedPage = new Map()

  for (const page of options.rootPages || []) {
    sourcePathsByGeneratedPage.set(normalizePath(page.generatedRelPath), [page.originalSourcePath || page.sourcePath])
  }

  for (const page of options.localePages || []) {
    sourcePathsByGeneratedPage.set(normalizePath(page.generatedRelPath), [
      page.originalSourcePath || page.sourcePath,
      page.poPath,
    ])
  }

  return async (pageData: { filePath: string }) => {
    const sourcePaths = sourcePathsByGeneratedPage.get(normalizePath(pageData.filePath))

    if (!sourcePaths) {
      return undefined
    }

    const lastUpdated = await getMaxGitTimestampMs(sourcePaths)

    if (!lastUpdated) {
      return undefined
    }

    return { lastUpdated }
  }
}
