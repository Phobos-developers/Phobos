export type GeneratedPage = {
  page: string
  sourcePath: string
  originalSourcePath?: string
  generatedRelPath: string
  outputPath: string
}

export type SourcePage = {
  page: string
  sourcePath: string
  originalSourcePath?: string
}

export type LocalePage = SourcePage & {
  locale: string
  poPath: string
  targetPath: string
  generatedRelPath: string
  outputPath: string
}
