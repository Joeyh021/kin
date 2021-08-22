# KIN - (Kin Isn't Nano)

KIN is a clone of GNU nano, written as part of my CS132 coursework, where the brief was to design a command-line based text editor

## Compilation

```
make
```

The program depends on `ncurses`, which can be easily installed on any GNU/Linux distro with your package manager of choice.

## Usage

- to open a file in the editor: `kin FILE` •
- To make a copy of a file: `kin -c FILE1 FILE2`
- To delete a file: `kin -d FILE`
- To create a new file and open it for editing: `kin -n FILE`

The editor will output a log of its operations to a `.kinlog` file.
