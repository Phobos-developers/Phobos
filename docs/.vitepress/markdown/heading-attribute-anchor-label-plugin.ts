import type MarkdownIt from 'markdown-it'

const markdownHeadingAttributeInTitleRegExp = /\s+\{#[A-Za-z0-9_.:-]+\}(?="?$)/u

export function headingAttributeAnchorLabelPlugin(md: MarkdownIt): void {
  md.core.ruler.after('anchor', 'docs_heading_attribute_anchor_label', state => {
    for (const token of state.tokens) {
      if (token.type !== 'inline' || !token.children) {
        continue
      }

      for (const child of token.children) {
        // markdown-it-anchor reads the heading title before markdown-it-attrs
        // strips `{#id}`, so clean the generated permalink label afterwards.
        const ariaLabel = child.attrGet('aria-label')
        if (ariaLabel) {
          child.attrSet('aria-label', ariaLabel.replace(markdownHeadingAttributeInTitleRegExp, ''))
        }
      }
    }
  })
}
