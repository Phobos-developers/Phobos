type SearchRenderEnv = {
  frontmatter?: {
    search?: boolean
  }
}

type SearchMarkdownRenderer = {
  render: (source: string, env: SearchRenderEnv) => string
}

// VitePress builds the local search index from rendered HTML, not from final
// page DOM. Strip media-only fragments here so filenames and captions do not
// become searchable while the actual documentation pages stay unchanged.
const searchMediaTagPattern = String.raw`(?:<img\b[^>]*>|<video\b[^>]*>[\s\S]*?<\/video>)`
const searchMediaWithNextCaptionRegExp = new RegExp(
  String.raw`${searchMediaTagPattern}\s*<p>\s*<em>[\s\S]*?<\/em>\s*<\/p>`,
  'giu',
)
const searchMediaParagraphWithInlineCaptionRegExp = new RegExp(
  String.raw`<p>\s*${searchMediaTagPattern}\s*<em>[\s\S]*?<\/em>\s*<\/p>`,
  'giu',
)
const searchMediaParagraphWithNextCaptionRegExp = new RegExp(
  String.raw`<p>\s*${searchMediaTagPattern}\s*<\/p>\s*<p>\s*<em>[\s\S]*?<\/em>\s*<\/p>`,
  'giu',
)
const searchMediaParagraphRegExp = new RegExp(String.raw`<p>\s*${searchMediaTagPattern}\s*<\/p>`, 'giu')
const searchStandaloneMediaRegExp = new RegExp(searchMediaTagPattern, 'giu')

function stripSearchMedia(html: string) {
  return html
    .replace(searchMediaWithNextCaptionRegExp, '')
    .replace(searchMediaParagraphWithNextCaptionRegExp, '')
    .replace(searchMediaParagraphWithInlineCaptionRegExp, '')
    .replace(searchMediaParagraphRegExp, '')
    .replace(searchStandaloneMediaRegExp, '')
}

export function renderSearchContent(source: string, env: SearchRenderEnv, md: SearchMarkdownRenderer) {
  if (env.frontmatter?.search === false) {
    return ''
  }

  // Keep VitePress default behavior for search rendering, then remove only the
  // generated media fragments from the copy that is handed to MiniSearch.
  return stripSearchMedia(md.render(source, env))
}
