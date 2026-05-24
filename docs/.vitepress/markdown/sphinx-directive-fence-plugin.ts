import type MarkdownIt from 'markdown-it'
import type StateBlock from 'markdown-it/lib/rules_block/state_block.mjs'

type SphinxDirectiveContainer = {
  title: string
  type: 'danger' | 'details' | 'info' | 'tip' | 'warning'
}

const sphinxDirectiveContainers: Record<string, SphinxDirectiveContainer> = {
  admonition: { type: 'info', title: 'Note' },
  attention: { type: 'warning', title: 'Attention' },
  caution: { type: 'warning', title: 'Caution' },
  danger: { type: 'danger', title: 'Danger' },
  dropdown: { type: 'details', title: 'Details' },
  error: { type: 'danger', title: 'Error' },
  hint: { type: 'tip', title: 'Hint' },
  important: { type: 'warning', title: 'Important' },
  note: { type: 'info', title: 'Note' },
  seealso: { type: 'info', title: 'See also' },
  tip: { type: 'tip', title: 'Tip' },
  warning: { type: 'warning', title: 'Warning' },
}

function normalizeDirectiveTitle(title: string | undefined, fallback: string) {
  return title?.trim() || fallback
}

function findClosingFence(source: string, marker: string, startLine: number, endLine: number, state: StateBlock) {
  for (let line = startLine; line < endLine; line += 1) {
    const pos = state.bMarks[line] + state.tShift[line]
    const max = state.eMarks[line]

    if (pos < max && state.sCount[line] < state.blkIndent) {
      break
    }

    if (source.slice(pos, max).trim() === marker) {
      return line
    }
  }

  return -1
}

function extractDropdownOptions(content: string) {
  const lines = content.split('\n')
  const firstContentLine = lines.findIndex((line) => line.trim() !== '')

  if (firstContentLine === -1 || lines[firstContentLine].trim() !== ':open:') {
    return { content, open: false }
  }

  lines.splice(firstContentLine, 1)
  return { content: lines.join('\n'), open: true }
}

export function sphinxDirectiveFencePlugin(md: MarkdownIt) {
  md.block.ruler.before('fence', 'sphinx_directive_fence', (state, startLine, endLine, silent) => {
    const start = state.bMarks[startLine] + state.tShift[startLine]
    const max = state.eMarks[startLine]
    const lineText = state.src.slice(start, max)
    const match = lineText.match(/^(`{3,}|~{3,})\{([A-Za-z][\w-]*)(?:\}\s*(.*)|\s+(.*)\}\s*)$/u)

    if (!match) {
      return false
    }

    const marker = match[1]
    const directiveName = match[2].toLowerCase()
    const directiveContainer = sphinxDirectiveContainers[directiveName]

    if (!directiveContainer) {
      return false
    }

    const closingLine = findClosingFence(state.src, marker, startLine + 1, endLine, state)
    if (closingLine === -1) {
      return false
    }

    if (silent) {
      return true
    }

    const title = normalizeDirectiveTitle(match[3] || match[4], directiveContainer.title)
    const token = state.push('sphinx_directive_container', '', 0)
    token.block = true
    token.info = directiveContainer.type
    const content = state.getLines(startLine + 1, closingLine, state.blkIndent, true)
    const dropdown = directiveName === 'dropdown' ? extractDropdownOptions(content) : undefined

    token.content = dropdown?.content ?? content
    token.markup = title
    token.meta = { open: dropdown?.open ?? false }
    token.map = [startLine, closingLine + 1]

    state.line = closingLine + 1
    return true
  })

  md.renderer.rules.sphinx_directive_container = (tokens, index, options, env) => {
    const token = tokens[index]
    const title = md.utils.escapeHtml(token.markup)
    const body = md.render(token.content, env)

    if (token.info === 'details') {
      const open = token.meta?.open ? ' open' : ''

      return `<details${open} class="details custom-block"><summary>${title}</summary>\n${body}</details>\n`
    }

    return `<div class="custom-block ${token.info}"><p class="custom-block-title">${title}</p>\n${body}</div>\n`
  }
}
