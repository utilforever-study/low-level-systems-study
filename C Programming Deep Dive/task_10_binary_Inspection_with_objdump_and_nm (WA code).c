#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_MAX_LENGTH 8192
#define MAX_SYMBOLS 4096

typedef struct SectionInfo
{
    char* name;
    int count;
    int bytes;
} SectionInfo;

typedef struct LineInfo
{
    int is_init;
    int is_func;
    int is_def;

    int is_static;
    int is_const;

    int total_size;
    int is_zero_init;
    int has_address_init;
    char* name;
} LineInfo;

typedef struct InitInfo
{
    int count;
    int is_zero;
    int has_address;
    int is_string;
    int string_length;
    int is_braced_list;
} InitInfo;

SectionInfo section_infos[7];
int line_count;
LineInfo line_infos[MAX_SYMBOLS];
int input_line_count;

char* string_literals[MAX_SYMBOLS];
int string_lengths[MAX_SYMBOLS];
int literal_count;
int literal_bytes;

int get_type_size(char* type)
{
    if (strcmp(type, "char") == 0)
        return 1;
    if (strcmp(type, "short") == 0)
        return 2;
    if (strcmp(type, "int") == 0)
        return 4;
    if (strcmp(type, "long") == 0)
        return 8;
    if (strcmp(type, "float") == 0)
        return 4;
    if (strcmp(type, "double") == 0)
        return 8;
    if (strcmp(type, "void") == 0)
        return 0;
    return -1;
}

void skip_spaces(char** c)
{
    while (isspace((unsigned char)**c))
        (*c)++;
}

// Read one identifier. Return 0 when it is not an identifier.
int read_word(char** c, char* word)
{
    int p = 0;
    skip_spaces(c);
    if (!isalpha((unsigned char)**c) && **c != '_')
        return 0;

    while (isalnum((unsigned char)**c) || **c == '_')
        word[p++] = *(*c)++;

    word[p] = '\0';
    return 1;
}

int read_keyword(char** c, char* keyword)
{
    char* start = *c;
    char word[LINE_MAX_LENGTH];
    
    if (read_word(c, word) && strcmp(word, keyword) == 0)
        return 1;
    *c = start;
    
    return 0;
}

int read_quoted(char** c, char* result)
{
    char quote = *(*c)++;
    int p = 0;
    
    while (**c != quote && **c != '\0')
    {
        char value = *(*c)++;

        if (value == '\\')
        {
            if (**c == '\0')
                return -1;

            value = *(*c)++;

            if (value == '0') 
                value = '\0';
            else if (value == 'n') 
                value = '\n';
        }
        result[p++] = value;
    }
    
    if (**c != quote)
        return -1;

    (*c)++;
    return p;
}

int read_init_value(char** c, InitInfo* init, int is_pointer)
{
    char word[LINE_MAX_LENGTH];
    char* end;
    skip_spaces(c);

    if (**c == '&')
    {
        (*c)++;
        if (!read_word(c, word))
            return 0;
        init->is_zero = 0;
        init->has_address = 1;
    }
    else if (**c == '"' || **c == '\'')
    {
        char quote = **c;
        int length = read_quoted(c, word);

        if (length < 0 || (quote == '\'' && length != 1))
            return 0;

        if (quote == '"')
        {
            init->is_string = 1;
            init->string_length = length;
            
            if (is_pointer)
            {
                init->has_address = 1;
                init->is_zero = 0;
            }
        }
        for (int i = 0; i < length; i++)
        {
            if (word[i] != '\0')
            {
                init->is_zero = 0;
            }
        }
    }
    else
    {
        long double number = strtold(*c, &end);
        
        if (end == *c)
            return 0;
        if (number != 0)
            init->is_zero = 0;
        *c = end;
        
        while (**c == 'u' || **c == 'U' || **c == 'l' || **c == 'L' || **c == 'f' || **c == 'F')
            (*c)++;
    }
    
    init->count++;
    return 1;
}

int read_initializer(char** c, InitInfo* init, int is_pointer)
{
    skip_spaces(c);
    if (**c == '{')
    {
        init->is_braced_list = 1;
        (*c)++;
        
        while (1)
        {
            if (!read_init_value(c, init, is_pointer))
                return 0;

            skip_spaces(c);
            
            if (**c == '}')
            {
                (*c)++;
                break;
            }
            if (**c != ',')
            {
                return 0;
            }

            (*c)++;
            skip_spaces(c);

            if (**c == '}')
            {
                (*c)++;
                break;
            }
        }
        return 1;
    }
    
    return read_init_value(c, init, is_pointer);
}

void add_literal(char* string, int length)
{
    for (int i = 0; i < literal_count; i++)
    {
        if (string_lengths[i] == length && memcmp(string_literals[i], string, length) == 0)
        {
            return;
        }
    }
    
    string_literals[literal_count] = malloc(length + 1);
    memcpy(string_literals[literal_count], string, length);
    
    string_lengths[literal_count++] = length;
    literal_bytes += length + 1;
}

void collect_literals(char* start, char* end)
{
    char word[LINE_MAX_LENGTH];

    while (start < end)
    {
        int is_double_quote = (unsigned char)*start == '\"';
        int is_single_quote = (unsigned char)*start == '\'';

        if (is_double_quote || is_single_quote)
        {
            int length = read_quoted(&start, word);

            if (length < 0)
                return;

            if (is_double_quote)
                add_literal(word, length);
        }
        else
        {
            start++;
        }
    }
}

int report_cannot_parse(char* line)
{
    int length = strlen(line);

    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r'))
    {
        line[--length] = '\0';
    }

    printf("line %d: cannot parse: %s\n", input_line_count, line);
    return 0;
}

int process_line(char* line)
{
    char word[LINE_MAX_LENGTH];
    int step = 0;

    int is_init = 0;
    int is_func = 0;
    int is_def = 0;

    int is_static = 0;
    int is_extern = 0;
    int is_const = 0;
    int is_signed_type = 0;
    int is_pointer = 0;

    int type_size = 0;
    int array_count = 0;
    int total_size = 0;
    char name[LINE_MAX_LENGTH] = {0};

    int is_array = 0;
    int copied_string = 0;
    char* init_start = NULL;
    char* init_end = NULL;
    InitInfo init = {0};
    init.is_zero = 1;

    char* c = line;
    skip_spaces(&c);
    
    if (*c == '\0' || (*c == '/' && c[1] == '/'))
        return 1;

    while (step <= 8)
    {
        skip_spaces(&c);
        if (step == 0)
        {
            if (read_keyword(&c, "static"))
                is_static = 1;
            else if (read_keyword(&c, "extern"))
                is_extern = 1;
            step++;
        }
        else if (step == 1)
        {
            if (read_keyword(&c, "const"))
                is_const = 1;
            step++;
        }
        else if (step == 2)
        {
            if (read_keyword(&c, "unsigned") || read_keyword(&c, "signed"))
                is_signed_type = 1;
            step++;
        }
        else if (step == 3)
        {
            char* before = c;
            if (!read_word(&c, word))
                return report_cannot_parse(line);

            type_size = get_type_size(word);
            if (type_size < 0)
            {
                if (!is_signed_type)
                    return report_cannot_parse(line);

                strcpy(word, "int");
                c = before;
                type_size = 4;
            }
            
            if (is_signed_type && (type_size == 0 || strcmp(word, "float") == 0 || strcmp(word, "double") == 0))
                return report_cannot_parse(line);
            step++;
        }
        else if (step == 4)
        {
            if (read_keyword(&c, "const"))
                is_const = 1;
            step++;
        }
        else if (step == 5)
        {
            while (1)
            {
                skip_spaces(&c);
                if (*c != '*')
                    break;

                c++;
                is_pointer = 1;
                is_const = read_keyword(&c, "const");
            }
            
            if (is_pointer)
                type_size = 8;
            step++;
        }
        else if (step == 6)
        {
            if (!read_word(&c, name))
                return report_cannot_parse(line);
            step++;
        }
        else if (step == 7)
        {
            skip_spaces(&c);
            if (*c == '(')
            {
                is_func = 1;
                int depth = 1;
                c++;
                while (*c && depth)
                {
                    if (*c == '(')
                        depth++;
                    else if (*c == ')')
                        depth--;
                    c++;
                }
                if (depth)
                    return report_cannot_parse(line);
            }
            else
            {
                if (type_size == 0 && !is_pointer)
                    return report_cannot_parse(line);

                if (*c == '[')
                {
                    is_array = 1;
                    c++;
                    skip_spaces(&c);
                    
                    if (*c != ']')
                    {
                        char* end;
                        long n = strtol(c, &end, 10);
                        if (end == c || n < 1 || n > 100000)
                            return report_cannot_parse(line);

                        c = end;
                        array_count = (int)n;
                        skip_spaces(&c);
                    }
                    
                    if (*c != ']')
                        return report_cannot_parse(line);
                    c++;
                }
            }
            step++;
        }
        else if (step == 8)
        {
            skip_spaces(&c);
            if (is_func)
            {
                if (*c == ';')
                {
                    c++;
                }
                else if (*c == '{')
                {
                    is_def = 1;
                    char* end = strrchr(c, '}');
                    
                    if (!end)
                        return report_cannot_parse(line);
                    c = end + 1;
                }
                else
                {
                    return report_cannot_parse(line);
                }
            }
            else
            {
                if (*c == '=')
                {
                    is_init = 1;
                    c++;
                    init_start = c;
                    if (!read_initializer(&c, &init, is_pointer))
                        return report_cannot_parse(line);
                    init_end = c;
                }
                skip_spaces(&c);
                
                if (*c != ';')
                    return report_cannot_parse(line);

                c++;
                is_def = !is_extern || is_init;
                
                copied_string = is_array &&
                    !is_pointer &&
                    type_size == 1 &&
                    init.is_string &&
                    !init.is_braced_list;

                if (is_array)
                {
                    if (array_count == 0)
                    {
                        if (!is_init)
                            return report_cannot_parse(line);

                        array_count = copied_string
                            ? init.string_length + 1
                            : init.count;
                    }

                    if (copied_string && array_count < init.string_length)
                        return report_cannot_parse(line);

                    total_size = type_size * array_count;
                }
                else
                {
                    total_size = type_size;
                }
            }
            step++;
        }
    }

    skip_spaces(&c);
    if (*c != '\0')
        return report_cannot_parse(line);

    LineInfo line_info = {0};
    line_info.is_init = is_init;
    line_info.is_func = is_func;
    line_info.is_def = is_def;
    line_info.is_static = is_static;
    line_info.is_const = is_const;
    line_info.total_size = total_size;
    line_info.is_zero_init = init.is_zero && !(init.is_string && !copied_string);
    line_info.has_address_init = init.has_address || (init.is_string && !copied_string);

    line_info.name = malloc(strlen(name) + 1);
    strcpy(line_info.name, name);

    if (is_init && init_start && init_end && !copied_string)
        collect_literals(init_start, init_end);
    int index = -1;

    for (int i = 0; i < line_count; i++)
    {
        if (strcmp(line_infos[i].name, name) == 0)
        {
            index = i;
            break;
        }
    }
    
    if (index >= 0)
    {
        if (line_infos[index].is_def && line_info.is_def)
        {
            printf("line %d: error: redefinition of %s\n", input_line_count, name);
            return 1;
        }

        if (!line_infos[index].is_def && line_info.is_def)
            line_infos[index] = line_info;
        else
            return 1;
    }
    else
    {
        line_infos[line_count++] = line_info;
    }

    return 1;
}

void process_line_info(LineInfo info)
{
    char* section;
    if (info.is_func)
    {
        if (info.is_def)
            section = ".text";
        else
            section = "undefined";
    }
    else
    {
        if (!info.is_def)
            section = "undefined";
        else if (info.is_const)
        {
            if (info.has_address_init)
                section = ".data.rel.ro";
            else
                section = ".rodata";
        }
        else
        {
            if (info.is_init && !info.is_zero_init)
                section = ".data";
            else
                section = ".bss";
        }
    }

    char section_char;
    if (section[0] == '.')
        section_char = section[1];
    else
        section_char = section[0];

    if (!info.is_static && strcmp(section, "undefined") != 0)
        section_char = section_char - 'a' + 'A';
    if (strcmp(section, "undefined") == 0)
        section_char = 'U';

    printf("%c %s", section_char, info.name);
    
    if (strcmp(section, "undefined") != 0)
        printf(" %s", section);
    if (!info.is_func && strcmp(section, "undefined") != 0)
        printf(" %d", info.total_size);
    printf("\n");

    for (int i = 0; i < 7; i++)
    {
        if (strcmp(section_infos[i].name, section) == 0)
        {
            section_infos[i].count++;
            section_infos[i].bytes += info.total_size;
            break;
        }
    }
}

int compare(const void *a, const void *b)
{
    const LineInfo *x = (const LineInfo *)a;
    const LineInfo *y = (const LineInfo *)b;
    return strcmp(x->name, y->name);
}

int main(void)
{
    section_infos[0].name = ".text";
    section_infos[1].name = ".rodata";
    section_infos[2].name = ".rodata.str1.1";
    section_infos[3].name = ".data";
    section_infos[4].name = ".data.rel.ro";
    section_infos[5].name = ".bss";
    section_infos[6].name = "undefined";

    char line[LINE_MAX_LENGTH];
    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        input_line_count++;
        process_line(line);
    }

    qsort(line_infos, line_count, sizeof(LineInfo), compare);

    for (int i = 0; i < line_count; i++)
    {
        process_line_info(line_infos[i]);
    }

    section_infos[2].count = literal_count;
    section_infos[2].bytes = literal_bytes;

    printf("--\n");

    for (int i = 0; i < 7; i++)
    {
        int count = section_infos[i].count;
        int bytes = section_infos[i].bytes;

        if (count == 0)
            continue;

        printf("%s: ", section_infos[i].name);

        if (strcmp(section_infos[i].name, ".text") == 0)
        {
            if (count >= 2)
            {
                printf("%d functions", count);
            }
            else
            {
                printf("%d function", count);
            }
        }
        else if (strcmp(section_infos[i].name, "undefined") == 0)
        {
            if (count >= 2)
            {
                printf("%d symbols", count);
            }
            else
            {
                printf("%d symbol", count);
            }
        }
        else if (strcmp(section_infos[i].name, ".rodata.str1.1") == 0)
        {
            if (count >= 2)
            {
                printf("%d literals, ", count);
            }
            else
            {
                printf("%d literal, ", count);
            }

            printf("%d bytes", bytes);
        }
        else
        {
            if (count >= 2)
            {
                printf("%d objects, ", count);
            }
            else
            {
                printf("%d object, ", count);
            }

            printf("%d bytes", bytes);
        }

        printf("\n");
    }

    return 0;
}
