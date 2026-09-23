---
name: 构建CHM Build CHM
description: "把 MJobos 的中文 Sphinx 文档（docs/locale/zh_CN 翻译）构建为在 HTML Help 查看器中正常显示的 CHM 文件。当用户要求将中文文档转为 CHM / chm / 帮助文件 / htmlhelp，或需要重建、更新 docs/_build/htmlhelp_zh_CN 产物时使用。覆盖 venv 搭建、classic 主题切换、sphinxcontrib-htmlhelp 补丁、JS 剥离、.hhp 二进制过滤、hhc 编译与强制验证。"
---

# 构建中文文档 CHM

## 概述

`sphinx-build -b htmlhelp` + `hhc.exe` 直接构建的 CHM 在中文环境下有四个坑，必须按本流程处理，否则产物不可用：

1. `sphinx_rtd_theme` 在 CHM 查看器的旧 MSHTML（IE7 兼容模式）下 `position: fixed` 失效，侧栏掉到页面顶部 → 必须换 `classic` 主题。
2. 页面 JS 全部是 ES6（Sphinx 7 自带 + 项目自带），旧 MSHTML 解析即弹"语法错误" → 必须剥离全部 `<script>`。
3. `index.md` 用 `:hidden:` toctree，htmlhelp 默认不解析 hidden toctree，产物 `.hhc` 为空、查看器 Contents 面板全白 → 必须打补丁。
4. hhc 逐个解析 `.hhp [FILES]` 里的二进制文件会崩溃（0xC0000005）→ 必须过滤。

## 前置条件

- 文档源：`docs/`（Sphinx 7.4 + MyST + `locale/zh_CN` 中文翻译）。
- 编译工具：`C:\Program Files (x86)\HTML Help Workshop\hhc.exe`。缺失时提示用户安装 HTML Help Workshop，不要静默跳过。
- Python 3.13+。**必须在 venv 中安装依赖**：沙箱会拒绝直接 `pip install` 到用户目录。

## 流程（5 步，顺序固定）

以下命令默认在仓库根目录执行。

### Step 0 环境

```powershell
python -m venv $env:TEMP\mjobos-docs-venv
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe -m pip install -r docs/requirements.txt
```

venv 已存在且依赖完好时跳过。若 `docs/requirements.txt` 变更必须重装。

### Step 1 给 venv 的 sphinxcontrib-htmlhelp 打补丁（幂等，两步都必需）

```powershell
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe -c "import sphinxcontrib.htmlhelp as m; print(m.__file__)"
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe .agents/skills/build-chm/patch_sphinxcontrib_htmlhelp.py <上一步输出的路径>
```

- 补丁 A：主题页输出 UTF-8。默认按 LCID 表写成 cp936，与主题模板硬编码的 `<meta charset="utf-8">` 矛盾，导致标题乱码、全文搜索索引损坏。
- 补丁 B：`build_toc_file` 传 `includehidden=True`。否则 `:hidden:` toctree 被解析为空（`sphinx/environment/adapters/toctree.py` 的 `hidden and not includehidden -> None`），`.hhc` 只有数百字节，CHM 目录面板全空。

脚本找不到目标模式会报错退出——不要手动绕过，改用编辑器核对 `update_page_context` / `build_toc_file` 的实际代码后再修脚本。

### Step 2 构建（主题必须 classic）

```powershell
& $env:TEMP\mjobos-docs-venv\Scripts\sphinx-build.exe -E -b htmlhelp -D language=zh_CN -D html_theme=classic docs docs/_build/htmlhelp_zh_CN
```

- 不要用 `sphinx_rtd_theme`：CHM 查看器不支持 fixed 布局，页内目录会错位。
- htmlhelp 构建器 `embedded=True`，设计即**不在页内嵌侧栏**：左侧目录由 CHM 查看器原生 Contents 面板承担，主题页只保留顶部导航栏（面包屑/上一页/下一页/总索引）。不需要、也不要去页面里加 sidebar。
- 构建输出里的 40 余条 `local id not found` / 术语引用警告是中文 `.po` 锚点与英文原文不一致导致的既有问题，非阻断，不要在本次流程内修。

### Step 3 后处理

```powershell
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe .agents/skills/build-chm/strip_scripts.py docs/_build/htmlhelp_zh_CN
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe .agents/skills/build-chm/filter_hhp.py docs/_build/htmlhelp_zh_CN/phobosdoc.hhp
```

- `strip_scripts.py`：删除全部 `<script>`（外链与内联），仅保留随脚本自带的 `html5shiv.min.js`（ES5，注入 `<head>`，保证 `<section>` 等 HTML5 元素在旧 MSHTML 下按块级渲染）。CHM 的搜索由查看器原生提供，不依赖页面 JS。
- `filter_hhp.py`：从 `.hhp` 的 `[FILES]` 删除二进制资产行（png/gif/jpg/eot/ttf/woff 等），防止 hhc 崩溃；图片/字体经 HTML/CSS 引用由 hhc 自动打包（编译统计应有 60+ Graphics）。脚本按 cp936 读写 `.hhp`——它是 ANSI 编码，勿用 UTF-8 工具改写。

### Step 4 编译并改名

```powershell
& "C:\Program Files (x86)\HTML Help Workshop\hhc.exe" docs/_build/htmlhelp_zh_CN/phobosdoc.hhp
```

- 成功标志：`Topics` 数 > 100（目录树为空时只有 16）、`Created ... phobosdoc.chm, ... bytes`。
- hhc 退出码恒为 1（有警告时）；以有无 `HHC3003`/`error` 行和输出统计为准。
- 改名：`phobosdoc.chm` → `Phobos-中文文档.chm`。
- 若删除/改名报"文件被占用"：用户正在查看器里打开旧 CHM。保留新文件为 `phobosdoc.chm`，明确告知用户先关闭查看器，再改名。

### Step 5 验证（硬性门禁，向用户报告完成前必须执行）

```powershell
& $env:TEMP\mjobos-docs-venv\Scripts\python.exe .agents/skills/build-chm/verify_chm.py docs/_build/htmlhelp_zh_CN --chm docs/_build/htmlhelp_zh_CN/Phobos-中文文档.chm
```

校验：主题页 UTF-8 与中文标题、无残留 script、TOC 条目数与锚点目标全部存在、chm 为 ITSF 签名且 LCID 0x804。**未通过不得报告完成**；失败时按故障速查定位，修好后重跑 Step 2–5。

## 已知正常现象（勿当 bug 修）

- **Index 面板为空**：项目没有 `.. index::` 指令，网页端 genindex 同样为空。导航用 Contents 面板。
- **hhc Topics 数量远大于文档页数**：目录树把每个标题锚点都算作一个 topic，属正常。
- **CHM 约 100MB**：图片与字体较多，属正常。
- **`_build/` 已被 `.gitignore` 忽略**：产物不要提交。

## 故障速查

| 现象 | 原因 | 处置 |
|------|------|------|
| CHM Contents/目录面板空白 | `.hhc` 空：hidden toctree 未解析 | Step 1 补丁 B |
| 打开页面弹"语法错误" | ES6 script 残留 | Step 3 `strip_scripts.py` |
| hhc 编译崩溃 0xC0000005 | `.hhp` 含二进制文件 | Step 3 `filter_hhp.py` |
| 侧栏/排版错乱、目录在页面顶部 | RTD 主题 fixed 布局 + 旧 MSHTML | Step 2 改用 classic |
| 页面标题乱码、搜索不到中文 | 主题页 cp936 与 meta utf-8 矛盾 | Step 1 补丁 A |
| 改名/删除 chm 失败 | 查看器占用旧文件 | 提示用户关闭后重试 |
| pip 安装失败/沙箱拒绝 | 直装用户目录被禁 | Step 0 用 venv |

## 脚本清单

| 脚本 | 用途 |
|------|------|
| `patch_sphinxcontrib_htmlhelp.py` | 幂等应用补丁 A/B（编码 + includehidden），退出码非 0 表示版本漂移需人工核对 |
| `strip_scripts.py` | 剥离 script、注入自带 html5shiv，并自检无残留 |
| `filter_hhp.py` | 删除 `.hhp [FILES]` 二进制行，cp936 安全读写 |
| `verify_chm.py` | 构建门禁验证（topics/toc/chm），失败退出码 1 |
| `html5shiv.min.js` | `strip_scripts.py` 内置资源，勿删 |
