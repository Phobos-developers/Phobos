<template>
  <div class="custom-game-speed-generator">
    <label class="custom-game-speed-generator__label" for="custom-game-speed-generator-input">
      {{ localizedMessages.desiredFpsLabel }}
    </label>
    <input
      id="custom-game-speed-generator-input"
      v-model.number="desiredFps"
      class="custom-game-speed-generator__input"
      max="60"
      min="10"
      step="1"
      type="number"
    />
    <p class="custom-game-speed-generator__caption">{{ localizedMessages.resultsCaption }}</p>
    <div v-if="highlightedCodeHtml" class="language-ini vp-adaptive-theme">
      <button :aria-label="localizedMessages.copyCode" :title="localizedMessages.copyCode" class="copy"></button>
      <span class="lang">ini</span>
      <pre
        class="shiki shiki-themes github-light github-dark vp-code"
        tabindex="0"
      ><code v-html="highlightedCodeHtml" />
    </pre>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, ref, watch } from 'vue'
import { useData } from 'vitepress'
import { createHighlighterCore } from 'shiki/core'
import { createJavaScriptRegexEngine } from 'shiki/engine/javascript'
import ini from '@shikijs/langs/ini'
import githubDark from '@shikijs/themes/github-dark'
import githubLight from '@shikijs/themes/github-light'

const localizedStrings = {
  'en-US': {
    desiredFpsLabel: 'Enter desired FPS',
    resultsCaption: 'Results (remember to replace N with your game speed number!):',
    copyCode: 'Copy Code',
    noResults: "Sorry, couldn't find anything!",
    or: 'Or',
  },
  'zh-CN': {
    desiredFpsLabel: '输入所需的 FPS',
    resultsCaption: '结果（别忘了把 N 替换成你的游戏速度编号）：',
    copyCode: '复制代码',
    noResults: '抱歉，未找到任何结果！',
    or: '或',
  },
} as const

const fallbackLocale = 'en-US'
const minGameSpeedDelay = 0
const maxStableGameSpeedDelay = 5
const minChangeInterval = 1
const maxChangeInterval = 40

type LocalizationLocale = keyof typeof localizedStrings

type GameSpeedMatch = {
  defaultDelay: number
  changeDelay: number
  changeInterval: number
}

const { lang } = useData()
const desiredFps = ref<number>(30)

const highlighterPromise = createHighlighterCore({
  langs: [ini],
  themes: [githubLight, githubDark],
  engine: createJavaScriptRegexEngine(),
})

const localizedMessages = computed(() => {
  return localizedStrings[lang.value as LocalizationLocale] || localizedStrings[fallbackLocale]
})

const matchingGameSpeedSettings = computed(() => {
  return findMatchingGameSpeedSettings(desiredFps.value)
})

const resultText = computed(() => {
  if (!matchingGameSpeedSettings.value.length) {
    return `; ${localizedMessages.value.noResults}`
  }

  const settings = matchingGameSpeedSettings.value
    .map((match, index) => formatGameSpeedMatch(match, index, localizedMessages.value.or))
    .join('\n')

  return `[General]\nCustomGS=true\n;\n${settings}`
})
const highlightedCodeHtml = ref<String | null>(null)

let highlightRequestId = 0
watch(
  resultText,
  async (code: string) => {
    // Shiki is loaded asynchronously; ignore stale highlights if the user
    // changes FPS again before the previous highlight request finishes.
    const requestId = (highlightRequestId += 1)
    highlightedCodeHtml.value = escapeHtml(code)

    const highlighter = await highlighterPromise
    const highlightedHtml = highlighter.codeToHtml(code, {
      lang: 'ini',
      themes: {
        light: 'github-light',
        dark: 'github-dark',
      },
    })

    if (requestId !== highlightRequestId) {
      return
    }

    highlightedCodeHtml.value = normalizeVitePressShikiTokenStyles(extractCodeHtml(highlightedHtml, code))
  },
  { immediate: true },
)

function findMatchingGameSpeedSettings(fps: number): GameSpeedMatch[] {
  if (!Number.isFinite(fps)) {
    return []
  }

  const result: GameSpeedMatch[] = []

  // The game accepts delay values, not FPS directly. Try every stable
  // combination and keep the ones that round to the requested frame rate.
  for (let defaultDelay = minGameSpeedDelay; defaultDelay <= maxStableGameSpeedDelay; defaultDelay += 1) {
    for (let changeDelay = minGameSpeedDelay; changeDelay <= maxStableGameSpeedDelay; changeDelay += 1) {
      for (let changeInterval = minChangeInterval; changeInterval <= maxChangeInterval; changeInterval += 1) {
        if (Math.round(calculateFps(changeDelay, defaultDelay, changeInterval)) === fps) {
          result.push({ defaultDelay, changeDelay, changeInterval })
        }
      }
    }
  }

  return result
}

function formatGameSpeedMatch(match: GameSpeedMatch, index: number, orLabel: string): string {
  const lines = [
    `CustomGSN.DefaultDelay=${match.defaultDelay}`,
    `CustomGSN.ChangeDelay=${match.changeDelay}`,
    `CustomGSN.ChangeInterval=${match.changeInterval}`,
  ]

  if (index > 0) {
    lines.unshift(`; -- ${orLabel} --`)
  }

  return lines.join('\n')
}

function escapeHtml(value: string): string {
  return value.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;')
}

function extractCodeHtml(html: string, fallbackCode: string): string {
  return html.match(/<code>([\s\S]*)<\/code>/)?.[1] || escapeHtml(fallbackCode)
}

function normalizeVitePressShikiTokenStyles(html: string): string {
  return html.replace(/style="([^"]*)"/g, (_match: string, style: string) => {
    const declarations = style
      .split(';')
      .map(declaration => declaration.trim())
      .filter(Boolean)
    const normalizedDeclarations = []
    let lightColor: string | null = null

    for (const declaration of declarations) {
      const separatorIndex = declaration.indexOf(':')

      if (separatorIndex === -1) {
        continue
      }

      const property = declaration.slice(0, separatorIndex).trim()
      const value = declaration.slice(separatorIndex + 1).trim()

      // VitePress themes expect Shiki colors in CSS variables so the same
      // generated markup can switch between light and dark mode.
      if (property === 'color') {
        lightColor = value
        continue
      }

      normalizedDeclarations.push(`${property}:${value}`)
    }

    if (lightColor) {
      normalizedDeclarations.unshift(`--shiki-light:${lightColor}`)
    }

    return `style="${normalizedDeclarations.join(';')};"`
  })
}

function calculateFps(changeDelay: number, defaultDelay: number, changeInterval: number): number {
  // CustomGS alternates one changed-delay frame with a run of default-delay
  // frames. This weighted average estimates the resulting visible FPS.
  return (
    (60 / (6 - changeDelay) + (60 / (6 - defaultDelay)) * ((changeInterval - 1) / (6 - changeDelay))) /
    (1 + (changeInterval - 1) / (6 - changeDelay))
  )
}
</script>

<style scoped>
.custom-game-speed-generator {
  display: grid;
  gap: 12px;
}

.custom-game-speed-generator__label {
  color: var(--vp-c-text-1);
  font-weight: 600;
}

.custom-game-speed-generator__input {
  width: 100%;
  border: 1px solid var(--vp-c-divider);
  border-radius: 6px;
  padding: 8px 10px;
  background: var(--vp-c-bg);
  color: var(--vp-c-text-1);
  font: inherit;
}

.custom-game-speed-generator__input:focus {
  border-color: var(--vp-c-brand-1);
  outline: 2px solid var(--vp-c-brand-soft);
}

.custom-game-speed-generator__caption {
  margin: 0;
}

.custom-game-speed-generator :deep(pre.vp-code) {
  max-height: 360px;
  overflow: auto;
}

.custom-game-speed-generator :deep(.language-ini > button.copy) {
  right: 28px;
}

.custom-game-speed-generator :deep(.language-ini > span.lang) {
  right: 24px;
}
</style>
