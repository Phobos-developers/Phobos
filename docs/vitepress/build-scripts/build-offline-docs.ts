import { spawn } from 'node:child_process'

const npmCommand = process.platform === 'win32' ? 'npm.cmd' : 'npm'

function runNpm(args: string[], env: NodeJS.ProcessEnv = {}): Promise<void> {
  return new Promise<void>((resolve, reject) => {
    const child = spawn(npmCommand, args, {
      env: { ...process.env, ...env },
      shell: false,
      stdio: 'inherit',
    })

    child.on('error', reject)
    child.on('exit', code => {
      if (code === 0) {
        resolve()
        return
      }

      reject(new Error(`${npmCommand} ${args.join(' ')} exited with code ${code}`))
    })
  })
}

await runNpm(['run', 'build'], { PHOBOS_VITEPRESS_OFFLINE: '1' })
await import('./build-export-offline-html.ts')
