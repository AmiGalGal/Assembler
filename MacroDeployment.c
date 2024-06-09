#include "Project.h"

/*
 * Within this file, we shall deploy the macros.
 */
int macrosDeploy(FILE *fp, FILE *newfp) {

    char line[MAX_LINE_SIZE];            /* Holds the current line */
    int flagMacro = ZERO;                /* Indicates whether a macro has been encountered */
    macro *macros = NULL;                        /* Pointer to an array of macros */
    int macrosCount = ZERO;            /* Number of macros encountered */
    macro *tempMacros;                    /* Used to ensure realloc didn't return NULL */
    int wordAmount = ZERO;            /* Number of words in the line */
    char *wordArray[MAX_LINE_SIZE];        /* Array containing words from the line */
    char copyLine[MAX_LINE_SIZE];        /* Stores the line before modifications */
    int macrosRunIndex = ZERO;        /* Index for macros */
    int endmFlag = ZERO;                /* Indicates whether an end of macro has been encountered */
    int currentIndex = ZERO;            /* Index until end of line */
    char newWord[MAX_LINE_SIZE];        /* Word after removing spaces */

    /*
     * Steps:
     * 1. Open a file.
     * 2. Retrieve its macros.
     * 3. Copy the content to a new file and modify the macro contents.
     */

    while (fgets(line, MAX_LINE_SIZE, fp))    /* Reads a line from the file */
    {
        strcpy(copyLine, line);                          /* Save the line */
        /*
         * Separate the line into words and insert into the array.
         */
        makeItArray(line, wordArray);
        /*
         * Check if the first word indicates the start of a macro.
         */

        if (strcmp(wordArray[ZERO], "macro") == ZERO) {
            /* Reset endmFlag after finding a macro */
            endmFlag = ZERO;
            macrosCount++;
            /* If this is the first macro observed */
            if (flagMacro == ZERO) {
                flagMacro = START;
                macros = (macro *) malloc(sizeof(macro));       /* Dynamically allocate array */
                if (macros == NULL) {
                    puts("The program will come to a halt as it is unable to dynamically allocate an array.");
                    return FALSE;
                }
            } else {
                tempMacros = (macro *) realloc(macros, macrosCount * sizeof(macro));
                if (tempMacros != NULL) {
                    /*
                     * Insert the macro's name into struct.
                     */
                    macros = tempMacros;
                } else {
                    puts("The program will come to a halt as it is unable to dynamically allocate an array.");
                    free(macros);  /* Free previously allocated memory */
                    return FALSE;
                }
            }

            /*
             * Insert the macro's name into the struct.
             */
            strcpy(macros[macrosCount - STEP].name, wordArray[START]);

            /*
             * Add lines until reaching the end of the macro.
             */
            while (endmFlag != START) {
                fgets(line, MAX_LINE_SIZE, fp);       /* Reads a line */
                /*
                 * Split the line into words.
                 */
                wordAmount = makeItArray(line, wordArray);
                currentIndex = ZERO;
                removeNewLine(wordArray[currentIndex], newWord);/* Remove '\n' at end */

                /*
                 * Iterate through words until encountering end of macro or end of line.
                 */

                while ((strcmp(newWord, "endm")) != ZERO && currentIndex < wordAmount) {
                    strcat(macros[macrosCount - STEP].content, wordArray[currentIndex]);
                    currentIndex++;
                    strcat(macros[macrosCount - STEP].content, " ");

                }

                /*
                 * Check if end of macro has been reached.
                 */

                if ((strcmp(newWord, "endm")) == ZERO) {
                    endmFlag = START;
                }
            }

        } else {
            /*
             * If not a macro definition, check if it's a macro use.
             */
            for (macrosRunIndex = ZERO; macrosRunIndex < macrosCount && macrosRunIndex >= ZERO; macrosRunIndex++)
                /*
                 * Enter loop and check if it's a macro.
                 * If it is, copy its content to the new file.
                 */
            {
                if (strcmp(macros[macrosRunIndex].name, wordArray[ZERO]) == ZERO) {
                    fputs(macros[macrosRunIndex].content, newfp);  /* Copy macro contents */
                    macrosRunIndex = MINUS_TWO;     /* Indicates macro is reached */
                }

            }
            /*
             * If it's not a macro name, print the line to the file.
             */
            if (macrosRunIndex >= ZERO)        /* If not a macro */
            {
                fputs(copyLine, newfp); /* Copy line */
            }
        }

    }
    free(macros);
    return TRUE;
}
