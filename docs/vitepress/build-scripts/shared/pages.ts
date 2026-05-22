export type PhobosGeneratedPage = {
  page: string
  sourcePath: string
  originalSourcePath?: string
  generatedRelPath: string
  outputPath: string
}

export type PhobosSourcePage = {
  page: string
  sourcePath: string
  originalSourcePath?: string
}

export type PhobosLocalePage = PhobosSourcePage & {
  locale: string
  poPath: string
  targetPath: string
  generatedRelPath: string
  outputPath: string
}
