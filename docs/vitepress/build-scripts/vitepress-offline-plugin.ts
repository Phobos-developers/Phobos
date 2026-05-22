import type { Plugin, ResolvedConfig } from 'vite'
import { offlineFilesDirName } from './shared/offline.ts'

const vitePressClientMarker = '/vitepress/dist/client/'
const offlineRetryDelays = '[100, 300, 700, 1200]'

type VitePressPatch = {
  search: string | RegExp
  replacement: string
  name: string
}

type VitePressModulePatcher = {
  match: (moduleId: string, id: string) => boolean
  patch: (source: string) => string
}

// Offline docs store the page route in location.hash, for example
// #/Miscellanous#player-colors. VitePress internals still expect just
// #player-colors when matching active anchors.
const normalizeOfflineHashHelper = `function normalizeOfflineHash(hash) {
    if (!hash.startsWith('#/')) {
        return hash;
    }
    const routeHash = hash.slice(1);
    const anchorIndex = routeHash.indexOf('#');
    return anchorIndex >= 0 ? routeHash.slice(anchorIndex) : '';
}`

// This replaces VitePress' URL normalizer. Offline docs have one browser
// document, so route changes must become hash changes instead of path changes.
const offlineRouterHelpers = `function normalizeOfflinePathname(pathname) {
    let normalizedPathname = pathname.replace(/(^|\\/)index(\\.html)?$/, '$1');
    if (siteDataRef.value.cleanUrls)
        normalizedPathname = normalizedPathname.replace(/\\.html$/, '');
    else if (!normalizedPathname.endsWith('/') && !normalizedPathname.endsWith('.html'))
        normalizedPathname += '.html';
    return normalizedPathname || '/';
}
function normalizeHref(href) {
    const url = new URL(href, fakeHost);
    // In the offline build the browser URL points to a single HTML file, while
    // the real VitePress route lives after the first hash: file.htm#/Page#anchor.
    if (url.hash.startsWith('#/') && (String(href).startsWith('#/') || !inBrowser || url.pathname === location.pathname)) {
        const hashValue = url.hash.slice(1);
        const anchorIndex = hashValue.indexOf('#');
        const pathAndSearch = anchorIndex >= 0 ? hashValue.slice(0, anchorIndex) : hashValue;
        const anchor = anchorIndex >= 0 ? hashValue.slice(anchorIndex) : '';
        const routeUrl = new URL(pathAndSearch || '/', fakeHost);
        return normalizeOfflinePathname(routeUrl.pathname) + routeUrl.search + anchor;
    }
    if (inBrowser && url.hash.startsWith('#/')) {
        const hashValue = url.hash.slice(1);
        const anchorIndex = hashValue.indexOf('#');
        const anchor = anchorIndex >= 0 ? hashValue.slice(anchorIndex) : '';
        return normalizeOfflinePathname(url.pathname) + url.search + anchor;
    }
    if (inBrowser && url.pathname === location.pathname) {
        return '/' + url.search + url.hash;
    }
    return normalizeOfflinePathname(url.pathname) + url.search + url.hash;
}
function toOfflineDocumentLinkHref(linkHref, routePath) {
    if (/^(?:[a-z][a-z0-9+.-]*:|\\/\\/)/i.test(linkHref)) {
        return null;
    }
    if (linkHref.startsWith('#/')) {
        return location.pathname + location.search + linkHref;
    }
    if (linkHref.startsWith('#')) {
        return null;
    }
    const currentUrl = new URL(location.href);
    const normalizedRoutePath = routePath || '/';
    const routeDir = normalizedRoutePath.endsWith('/')
        ? normalizedRoutePath
        : normalizedRoutePath.replace(/[^/]*$/, '');
    const targetUrl = linkHref.startsWith('/')
            ? new URL(linkHref, currentUrl)
            : new URL(linkHref, currentUrl.origin + routeDir);
    if (targetUrl.origin === currentUrl.origin && treatAsHtml(targetUrl.pathname)) {
        return toOfflineBrowserHref(targetUrl.href);
    }
    return null;
}
function rewriteOfflineDocumentLinks(routePath) {
    const rewriteLinks = () => {
        document.querySelectorAll('.vp-doc a[href]').forEach(link => {
            if (link.closest('.vp-raw') || link.hasAttribute('download') || link.hasAttribute('target')) {
                return;
            }
            const linkHref = link.getAttribute('href')
                || (link instanceof SVGAElement ? link.getAttribute('xlink:href') : null);
            if (linkHref == null) {
                return;
            }
            const offlineHref = toOfflineDocumentLinkHref(linkHref, routePath);
            if (offlineHref) {
                link.setAttribute('href', offlineHref);
            }
        });
    };
    rewriteLinks();
    // Markdown content can be swapped after route changes; retry a few times so
    // links rendered by async Vue updates are also normalized.
    for (const delay of ${offlineRetryDelays}) {
        setTimeout(rewriteLinks, delay);
    }
}
function toOfflineBrowserHref(href) {
    return location.pathname + location.search + '#' + normalizeHref(href);
}
function toOfflineAnchorBrowserHref(routePath, hash) {
    return location.pathname + location.search + '#' + (routePath || '/') + (hash || '');
}
function scrollToOfflineHashTarget(target, hash) {
    scrollTo(target, hash);
    // Deep-link targets may appear after the first tick while the offline
    // runtime mounts the page module.
    for (const delay of ${offlineRetryDelays}) {
        setTimeout(() => {
            let retryTarget = null;
            try {
                retryTarget = document.getElementById(decodeURIComponent(hash).slice(1));
            }
            catch (e) {
                console.warn(e);
            }
            if (retryTarget) {
                scrollTo(retryTarget, hash);
            }
        }, delay);
    }
}`

// VitePress route navigation normally writes /Some-Page to history. Offline
// docs must keep the single HTML file and put the route after '#/' instead.
const routePushStatePatch = `const oldURL = location.href;
            const offlineHref = toOfflineBrowserHref(href);
            history.pushState({}, '', offlineHref);
            if (new URL(oldURL).hash !== new URL(offlineHref, location.href).hash) {
                window.dispatchEvent(new HashChangeEvent('hashchange', {
                    oldURL,
                    newURL: location.href
                }));
            }`

const anchorPushStatePatch = `const offlineHref = toOfflineAnchorBrowserHref(route.path, hash);
                        history.pushState({}, '', offlineHref);
                        // still emit the event so we can listen to it in themes
                        window.dispatchEvent(new HashChangeEvent('hashchange', {
                            oldURL: currentUrl.href,
                            newURL: offlineHref
                        }));`

// Links like file.htm#/Page#anchor are valid for Ctrl-click/open-in-new-tab, but
// normal clicks on the current page should stay instant and skip router.go().
const hashRouteClickPatch = `if (origin === currentUrl.origin && treatAsHtml(pathname)) {
                e.preventDefault();
                if (hash.startsWith('#/') && pathname === currentUrl.pathname && search === currentUrl.search) {
                    const targetHref = normalizeHref(hash);
                    const targetLoc = new URL(targetHref, fakeHost);
                    // Same-page outline/header anchors should scroll immediately.
                    // Calling go() here reloads the current page module and feels laggy.
                    if (targetLoc.pathname === route.path) {
                        const offlineHref = location.pathname + location.search + hash;
                        if (hash !== currentUrl.hash) {
                            history.pushState({}, '', offlineHref);
                            window.dispatchEvent(new HashChangeEvent('hashchange', {
                                oldURL: currentUrl.href,
                                newURL: location.href
                            }));
                        }
                        if (targetLoc.hash) {
                            scrollTo(link, targetLoc.hash, link.classList.contains('header-anchor'));
                        }
                        else {
                            window.scrollTo(0, 0);
                        }
                        return;
                    }
                    go(targetHref);
                    return;
                }`

// VitePress builds the right aside outline from raw heading anchors. These
// helpers keep those links route-aware in the offline URL scheme.
const outlineHelpers = `${normalizeOfflineHashHelper}
function getOfflineRouteHash() {
    if (!location.hash.startsWith('#/')) {
        return '';
    }
    const routeHash = location.hash.slice(1);
    const anchorIndex = routeHash.indexOf('#');
    return anchorIndex >= 0 ? routeHash.slice(0, anchorIndex) : routeHash;
}
function toOfflineOutlineLink(hash) {
    // Right aside links are generated from raw heading IDs. Make them usable from
    // file:// and Ctrl-click by preserving the current offline route before the anchor.
    if (!hash || !hash.startsWith('#')) {
        return hash;
    }
    if (hash.startsWith('#/')) {
        return location.pathname + location.search + hash;
    }
    const routeHash = getOfflineRouteHash();
    return routeHash ? location.pathname + location.search + '#' + routeHash + hash : hash;
}`

// Components should still see useData().hash as a pure anchor. The #/Page part
// is only a transport detail for the offline router.
const appDataPatch = `const data = initData(router.route);
    const rawHash = data.hash;
    ${normalizeOfflineHashHelper}
    // Expose only the anchor part to components that consume useData().hash.
    // The offline route prefix is an implementation detail of the single-file build.
    data.hash = {
        get value() {
            return normalizeOfflineHash(rawHash.value);
        }
    };
    globalThis.__PHOBOS_VITEPRESS_DATA__ = data;`

// VitePress' lean initial module is an optimization for regular multi-file
// builds. Offline docs bundle complete page modules into the runtime instead.
const disableLeanInitialLoadPatch = `// The offline runtime bundles full page modules into one file. Lean modules
            // are separate dynamic chunks in regular VitePress builds.
            if (false) {
                pageFilePath = pageFilePath.replace(/\\.js$/, '.lean.js');
            }`

// Initial hash scrolling needs a retry loop because the page component is
// loaded from the offline runtime and the target heading can appear after mount.
const initialHashScrollPatch = `// scroll to hash after the offline app is mounted. The initial router.go()
            // runs before mount, so deep-link anchors inside #/page#anchor are
            // not in the DOM yet at that point.
            if (location.hash) {
                const hashValue = location.hash.startsWith('#/') ? location.hash.slice(1) : location.hash;
                const anchorIndex = hashValue.indexOf('#');
                const anchorHash = location.hash.startsWith('#/') && anchorIndex >= 0
                    ? hashValue.slice(anchorIndex)
                    : location.hash;
                const scrollToAnchor = () => {
                    if (!anchorHash || anchorHash === '#/') {
                        return;
                    }
                    let target = null;
                    try {
                        target = document.getElementById(decodeURIComponent(anchorHash).slice(1));
                    }
                    catch (e) {
                        console.warn(e);
                    }
                    if (target) {
                        scrollTo(target, anchorHash);
                    }
                };
                scrollToAnchor();
                for (const delay of ${offlineRetryDelays}) {
                    setTimeout(scrollToAnchor, delay);
                }
            }`

function normalizeModuleId(id: string): string {
  return id.split('?')[0].replace(/\\/gu, '/')
}

function replaceRequired(source: string, search: string | RegExp, replacement: string, patchName: string): string {
  const rewritten = source.replace(search, replacement)

  if (rewritten === source) {
    // VitePress is patched by matching its compiled client code. Failing loudly
    // makes upstream changes visible instead of silently producing broken offline docs.
    throw new Error(`VitePress offline patch failed: ${patchName}`)
  }

  return rewritten
}

function applyRequiredPatches(source: string, patches: VitePressPatch[]): string {
  return patches.reduce((output, patch) => replaceRequired(output, patch.search, patch.replacement, patch.name), source)
}

function vitePressPatch(name: string, search: string | RegExp, replacement: string): VitePressPatch {
  return { name, replacement, search }
}

// Static assets still live next to the offline HTML payload, while internal
// documentation links must route through the hash-based single-page shell.
const vitePressUtilsPatches = [
  vitePressPatch(
    'client/app/utils.js withBase',
    `export function withBase(path) {
    return EXTERNAL_URL_RE.test(path) || !path.startsWith('/')
        ? path
        : joinPath(siteDataRef.value.base, path);
}`,
    `export function withBase(path) {
    if (EXTERNAL_URL_RE.test(path) || !path.startsWith('/'))
        return path;
    if (path.startsWith('/favicon.') || path.startsWith('/images/') || path.startsWith('/assets/'))
        return './${offlineFilesDirName}/' + path.split('/').pop();
    return '#' + joinPath(siteDataRef.value.base, path);
}`,
  ),
]

function patchVitePressUtils(source: string): string {
  return applyRequiredPatches(source, vitePressUtilsPatches)
}

// Active nav/link checks receive offline hash URLs in several places. Normalize
// those back to VitePress' expected route/anchor shape before matching.
const vitePressSharedPatches = [
  vitePressPatch(
    'client/shared.js isActive offline match path',
    `export function isActive(currentPath, matchPath, asRegex = false) {
    if (matchPath === undefined) {
        return false;
    }`,
    `${normalizeOfflineHashHelper}
function normalizeOfflineMatchPath(matchPath) {
    if (typeof matchPath !== 'string' || !matchPath.startsWith('#/')) {
        return matchPath;
    }
    const routeHash = matchPath.slice(1);
    const anchorIndex = routeHash.indexOf('#');
    return anchorIndex >= 0 ? routeHash.slice(0, anchorIndex) : routeHash;
}
export function isActive(currentPath, matchPath, asRegex = false) {
    if (matchPath === undefined) {
        return false;
    }
    matchPath = normalizeOfflineMatchPath(matchPath);`,
  ),
  vitePressPatch(
    'client/shared.js isActive offline hash',
    `return (inBrowser ? location.hash : '') === hashMatch[0];`,
    `return (inBrowser ? normalizeOfflineHash(location.hash) : '') === hashMatch[0];`,
  ),
]

function patchVitePressShared(source: string): string {
  return applyRequiredPatches(source, vitePressSharedPatches)
}

// Router patches are the core of offline navigation: they translate VitePress'
// pathname-based routing into single-file hash routing and rewrite rendered
// markdown links after every page load.
const vitePressRouterPatches = [
  vitePressPatch(
    'client/app/router.js normalizeHref',
    `function normalizeHref(href) {
    const url = new URL(href, fakeHost);
    url.pathname = url.pathname.replace(/(^|\\/)index(\\.html)?$/, '$1');
    // ensure correct deep link so page refresh lands on correct files.
    if (siteDataRef.value.cleanUrls)
        url.pathname = url.pathname.replace(/\\.html$/, '');
    else if (!url.pathname.endsWith('/') && !url.pathname.endsWith('.html'))
        url.pathname += '.html';
    return url.pathname + url.search + url.hash;
}`,
    offlineRouterHelpers,
  ),
  vitePressPatch(
    'client/app/router.js document links',
    `if (targetLoc.hash && !scrollPosition) {`,
    `rewriteOfflineDocumentLinks(route.path);
                        if (targetLoc.hash && !scrollPosition) {`,
  ),
  vitePressPatch('client/app/router.js route pushState', `history.pushState({}, '', href);`, routePushStatePatch),
  vitePressPatch(
    'client/app/router.js replaceState href',
    `history.replaceState({}, '', href);`,
    `history.replaceState({}, '', toOfflineBrowserHref(href));`,
  ),
  vitePressPatch(
    'client/app/router.js anchor pushState',
    `history.pushState({}, '', href);
                        // still emit the event so we can listen to it in themes
                        window.dispatchEvent(new HashChangeEvent('hashchange', {
                            oldURL: currentUrl.href,
                            newURL: href
                        }));`,
    anchorPushStatePatch,
  ),
  vitePressPatch(
    'client/app/router.js delayed hash scroll',
    `scrollTo(target, targetLoc.hash);
                                return;`,
    `scrollToOfflineHashTarget(target, targetLoc.hash);
                                return;`,
  ),
  vitePressPatch(
    'client/app/router.js hash route click',
    `if (origin === currentUrl.origin && treatAsHtml(pathname)) {
                e.preventDefault();`,
    hashRouteClickPatch,
  ),
]

function patchVitePressRouter(source: string): string {
  return applyRequiredPatches(source, vitePressRouterPatches)
}

// The right aside outline is generated outside markdown content, so it needs
// its own route-aware links and hashchange handling.
const outlinePatches = [
  vitePressPatch(
    'theme-default/composables/outline.js offline hash normalizer',
    `const resolvedHeaders = [];`,
    `const resolvedHeaders = [];
${outlineHelpers}`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js route-aware outline link',
    `link: '#' + el.id,`,
    `link: toOfflineOutlineLink('#' + el.id),`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js active hash',
    `activateLink(location.hash);`,
    `activateLink(normalizeOfflineHash(location.hash));`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js active outline link selector',
    `prevActiveLink = container.value.querySelector(\`a[href="\${decodeURIComponent(hash)}"]\`);`,
    `prevActiveLink = container.value.querySelector(\`a[href="\${decodeURIComponent(toOfflineOutlineLink(hash))}"]\`);`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js hashchange handler',
    `let prevActiveLink = null;`,
    `let prevActiveLink = null;
    const onHashChange = () => activateLink(normalizeOfflineHash(location.hash));`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js hashchange listener',
    `window.addEventListener('scroll', onScroll);`,
    `window.addEventListener('scroll', onScroll);
        window.addEventListener('hashchange', onHashChange);`,
  ),
  vitePressPatch(
    'theme-default/composables/outline.js remove hashchange listener',
    `window.removeEventListener('scroll', onScroll);`,
    `window.removeEventListener('scroll', onScroll);
        window.removeEventListener('hashchange', onHashChange);`,
  ),
]

function patchOutline(source: string): string {
  return applyRequiredPatches(source, outlinePatches)
}

// App bootstrap patches keep the offline runtime self-contained: no SSR app,
// no prefetching/dynamic page chunks, and anchor scrolling after mount.
const vitePressAppPatches = [
  vitePressPatch(
    'client/app/index.js prefetch',
    `if (import.meta.env.PROD && site.value.router.prefetchLinks) {`,
    `if (false) {`,
  ),
  vitePressPatch(
    'client/app/index.js expose router',
    `const router = newRouter();`,
    `const router = newRouter();
    globalThis.__PHOBOS_VITEPRESS_ROUTER__ = router;`,
  ),
  vitePressPatch('client/app/index.js expose data', `const data = initData(router.route);`, appDataPatch),
  vitePressPatch(
    'client/app/index.js client app',
    `? createSSRApp(VitePressApp)
        : createClientApp(VitePressApp);`,
    `? createClientApp(VitePressApp)
        : createClientApp(VitePressApp);`,
  ),
  vitePressPatch(
    'client/app/index.js lean initial load',
    `if (isInitialPageLoad) {
                pageFilePath = pageFilePath.replace(/\\.js$/, '.lean.js');
            }`,
    disableLeanInitialLoadPatch,
  ),
  vitePressPatch(
    'client/app/index.js offline initial hash scroll',
    `// scroll to hash on new tab during dev
            if (import.meta.env.DEV && location.hash) {
                const target = document.getElementById(decodeURIComponent(location.hash).slice(1));
                if (target) {
                    scrollTo(target, location.hash);
                }
            }`,
    initialHashScrollPatch,
  ),
  vitePressPatch(
    'client/app/index.js page loader',
    `pageModule = import(/*@vite-ignore*/ pageFilePath);`,
    `pageModule = globalThis.__PHOBOS_OFFLINE_LOAD_PAGE__(pageFilePath);`,
  ),
]

function patchVitePressApp(source: string): string {
  return applyRequiredPatches(source, vitePressAppPatches)
}

// Search excerpts lazy-load page modules in regular VitePress. Offline docs
// load them from the bundled runtime map instead.
const localSearchBoxPatches = [
  vitePressPatch(
    'theme-default/VPLocalSearchBox.vue excerpt loader',
    /return\s+\{\s*id,\s*mod:\s*await\s+import\(\s*\/\*@vite-ignore\*\/\s*file\s*\)\s*\}/u,
    `return { id, mod: await globalThis.__PHOBOS_OFFLINE_LOAD_PAGE__(file) }`,
  ),
  vitePressPatch('theme-default/VPLocalSearchBox.vue result href', `:href="p.id"`, `:href="'#' + p.id"`),
]

function patchLocalSearchBox(source: string): string {
  return applyRequiredPatches(source, localSearchBoxPatches)
}

// The search box component itself must be statically included; otherwise Vite
// would emit an async chunk that is removed from the offline payload.
const navBarSearchPatches = [
  vitePressPatch(
    'theme-default/VPNavBarSearch.vue static search import',
    `import VPNavBarSearchButton from './VPNavBarSearchButton.vue'`,
    `import VPNavBarSearchButton from './VPNavBarSearchButton.vue'
// Avoid an async component chunk: offline docs keep the client runtime in one JS file.
import VPLocalSearchBoxOffline from './VPLocalSearchBox.vue'`,
  ),
  vitePressPatch(
    'theme-default/VPNavBarSearch.vue local search component',
    `const VPLocalSearchBox = __VP_LOCAL_SEARCH__
  ? defineAsyncComponent(() => import('./VPLocalSearchBox.vue'))
  : () => null`,
    `const VPLocalSearchBox = __VP_LOCAL_SEARCH__
  ? VPLocalSearchBoxOffline
  : () => null`,
  ),
]

function patchNavBarSearch(source: string): string {
  return applyRequiredPatches(source, navBarSearchPatches)
}

// Vite's transform hook is called for many modules. Keep the dispatch table
// narrow so patches only run against known compiled VitePress client files.
const vitePressModulePatchers: VitePressModulePatcher[] = [
  {
    match: moduleId => moduleId.endsWith('/shared.js'),
    patch: patchVitePressShared,
  },
  {
    match: moduleId => moduleId.endsWith('/app/utils.js'),
    patch: patchVitePressUtils,
  },
  {
    match: moduleId => moduleId.endsWith('/app/router.js'),
    patch: patchVitePressRouter,
  },
  {
    match: moduleId => moduleId.endsWith('/app/index.js'),
    patch: patchVitePressApp,
  },
  {
    match: (moduleId, id) => moduleId.endsWith('/theme-default/components/VPLocalSearchBox.vue') && !id.includes('?'),
    patch: patchLocalSearchBox,
  },
  {
    match: (moduleId, id) => moduleId.endsWith('/theme-default/components/VPNavBarSearch.vue') && !id.includes('?'),
    patch: patchNavBarSearch,
  },
  {
    match: moduleId => moduleId.endsWith('/theme-default/composables/outline.js'),
    patch: patchOutline,
  },
]

export function phobosOfflineVitePressPlugin(): Plugin {
  let isSsrBuild = false

  return {
    name: 'phobos-offline-vitepress',
    apply: 'build',
    enforce: 'pre',
    configResolved(config: ResolvedConfig) {
      isSsrBuild = Boolean(config.build.ssr)
    },
    transform(source: string, id: string) {
      if (isSsrBuild) {
        return null
      }

      const moduleId = normalizeModuleId(id)

      if (!moduleId.includes(vitePressClientMarker)) {
        return null
      }

      const modulePatcher = vitePressModulePatchers.find(patcher => patcher.match(moduleId, id))

      return modulePatcher ? modulePatcher.patch(source) : null
    },
  }
}
