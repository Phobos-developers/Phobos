import type { Dirent } from 'node:fs'
import { readdir } from 'node:fs/promises'
import { join } from 'node:path'

export type ReadDirSafeOptions = {
  withFileTypes: true
}

export function isErrnoException(error: unknown): error is NodeJS.ErrnoException {
  return error instanceof Error && 'code' in error
}

export async function readDirSafe(dir: string, options: ReadDirSafeOptions): Promise<Dirent[]> {
  try {
    return await readdir(dir, options)
  } catch (error) {
    if (isErrnoException(error) && error.code === 'ENOENT') {
      return []
    }

    throw error
  }
}

export async function collectFiles(dir: string, predicate: (entry: Dirent) => boolean): Promise<string[]> {
  const result: string[] = []
  const entries = await readdir(dir, { withFileTypes: true })

  for (const entry of entries) {
    const entryPath = join(dir, entry.name)

    if (entry.isDirectory()) {
      result.push(...(await collectFiles(entryPath, predicate)))
      continue
    }

    if (entry.isFile() && predicate(entry)) {
      result.push(entryPath)
    }
  }

  return result
}
