#include "Project.h"

/*
 * Generate an output file for each label in the label table marked as an entry.
 * Each file includes the icon name, base address, and offset.
 */
void createEntryFile(FILE *entryFile, label **labels, int labelsCount) {

    int index = ZERO;
    int insideIndex = ZERO;

    /*
     * Set the file pointer to the beginning of the file.
     */
    fseek(entryFile, ZERO, SEEK_SET); /* Rewind to the start of the file */

    /*
     * Iterate through the entire label table to identify labels marked as entry.
     */
    for (index = ZERO; index < labelsCount; index++) {
        for (insideIndex = ZERO; insideIndex < (*labels)[index].attributesCount; insideIndex++) {
            /* Write to the file if an entry label is found. */
            if ((strcmp((*labels)[index].attributes[insideIndex], "entry")) == ZERO) {
                fprintf(entryFile, "%s,%d,%d\n", (*labels)[index].name, (*labels)[index].base, (*labels)[index].offset); /* Write the label name, base, and offset to the file */
            }
        }
    }
}

