import { defineConfig } from 'vitepress'
import type { PluginOption } from 'vite'
import {
  generatePoLocalePages,
  getPoLocaleRewrites,
  poLocalePlugin,
  readLocaleIndexTranslationMap,
} from '../vitepress/build-scripts/vitepress-po-locale-plugin.ts'
import {
  generateRootPages,
  getRootPageRewrites,
  rootPagesPlugin,
} from '../vitepress/build-scripts/vitepress-root-pages-plugin.ts'
import { createLastUpdatedTransform } from '../vitepress/build-scripts/vitepress-last-updated.ts'
import { offlineVitePressPlugin } from '../vitepress/build-scripts/vitepress-offline-plugin.ts'
import { mediaDimensionsPlugin } from '../vitepress/build-scripts/media-dimensions-plugin.ts'
import { artifactsDistDir, outputDir as offlineOutputDir } from '../vitepress/build-scripts/shared/offline.ts'

const isOfflineBuild = process.env.DOCS_VITEPRESS_OFFLINE === '1'
const vitePressBase = process.env.READTHEDOCS_CANONICAL_URL
  ? new URL(process.env.READTHEDOCS_CANONICAL_URL).pathname.replace(/\/$/u, '')
  : '/'
const rootPages = await generateRootPages()
const poLocalePages = await generatePoLocalePages({ sourcePages: rootPages })
const rootPageRewrites = getRootPageRewrites(rootPages)
const poLocaleRewrites = await getPoLocaleRewrites(poLocalePages)
const transformPageData = createLastUpdatedTransform({ rootPages, localePages: poLocalePages })
const staticPublicDirMarkdownPlugin: PluginOption = {
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
const vitePlugins: PluginOption[] = [
  staticPublicDirMarkdownPlugin,
  mediaDimensionsPlugin(),
  rootPagesPlugin(),
  poLocalePlugin({
    sourcePages: rootPages,
    prepareSources: generateRootPages,
  }),
]

if (isOfflineBuild) {
  vitePlugins.push(offlineVitePressPlugin())
}

const englishNav = [
  { text: 'Home', link: '/' },
  { text: "What's New", link: '/Whats-New' },
  { text: 'Contributing', link: '/Contributing' },
]

const englishSidebar = [
  {
    text: 'Project Info',
    items: [
      { text: 'General Info', link: '/General-Info' },
      { text: "What's New", link: '/Whats-New' },
      { text: 'Contributing', link: '/Contributing' },
      { text: 'Credits', link: '/CREDITS' },
      { text: 'License', link: '/License' },
    ],
  },
  {
    text: 'Extension Documentation',
    items: [
      { text: 'New or Enhanced Logics', link: '/New-or-Enhanced-Logics' },
      { text: 'Fixed or Improved Logics', link: '/Fixed-or-Improved-Logics' },
      { text: 'AI Scripting and Mapping', link: '/AI-Scripting-and-Mapping' },
      { text: 'User Interface', link: '/User-Interface' },
      { text: 'Miscellanous', link: '/Miscellanous' },
    ],
  },
]

async function translateLocaleThemeText(locale: string, msgid: string) {
  const translations = await readLocaleIndexTranslationMap(locale)
  return translations.get(msgid) || msgid
}

async function createLocaleConfig(locale: string, labelMsgid: string, lang: string) {
  const localeRoot = `/${locale}`

  return {
    label: await translateLocaleThemeText(locale, labelMsgid),
    lang,
    title: await translateLocaleThemeText(locale, 'Phobos Documentation'),
    description: await translateLocaleThemeText(locale, 'Community documentation for Phobos YR engine extension'),
    themeConfig: {
      nav: [
        { text: await translateLocaleThemeText(locale, 'Home'), link: `${localeRoot}/` },
        { text: await translateLocaleThemeText(locale, "What's New"), link: `${localeRoot}/Whats-New` },
        { text: await translateLocaleThemeText(locale, 'Contributing'), link: `${localeRoot}/Contributing` },
      ],
      sidebar: [
        {
          text: await translateLocaleThemeText(locale, 'Project Info'),
          items: [
            { text: await translateLocaleThemeText(locale, 'General Info'), link: `${localeRoot}/General-Info` },
            { text: await translateLocaleThemeText(locale, "What's New"), link: `${localeRoot}/Whats-New` },
            { text: await translateLocaleThemeText(locale, 'Contributing'), link: `${localeRoot}/Contributing` },
            { text: await translateLocaleThemeText(locale, 'Credits'), link: `${localeRoot}/CREDITS` },
            { text: await translateLocaleThemeText(locale, 'License'), link: `${localeRoot}/License` },
          ],
        },
        {
          text: await translateLocaleThemeText(locale, 'Extension Documentation'),
          items: [
            {
              text: await translateLocaleThemeText(locale, 'New or Enhanced Logics'),
              link: `${localeRoot}/New-or-Enhanced-Logics`,
            },
            {
              text: await translateLocaleThemeText(locale, 'Fixed or Improved Logics'),
              link: `${localeRoot}/Fixed-or-Improved-Logics`,
            },
            {
              text: await translateLocaleThemeText(locale, 'AI Scripting and Mapping'),
              link: `${localeRoot}/AI-Scripting-and-Mapping`,
            },
            { text: await translateLocaleThemeText(locale, 'User Interface'), link: `${localeRoot}/User-Interface` },
            { text: await translateLocaleThemeText(locale, 'Miscellanous'), link: `${localeRoot}/Miscellanous` },
          ],
        },
      ],
      outline: {
        level: [2, 3] as [number, number],
        label: await translateLocaleThemeText(locale, 'On this page'),
      },
      editLink: {
        pattern: getEditLink,
        text: await translateLocaleThemeText(locale, 'Edit this page'),
      },
      lastUpdated: {
        text: await translateLocaleThemeText(locale, 'Last updated'),
      },
      docFooter: {
        prev: await translateLocaleThemeText(locale, 'Previous page'),
        next: await translateLocaleThemeText(locale, 'Next page'),
      },
      darkModeSwitchLabel: await translateLocaleThemeText(locale, 'Appearance'),
      lightModeSwitchTitle: await translateLocaleThemeText(locale, 'Switch to light theme'),
      darkModeSwitchTitle: await translateLocaleThemeText(locale, 'Switch to dark theme'),
      sidebarMenuLabel: await translateLocaleThemeText(locale, 'Menu'),
      returnToTopLabel: await translateLocaleThemeText(locale, 'Return to top'),
      langMenuLabel: await translateLocaleThemeText(locale, 'Select language'),
    },
  }
}

function getEditLink({ filePath }: { filePath: string }) {
  const repositoryEditRootUrl = 'https://github.com/Phobos-developers/Phobos/edit/develop/'
  const getPoPathForGeneratedLocalePage = (locale: string, pagePath: string) => {
    if (pagePath === 'README.md') {
      return `docs/locale/${locale}/LC_MESSAGES/index.po`
    }

    if (pagePath.endsWith('/README.md')) {
      return `docs/locale/${locale}/LC_MESSAGES/${pagePath.replace(/\/README\.md$/u, '/index.po')}`
    }

    return `docs/locale/${locale}/LC_MESSAGES/${pagePath.replace(/\.md$/u, '.po')}`
  }

  const rootPage = filePath.match(/^vitepress\/generated\/root\/(.+)$/u)
  if (rootPage) {
    const sourcePath = rootPage[1] === 'License.md' ? 'LICENSE.md' : rootPage[1]
    return `${repositoryEditRootUrl}${sourcePath}`
  }

  const localePage = filePath.match(/^vitepress\/generated\/locales\/([^/]+)\/(.+)$/u)
  if (localePage) {
    return `${repositoryEditRootUrl}${getPoPathForGeneratedLocalePage(localePage[1], localePage[2])}`
  }

  return `${repositoryEditRootUrl}docs/${filePath}`
}

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

function renderSearchContent(source: string, env: SearchRenderEnv, md: SearchMarkdownRenderer) {
  if (env.frontmatter?.search === false) {
    return ''
  }

  // Keep VitePress default behavior for search rendering, then remove only the
  // generated media fragments from the copy that is handed to MiniSearch.
  return stripSearchMedia(md.render(source, env))
}

export default defineConfig({
  title: 'Phobos Documentation',
  description: 'Community documentation for Phobos YR engine extension',
  base: vitePressBase,
  outDir: isOfflineBuild ? offlineOutputDir : artifactsDistDir,
  cleanUrls: false,
  lastUpdated: true,
  ignoreDeadLinks: true,
  transformPageData,
  rewrites: {
    ...rootPageRewrites,
    ...poLocaleRewrites,
  },
  vite: {
    publicDir: '_static',
    plugins: vitePlugins,
  },
  head: [
    ['link', { rel: 'icon', href: '/favicon.ico', type: 'image/x-icon' }],
    ['link', { rel: 'shortcut icon', href: '/favicon.ico' }],
    ['link', { rel: 'icon', href: '/favicon.png', type: 'image/png' }],
    ['link', { rel: 'apple-touch-icon', href: '/favicon.png' }],
  ],
  themeConfig: {
    logo: '/favicon.png',
    siteTitle: 'Phobos',
    outline: {
      level: [2, 3],
      label: 'On this page',
    },
    nav: englishNav,
    sidebar: englishSidebar,
    search: {
      provider: 'local',
      options: {
        _render: renderSearchContent,
      },
    },
    editLink: {
      pattern: getEditLink,
      text: 'Edit this page',
    },
    socialLinks: [{ icon: 'github', link: 'https://github.com/Phobos-developers/Phobos' }],
  },
  locales: {
    root: {
      label: 'English',
      lang: 'en-US',
    },
    zh_CN: {
      ...(await createLocaleConfig('zh_CN', 'Simplified Chinese', 'zh-CN')),
    },
  },
})
