
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main (int argc, char *argv[])
{
    char* input_name;
    FILE* input_file;

    char* output_name;
    FILE* output_file;

    char* variable_name;

    if (4 != argc)
    {
        puts("Usage: bin2h [input] [output] [variable]");
        return EXIT_FAILURE;
    }

    input_name = argv[1];
    output_name = argv[2];
    variable_name = argv[3];

    input_file = fopen(input_name, "rb");

    if (NULL == input_file)
    {
        printf("Could not open input file '%s': %s\n", input_name, strerror(errno));
        return EXIT_FAILURE;
    }

    output_file = fopen(output_name, "w");

    if (NULL == output_file)
    {
        printf("Could not open output file '%s': %s\n", output_name, strerror(errno));
        return EXIT_FAILURE;
    }

    if (fprintf(output_file, "static const unsigned char %s[] = {", variable_name) < 0)
    {
        printf("Could not write to output file '%s': %s\n", output_name, strerror(ferror(output_file)));
        return EXIT_FAILURE;
    }

    while (0 == feof(input_file))
    {
        unsigned char buffer[4096];
        size_t i, count = fread(buffer, 1, sizeof(buffer), input_file);

        if (sizeof(buffer) != count)
        {
            if (0 == feof(input_file) || 0 != ferror(input_file))
            {
                printf("Could not read from input file '%s': %s\n", input_name, strerror(ferror(input_file)));
                return EXIT_FAILURE;
            }
        }

        for (i = 0; i < count; ++i)
        {
            if ((i & 15) == 0)
            {
                if (fprintf(output_file, "\n  ") < 0)
                {
                    printf("Could not write to output file '%s': %s\n", output_name, strerror(ferror(output_file)));
                    return EXIT_FAILURE;
                }
            }

            if (fprintf(output_file, "0x%2.2x, ", buffer[i]) < 0)
            {
                printf("Could not write to output file '%s': %s\n", output_name, strerror(ferror(output_file)));
                return EXIT_FAILURE;
            }

        }
    }

    if (fprintf(output_file, "\n};\n") < 0)
    {
        printf("Could not write to output file '%s': %s\n", output_name, strerror(ferror(output_file)));
        return EXIT_FAILURE;
    }

    if (fclose(output_file) < 0)
    {
        printf("Could not close output file '%s': %s\n", output_name, strerror(ferror(output_file)));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
