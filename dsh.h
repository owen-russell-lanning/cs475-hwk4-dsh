/*
 * dsh.h
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */

#define MAXBUF 256

void dsh();
int execCommand(char* command);
int execAbsoluteCommand(char* command, char** args);
int pathExists(char* path);
int execAbsoluteCommandString(char *absPath, char *command);