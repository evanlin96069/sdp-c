# sdp-c
Source demo parser in C.

## Usage
```
sdp [-d|-t] [options] <demo> 
```
Options:
- `-h, --help` 
&emsp;Display help information.
- `-v, --version`
&emsp;Display parser version information.
- `-d, --dump`
&emsp;Create a text representation of parsed data in the demo.
- `-t, --tas`
&emsp;Create a afterframes TAS script of the demo to be run with the [SPT plugin](https://github.com/YaLTeR/SourcePauseTool).
- `-o <file>`
&emsp;Place the output into `<file>`.
- `-P[level]`
&emsp;Set the parsing level for dump mode. (default = auto select)
    - `-P0`
    &emsp;Parse the header.
    - `-P1, -P`
    &emsp;Basic timging and Parse Usercmd.
    - `-P2`
    &emsp;Parse Net/Svc messages and DataTables.
- `--debug`
&emsp; Print debug information.

## Supported Game Versions
| Game | Basic<br/>Timing | UserCmd<br/>Parsing | Net/Svc Message<br/>Parsing | DataTables<br/>Parsing | 
| ----------------------------- | :-: | :-: | :-: | :-: |
| Half-Life 2 Old Engine (2153) | ✅ | ✅ | ✅ | ❌ |
| Half-Life 2 Old Engine (2187) | ✅ | ✅ | ✅ | ❌ |
| Dark Messiah of Might & Magic | ✅ | 🔶 | ❌ | ❌ |
| Half-Life 2 Old Engine (4044) | ✅ | ✅ | ✅ | ✅ |
| Portal (3258)                 | ✅ | ✅ | ✅ | ✅ |
| Portal (3420)                 | ✅ | ✅ | ✅ | ✅ |
| Source Unpack (5135)          | ✅ | ✅ | ✅ | ✅ |
| Steampipe HL2/Portal          | ✅ | ✅ | ✅ | ✅ |
| Portal 2                      | ✅ | ✅ | ✅ | ✅ |

- Not implemented
    - StringTables parsing
    - User message parsing
    - Entity parsing
    - Game event parsing
    - Sound parsing

## Building
```bash
./compile.sh
```

## Other Useful Resources
- [.dem](https://nekz.me/dem/)
- [Uncrafted Demo Parser](https://github.com/UncraftedName/UncraftedDemoParser)
- [demogobbler](https://github.com/lipsanen/demogobbler)

If you are looking for demo to TAS converter for Portal 2:
- [dem2tas](https://github.com/mlugg/dem2tas)
