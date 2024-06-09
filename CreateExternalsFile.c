#include "Project.h"

/*
 * Generate an output file for each label in the label table marked as an extern.
 * Each entry includes the external label name followed by the keyword "BASE" and the corresponding address in the machine code where the base address is needed.
 * Subsequently, another line follows with the external symbol name, the keyword "OFFSET", and the address in the machine code where the offset is required.
 */
void createExternFile(FILE *externFile, label *externs, int externsCount) {

    int index = ZERO;

    fseek(externFile, ZERO, SEEK_SET); /* Reset to the beginning of externFile*/

    /*
     * Iterate through the instances where externals are utilized and record the name, base, and offset.
     */
    
    for (index = ZERO; index < externsCount; index++) {
        fprintf(externFile, "%s BASE %04d\n", externs[index].name, externs[index].base);/* Writes the line with the name and address to the file */
        fprintf(externFile, "%s OFFSET %04d\n", externs[index].name, externs[index].base + ONE);/* Writes the line with the name and offset to the file */
    }
}
