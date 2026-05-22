<template>
  <div class="custom-game-speed-generator">
    <label class="custom-game-speed-generator__label" for="custom-game-speed-generator-input">
      Enter desired FPS
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
    <p class="custom-game-speed-generator__caption">Results (remember to replace N with your game speed number!):</p>
    <div class="language-ini vp-adaptive-theme">
      <button title="Copy Code" class="copy"></button>
      <span class="lang">ini</span>
      <pre
        class="shiki shiki-themes github-light github-dark vp-code"
        tabindex="0"
      ><code v-html="highlightedCodeHtml"></code></pre>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed, onBeforeUnmount, ref, watch } from 'vue'
import { createHighlighterCore } from 'shiki/core'
import { createJavaScriptRegexEngine } from 'shiki/engine/javascript'
import ini from '@shikijs/langs/ini'
import githubDark from '@shikijs/themes/github-dark'
import githubLight from '@shikijs/themes/github-light'

type GameSpeedMatch = {
  defaultDelay: number
  changeDelay: number
  changeInterval: number
}

const desiredFps = ref<number>(30)
const highlightedCodeHtml = ref<string>('')
let highlightRequestId = 0

const highlighterPromise = createHighlighterCore({
  langs: [ini],
  themes: [githubLight, githubDark],
  engine: createJavaScriptRegexEngine(),
})

function calculateFps(changeDelay: number, defaultDelay: number, changeInterval: number): number {
  return (
    (60 / (6 - changeDelay) + (60 / (6 - defaultDelay)) * ((changeInterval - 1) / (6 - changeDelay))) /
    (1 + (changeInterval - 1) / (6 - changeDelay))
  )
}

const matches = computed<GameSpeedMatch[]>(() => {
  const fps = Number(desiredFps.value)

  if (!Number.isFinite(fps)) {
    return []
  }

  const result: GameSpeedMatch[] = []

  for (let defaultDelay = 0; defaultDelay <= 5; defaultDelay += 1) {
    for (let changeDelay = 0; changeDelay <= 5; changeDelay += 1) {
      for (let changeInterval = 1; changeInterval <= 40; changeInterval += 1) {
        if (Math.round(calculateFps(changeDelay, defaultDelay, changeInterval)) === fps) {
          result.push({ defaultDelay, changeDelay, changeInterval })
        }
      }
    }
  }

  return result
})

const resultText = computed(() => {
  if (!matches.value.length) {
    return "// Sorry, couldn't find anything!"
  }

  return matches.value
    .map((match, index) => {
      const lines = [
        `CustomGSN.DefaultDelay=${match.defaultDelay}`,
        `CustomGSN.ChangeDelay=${match.changeDelay}`,
        `CustomGSN.ChangeInterval=${match.changeInterval}`,
      ]

      if (index > 0) {
        lines.unshift('// -- Or --')
      }

      return lines.join('\n')
    })
    .join('\n')
})

function escapeHtml(value: string): string {
  return value.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;')
}

function extractCodeHtml(html: string): string {
  return html.match(/<code>([\s\S]*)<\/code>/)?.[1] || escapeHtml(resultText.value)
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

const stopHighlightWatcher = watch(
  resultText,
  async code => {
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

    highlightedCodeHtml.value = normalizeVitePressShikiTokenStyles(extractCodeHtml(highlightedHtml))
  },
  { immediate: true },
)

onBeforeUnmount(stopHighlightWatcher)
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
