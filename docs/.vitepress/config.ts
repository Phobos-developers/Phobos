import { defineConfig } from 'vitepress'
import type { PluginOption } from 'vite'
import {
  generatePoLocalePages,
  getPoLocaleRewrites,
  poLocalePlugin,
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
import { getEditLink } from './edit-link.ts'
import { headingAttributeAnchorLabelPlugin } from './markdown/heading-attribute-anchor-label-plugin.ts'
import { sphinxDirectiveFencePlugin } from './markdown/sphinx-directive-fence-plugin.ts'
import { staticPublicDirMarkdownPlugin } from './markdown/static-public-dir-markdown-plugin.ts'
import { renderSearchContent } from './search/local-search-renderer.ts'
import { createLocaleConfig, englishNav, englishSidebar } from './theme-config.ts'

const isOfflineBuild = process.env.DOCS_VITEPRESS_OFFLINE === '1'
const vitePressBase = process.env.READTHEDOCS_CANONICAL_URL
  ? new URL(process.env.READTHEDOCS_CANONICAL_URL).pathname.replace(/\/$/u, '')
  : '/'
const rootPages = await generateRootPages()
const poLocalePages = await generatePoLocalePages({ sourcePages: rootPages })
const rootPageRewrites = getRootPageRewrites(rootPages)
const poLocaleRewrites = await getPoLocaleRewrites(poLocalePages)
const transformPageData = createLastUpdatedTransform({ rootPages, localePages: poLocalePages })
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

export default defineConfig({
  title: 'Phobos Documentation',
  description: 'Community documentation for Phobos YR engine extension',
  base: vitePressBase,
  outDir: isOfflineBuild ? offlineOutputDir : artifactsDistDir,
  cleanUrls: false,
  lastUpdated: true,
  ignoreDeadLinks: true,
  transformPageData,
  markdown: {
    attrs: {
      allowedAttributes: ['id'],
    },
    config(md) {
      md.use(headingAttributeAnchorLabelPlugin)
      md.use(sphinxDirectiveFencePlugin)
    },
  },
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
