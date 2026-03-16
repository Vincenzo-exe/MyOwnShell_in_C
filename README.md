# MyOwnShell_in_C
A UNIX shell implemented from scratch in C — supports REPL, built-in commands, process spawning, piping, redirection, and more.

## Features

- Interactive REPL loop
- Built-in commands: `cd`, `pwd`, `echo`, `exit`
- External command execution via `$PATH` resolution
- Descriptive error handling for invalid commands
- I/O redirection and piping

## Build & Run

you might sudo apt install libreadline-dev

```bash
gcc -o shell main.c
./shell
```

## Concepts Covered

- REPL architecture
- Process creation with `fork()` and `exec()`
- `$PATH` resolution and executable lookup
- Built-in vs external command handling
- Signal handling and process lifecycle

## Built With

- **Language:** C
- **Standard:** POSIX / C99
