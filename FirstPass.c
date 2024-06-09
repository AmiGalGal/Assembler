#include "Project.h"

/*
 * The first pass through the code, handling initial processing of the text.
 */

int firstPass(FILE *newfp, int RAM[RAM_SIZE], label **labels, int *labelCount, command commands[],
              char registerTable[MAX_LINE_SIZE][MAX_LINE_SIZE], int *DCF, int *extFlag, int *entryFlag) {
    char line[MAX_LINE_SIZE];            /* Stores the current line */
    int wordAmount = ZERO;            /* Number of words in the line */
    char *wordArray[MAX_LINE_SIZE];        /* Array containing the line's words */
    int labelFlag = ZERO;                   /* Indicates presence of a label */
    label *tempLabels;                      /* Temporary storage for labels array */
    int IC = RAM_START;               /* RAM memory allocation index */
    char intLine[MAX_LINE_SIZE];           /* Temporary line of numbers separated by "," */
    int intArray[MAX_LINE_SIZE] = {ZERO};     /* Array to store numbers */
    int numInLineCounter = ZERO;            /* Number of numbers in the line */
    int numOfCommas = ZERO;                 /* Number of commas in the line */
    int dataLineIndex = ZERO;              /* Index for numbers in data */
    int distance = ZERO;                    /* Size of the label in memory */
    char strLabelWithoutColon[MAX_LINE_SIZE];/* Label name without colon */
    int labelStatus = ZERO;                        /* Result of checkLabelName method */
    int commandIndex = ZERO;                /* Command number */
    int lengthOfCommand = ZERO;             /* Command variables */
    char commandOperands[MAX_LINE_SIZE][MAX_LINE_SIZE];       /* Command operands */
    int operandsCounter = ZERO;               /* Number of operands in a line  */
    int firstOperandAddressMethod = ZERO;      /* Address method of first operand */
    int secondOperandAddressMethod = ZERO;      /* Address method of second operand */
    int lineCounter = ZERO;        /* Line index */
    int errorFlag = FALSE;          /* Error flag */
    int DC = ZERO;                  /* Progress in the data set */
    int DCTable[RAM_SIZE] = {ZERO}; /* Data storage */
    int ICF;                        /* Final IC */
    int index = ZERO;       /* Loop index */


    fseek(newfp, ZERO, SEEK_SET); /* Move to the beginning of newfp */

    while (fgets(line, MAX_LINE_SIZE, newfp))    /* Read a line from the file */
    {
        wordAmount = makeItArray(line, wordArray);    /* Split the line into an array of words */
        lineCounter++;
        if (wordArray[ZERO][ZERO] == ';' || strcmp(wordArray[ZERO], "\n") == ZERO ||
            strcmp(wordArray[ZERO], "\t") == ZERO)    /* Empty or comment line */
        {
            /* Ignore the line */
        } else {
            labelFlag = ZERO;
            if ((wordArray[ZERO][strlen(wordArray[ZERO]) - ONE]) == ':')  /* Label detection */
            {

                labelWithoutColon(wordArray[ZERO], strLabelWithoutColon);
                /* Check if the label already exists */
                if (checkLabelName(strLabelWithoutColon, *labels, *labelCount) == MINUS_TWO) {
                    printf("Line %d: There's an issue with the label name.\n", lineCounter);
                    errorFlag = TRUE;
                    continue;   /* Move to the next iteration */
                } else {
                    labelFlag = TRUE;/* Indicate presence of a label */
                    distance = ZERO;/* Reset distance */
                }
            }
            /* Check if it's a data command */
            /*
             * ZERO + labelFlag  can be 0 or 1*/
            if ((strcmp(wordArray[ZERO + labelFlag], ".data") == ZERO ||
                 strcmp(wordArray[ZERO + labelFlag], ".string") == ZERO)) /* If the second word is .data or .string */
            {
                if (labelFlag == TRUE) {
                    if (*labelCount == ZERO)    /* If this is the first encountered label */
                    {
                        (*labels) = (label *) malloc(sizeof(label));
                        if ((*labels) == NULL) {
                            puts("Error - The program terminates due to the inability to dynamically allocate a labels array. \n");
                            exit(EXIT);
                        }
                        (*labelCount)++; /* Increment label count*/
                    } else {
                        if ((*labels) != NULL) {
                            free(*labels); /* Free previously allocated memory */
                        }
                        (*labelCount)++; /* Increment label count*/
                        tempLabels = (label *) realloc((*labels), (*labelCount) * sizeof(label));
                        if (tempLabels == NULL) {
                            puts("Error - The program terminates due to the inability to dynamically allocate a labels array. \n");
                            exit(EXIT);
                        }
                        (*labels) = tempLabels;/* If realloc successful */
                    }

                    /* Enter the data */
                    removesNewLine(strLabelWithoutColon);
                    strcpy((*labels)[*labelCount - STEP].name, strLabelWithoutColon);
                    (*labels)[(*labelCount) - STEP].attributesCount = ZERO;
                    (*labels)[(*labelCount) - STEP].address = DC;
                    (*labels)[(*labelCount) -
                              STEP].attributesCount++;        /* Num of attributes plus one because we add an attribute */

                    if (strcmp(wordArray[START], ".data") == ZERO) {
                        strcpy((*labels)[*labelCount - STEP].attributes[(*labels)[*labelCount - STEP].attributesCount -
                                                                        STEP], "data");
                        /* Concat the numbers into a line separated by commas */
                        makeIntLine(wordArray, wordAmount, intLine, labelFlag);

                        /* Count the number of commas */
                        numOfCommas = countCommas(intLine);

                        /* Convert the line of numbers to an array. If there are problems, return -2 */
                        numInLineCounter = makeItNumArray(intArray, intLine);
                        if (numInLineCounter < ZERO) {
                            printf("Line %d: The data entry is incorrect. \n", lineCounter);
                            errorFlag = TRUE;
                            continue;/* Move to the next iteration*/
                        }

                        /*
                         * Check if the line is balanced in numbers and commas
                         */
                        if (numOfCommas + ONE != numInLineCounter) {
                            printf("Line %d: The numbers and commas are not balanced.\n", lineCounter);
                            errorFlag = TRUE;
                            continue; /* Move to the next iteration*/
                        }
                        for (dataLineIndex = ZERO; dataLineIndex < numInLineCounter; dataLineIndex++) {

                            DCTable[DC] = intArray[dataLineIndex];
                            DCTable[DC] |= (ONE << INT_A);
                            DCTable[DC] &= TURN_OFF_BITS_FOR_NEGATIVE_NUMBERS;
                            DC++;
                            distance++;
                        }
                    } else if (strcmp(wordArray[START], ".string") == ZERO) {
                        strcpy((*labels)[(*labelCount) - STEP].attributes[
                                       (*labels)[*labelCount - STEP].attributesCount - STEP], "data");

                        if (wordAmount != THREE) {
                            printf("Line %d: There are too many arguments in the data line.\n", lineCounter);
                            errorFlag = TRUE;
                            continue;   /* Move to the next iteration*/
                        } else {
                            /*
                             * Enter each letter as a word in the RAM until
                             * 1.  Enter each letter ASCII value to line in RAM
                             * 2. RAMIndex++
                             * 3. label.length++
                             * 4. Turn on 19 switch on (R is on)
                             */
                            for (dataLineIndex = ZERO; wordArray[TWO][dataLineIndex] != '\0'; dataLineIndex++) {
                                if (wordArray[TWO][dataLineIndex] != '"' && wordArray[TWO][dataLineIndex] != '\n') {
                                    DCTable[DC] = (int) wordArray[TWO][dataLineIndex];
                                    DCTable[DC] |= (ONE << INT_A);
                                    DC++;
                                    distance++;
                                }

                                /*
                                 * 1. Enter each letter ASCII value to line in RAM
                                 * 2. RAMIndex++
                                 * 3. Label.length++
                                 * 4. Turn on 19 switch on (R is on)
                                 */
                            }
                            DCTable[DC] = ZERO;
                            DCTable[DC] |= (ONE << INT_A);
                            DC++;
                            distance++;
                        }
                    }
                    (*labels)[(*labelCount) - STEP].length = distance;
                }
                    /*
                     * Data but label flag is off
                     */
                else {
                    /* Concatenate the numbers into a line separated by commas */

                    makeIntLine(wordArray, wordAmount, intLine, labelFlag);

                    /* Count the number of commas */
                    numOfCommas = countCommas(intLine);

                    /* Convert the line of numbers into an array. If there are problems, return -2 */
                    numInLineCounter = makeItNumArray(intArray, intLine);
                    if (numInLineCounter < ZERO) {
                        printf("Line %d: The data entry is incorrect. \n", lineCounter);
                        errorFlag = TRUE;
                        continue;   /* Move to the next iteration*/
                    }

                    /*
                     * Check if the line is balanced in numbers and commas
                     */
                    if (numOfCommas + ONE != numInLineCounter && numOfCommas != ZERO) {
                        printf("Line %d: The numbers and commas are not balanced.\n", lineCounter);
                        errorFlag = TRUE;
                        continue; /* Move to the next iteration*/
                    }

                    for (dataLineIndex = ZERO; dataLineIndex < numInLineCounter; dataLineIndex++) {
                        DCTable[DC] = intArray[dataLineIndex];
                        DCTable[DC] |= (ONE << INT_A);
                        DCTable[DC] &= TURN_OFF_BITS_FOR_NEGATIVE_NUMBERS;
                        DC++;
                        distance++;
                    }

                }
            }
                /*
                 * Check if we encountered extern or entry type lines
                 */
            else if ((strcmp(wordArray[ZERO + labelFlag], ".extern") == ZERO) ||
                     (strcmp(wordArray[ZERO + labelFlag], ".entry")) == ZERO) {
                labelWithoutColon(wordArray[START + labelFlag], strLabelWithoutColon);
                labelStatus = checkLabelName(strLabelWithoutColon, (*labels), *labelCount);
                if ((strcmp(wordArray[ZERO + labelFlag], ".entry")) == ZERO) {
                    *entryFlag = TRUE;
                    continue;   /* Move to the next iteration */
                } else if (labelStatus == MINUS_TWO) {
                    printf("Line %d: There's an issue with the label name.\n", lineCounter);
                    errorFlag = TRUE;
                    continue;       /* Move to the next iteration*/
                } else if (labelStatus == MINUS_ONE) {
                    if (*labelCount == ZERO)    /* If this is the first label encountered */
                    {
                        (*labels) = (label *) malloc(sizeof(label));
                        if ((*labels) == NULL) {
                            puts("Error - The program terminates due to the inability to dynamically allocate a labels array. \n");
                            exit(EXIT);
                        }
                        (*labelCount)++; /* Increment label count*/
                    } else {
                        (*labelCount)++; /* Increment label count*/
                        tempLabels = (label *) realloc((*labels), (*labelCount) * sizeof(label));
                        if (tempLabels == NULL) {
                            puts("Error- The program terminates due to the inability to dynamically allocate a labels array. \n");
                            exit(EXIT);
                        }
                        (*labels) = tempLabels;/* If realloc worked*/
                    }
                    removesNewLine(wordArray[START + labelFlag]);
                    strcpy((*labels)[(*labelCount) - STEP].name, wordArray[START + labelFlag]);
                    (*labels)[(*labelCount) - STEP].attributesCount = ZERO;
                    (*labels)[(*labelCount) - STEP].attributesCount++;        /* Num of attributes plus one because we add an attribute */
                    (*labels)[(*labelCount) - STEP].address = ZERO;
                    (*labels)[(*labelCount) - STEP].base = ZERO;
                    (*labels)[(*labelCount) - STEP].offset = ZERO;
                    strcpy((*labels)[(*labelCount) - STEP].attributes[(*labels)[(*labelCount) - STEP].attributesCount - STEP], "extern");
                    *extFlag = TRUE;

                } else {
                    /* If the label already exists, check if it has an extern attribute too */
                    if (strcmp((*labels)[labelStatus].attributes[ZERO], "extern") != ZERO &&
                        (*labels)[labelStatus].attributesCount != START) {
                        /* The label has other attributes beside extern attribute, resulting in an error */
                        printf("Line %d: The label has attributes other than extern, which is incorrect. An extern label should only have an extern attribute, but extern was not added to the label.\n",
                               lineCounter);
                        errorFlag = TRUE;
                        continue;       /* Move to the next iteration*/
                    }
                }

            } else {
                /*
               * Line 11 page 37 - It's a command line!
               */
                if (labelFlag == TRUE) {
                    labelWithoutColon(wordArray[ZERO], strLabelWithoutColon);


                    labelStatus = checkLabelName(strLabelWithoutColon, (*labels), *labelCount);
                    if (labelStatus == MINUS_TWO) {
                        printf("Line %d: There's an issue with the label name.\n", lineCounter);
                        errorFlag = TRUE;
                        continue;/* Move to the next iteration*/
                    } else if (labelStatus > MINUS_ONE) {
                        printf("Line %d: The label has already been defined, making it impossible to define it again.\n",
                               lineCounter);
                        errorFlag = TRUE;
                        continue;       /* Move to the next iteration*/
                    } else {
                        if ((*labelCount) == ZERO)      /* If this is the first label encountered */
                        {
                            (*labels) = (label *) malloc(sizeof(label));
                            if ((*labels) == NULL) {
                                puts("Error - The program terminates due to the inability to dynamically allocate a labels array. \n");
                                exit(EXIT);
                            }
                            (*labelCount)++; /* Increment label count*/
                        } else {
                            (*labelCount)++; /* Increment label count*/
                            tempLabels = (label *) realloc((*labels), (*labelCount) * sizeof(label));
                            (*labels)[(*labelCount) - ONE].length = ZERO;
                            if (tempLabels == NULL) {
                                puts("Error- The program terminates due to the inability to dynamically allocate a labels array. \n");
                                exit(EXIT);
                            }
                            (*labels) = tempLabels; /* If realloc worked*/
                        }
                        removesNewLine(strLabelWithoutColon);
                        strcpy((*labels)[*labelCount - ONE].name, strLabelWithoutColon);
                        (*labels)[(*labelCount) - ONE].address = IC;
                        (*labels)[(*labelCount) - ONE].attributesCount = ZERO;
                        (*labels)[(*labelCount) - ONE].attributesCount++;        /* Num of attributes plus one because we add an attribute */
                        enterBaseAndOffset(&(*labels)[*labelCount - ONE], IC);
                        strcpy((*labels)[*labelCount - ONE].attributes[(*labels)[(*labelCount) - ONE].attributesCount -
                                                                       ONE], "code");
                    }
                }
                lengthOfCommand = ZERO;
                /* Search for the name of the command in the table */
                commandIndex = searchCommand(commands, wordArray[ZERO + labelFlag]);

                if (commandIndex == MINUS_ONE) {
                    printf("Line %d: The command was not found, proceeding to the next line. \n", lineCounter);
                    errorFlag = TRUE;
                    continue;   /* Move to the next iteration*/
                } else {

                    removesNewLine(wordArray[ZERO + labelFlag]);
                    if ((strcmp(wordArray[ZERO + labelFlag], "rts")) == ZERO ||
                        (strcmp(wordArray[ZERO + labelFlag], "stop")) == ZERO) {
                        operandsCounter = ZERO;
                    } else {
                        operandsCounter = lineForCommand(wordArray, labelFlag, wordAmount, commandOperands,
                                                         lineCounter);
                        if (operandsCounter == MINUS_ONE) {
                            /* An error printing was made inside the lineForCommand function */
                            continue;   /* Move to the next iteration*/
                        }
                    }

                    switch (commandIndex) {
                        /* The methods that get 2 operands */
                        case ZERO:
                        case ONE:
                        case TWO:
                        case THREE:
                        case FOUR:

                            /* Check if we have enough operands for that method*/
                            if (operandsCounter != TWO) {
                                numOfOperandsNotMatch(lineCounter);
                                errorFlag = TRUE;
                                continue;   /* Move to the next iteration*/
                            }

                            /* Find the number of the Addressing Methods*/
                            firstOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                            secondOperandAddressMethod = findAddressMethod(commandOperands[STEP], registerTable, lineCounter);

                            /* Checks whether there is a match between the method and the operand it can receive */
                            if (firstOperandAddressMethod == MINUS_ONE || secondOperandAddressMethod == MINUS_ONE) {
                                printf("Line %d: Error - There is an error in the address operand.\n", lineCounter);
                                errorFlag = TRUE;
                            }

                            if (commandIndex == FOUR &&
                                (firstOperandAddressMethod == ZERO || firstOperandAddressMethod == THREE)) {
                                printf("Line %d: The lea command cannot utilize this addressing method for the source operand.", lineCounter);
                                errorFlag = TRUE;
                            }

                            if (secondOperandAddressMethod == ZERO && commandIndex != ONE) {
                                printf("Line %d- This method is unable to retrieve the immediate address mode in the destination operand.", lineCounter);
                                errorFlag = TRUE;
                            }

                            lengthOfCommand += TWO;      /* Two more lines - One for the opcode - Second for funct and  the addressing methods and source and destination registers */

                            /* Adds the number of rows according to the appropriate addressing method */
                            lengthOfCommand += addLength(firstOperandAddressMethod);    /* Will add the length needed based on the addressing method */
                            lengthOfCommand += addLength(secondOperandAddressMethod);    /* Will add the length needed based on the addressing method */

                            break;
                            /*The method gets 1 operand */
                        case FIVE:
                        case SIX:
                        case SEVEN:
                        case EIGHT:
                        case NINE:
                        case TEN:
                        case ELEVEN:
                        case TWELVE:
                        case THIRTEEN:


                            /* Check if we have enough operands for that method*/
                            if (operandsCounter != ONE) {
                                numOfOperandsNotMatch(lineCounter);
                                errorFlag = TRUE;
                                continue;   /* Move to the next iteration*/
                            }
                            /* Find the number of the Addressing Methods*/
                            firstOperandAddressMethod = findAddressMethod(commandOperands[ZERO], registerTable, lineCounter);
                            /* Checking for errors*/
                            if (firstOperandAddressMethod == MINUS_ONE) {
                                printf("Line %d: There is an error in the address operand. \n", lineCounter);
                                errorFlag = TRUE;
                                continue;
                            }

                            /* Checks whether there is a match between the method and the operand it can receive */
                            if (commandIndex != THIRTEEN && firstOperandAddressMethod == ZERO) {
                                printf("Line %d: This method cannot retrieve the register direct address mode in the destination operand.", lineCounter);
                                errorFlag = TRUE;
                                continue;
                            }

                            if (firstOperandAddressMethod == THREE &&
                                (commandIndex == NINE || commandIndex == TEN || commandIndex == ELEVEN)) {
                                printf("Line %d: This method cannot retrieve the register direct address mode in the destination operand.", lineCounter);
                                errorFlag = TRUE;
                                continue;
                            }

                            /* Adds the number of rows according to the appropriate addressing method */
                            lengthOfCommand += TWO;      /* Two more lines - One for the opcode - Second for funct and  the addressing methods and source and destination registers */
                            lengthOfCommand += addLength(firstOperandAddressMethod);    /* Will add the length needed based on the addressing method */

                            break;

                            /* The method doesn't get any operand */
                        case FOURTEEN:
                        case FIFTEEN:

                            /* Check if we have enough operands for that method */
                            if (operandsCounter != ZERO) {
                                numOfOperandsNotMatch(lineCounter);
                                errorFlag = TRUE;
                                continue; /* Move to the next iteration*/
                            }

                            lengthOfCommand = ONE;
                            break;
                    }
                    /* The method will change the RAM to contain the values that are supposed to be inserted to the RAM*/
                    changeMemoryFirstCross(firstOperandAddressMethod, secondOperandAddressMethod, RAM, IC, commandIndex, commands, operandsCounter, registerTable, commandOperands);
                    /* The label length is change only if we have a label */


                    if (labelFlag == START) {
                        (*labels)[(*labelCount) - ONE].length = lengthOfCommand;
                    }
                    IC += lengthOfCommand; /* Setting RAMIndex to the next free index*/
                }
            }
        }
    }


    if (errorFlag == TRUE)   /* If there was en error in one of the lines*/
    {
        return FALSE;
    }
    *DCF = DC;
    ICF = IC;

    for (index = ZERO; index < *labelCount; index++)   /* Setting the new addresses for data labels*/
    {
        if (strcmp((*labels)[index].attributes[ZERO], "data") == ZERO) {
            enterBaseAndOffset(&(*labels)[index], ICF + (*labels)[index].address);
        }
    }

    for (index = ZERO; index < *DCF; index++)   /* Copying all data from DCTable to the end of the RAM*/
    {
        RAM[IC + index] = DCTable[index];
    }
    return ICF;
}
