//
// Created by arthurallilaire on 10/06/24.
//
#include "globals.h"
#include "parse_file.h"
#include "dispatch.h"

void parse_file(FILE *inputFile, line_processor processor, FILE *outputFile, context file_context) {
    // Read input file line by line
    file_context->prog_lineno = file_context->file_lineno = 1;
    for ( ; fgets( file_context->cur_line, MAXLINELEN, inputFile ) != NULL; file_context->prog_lineno++, file_context->file_lineno++)
    {
        // Remove newline character
        int len = strlen(file_context->cur_line);
        if( file_context->cur_line[len-1] == '\n' )
        {
            file_context->cur_line[len-1] = '\0';
        } else if (len == MAXLINELEN-1) {
//          Have to check as last line may not have \n
            error("Max buffer reached, line is too long, skipping line!", file_context);
            continue;
        }
        // Skip empty lines and comments and build_symbol table if label then skip line for continue
        if (file_context->cur_line[0] == '#' || file_context->cur_line[0] == '\0' || processor(file_context, outputFile)) {
            file_context->prog_lineno--; // Ignore blank lines
            continue;
        }
    }
}
//returns an instruction
uint32_t parseInstruction(context file_context) {
    char *opc_str;
    char *saveptr;
    char *rest_instr;
    char line_copy[MAXLINELEN];
    strncpy(line_copy, file_context->cur_line, MAXLINELEN); //strtok_r edits array
//    printf("%s\n", line_copy);
    opc_str = strtok_r(line_copy, " ", &saveptr);
//    printf("%s\n", opc_str);
    instr_processor opc_fun = get_instr_processor(opc_str);
    ERROR_ON_COND(opc_fun == NULL, "Invalid op-code!", file_context);

    rest_instr = strtok_r(NULL, "", &saveptr);
    rest_instr = (rest_instr == NULL) ? "" : rest_instr;

    return opc_fun(opc_str, rest_instr, file_context);
}
//Returns true if no errors
static bool write_instr_little_edian(uint32_t instr, FILE *file){
    int num_bytes = 4; uint8_t bytes[num_bytes];
    bytes[0] = (instr >> 0) & 0xFF;
    bytes[1] = (instr >> 8) & 0xFF;
    bytes[2] = (instr >> 16) & 0xFF;
    bytes[3] = (instr >> 24) & 0xFF;

    // Write the bytes to the file in little-endian order
    return (num_bytes == fwrite(bytes, sizeof(bytes[0]), num_bytes, file));
}

bool process_line(context file_context, FILE *outputFile) {
    // Check if line is a label
    if (strchr(file_context->cur_line, ':') != NULL) {
        return true; // Skip all labels
    }
    uint32_t binary_instr = parseInstruction(file_context);
    // Write little edian style
    bool no_errors = write_instr_little_edian(binary_instr, outputFile);
    if (!no_errors){
        error("Unable to write instruction to output file.", file_context);
    }
    return false;
}
