import { getEditLink } from './edit-link.ts'
import { readLocaleIndexTranslationMap } from '../vitepress/build-scripts/vitepress-po-locale-plugin.ts'

export const englishNav = [
  { text: 'Home', link: '/' },
  { text: "What's New", link: '/Whats-New' },
  { text: 'Contributing', link: '/Contributing' },
]

export const englishSidebar = [
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

export async function createLocaleConfig(locale: string, labelMsgid: string, lang: string) {
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
