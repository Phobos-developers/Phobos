import { h, nextTick, provide, type App, type Ref } from 'vue'
import { useData } from 'vitepress'
import DefaultTheme from 'vitepress/theme'
import CustomGameSpeedGenerator from './components/CustomGameSpeedGenerator.vue'
import './custom.css'

type ViewTransitionDocument = Document & {
  startViewTransition?: (callback: () => void | Promise<void>) => {
    ready: Promise<void>
  }
}

type PhobosWindow = Window & {
  __PHOBOS_SEARCH_HOTKEY_INSTALLED__?: boolean
  __PHOBOS_RTD_FILETREEDIFF_POSITION_PATCH_INSTALLED__?: boolean
}

function isSearchKeyboardShortcut(event: KeyboardEvent) {
  return (event.code === 'KeyK' || event.key.toLowerCase() === 'k') && (event.metaKey || event.ctrlKey)
}

function installPhysicalSearchHotKey() {
  const phobosWindow = window as PhobosWindow

  if (phobosWindow.__PHOBOS_SEARCH_HOTKEY_INSTALLED__) {
    return
  }

  phobosWindow.__PHOBOS_SEARCH_HOTKEY_INSTALLED__ = true

  window.addEventListener('keydown', event => {
    if (!isSearchKeyboardShortcut(event)) {
      return
    }

    const searchButton = document.querySelector<HTMLButtonElement>('#local-search .DocSearch-Button')
    if (!searchButton) {
      return
    }

    event.preventDefault()
    searchButton.click()
  })
}

function shouldPatchReadTheDocsFileTreeDiffPosition() {
  return window.location.hostname.endsWith('.readthedocs.build')
}

function patchReadTheDocsFileTreeDiffPosition(host: Element) {
  const shadowRoot = host.shadowRoot

  if (!shadowRoot || shadowRoot.querySelector('[data-phobos-rtd-filetreediff-position]')) {
    return
  }

  const style = document.createElement('style')
  style.dataset.phobosRtdFiletreediffPosition = 'true'
  style.textContent = `
    :host > div {
      top: 72px !important;
      right: 16px !important;
      border-radius: 6px !important;
    }

    @media (max-width: 768px) {
      :host > div {
        top: 64px !important;
      }
    }
  `

  shadowRoot.append(style)
}

function installReadTheDocsFileTreeDiffPositionPatch() {
  const phobosWindow = window as PhobosWindow

  if (
    phobosWindow.__PHOBOS_RTD_FILETREEDIFF_POSITION_PATCH_INSTALLED__ ||
    !shouldPatchReadTheDocsFileTreeDiffPosition()
  ) {
    return
  }

  phobosWindow.__PHOBOS_RTD_FILETREEDIFF_POSITION_PATCH_INSTALLED__ = true

  const patchAll = () => {
    document.querySelectorAll('readthedocs-filetreediff').forEach(patchReadTheDocsFileTreeDiffPosition)
  }

  patchAll()

  new MutationObserver(patchAll).observe(document.documentElement, {
    childList: true,
    subtree: true,
  })
}

function shouldUseViewTransition() {
  return (
    typeof window !== 'undefined' &&
    typeof document !== 'undefined' &&
    typeof (document as ViewTransitionDocument).startViewTransition === 'function' &&
    !window.matchMedia('(prefers-reduced-motion: reduce)').matches
  )
}

function getViewTransitionOrigin(event?: MouseEvent) {
  if (event) {
    return {
      x: event.clientX,
      y: event.clientY,
    }
  }

  return {
    x: window.innerWidth - 48,
    y: 32,
  }
}

function getViewTransitionEndRadius(x: number, y: number) {
  return Math.hypot(Math.max(x, window.innerWidth - x), Math.max(y, window.innerHeight - y))
}

function toggleAppearanceWithTransition(isDark: Ref<boolean>, event?: MouseEvent) {
  if (!shouldUseViewTransition()) {
    isDark.value = !isDark.value
    return
  }

  const { x, y } = getViewTransitionOrigin(event)
  const endRadius = getViewTransitionEndRadius(x, y)

  const transition = (document as ViewTransitionDocument).startViewTransition?.(async () => {
    isDark.value = !isDark.value
    await nextTick()
  })

  transition?.ready.then(() => {
    document.documentElement.animate(
      [{ clipPath: `circle(0px at ${x}px ${y}px)` }, { clipPath: `circle(${endRadius}px at ${x}px ${y}px)` }],
      {
        duration: 300,
        easing: 'cubic-bezier(.22, 1, .36, 1)',
        pseudoElement: '::view-transition-new(root)',
      },
    )
  })
}

const Layout = {
  name: 'PhobosThemeLayout',
  setup() {
    const { isDark } = useData()

    provide('toggle-appearance', (event?: MouseEvent) => {
      toggleAppearanceWithTransition(isDark, event)
    })

    return () => h(DefaultTheme.Layout)
  },
}

export default {
  extends: DefaultTheme,
  Layout,
  enhanceApp({ app }: { app: App }) {
    app.component('CustomGameSpeedGenerator', CustomGameSpeedGenerator)

    if (typeof window !== 'undefined') {
      installPhysicalSearchHotKey()
      installReadTheDocsFileTreeDiffPositionPatch()
    }
  },
}
