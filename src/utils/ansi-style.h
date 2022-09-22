#ifndef ANSI_STYLE_H
#define ANSI_STYLE_H

#define RESET "\x1b[0m"

#define BOLD "\x1b[1m"
#define ITALIC "\x1b[3m"
#define UNDERLINE "\x1b[4m"
#define STRIKE "\x1b[9m"

#define BLK 0
#define RED 1
#define GRN 2
#define YEL 3
#define BLU 4
#define MAG 5
#define CYN 6
#define WHT 7
#define HBLK 8
#define HRED 9
#define HGRN 10
#define HYEL 11
#define HBLU 12
#define HMAG 13
#define HCYN 14
#define HWHT 15

#define _STR(n) #n
#define SET_FG(color) "\x1b[38;5;" _STR(color) "m"
#define SET_BG(color) "\x1b[48;5;" _STR(color) "m"

#endif
