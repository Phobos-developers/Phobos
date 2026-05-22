import { dirname, posix, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

export const buildScriptsDir = dirname(fileURLToPath(import.meta.url))
export const docsDir = resolve(buildScriptsDir, '..', '..', '..')
export const rootDir = resolve(docsDir, '..')

export function normalizePath(value: string): string {
  return value.replace(/\\/gu, '/')
}

export function normalizeUrlPath(pathname: string): string {
  const normalized = posix.normalize(pathname.replace(/\\/gu, '/'))
  return normalized === '.' ? '' : normalized.replace(/^\.\//u, '')
}
