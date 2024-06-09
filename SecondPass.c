#include "Project.h"

/*
 * Here begins the second pass over the text.
 * This pass is crucial for finalizing the assembly process.
 */

int secondPass(FILE *newfp, int RAM[RAM_SIZE], label **labels, int labelCount, command commands[],
               char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], label **externs, int *externsCount) {
    char line[MAX_LINE_SIZE];            /* Holds the current line */
    int wordAmount = ZERO;            /* Number of words in the line */
    char *wordArray[MAX_LINE_SIZE];        /* Array containing the line divided into words */
    int lineCounter = ZERO;  /* Index for lines */
    int labelFlag = ZERO;
    int entryLabelIndex = ZERO; /* Index of entry label */
    int errorFlag = FALSE;
    int commandIndex = ZERO;                /* Holds the command number */
    int firstOperandAddressMethod = ZERO;
    int secondOperandAddressMethod = ZERO;
    int RAMIndex = RAM_START;  /* Used for RAM memory allocation */
    char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE];
    int labelIndex = ZERO; /* Index of label */
    char labelToken[MAX_LINE_SIZE];
    label *tempExterns;    /* Temporarily holds dynamically allocated externs array */

    fseek(newfp, ZERO, SEEK_SET); /* Return to the beginning of newfp */
    while (fgets(line, MAX_LINE_SIZE, newfp))    /* Read a line from the file */
    {
        labelFlag = ZERO; /* Reset labelFlag */
        wordAmount = makeItArray(line, wordArray);    /* Divide the line into an array of words */
        lineCounter++;

        if (wordArray[ZERO][ZERO] == ';' || strcmp(wordArray[ZERO], "\n") == ZERO ||
            strcmp(wordArray[ZERO], "\t") == ZERO)    /* If it's an empty or a comment line */
        {
            /* Ignore the line */
        } else/*if it wasn't an empty line*/
        {
            if ((wordArray[ZERO][strlen(wordArray[ZERO]) - ONE]) ==
                ':')  /* The last char of the first word is ':' means it's a label */
            {
                labelFlag = ONE;
            }
            if ((strcmp(wordArray[ZERO + labelFlag], ".data")) == ZERO ||
                (strcmp(wordArray[ZERO + labelFlag], ".string")) == ZERO ||
                (strcmp(wordArray[ZERO + labelFlag], ".extern")) == ZERO) /* If the second word is .data or .string */
            {
                continue; /* Continue to the next line */
            }
            if ((strcmp(wordArray[ZERO + labelFlag], ".entry")) == ZERO) {
                /* This function gets a label and returns its number in the label array */

                entryLabelIndex = labelNum(wordArray[ONE + labelFlag], (*labels), labelCount);

                if (entryLabelIndex == MINUS_ONE) {
                    printf("Line %d- Error - Label wasn't found 1 \n", lineCounter);
                    errorFlag = TRUE; /* Encountered an error */
                    continue;/* Continue to the next line */
                }
                (*labels)[entryLabelIndex].attributesCount++;
                strcpy((*labels)[entryLabelIndex].attributes[(*labels)[entryLabelIndex].attributesCount - ONE],
                       "entry"); /* Add "entry" to the label attribute */
            }
            commandIndex = searchCommand(commands, wordArray[ZERO +
                                                             labelFlag]); /* Search if the command is in the commands table */
            lineForCommand(wordArray, labelFlag, wordAmount, commandOperands,
                                             lineCounter); /* Get the value returned from the lineForCommand method */
            switch (commandIndex) {
                /* Methods that take 2 operands */
                case ZERO:
                case ONE:
                case TWO:
                case THREE:
                case FOUR:

                    /* Find the number of the Addressing Methods */
                    firstOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                    secondOperandAddressMethod = findAddressMethod(commandOperands[STEP], registerTable, lineCounter);
                    RAMIndex += TWO;      /* Two more lines - One for the opcode - Second for funct and  the addressing methods and source and destination */
                    if (firstOperandAddressMethod == ONE || firstOperandAddressMethod == TWO) {
                        if (firstOperandAddressMethod == ONE) {
                            strcpy(labelToken, commandOperands[ZERO]);
                            labelIndex = labelNum(labelToken, (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 2 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if (firstOperandAddressMethod == TWO) {
                            strcpy(labelToken, strtok(commandOperands[ZERO], "["));
                            labelIndex = labelNum(labelToken, (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 3 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if ((*labels)[labelIndex].base != ZERO) {
                            RAM[RAMIndex] |= (*labels)[labelIndex].base;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                            RAM[RAMIndex] |= (*labels)[labelIndex].offset;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                        } else /* If it's an extern label */
                        {
                            if ((*externsCount) == ZERO)/* If this is the first label encountered */
                            {
                                (*externs) = (label *) malloc(sizeof(label));
                                if ((*externs) == NULL) {
                                    puts("Dynamic allocation for the externs array failed, causing the program to terminate prematurely.\n");
                                    exit(EXIT);
                                }
                                (*externsCount)++; /* Raise the label count */
                            } else {
                                (*externsCount)++; /* Raise the label count */
                                tempExterns = (label *) realloc((*externs), (*externsCount) * sizeof(label));
                                if (tempExterns == NULL) {
                                    puts("Dynamic allocation for the externs array failed, causing the program to terminate prematurely.\n");
                                    exit(EXIT);
                                }
                                (*externs) = tempExterns;/* If realloc worked */
                            }
                            strcpy((*externs)[(*externsCount) - ONE].name, labelToken);
                            (*externs)[(*externsCount) - ONE].base = RAMIndex;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                        }
                    } else if (firstOperandAddressMethod == ZERO) {
                        RAMIndex++;
                    }

                    if (secondOperandAddressMethod == ONE || secondOperandAddressMethod == TWO) {
                        if (secondOperandAddressMethod == ONE) {
                            strcpy(labelToken, commandOperands[ONE]);
                            labelIndex = labelNum(labelToken, (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 4 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if (secondOperandAddressMethod == TWO) {
                            strcpy(labelToken, strtok(commandOperands[ONE], "["));
                            labelIndex = labelNum(labelToken, (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 5 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if ((*labels)[labelIndex].base != ZERO) {
                            RAM[RAMIndex] |= (*labels)[labelIndex].base;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                            RAM[RAMIndex] |= (*labels)[labelIndex].offset;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                        } else /* If it's an extern label */
                        {
                            if (*externsCount == ZERO)/* If this is the first label encountered */
                            {
                                (*externs) = (label *) malloc(sizeof(label));
                                if ((*externs) == NULL) {
                                    puts("Error - Dynamic allocation for the externs array failed, causing the program to terminate prematurely. \n");
                                    exit(EXIT);
                                }
                                (*externsCount)++; /* Raise the label count */
                            } else {
                                (*externsCount)++; /* Raise the label count */
                                tempExterns = (label *) realloc((*externs), (*externsCount) * sizeof(label));
                                if (tempExterns == NULL) {
                                    puts("Dynamic allocation for the externs array failed, causing the program to terminate prematurely. \n");
                                    exit(EXIT);
                                }
                                (*externs) = tempExterns;/* If realloc worked */
                            }
                            strcpy((*externs)[(*externsCount) - ONE].name, labelToken);
                            (*externs)[(*externsCount) - ONE].base = RAMIndex;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                        }
                    } else if (secondOperandAddressMethod == ZERO) {
                        RAMIndex++;
                    }

                    break;
                    /* Methods that take 1 operand */
                case FIVE:
                case SIX:
                case SEVEN:
                case EIGHT:
                case NINE:
                case TEN:
                case ELEVEN:
                case TWELVE:
                case THIRTEEN:

                    firstOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                    RAMIndex += TWO;       /* Two more lines - One for the opcode - Second for funct and  the addressing methods and source and destination */
                    if (firstOperandAddressMethod == ONE || firstOperandAddressMethod == TWO) {
                        if (firstOperandAddressMethod == ONE) {
                            strcpy(labelToken, commandOperands[ZERO]);
                            labelIndex = labelNum(commandOperands[ZERO], (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 6 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if (firstOperandAddressMethod == TWO) {
                            strcpy(labelToken, strtok(commandOperands[ZERO], "["));
                            labelIndex = labelNum(labelToken, (*labels),labelCount);  /* This function gets a label and returns its number in the label array */
                            if (labelIndex == MINUS_ONE) {
                                printf("Line %d: Label wasn't found 7 \n", lineCounter);
                                errorFlag = TRUE; /* Encountered an error */
                                continue;/* Continue to the next line */
                            }
                        }
                        if ((*labels)[labelIndex].base != ZERO) {
                            RAM[RAMIndex] |= (*labels)[labelIndex].base;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                            RAM[RAMIndex] |= (*labels)[labelIndex].offset;/* Enter the base of the label into the RAM */
                            RAM[RAMIndex] |= ONE << INT_R;
                            RAMIndex++;
                        } else /* If it's an extern label */
                        {
                            if ((*externsCount) == ZERO)/* If this is the first label encountered */
                            {
                                (*externs) = (label *) malloc(sizeof(label));
                                if ((*externs) == NULL) {
                                    puts("Error - Dynamic allocation for the externs array failed, causing the program to terminate prematurely.\n");
                                    exit(EXIT);
                                }
                                (*externsCount)++; /* Raise the label count */
                            } else {
                                (*externsCount)++; /* Raise the label count */
                                tempExterns = (label *) realloc((*externs), (*externsCount) * sizeof(label));
                                if (tempExterns == NULL) {
                                    puts("Dynamic allocation for the externs array failed, causing the program to terminate prematurely. \n");
                                    exit(EXIT);
                                }
                                (*externs) = tempExterns;/* If realloc worked */
                            }
                            strcpy((*externs)[(*externsCount) - ONE].name, labelToken);
                            (*externs)[(*externsCount) - ONE].base = RAMIndex;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                            RAM[RAMIndex] |= ONE << INT_E;
                            RAMIndex++;
                        }
                    }
                    if (firstOperandAddressMethod == ZERO) {
                        RAMIndex++;
                    }

                    break;

                    /* Methods that don't take any operands */
                case FOURTEEN:
                case FIFTEEN:

                    RAMIndex++;
                    break;
            }
        }
    }

    if (errorFlag == TRUE) {
        return MINUS_ONE;
    }
    return ONE;
}

