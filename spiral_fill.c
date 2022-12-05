vi #define _GNU_SOURCE
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>

const int buffer_size = 4096;
const char* left_down_corner = "\u2514"; /// └
const char* right_down_corner = "\u2518"; /// ┘
const char* left_top_corner = "\u250C"; /// ┌
const char* right_top_corner = "\u2510"; /// ┐
const char* vertical_right = "\u251c"; /// ├
const char* vertical_left = "\u2524"; /// ┤
const char* vertical_horizontal = "\u253C"; /// ┼
const char* vertical_line = "\u2502"; /// │
const char* horizontal_line = "\u2500"; /// ─
const char* horizontal_down = "\u252C"; /// ┬
const char* horizontal_up = "\u2534"; /// ┴

void draw_matrix(char *file_contents, const int height, const int cell_width, const int horizontal_delimiter_width,
                 const int number_line_width) {

    int index = 0;

    for (int i = 0; i < (2 * height + 1); ++i) {

        if (i % 2 == 0) {
            /// horizontal delimiter
            char left_delimiter[4];
            char right_delimiter[4];
            char horizontal_delimiter[4];

            if (i == 0) {
                strcpy(left_delimiter, left_top_corner);
                strcpy(right_delimiter, right_top_corner);
                strcpy(horizontal_delimiter, horizontal_down);
            } else if (i == 2 * height) {
                /// last line
                strcpy(left_delimiter, left_down_corner);
                strcpy(right_delimiter, right_down_corner);
                strcpy(horizontal_delimiter, horizontal_up);
            } else {
                /// other lines
                strcpy(left_delimiter, vertical_right);
                strcpy(right_delimiter, vertical_left);
                strcpy(horizontal_delimiter, vertical_horizontal);
            }

            int j = 0;
            int hor_line = 0;

            sprintf(file_contents + index, "%s", left_delimiter);

            j += 3;
            index += 3;

            while (j < horizontal_delimiter_width - 4) {
                if (hor_line < cell_width) {
                    /// add horizontal line
                    sprintf(file_contents + index, "%s", horizontal_line);
                    ++hor_line;
                } else {
                    /// add horizontal delimiter for the specific line
                    sprintf(file_contents + index, "%s", horizontal_delimiter);
                    hor_line = 0;
                }

                j += 3;
                index += 3;
            }

            sprintf(file_contents + index, "%s", right_delimiter);
            index += 3;
            sprintf(file_contents + index, "%c", '\n');
            index += 1;

        } else {
            /// number lines
            int j = 0;
            int blank_space = 0;
            sprintf(file_contents + index, "%s", vertical_line);
            j += 3;
            index += 3;

            while (j < number_line_width - 4) {
                if (blank_space < cell_width) {
                    sprintf(file_contents + index, "%c", ' ');
                    ++j;
                    ++index;
                    blank_space++;
                } else {
                    sprintf(file_contents + index, "%s", vertical_line);
                    j += 3;
                    index += 3;
                    blank_space = 0;
                }
            }

            sprintf(file_contents + index, "%s", vertical_line);
            index += 3;
            sprintf(file_contents + index, "%c", '\n');
            ++index;
        }
    }
}

void fill_matrix(char* file_contents, const int width, const int height, const int cell_width,
                 const int max_width, const int max_height, int* value, int index,
                 const int horizontal_delimiter_width, const int number_line_width, const int max_value) {

    /// horizontal top line
    bool is_vertical = true;
    for (int i = width; i < max_width;) {
        if (is_vertical) {
            index += 3;
            i += 3;
            is_vertical = false;
        } else {
            char content[cell_width + 1];
            memset(content, '\0', cell_width);
            sprintf(content, "%d", *value);
            for (int j = 0; j < strlen(content); ++j)
                file_contents[index + j] = content[j];
            index += cell_width;
            i += cell_width;
            is_vertical = true;
            *value += 1;
            if (*value > max_value)
                return;
        }
    }

    /// vertical right line
    index += number_line_width + horizontal_delimiter_width - 4 - cell_width + 1;
    for (int i = height; i < max_height; ++i) {
        char content[cell_width + 1];
        memset(content, '\0', cell_width);
        sprintf(content, "%d", *value);
        for (int j = 0; j < strlen(content); ++j)
            file_contents[index + j] = content[j];
        *value += 1;
        if (*value > max_value)
            return;
        index += number_line_width + horizontal_delimiter_width;
    }

    if (height + 1 > max_height || max_height - 1 < height)
        return;

    /// horizontal bottom line
    index = index - horizontal_delimiter_width - number_line_width;

    is_vertical = true;
    for (int i = max_width - 3; i > width + 3;) {
        if (is_vertical) {
            index -= 3;
            i -= 3;
            is_vertical = false;
        } else {
            index -= cell_width;
            i -= cell_width;
            char content[cell_width + 1];
            memset(content, '\0', cell_width);
            sprintf(content, "%d", *value);
            for (int j = 0; j < strlen(content); ++j)
                file_contents[index + j] = content[j];
            is_vertical = true;
            *value += 1;
            if (*value > max_value)
                return;
        }
    }

    if (width + cell_width + 3 >= max_width - 3 || max_width - 3 - cell_width <= width + 3)
        return;

    index = index - horizontal_delimiter_width - number_line_width + 3;

    /// vertical left line
    for (int i = max_height; i > height + 1; --i) {
        char content[cell_width + 1];
        memset(content, '\0', cell_width);
        sprintf(content, "%d", *value);
        for (int j = 0; j < strlen(content); ++j)
            file_contents[index + j] = content[j];
        *value += 1;
        if (*value > max_value)
            return;
        index = index - number_line_width - horizontal_delimiter_width;
    }

    index += number_line_width + horizontal_delimiter_width + cell_width;

    fill_matrix(file_contents, width + cell_width + 3, height + 1, cell_width,
                max_width - 3 - cell_width, max_height - 1, value, index, horizontal_delimiter_width, number_line_width, max_value);
}

int main(int argc, char* argv[]) {
    char* file_name = argv[1];
    const int width = atoi(argv[2]);
    const int height = atoi(argv[3]);

    char greatest_value[buffer_size];
    memset(greatest_value, '\0', buffer_size);
    sprintf(greatest_value, "%d", width * height);

    int cell_width = strlen(greatest_value);
    int horizontal_delimiter_width = 3 * (width + 1) + 3 * width * cell_width + 1;
    int number_line_width = 3 * (width + 1) + width * cell_width + 1;
    int file_size = (height + 1) * horizontal_delimiter_width + height * number_line_width;

    int file_descriptor = open(file_name, O_RDWR | O_CREAT, (mode_t)0600);
    ftruncate(file_descriptor, file_size);

    if (file_descriptor == -1) {
        printf("Couldn't open the file\n");
        return 1;
    }

    struct stat file_stat;
    fstat(file_descriptor, &file_stat);
    char* file_contents = mmap(NULL, file_stat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, file_descriptor, 0);

    if (file_contents == MAP_FAILED) {
        printf("Map failed\n");
        return 1;
    }
    /// draw matrix
    draw_matrix(file_contents, height, cell_width, horizontal_delimiter_width, number_line_width);
    int value = 1;
    fill_matrix(file_contents, 0, 1, cell_width, number_line_width - 1, height, &value,
                horizontal_delimiter_width, horizontal_delimiter_width, number_line_width, height * width);
    /// fill_matrix
    munmap(file_contents, file_stat.st_size);
    close(file_descriptor);
    return 0;
}
