# Bottom command panel

The panel follows the ZIMA-CAD console layout: read-only output, a command
input, **Run** and **Clear output**, below the main content. The terminal
button in the top toolbar or **Ctrl+Shift+P** toggles it. The close button in
its header also hides it. Panel state is saved with the window layout.

## Commands

- `help` lists syntax and available options.
- `list` uses the active tab's directory.
- `list "path to directory"` uses the specified path.
- `list --name screw` filters names in the current directory.
- `params "part.pdf"` reads saved parameters in the current directory.

Relative paths are resolved from the active tab's directory. The path and
language are captured when the command is submitted: switching tabs during
execution does not change the operation. The directory is printed before
the command in the output. Single and double quotes are supported, and
backslashes in Windows paths are preserved. The panel does not run a system
shell or command substitutions. Built-in PS2PDF starts only the Ghostscript
conversion process.

The Up and Down keys navigate the last 100 commands in the current session.
Returning from history restores the unfinished input. History is not saved
to disk. **Clear output** does not clear history or project data.

## Execution and results

The panel and standalone CLI use `PartsCore::executeCommand`. Results are
JSON; help and technical error messages are in English. Controls are
available in English and all four application translations. Read behavior
and limitations are described in the [CLI documentation](cli.md).

Commands run in a worker thread. Input is temporarily disabled while a
command runs, but the rest of the application remains usable. Closing the
application requests interruption and waits for the operation to finish.
An ongoing filesystem read, for example on a network drive, may not be
immediately interruptible.

The output is limited to 1,500 text blocks and each result to 24,000
characters. A notice appears when output is truncated; use the standalone
CLI for complete JSON. The panel also supports [built-in
tools](integrated-tools.md): `ps2pdf`, `ptc-clean`, `zima-clean` and `step-edit`. Without
`--apply`, they return a preview; with `--apply`, they modify files.

Enter `codex` to use the optional [Codex conversation mode](ai-command-panel.md).
It works with the available commands and directories, obtaining information
through tools as needed. `/exit` returns to ordinary Parts commands. AI
file operations require a local review. Drag files or directories into the
panel, or use **Add to AI question** in a context menu, to insert full quoted
paths at the cursor. Edit or delete them as ordinary draft text; nothing is
sent until you submit the question. `/new` clears the conversation's paths.
You can draft the next question while AI is working. Sending waits for the
current request to end; completion, an error or Stop preserves your draft.
System tools are available through reviewed PowerShell or `/bin/sh` commands;
each command needs explicit confirmation and runs with the OS user's rights.

## Verification

The integration tests `commandPanelUsesCapturedContextAndHistory` and
`commandPanelCanBeHiddenAndRestored` cover captured context, reading,
relative paths, history, draft restoration, quote errors, help and visibility.
Standalone CLI process tests protect compatibility of public commands.
Panel visibility and the keyboard shortcut were also checked in the running
Windows application. See [verification](verification.md) for build details
and current test results. Linux execution remains unverified.

The panel also accepts the [update commands](github-updates.md#cli). Checking
and downloading run in its worker; approved installation/rollback hand off to
the helper and request normal application shutdown.
