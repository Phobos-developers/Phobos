import { offlineRuntimeRelPath } from './shared/offline.ts'

function normalizeLocalHref(targetPath: string): string {
  if (!targetPath || targetPath === '/') {
    return '/index.html'
  }

  if (targetPath.endsWith('/')) {
    return `${targetPath}index.html`
  }

  if (!/\.[^/]+$/u.test(targetPath)) {
    return `${targetPath}.html`
  }

  return targetPath
}

function getRelativePrefix(rootDepth: number): string {
  return rootDepth === 0 ? './' : '../'.repeat(rootDepth)
}

function resolveLocalUrl(absoluteUrl: string, rootDepth: number): string {
  const [pathAndQuery, hash = ''] = absoluteUrl.split('#')
  const [pathname, query = ''] = pathAndQuery.split('?')
  const normalizedPathname = normalizeLocalHref(pathname)
  const relPrefix = getRelativePrefix(rootDepth)
  const queryPart = query ? `?${query}` : ''
  const hashPart = hash ? `#${hash}` : ''

  return `${relPrefix}${normalizedPathname.replace(/^\//u, '')}${queryPart}${hashPart}`
}

function injectOfflineRuntimeScript(html: string, rootDepth: number): string {
  const relativePrefix = getRelativePrefix(rootDepth)
  const scriptTag = `<script src="${relativePrefix}${offlineRuntimeRelPath}?v=full-runtime"></script>`

  if (html.includes('</body>')) {
    return html.replace('</body>', `${scriptTag}</body>`)
  }

  return `${html}${scriptTag}`
}

function normalizeOfflineNavBarClasses(html: string): string {
  if (!html.includes('class="VPSidebar"') && !html.includes("class='VPSidebar'")) {
    return html
  }

  return html.replace(/class=(["'])([^"']*\bVPNavBar\b[^"']*)\1/u, (_match, quote: string, classValue: string) => {
    const classes = classValue.split(/\s+/u).filter(Boolean)

    for (const className of ['has-sidebar', 'top']) {
      if (!classes.includes(className)) {
        classes.push(className)
      }
    }

    return `class=${quote}${classes.join(' ')}${quote}`
  })
}

export function rewriteHtmlForOffline(html: string, rootDepth: number): string {
  let output = html

  // Clean up any legacy offline layout overrides from previous exporter versions.
  output = output.replace(/<style\s+id=(["'])docs-offline-layout-fixes\1>[\s\S]*?<\/style>\s*/giu, '')
  output = normalizeOfflineNavBarClasses(output)

  // The offline runtime is bundled into a classic script, so browsers do not
  // need file:// ES module loading support.
  output = output.replace(/<script\b[^>]*\btype=(["'])module\1[^>]*>[\s\S]*?<\/script>\s*/giu, '')
  output = output.replace(/<link\s+rel="modulepreload"[^>]*>\s*/giu, '')
  output = output.replace(/<link\s+rel="prefetch"[^>]*>\s*/giu, '')
  output = output.replace(/<link\b[^>]*\bas=(["'])font\1[^>]*>\s*/giu, '')

  // Convert root-absolute links/assets to file-relative links.
  output = output.replace(
    /\b(href|src)=(")\/(?!\/)([^"]*)\2/gu,
    (_match, attr: string, quote: string, path: string) => {
      const localUrl = resolveLocalUrl(`/${path}`, rootDepth)
      return `${attr}=${quote}${localUrl}${quote}`
    },
  )

  output = output.replace(
    /\b(href|src)=(')\/(?!\/)([^']*)\2/gu,
    (_match, attr: string, quote: string, path: string) => {
      const localUrl = resolveLocalUrl(`/${path}`, rootDepth)
      return `${attr}=${quote}${localUrl}${quote}`
    },
  )

  output = injectOfflineRuntimeScript(output, rootDepth)
  return output
}
