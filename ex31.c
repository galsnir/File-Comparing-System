// Gal Snir 313588279

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

int main(int argc, char* argv[] ) {

    int file1, file2;
    char buffer1[1];
    char buffer2[1];
    file1 = open(argv[1],O_RDONLY);
    file2 = open(argv[2],O_RDONLY);
    int res = 1;

    // First we check if there are any charachter left in the first file
    while (read(file1,buffer1,1) > 0) {
        if (read(file2, buffer2, 1) > 0) {
            // if they both have charachters left then we check if they are equal,
            // if not we say they can be still similar
            if ((*buffer1 != *buffer2) && (res == 1)) {
                res = 3;
            }

            // if we saw that there where different charechter in the file we check if
            // they fall into the similar files execption cases if not we will mark them as different
            if (res == 3) {
                while (*buffer1 == ' ' || *buffer1 == '\n') {
                    if (read(file1, buffer1, 1) == 0)
                        break;
                }

                while (*buffer2 == ' ' || *buffer2 == '\n') {
                    if (read(file2, buffer2, 1) == 0)
                        break;
                }

                if (*buffer1 != *buffer2) {
                    if (tolower(*buffer1) == tolower(*buffer2))
                        continue;
                    else {
                        res = 2;
                    }
                }
            }
        }
        else
        {
            res = 2;
        }
    }

    // if the first and the second file have ended we will return the result
    if (read(file2, buffer2, 1) == 0)
    {
        return res;
    }

    // if the first file ended but the second file didn't we will check if the rest of the charachters fall
    // into the exception of the similar files or not and return the result accordingly
    else
    {
        res = 3;
            while (*buffer2 == ' ' || *buffer2 == '\n')
            {
                if (read(file2, buffer2, 1) == 0)
                    return res;
            }

            res = 2;
            return res;
    }

}
