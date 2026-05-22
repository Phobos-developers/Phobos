import js from '@eslint/js'
import globals from 'globals'
import tseslint from 'typescript-eslint'

export default tseslint.config(
  {
    name: 'docs/ignores',
    ignores: [
      'node_modules/**',
      '.vitepress/cache/**',
      '.vitepress/.temp/**',
      '.artifacts/**',
      '.cache/**',
      '.temp/**',
      'vitepress/generated/**',
      'zh_CN/**',
      '**/*.d.ts',
    ],
  },
  {
    name: 'docs/files',
    files: ['**/*.{js,mjs,cjs,ts,mts,cts,vue}'],
    languageOptions: {
      ecmaVersion: 'latest',
      sourceType: 'module',
      globals: {
        ...globals.browser,
        ...globals.node,
      },
    },
  },
  js.configs.recommended,
  ...tseslint.configs.recommended,
)
