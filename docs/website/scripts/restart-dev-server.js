const { execFileSync, spawn } = require('node:child_process');
const path = require('node:path');

const websiteRoot = path.resolve(__dirname, '..');
const port = 3000;

function getListeningProcesses() {
  const script = `
$ErrorActionPreference = 'Stop'
$ownerIds = @(Get-NetTCPConnection -LocalPort ${port} -State Listen -ErrorAction SilentlyContinue |
  Select-Object -ExpandProperty OwningProcess -Unique)
$processes = @(
  foreach ($ownerId in $ownerIds) {
    Get-CimInstance Win32_Process -Filter "ProcessId = $ownerId" -ErrorAction SilentlyContinue |
      Select-Object ProcessId, Name, CommandLine
  }
)
if ($processes.Count -eq 0) { '[]' }
else { ConvertTo-Json -InputObject @($processes) -Compress }
`;

  const output = execFileSync(
    'powershell.exe',
    ['-NoProfile', '-NonInteractive', '-Command', script],
    { encoding: 'utf8' },
  );
  const processes = JSON.parse(output);
  return Array.isArray(processes) ? processes : [processes];
}

function isThisDocusaurusServer(process) {
  const commandLine = (process.CommandLine || '').replaceAll('/', '\\').toLowerCase();
  const root = websiteRoot.replaceAll('/', '\\').toLowerCase();
  const args = commandLine.split(/\s+/);

  return (
    process.Name.toLowerCase() === 'node.exe' &&
    commandLine.includes(root) &&
    commandLine.includes('docusaurus.mjs') &&
    args.includes('start')
  );
}

function stopProcess(processId) {
  execFileSync(
    'powershell.exe',
    [
      '-NoProfile',
      '-NonInteractive',
      '-Command',
      `Stop-Process -Id ${Number(processId)} -ErrorAction Stop`,
    ],
    { stdio: 'inherit' },
  );
}

async function waitForPortToClear(timeoutMs = 10000) {
  const deadline = Date.now() + timeoutMs;

  while (Date.now() < deadline) {
    if (getListeningProcesses().length === 0) return;
    await new Promise((resolve) => setTimeout(resolve, 250));
  }

  throw new Error(`Port ${port} is still in use after stopping the Docusaurus server.`);
}

async function main() {
  if (process.platform !== 'win32') {
    throw new Error('This restart command currently supports Windows only.');
  }

  const listeners = getListeningProcesses();
  const unrelated = listeners.filter((listener) => !isThisDocusaurusServer(listener));
  if (unrelated.length > 0) {
    const details = unrelated
      .map((listener) => `PID ${listener.ProcessId}: ${listener.Name} (${listener.CommandLine})`)
      .join('\n');
    throw new Error(
      `Port ${port} is in use by a process other than this project's Docusaurus server. ` +
        `It was not stopped:\n${details}`,
    );
  }

  for (const listener of listeners) {
    console.log(`Stopping this project's Docusaurus server (PID ${listener.ProcessId})...`);
    stopProcess(listener.ProcessId);
  }

  if (listeners.length > 0) await waitForPortToClear();

  const executable = require.resolve('@docusaurus/core/bin/docusaurus.mjs');
  const server = spawn(process.execPath, [executable, 'start'], {
    cwd: websiteRoot,
    stdio: 'inherit',
  });

  server.on('error', (error) => {
    console.error(`Could not start the Docusaurus server: ${error.message}`);
    process.exitCode = 1;
  });
  server.on('exit', (code, signal) => {
    process.exitCode = code ?? (signal ? 1 : 0);
  });
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
