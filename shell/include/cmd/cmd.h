#ifndef __CMD_H__
#define __CMD_H__

#include "shell/main.h"
#include <stdio.h>

extern cli_t cli;

int do_help(int argc, char **argv);

int do_clear(int argc, char ** argv);

int do_echo(int argc, char ** argv);

int do_exit(int argc, char ** argv);

int do_ls(int argc, char ** argv);

int do_less(int argc, char ** argv);

int do_cat(int argc, char ** argv);

int do_cp(int argc, char ** argv);

int do_rm(int argc, char ** argv);

int do_date(int argc, char ** argv);

int do_reboot(int argc, char ** argv);

#endif