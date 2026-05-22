declare module 'gettext-parser' {
  export type PoTranslationEntry = {
    comments?: {
      flag?: string
    }
    msgid?: string
    msgstr?: string[]
  }

  export type PoParseResult = {
    headers: Record<string, string>
    translations: Record<string, Record<string, PoTranslationEntry>>
  }

  const gettextParser: {
    po: {
      compile(parsed: PoParseResult): Buffer
      parse(buffer: Buffer): PoParseResult
    }
  }

  export default gettextParser
}
