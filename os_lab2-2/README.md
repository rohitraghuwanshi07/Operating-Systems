# CSL301 Lab-2 — Shell (Fork/Exec/Wait) — macOS M2 Guide

Good news: you do **NOT** need Docker or a Linux VM for this assignment.
`fork()`, `exec()`, `wait()`, `waitpid()` are all standard POSIX calls and
macOS (Darwin) supports them natively via its built-in `clang` compiler.

## Files in this zip
- `q2_fork_skeleton.c` — Q2: basic fork, child/parent print + failure handling
- `q3_runcmd.c` — Q3: fork + exec + wait, runs `<command> <arg>`
- `q4_shell.c` — Q4: mini shell with foreground + background (`&`) execution
- `README.md` — this guide

All three were compiled clean (zero warnings) and smoke-tested before packaging.

## 1. One-time setup
Open **Terminal** and check you have the command line tools (installs `clang`,
which macOS aliases as `gcc`):

```bash
xcode-select -p
```

If that errors out ("not found"), install with:
```bash
xcode-select --install
```
Click "Install" in the popup, wait for it to finish, then re-run the check.

## 2. Unzip and go to the folder
```bash
cd ~/Downloads
unzip os_lab2.zip
cd os_lab2
```

## 3. Q2 — fork skeleton
```bash
gcc -Wall -o q2 q2_fork_skeleton.c
./q2
```
Expected output (order may vary — that's the point of Q1.2):
```
I am parent. PID = 1234
I am child. PID = 1235
```

## 4. Q3 — runcmd (fork + exec + wait)
```bash
gcc -Wall -o q3 q3_runcmd.c
./q3 echo hello
./q3 sleep 2
./q3           # wrong arg count -> should print usage error
```
Expected:
```
hello
Child 1236 exited with status 0
Parent: command executed successfully.
```

> Note: macOS's `sleep` and `echo` are in `/bin`, `execlp` searches `$PATH`
> automatically, so no path changes are needed.

## 5. Q4 — shell with background execution
```bash
gcc -Wall -o q4 q4_shell.c
./q4
```
Then at the `myshell>` prompt, try:
```
myshell> sleep 20 &
myshell> sleep 3
myshell> ls
myshell> exit        # (Ctrl+D also works to quit — no built-in "exit" cmd,
                      #  so just press Ctrl+D)
```

### How to actually test background vs foreground (for your report)
Open a **second Terminal tab** (Cmd+T) and while `sleep 20 &` is running in
tab 1, run in tab 2:
```bash
ps -ax | grep sleep
```
You should see the `sleep` process alive and owned by your shell.

To verify reaping: start a background sleep, then immediately run a longer
foreground sleep. Confirm the "Shell: Background process finished" message
appears *without* the shell returning to the prompt early:
```
myshell> sleep 3 &
Shell: Started background process 1240
myshell> sleep 8
Shell: Background process finished
myshell>
```
(The `Shell: Background process finished` line appears once you press Enter
again since reaping happens each loop iteration — for exact timing you could
also add a `SIGCHLD` handler, but polling via `WNOHANG` on each prompt loop,
as implemented, satisfies the assignment's stated requirement.)

## 6. Taking screenshots for your LaTeX report
- Use `Cmd+Shift+4` then `Space`, then click the Terminal window to capture
  just that window as a PNG.
- Or `Cmd+Shift+5` for the full screenshot toolbar (lets you record short
  clips too, useful for showing background/foreground interleaving live).

## 7. Copying real terminal output into your report
Run each program, then copy actual output — don't fabricate it. You can
pipe output to a file to grab exact text if needed:
```bash
./q3 echo hello > q3_output.txt
```

## Notes on portability
- No Linux-specific headers are used (no `<linux/...>`), so nothing needs
  translation for Darwin/BSD.
- `waitpid(-1, &status, WNOHANG)` behaves identically on macOS and Linux —
  it reaps any terminated child without blocking.
- If your grader specifically requires testing on Linux, you can still
  compile identically inside a Docker container (`docker run -it --rm -v
  $(pwd):/lab -w /lab gcc:latest bash`) but for this lab it's unnecessary.
