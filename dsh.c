/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>

void dsh()
{

    int running = 1;

    while (running)
    {
        printf("dsh>");

        // get input from user
        char *line = (char *)malloc(MAXBUF);
        fgets(line, MAXBUF, stdin);

        char inp[MAXBUF];
        int strPos = 0;
        int start = 1;               // if we're at the start
        int whiteSpaceEndStart = -1; // the start of the ending whitespace
        char lastChar = ' ';

        // remove white space from the line
        for (int i = 0; i < MAXBUF; i++)
        {

            char c = line[i];
            if (c == '\0' || c == '\n')
            {

                inp[strPos] = '\0';
                if (lastChar != ' ')
                {
                    whiteSpaceEndStart = strPos;
                }
                break;
            }

            // remove white space from the front
            if (start)
            {

                if (c == ' ')
                {
                    continue;
                }
                start = 0;
            }

            if ((c == ' ' || c == '\n') && lastChar != ' ')
            {
                whiteSpaceEndStart = strPos;
            }

            lastChar = c;

            inp[strPos] = c;
            strPos++;
        }

        free(line);
        // put a null where the white space starts
        inp[whiteSpaceEndStart] = '\0';

        if (!strcmp(inp, ""))
        {
            continue;
        }

        // excute the command
        int result = execCommand(inp);

        if (result == -1)
        {
            running = 0; // close the current program
        }
    }
}

/**
 * @brief executes a command in /bin/bash
 * @param command the command string to execute
 * @returns -1 if there was an error
 */
int execCommand(char *command)
{

    char commandClone[MAXBUF];
    strcpy(commandClone, command);
    char *p = strtok(commandClone, " ");

    // p is first element in the command
    if (p[0] == '/')
    {

        // absolute path
        // execute command
        char path[MAXBUF];
        strcpy(path, p);

        // compile rest of command into arguments
        char *args[MAXBUF];
        int i = 0;

        while (p != NULL)
        {
            args[i] = p;
            p = strtok(NULL, " ");
            i++;
        }

        args[i] = NULL;

        return execAbsoluteCommand(path, args);
    }
    else
    {
        // relative path

        // check if file is found at the cwd
        char cwd[MAXBUF];
        getcwd(cwd, MAXBUF);

        // concat to path
        char absPath[MAXBUF];

        memset(absPath, '\0', MAXBUF);
        strcat(absPath, cwd);
        strcat(absPath, "/");
        strcat(absPath, p);

        if (pathExists(absPath))
        {
            return execAbsoluteCommandString(absPath, command);
        }

        // otherwise go through enviroment paths until we have a match

        char envPath[MAXBUF];
        strcpy(envPath, getenv("PATH"));
        char *cPath = strtok(envPath, ":");
        memset(absPath, '\0', MAXBUF);
        while (cPath != NULL)
        {
            // concat to abs path
            strcat(absPath, cPath);
            strcat(absPath, "/");
            strcat(absPath, p);

            if (pathExists(absPath))
            {
                return execAbsoluteCommandString(absPath, command);
            }

            cPath = strtok(NULL, ":");
            memset(absPath, '\0', MAXBUF);
        }

        strcpy(commandClone, command);
        char *p = strtok(commandClone, " ");
        // check for built in command
        if (!strcmp(p, "exit"))
        {
            return -1;
        }

        if (!strcmp(p, "pwd"))
        {
            char cwd[MAXBUF];
            getcwd(cwd, MAXBUF);
            printf("%s\n", cwd);
            return 0;
        }
        if (!strcmp(p, "cd"))
        {
            char *path[MAXBUF];

            p = strtok(NULL, " ");
            if (p == NULL)
            {
                printf("No directory given\n");
            }
            else
            {
                if (chdir(p))
                {
                    printf("Directory does not exist\n");
                }
            }
            return 0;
        }

        printf("Command Not Found\n");
    }

    return 0;
}

/**
 * executes an absolute command given the absolute path and the command string
 */
int execAbsoluteCommandString(char *absPath, char *command)
{
    if (pathExists(absPath))
    {

        // make args and execute
        char path[MAXBUF];
        strcpy(path, absPath);

        // compile rest of command into arguments
        char *args[MAXBUF];
        int i = 0;

        char *p = strtok(command, " ");

        while (p != NULL)
        {
            args[i] = p;
            if (i == 0)
            {
                args[i] = absPath;
            }

            p = strtok(NULL, " ");
            i++;
        }

        args[i] = NULL;

        return execAbsoluteCommand(path, args);
    }
    return 0;
}

/**
 * executes a command at an absolute path
 * @param path the absolute path to the command
 * @param args the arguments for the command
 * @returns returns -1 if there was an error
 **/
int execAbsoluteCommand(char *path, char **args)
{

    if (!pathExists(path))
    {

        // file dne or invalid
        printf("File dosen't exist or isn't executable\n");
        return 0;
    }

    char *lastArg = ""; // the last args
    // check for & in args
    for (int i = 0; i < MAXBUF; i++)
    {
        if (args[i] == NULL)
        {
            break;
        }
        lastArg = args[i];
        args[i] = NULL;
    }

    // fork and execute in the new fork
    int process = fork();
    if (process != 0)
    {
        if (strcmp(lastArg, "&"))
        {
            wait(NULL); // wait for child to finish
        }
    }
    else
    {
        execv(path, args);
        printf("There was an error executing the given command\n");
        return -1;
    }

    return 0;
}

/**
 * @brief checks if a path exists
 * @returns true if exists
 *
 */
int pathExists(char *path)
{
    return access(path, F_OK | X_OK) == 0;
}