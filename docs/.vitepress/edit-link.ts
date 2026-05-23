export function getEditLink({ filePath }: { filePath: string }) {
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
