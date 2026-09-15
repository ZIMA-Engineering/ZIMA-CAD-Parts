# Codex in the command panel

The first AI provider is Codex, connected through the official Codex App
Server protocol. The user signs in with their own ChatGPT account. Other
providers can implement the `AiProvider` interface without changing Parts
file operations. Voice and pointer input are outside this first version.

## Setup and use

1. Install a native Codex CLI executable for your operating system, or use
   an existing compatible executable. Parts detects `codex` on PATH and
   also checks the Windows Codex application runtime directory. An npm
   `.cmd` wrapper is not a native executable; choose its actual binary.
2. Open **Settings > AI**, check the executable path, and select **Connect**.
3. Select **Sign in with ChatGPT** and complete the official browser flow.
   A connected account can select a model returned by Codex; the default
   entry leaves model selection to Codex.
4. Show the bottom command panel with **Ctrl+Shift+P** and enter `codex`.
   Write normal requests or questions, for example "What is in this
   directory?", "Read the project notes", or "Clean old PTC versions here".
5. Use `/new` to start another conversation, `/exit` to return to ordinary
   Parts commands, and **Stop** to interrupt an active AI request.

The input stays editable while Codex works, including during tool calls and
inline approval. You can write or paste the next question immediately. Send
is disabled until the current request ends; Enter leaves the draft intact
while busy. Completion, errors and Stop preserve the draft. It is never sent
automatically: review it and submit when ready.

Drag local files or directories into the panel, including its input field,
or select **Add to AI question** in the Parts list, directory tree, or data
source root context menu. Multiple highlighted Parts rows are supported.
The action inserts full quoted paths into your draft at the cursor and opens
AI mode without connecting or sending a request. Existing text is preserved;
selected text is replaced just like an ordinary paste. Type a question such
as "Look at this file" beside the path, then send it.

Paths are ordinary editable text, with no attachment chips. Delete or undo
an insertion before sending to remove it completely. Duplicate inserted
paths are ignored. Quoting preserves spaces and unusual characters; manually
pasted quoted absolute paths are supported too. Only existing files and
directories mentioned in a submitted message become available to the
structured host tools. A removed draft path grants no access.

Up to 32 paths are retained as conversation context for follow-up questions.
`/new`, `/exit`, or changing directory roots clears this context. Adding a path
to the draft while a request is running affects only the next submission.
New paths invalidate earlier Parts previews but do not restart the conversation.
Paths are not saved between application sessions. Remote URLs are not local
file references.

Connecting and signing in are explicit actions and take effect immediately,
even if the Settings dialog is later cancelled. Merely opening Settings or
starting Parts does not start Codex or contact an AI service. Entering
`codex` can start the local connection and open Settings when sign-in is
needed. Stopping a request disconnects this integration; connect again to
continue. Completed file operations are not undone by Stop.

The `codex` entry command belongs to the GUI panel, not the standalone Parts
CLI. That CLI retains its existing command syntax.

## Directories, tools, and approval

Each request captures the active directory and the separately configured
working directory. Both are accessible roots; relative paths use the active
directory. Switching tabs while Codex works does not redirect its pending
tools. A subsequent request uses the newly active directory and starts a
fresh conversation when the directory roots or selected model change.

Parts initially supplies those paths, the UI metadata language, operating
system/shell identification, and paths/types mentioned in this conversation. It
does not automatically send a directory inventory, file contents, selected
parts, screenshots, or documents. Codex requests information as needed:

- `parts_command` calls the shared Parts command executor with an argument
  array. `help` describes available syntax; `list`, `params`, and the built-in
  file tools use the same implementation as the GUI and CLI.
- `directory_list` explores directories, including hidden entries, in pages
  of 100 entries. It also identifies links without following them.
- `read_text` reads a requested text file in chunks of at most 16 KiB.
- `parts_apply` shows an inline review of an exact locally retained preview.
  The user selects files and clicks **Allow selected** before modification.
- `system_command` asks to run a non-interactive command using Windows
  PowerShell or `/bin/sh` on Linux. Every command requires explicit review.

`parts_command` rejects `--apply`; only the review can execute a retained
plan. Plan identifiers are single-use for review and become invalid when
the context changes, the conversation is reset, or an error/cancellation
ends the request. Denying a review does not grant approval. Existing
fingerprint checks, removal locks, no-overwrite rules, and STEP backups
remain in force. The approval covers the displayed local plan, not a newly
generated operation. Application updates remain in Settings > Updates.

Structured tools accept the captured roots, exact file paths sent in the
conversation, and descendants of directory paths sent there. A file path does
not grant access to its siblings or parent directory. Paths outside this scope
and symbolic links or junctions are rejected. Reading system metadata is possible; Parts
file tools retain their existing exclusion of `0000-index`.

## System commands

Use system commands for installed utilities and operations not implemented
by Parts, including creating and editing files. A review displays the purpose,
exact command text, shell executable, and working directory inside the command
panel. Nothing starts until **Allow** is clicked. **Deny** returns cancellation to
AI. Approval covers one command; it does not grant session-wide permission.

Reviews do not open modal windows or block browsing. The approval buttons
remain visible while long details scroll above them. Parts operations also
use this inline review without a second confirmation popup. Stop, a lost
connection, or the end of a request cancels any outstanding approval.

The host runs commands with the current OS user's permissions. This is not
a filesystem sandbox: commands may access files and network services outside
the conversation's paths and do not automatically enforce Parts directory locks.
This distinction is displayed in the review. The assistant is instructed to
prefer Parts tools, respect locks, and never work around a declined operation.
The host does not request administrator elevation or supply Codex/API
credentials from its environment.

Commands have no interactive stdin and should remain in the foreground.
PowerShell uses no profile, non-interactive mode and UTF-8 output; Python
also receives a UTF-8 output setting. The result includes stdout/stderr,
exit code, crash, cancellation, timeout and truncation status. Output sent
back to AI is capped at 48 KiB. A requested timeout must be 1–600 seconds.
Stop terminates the shell and its ordinary child processes using a Windows
job or a Linux process group. Deliberately detached processes are not
supported. Completed changes are not undone by cancellation or timeout.

Conversations are ephemeral Codex threads, reused for up to 20 requests in
the same context. An automatic fresh thread after that limit bounds history.
Tool outputs are limited to 96 KiB; oversized results request a narrower
query. Up to eight pending previews and 64 tool calls per request are allowed.
These limits do not truncate the local approval plan.

## Accounts, data, and distribution

User requests, directory paths, requested text, and tool results are sent to
OpenAI through the user's Codex session. The user's ChatGPT plan limits,
workspace rules, and applicable data settings govern this use. This adapter
uses ChatGPT login; it does not silently switch to separately billed API
access or inherit API credentials from the parent process.

Parts uses a separate Codex profile in its user application-data directory,
under `ai/codex`. Credentials are managed by Codex using the operating system
credential store (`keyring` mode); an unavailable store must be fixed before
persistent login can work. No credentials belong in project directories,
source control, or release archives. Sign out affects this Parts profile.

The Codex process uses stdio, a read-only sandbox, disabled native shell
tools, and the supplied host tool catalog. Project instruction discovery,
shell snapshots, plugins/apps, and inherited user hooks are disabled for
this integration. System commands run through the reviewed host tool,
not an unreviewed provider-native shell. Unrecognized server requests fail
closed. Child stderr is drained without being logged, since it can contain
sensitive diagnostics. Model errors are shown without copying arbitrary
server diagnostics containing credentials.

The thread's developer instructions explicitly distinguish the provider's
read-only sandbox from these host-reviewed write capabilities. There is no
read-only/full-access switch in Parts settings. When asked to edit files,
the assistant should propose a host operation and wait for inline approval,
rather than tell the user to enable a nonexistent permission setting.

The release currently includes the Parts adapter and its source; Codex is
an optional external executable. The complete proprietary desktop app and
model weights are not copied into the Parts bundle. The open-source Codex
CLI uses Apache-2.0, compatible with GPLv3; any future bundled binary must
include the exact release's license, NOTICE, and dependency notices. Parts
remains GPL-3.0-or-later. Each user supplies their own account; no shared
developer subscription or API key is distributed.

## Protocol and verification

This adapter uses the official JSON-RPC App Server interface. Host-supplied
`dynamicTools` and `item/tool/call` are currently experimental protocol
features. Parts opts in explicitly and reports incompatible runtimes rather
than falling back to unrestricted tools. Native Windows and Debian runtime
compatibility must be checked against the chosen Codex release.

Automated integration checks use a local protocol fixture and never contact
OpenAI. They cover mode switching, minimal initial context, captured roots,
directory pagination, text reads, command discovery, rejected direct apply,
locked directories, stale previews, review cancellation/replay, protocol
errors, native-tool rejection, reference capture/removal, exact-file scopes,
local drag/drop, editable drafts during active requests, menu routing,
system-command approval, real fixture file creation, UTF-8 output, exit
failures, timeout, and cancellation. Inline
reviews are checked for non-modal behavior, selected-file application, and
invalidation on provider failure. Live inference checks are separate from
the automated suite and require an already connected user account.

Official references:

- [Codex App Server](https://learn.chatgpt.com/docs/app-server)
- [Codex authentication](https://learn.chatgpt.com/docs/auth)
- [Codex source license](https://github.com/openai/codex/blob/main/LICENSE)
- [Apache-2.0 and GPLv3 compatibility](https://www.apache.org/licenses/GPL-compatibility.html)
