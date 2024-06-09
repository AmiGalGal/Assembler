#include "Project.h"

int main(int argc, char *argv[]) {

    FILE *fp;                    /* Pointer to the given file */
    FILE *amFp;                /* Pointer to a new file for the program after macro placement*/
    FILE *extFp = NULL;         /* Pointer to a new file for the externals output*/
    FILE *obfile = NULL;        /* Pointer to a new file for the object output*/
    FILE *entFp = NULL;         /* Pointer to a new file for the entry output*/
    int fileIndex = START;            /* Index for the current file*/
    char originalFileName[MAX_LINE_SIZE];    /* File name without .as*/
    char FileNameWithAm[MAX_LINE_SIZE];    /* File name without .as with .am*/
    char FileNameWithExt[MAX_LINE_SIZE];    /* File name without .as with .ext*/
    char FileNameWithObj[MAX_LINE_SIZE];    /* File name without .as with .obj*/
    char FileNameWithEnt[MAX_LINE_SIZE];    /* File name without .as with .ent*/
    char openingFileName[MAX_LINE_SIZE];    /* File name with .as*/
    int RAM[RAM_SIZE] = {ZERO};              /* The RAM memory */
    int labelCount = ZERO;                  /* Count of encountered labels */
    label *labels = NULL;                          /* Pointer to dynamically allocated array*/
    int ICF = ZERO;           /* Signal for RAM memory use end*/
    int firstPassStatus = ZERO;     /* Status of the first pass*/
    label *externs = NULL;        /* Pointer to dynamically allocated array*/
    int externsCount = ZERO;                  /* Count of encountered extern labels */
    int extFlag = ZERO;                     /* Extern flag*/
    int entryFlag = ZERO;                   /* Entry flag*/


    command commands[] =                /* Array of structs representing actions and numerical data */
            {
                    {"mov",  0,  0},
                    {"cmp",  1,  0},
                    {"add",  2,  10},
                    {"sub",  2,  11},
                    {"lea",  4,  0},
                    {"clr",  5,  10},
                    {"not",  5,  11},
                    {"inc",  5,  12},
                    {"dec",  5,  13},
                    {"jmp",  9,  10},
                    {"bne",  9,  11},
                    {"jsr",  9,  12},
                    {"red",  12, 0},
                    {"prn",  13, 0},
                    {"rts",  14, 0},
                    {"stop", 15, 0}
            };
    /*
     * Array of register names
     */
    char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE] =
            {
                    "r0",
                    "r1",
                    "r2",
                    "r3",
                    "r4",
                    "r5",
                    "r6",
                    "r7",
                    "r8",
                    "r9",
                    "r10",
                    "r11",
                    "r12",
                    "r13",
                    "r14",
                    "r15",
            };


    if (argc < MIN_ARGUMENTS) /* Check if enough arguments were provided */
    {
        puts("Insufficient arguments provided");
        return EXIT;
    }

    for (fileIndex = LOOP_START; fileIndex < argc; fileIndex++) {
        externsCount = ZERO;/* Reset externs count*/
        labelCount = ZERO;/* Reset label count*/
        strcpy(originalFileName, argv[fileIndex]); /* Copy the name of the current file */
        strcpy(openingFileName, originalFileName); /* Copy the name of the current file */
        strcpy(FileNameWithAm, originalFileName); /* Copy the name of the current file */
        strcpy(FileNameWithExt, originalFileName); /* Copy the name of the current file */
        strcpy(FileNameWithObj, originalFileName); /* Copy the name of the current file */
        strcpy(FileNameWithEnt, originalFileName); /* Copy the name of the current file */
        strcat(openingFileName, ".as");
        fp = fopen(openingFileName, "r+");            /* Open the file */
        amFp = fopen(strcat(FileNameWithAm, ".am"), "w+");                  /* Create and open a new file with the name of the old file + new */

        if (fp == NULL) {                                                 /* Unable to open the file */
            puts("Unable to open the current file, proceeding to the next file");
            continue;
        }

        else {
            if ((macrosDeploy(fp, amFp)) == FALSE)      /* Deploy all macros to newfp*/
            {
                puts("Error occurred during macro deployment.");
                fclose(fp);                /* Close the original file */
                fclose(amFp);            /* Close the new file */
                continue;      /* Proceed to the next file*/
            }

            fclose(fp);  /* Close the original file */
            fclose(amFp);            /* Close the new file */

            /* Perform first pass */
            firstPassStatus = firstPass(amFp, RAM, &labels, &labelCount, commands, registerTable, &ICF, &extFlag, &entryFlag);
            if (firstPassStatus == FALSE) {
                printf("An error occurred during the initial code reading, preventing advancement to the second pass.\n");
                continue;   /* Proceed to the next file*/
            }

            ICF = firstPassStatus; /* Save the index where the RAM usage was ended*/

            /* Perform second pass */
            if (secondPass(amFp, RAM, &labels, labelCount, commands, registerTable, &externs, &externsCount) == MINUS_ONE) {
                continue; /* Proceed to the next file*/
            }

            /* Create output files */
            if (extFlag == TRUE) {
                char ExtFileName[MAX_LINE_SIZE];   /* Separate variable for .ext file name */
                strcpy(ExtFileName, FileNameWithExt); /* Create separate copy to avoid modifying original name */
                extFp = fopen(strcat(ExtFileName, ".ext"), "w+"); /* Open .ext file for writing */
                if (extFp == NULL) {
                    printf("Error opening .ext file for writing.\n");
                } else {
                    createExternFile(extFp, externs, externsCount); /* Create the externals output file*/
                    fclose(extFp); /* Close the external file */
                }
            }

            obfile = fopen(strcat(FileNameWithObj, ".ob"), "w+"); /* Open .ob file for writing */
            createObjectFile(obfile, ICF, ICF, RAM);

            if (entryFlag == TRUE) {
                char EntFileName[MAX_LINE_SIZE];   /* Separate variable for .ent file name */
                strcpy(EntFileName, FileNameWithEnt); /* Create separate copy to avoid modifying original name */
                entFp = fopen(strcat(EntFileName, ".ent"), "w+"); /* Open .ent file for writing */
                createEntryFile(entFp, &labels, labelCount); /* Create the entry output file */
                fclose(entFp); /* Close the entry file */
            }
        }

        /* Memory deallocation and checks */
        if (labels != NULL) {
            free(labels);
            labels = NULL; /* Set to NULL after freeing */
        }
        if (externs != NULL) {
            free(externs);
            externs = NULL; /* Set to NULL after freeing */
        }

        if (obfile != NULL) fclose(obfile); /* Close the object file */
    }

    return ZERO;
}

