import { readFile } from 'node:fs/promises'
import { dirname, resolve } from 'node:path'
import type { Plugin } from 'vite'
import { imageSize } from 'image-size'
import { docsDir, normalizePath } from './shared/paths.ts'

type MediaDimensions = {
  width: number
  height: number
}

type Vint = {
  length: number
  value: number
}

const htmlVideoTagRegExp = /<video\b([^>]*)>/giu
const htmlImageTagRegExp = /<img\b([^>]*)>/giu
const markdownImageRegExp = /!\[([^\]]*)\]\(([^)\s]+)(?:\s+"[^"]*")?\)/gu
const markdownVideoLinkRegExp = /(?<!!)\[([^\]]*)\]\(([^)\s]+?\.(?:webm|mp4))(?:\s+"[^"]*")?\)/giu
const htmlAttributeRegExp = /\s([^\s=]+)(?:=(?:"([^"]*)"|'([^']*)'|([^\s"'=<>`]+)))?/gu
const videoDimensionsCache = new Map<string, Promise<MediaDimensions | null>>()
const imageDimensionsCache = new Map<string, Promise<MediaDimensions | null>>()
const markdownCaptionAfterHtmlMediaRegExp = /((?:<img\b[^>]*>|<video\b[^>]*><\/video>))\n(?=[_*])/giu

function readVint(buffer: Buffer, offset: number, keepMarker: boolean): Vint | null {
  if (offset >= buffer.length) {
    return null
  }

  const firstByte = buffer[offset]
  let marker = 0x80
  let length = 1

  while (length <= 8 && (firstByte & marker) === 0) {
    marker >>= 1
    length += 1
  }

  if (length > 8 || offset + length > buffer.length) {
    return null
  }

  let value = keepMarker ? firstByte : firstByte & (marker - 1)

  for (let index = 1; index < length; index += 1) {
    value = value * 256 + buffer[offset + index]
  }

  return { length, value }
}

function readUnsignedInteger(buffer: Buffer, start: number, end: number): number {
  let value = 0

  for (let index = start; index < end; index += 1) {
    value = value * 256 + buffer[index]
  }

  return value
}

function isTopLevelContainer(id: number): boolean {
  return id === 0x1a45dfa3 || id === 0x18538067 || id === 0x1654ae6b
}

function findWebmVideoDimensions(buffer: Buffer): MediaDimensions | null {
  const parseElements = (start: number, end: number): MediaDimensions | null => {
    let offset = start

    while (offset < end) {
      const id = readVint(buffer, offset, true)

      if (!id) {
        return null
      }

      const size = readVint(buffer, offset + id.length, false)

      if (!size) {
        return null
      }

      const dataStart = offset + id.length + size.length
      const dataEnd = Math.min(dataStart + size.value, end)

      if (id.value === 0xae) {
        const dimensions = parseTrackEntry(dataStart, dataEnd)

        if (dimensions) {
          return dimensions
        }
      } else if (isTopLevelContainer(id.value)) {
        const dimensions = parseElements(dataStart, dataEnd)

        if (dimensions) {
          return dimensions
        }
      }

      offset = dataEnd
    }

    return null
  }

  const parseTrackEntry = (start: number, end: number): MediaDimensions | null => {
    let offset = start
    let isVideoTrack = false
    let videoDimensions: MediaDimensions | null = null

    while (offset < end) {
      const id = readVint(buffer, offset, true)

      if (!id) {
        return null
      }

      const size = readVint(buffer, offset + id.length, false)

      if (!size) {
        return null
      }

      const dataStart = offset + id.length + size.length
      const dataEnd = Math.min(dataStart + size.value, end)

      if (id.value === 0x83) {
        isVideoTrack = readUnsignedInteger(buffer, dataStart, dataEnd) === 1
      } else if (id.value === 0xe0) {
        videoDimensions = parseVideoElement(dataStart, dataEnd)
      }

      offset = dataEnd
    }

    return isVideoTrack ? videoDimensions : null
  }

  const parseVideoElement = (start: number, end: number): MediaDimensions | null => {
    let offset = start
    let width: number | null = null
    let height: number | null = null

    while (offset < end) {
      const id = readVint(buffer, offset, true)

      if (!id) {
        return null
      }

      const size = readVint(buffer, offset + id.length, false)

      if (!size) {
        return null
      }

      const dataStart = offset + id.length + size.length
      const dataEnd = Math.min(dataStart + size.value, end)

      if (id.value === 0xb0) {
        width = readUnsignedInteger(buffer, dataStart, dataEnd)
      } else if (id.value === 0xba) {
        height = readUnsignedInteger(buffer, dataStart, dataEnd)
      }

      offset = dataEnd
    }

    return width && height ? { width, height } : null
  }

  return parseElements(0, buffer.length)
}

function separateMarkdownCaptionsFromHtmlMedia(source: string): string {
  return source.replace(markdownCaptionAfterHtmlMediaRegExp, '$1\n\n')
}

function readMp4BoxSize(buffer: Buffer, offset: number, end: number): { size: number; headerSize: number } | null {
  if (offset + 8 > end) {
    return null
  }

  const smallSize = buffer.readUInt32BE(offset)

  if (smallSize === 1) {
    if (offset + 16 > end) {
      return null
    }

    const largeSize = Number(buffer.readBigUInt64BE(offset + 8))

    return { size: largeSize, headerSize: 16 }
  }

  if (smallSize === 0) {
    return { size: end - offset, headerSize: 8 }
  }

  return { size: smallSize, headerSize: 8 }
}

function findMp4VideoDimensions(buffer: Buffer): MediaDimensions | null {
  type Mp4Box = {
    dataStart: number
    end: number
    type: string
  }

  const findChildBoxes = (start: number, end: number, type: string): Mp4Box[] => {
    const boxes: Mp4Box[] = []
    let offset = start

    while (offset + 8 <= end) {
      const size = readMp4BoxSize(buffer, offset, end)

      if (!size || size.size < size.headerSize) {
        break
      }

      const boxEnd = Math.min(offset + size.size, end)
      const boxType = buffer.subarray(offset + 4, offset + 8).toString('ascii')

      if (boxType === type) {
        boxes.push({ dataStart: offset + size.headerSize, end: boxEnd, type: boxType })
      }

      offset = boxEnd
    }

    return boxes
  }

  const readHandlerType = (mdia: Mp4Box): string | null => {
    const hdlr = findChildBoxes(mdia.dataStart, mdia.end, 'hdlr')[0]

    if (!hdlr || hdlr.dataStart + 12 > hdlr.end) {
      return null
    }

    return buffer.subarray(hdlr.dataStart + 8, hdlr.dataStart + 12).toString('ascii')
  }

  const readTrackHeaderDimensions = (trak: Mp4Box): MediaDimensions | null => {
    const tkhd = findChildBoxes(trak.dataStart, trak.end, 'tkhd')[0]

    if (!tkhd || tkhd.dataStart >= tkhd.end) {
      return null
    }

    const version = buffer[tkhd.dataStart]
    const dimensionsOffset = version === 1 ? tkhd.dataStart + 88 : tkhd.dataStart + 76

    if (dimensionsOffset + 8 > tkhd.end) {
      return null
    }

    const width = buffer.readUInt32BE(dimensionsOffset) / 0x10000
    const height = buffer.readUInt32BE(dimensionsOffset + 4) / 0x10000

    return width && height ? { width: Math.round(width), height: Math.round(height) } : null
  }

  for (const moov of findChildBoxes(0, buffer.length, 'moov')) {
    for (const trak of findChildBoxes(moov.dataStart, moov.end, 'trak')) {
      const mdia = findChildBoxes(trak.dataStart, trak.end, 'mdia')[0]

      if (!mdia || readHandlerType(mdia) !== 'vide') {
        continue
      }

      const dimensions = readTrackHeaderDimensions(trak)

      if (dimensions) {
        return dimensions
      }
    }
  }

  return null
}

function findVideoDimensions(buffer: Buffer): MediaDimensions | null {
  return findWebmVideoDimensions(buffer) || findMp4VideoDimensions(buffer)
}

function findImageDimensions(buffer: Buffer): MediaDimensions | null {
  const dimensions = imageSize(buffer)

  return dimensions.width && dimensions.height ? { width: dimensions.width, height: dimensions.height } : null
}

function getAttributes(tagAttributes: string): Map<string, string> {
  const attributes = new Map<string, string>()

  for (const match of tagAttributes.matchAll(htmlAttributeRegExp)) {
    attributes.set(match[1].toLowerCase(), match[2] ?? match[3] ?? match[4] ?? '')
  }

  return attributes
}

function resolveVideoPath(src: string, markdownPath: string): string | null {
  if (/^(?:[a-z]+:)?\/\//iu.test(src) || src.startsWith('data:')) {
    return null
  }

  if (src.startsWith('/')) {
    return resolve(docsDir, '_static', src.slice(1))
  }

  if (src.startsWith('_static/')) {
    return resolve(docsDir, src)
  }

  return resolve(dirname(markdownPath), src)
}

function escapeHtml(value: string): string {
  return value //
    .replace(/&/gu, '&amp;')
    .replace(/"/gu, '&quot;')
    .replace(/</gu, '&lt;')
    .replace(/>/gu, '&gt;')
}

function normalizeHtmlMediaSrc(src: string): string {
  if (src.startsWith('/') || src.startsWith('./') || src.startsWith('../') || src.startsWith('_static/')) {
    return src
  }

  return `./${src}`
}

function readVideoDimensions(videoPath: string): Promise<MediaDimensions | null> {
  const normalizedVideoPath = normalizePath(videoPath)
  const cachedDimensions = videoDimensionsCache.get(normalizedVideoPath)

  if (cachedDimensions) {
    return cachedDimensions
  }

  const dimensions = readFile(videoPath)
    .then(buffer => findVideoDimensions(buffer))
    .catch(() => null)

  videoDimensionsCache.set(normalizedVideoPath, dimensions)
  return dimensions
}

function readImageDimensions(imagePath: string): Promise<MediaDimensions | null> {
  const normalizedImagePath = normalizePath(imagePath)
  const cachedDimensions = imageDimensionsCache.get(normalizedImagePath)

  if (cachedDimensions) {
    return cachedDimensions
  }

  const dimensions = readFile(imagePath)
    .then(buffer => findImageDimensions(buffer))
    .catch(() => null)

  imageDimensionsCache.set(normalizedImagePath, dimensions)
  return dimensions
}

async function addDimensionsToHtmlMediaTags(
  source: string,
  modulePath: string,
  tagRegExp: RegExp,
  dimensionsReader: (path: string) => Promise<MediaDimensions | null>,
): Promise<{ source: string; didChange: boolean }> {
  let didChange = false
  const replacements = await Promise.all(
    [...source.matchAll(tagRegExp)].map(async match => {
      const fullTag = match[0]
      const attributes = getAttributes(match[1])
      const src = attributes.get('src')

      if (!src || (attributes.has('width') && attributes.has('height'))) {
        return fullTag
      }

      const mediaPath = resolveVideoPath(src, modulePath)

      if (!mediaPath) {
        return fullTag
      }

      const dimensions = await dimensionsReader(mediaPath)

      if (!dimensions) {
        return fullTag
      }

      didChange = true
      const width = attributes.has('width') ? '' : ` width="${dimensions.width}"`
      const height = attributes.has('height') ? '' : ` height="${dimensions.height}"`

      return fullTag.replace(/>$/u, `${width}${height}>`)
    }),
  )

  let replacementIndex = 0

  return {
    source: didChange ? source.replace(tagRegExp, () => replacements[replacementIndex++]) : source,
    didChange,
  }
}

async function addDimensionsToMarkdownImages(
  source: string,
  modulePath: string,
): Promise<{ source: string; didChange: boolean }> {
  let didChange = false
  const replacements = await Promise.all(
    [...source.matchAll(markdownImageRegExp)].map(async match => {
      const fullImage = match[0]
      const alt = match[1]
      const src = match[2]
      const imagePath = resolveVideoPath(src, modulePath)

      if (!imagePath) {
        return fullImage
      }

      const dimensions = await readImageDimensions(imagePath)

      if (!dimensions) {
        return fullImage
      }

      didChange = true

      return `<img src="${escapeHtml(normalizeHtmlMediaSrc(src))}" alt="${escapeHtml(alt)}" width="${dimensions.width}" height="${dimensions.height}">`
    }),
  )

  let replacementIndex = 0

  return {
    source: didChange ? source.replace(markdownImageRegExp, () => replacements[replacementIndex++]) : source,
    didChange,
  }
}

async function rewriteMarkdownVideoLinks(
  source: string,
  modulePath: string,
): Promise<{ source: string; didChange: boolean }> {
  let didChange = false
  const replacements = await Promise.all(
    [...source.matchAll(markdownVideoLinkRegExp)].map(async match => {
      const fullLink = match[0]
      const src = match[2]
      const videoPath = resolveVideoPath(src, modulePath)

      if (!videoPath) {
        return fullLink
      }

      const dimensions = await readVideoDimensions(videoPath)

      if (!dimensions) {
        return fullLink
      }

      didChange = true

      return [
        `<video class="docs-video" src="${escapeHtml(normalizeHtmlMediaSrc(src))}"`,
        ' autoplay loop muted playsinline disablepictureinpicture',
        ` aria-label="image" width="${dimensions.width}" height="${dimensions.height}"></video>`,
      ].join('')
    }),
  )

  let replacementIndex = 0

  return {
    source: didChange ? source.replace(markdownVideoLinkRegExp, () => replacements[replacementIndex++]) : source,
    didChange,
  }
}

export function mediaDimensionsPlugin(): Plugin {
  return {
    name: 'media-dimensions',
    enforce: 'pre',
    async transformIndexHtml(html) {
      let rewritten = html
      const modulePath = resolve(docsDir, 'index.html')
      const videoResult = await addDimensionsToHtmlMediaTags(
        rewritten,
        modulePath,
        htmlVideoTagRegExp,
        readVideoDimensions,
      )
      rewritten = videoResult.source

      const imageTagResult = await addDimensionsToHtmlMediaTags(
        rewritten,
        modulePath,
        htmlImageTagRegExp,
        readImageDimensions,
      )
      rewritten = imageTagResult.source

      return rewritten === html ? undefined : rewritten
    },
    async transform(source, id) {
      const modulePath = id.split('?')[0]

      if (
        !modulePath.endsWith('.md') ||
        (!source.includes('<video') && !source.includes('<img') && !source.includes('![') && !source.includes(']('))
      ) {
        return null
      }

      let rewritten = source
      let didChange = false
      const markdownVideoResult = await rewriteMarkdownVideoLinks(rewritten, modulePath)
      rewritten = markdownVideoResult.source
      didChange ||= markdownVideoResult.didChange

      const videoResult = await addDimensionsToHtmlMediaTags(
        rewritten,
        modulePath,
        htmlVideoTagRegExp,
        readVideoDimensions,
      )
      rewritten = videoResult.source
      didChange ||= videoResult.didChange

      const imageTagResult = await addDimensionsToHtmlMediaTags(
        rewritten,
        modulePath,
        htmlImageTagRegExp,
        readImageDimensions,
      )
      rewritten = imageTagResult.source
      didChange ||= imageTagResult.didChange

      const markdownImageResult = await addDimensionsToMarkdownImages(rewritten, modulePath)
      rewritten = markdownImageResult.source
      didChange ||= markdownImageResult.didChange

      const separatedCaptions = separateMarkdownCaptionsFromHtmlMedia(rewritten)
      if (separatedCaptions !== rewritten) {
        rewritten = separatedCaptions
        didChange = true
      }

      if (!didChange) {
        return null
      }

      return rewritten
    },
  }
}
