import type { PluginOption } from 'vite'

export const staticPublicDirMarkdownPlugin: PluginOption = {
  name: 'docs-static-public-dir-markdown',
  enforce: 'pre',
  transform(source, id) {
    const modulePath = id.split('?')[0]

    if (!modulePath.endsWith('.md')) {
      return null
    }

    const rewritten = source.replace(/\bsrc=(["'])_static\//gu, 'src=$1/').replace(/(\]\()_static\//gu, '$1/')

    return rewritten === source ? null : rewritten
  },
}
