

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int main( int argc, char * argv[] )
{
    FILE *input;
    char word[80];
    char ** array1 = (char**)calloc( 8, sizeof( char* ) );
    int * array2 = (int*)calloc( 8, sizeof(int));
    printf("Allocated initial parallel arrays of size 8.\n");
    int arrsize = 8;
    int unique_words = 0;
    int total_words = 0;
    int to_show = 0;
    if(argv[2])
    {
        to_show = atoi(argv[2]);
    }
    input = fopen(argv[1], "r");
    if(input == NULL)
    {
        fprintf(stderr, "ERROR: INVALID COMMAND LINE ARGUMENT");
        return 1;
    }
    while(1)
    {
        int unique = 1;
        int i;
        for(i = 0; i < 80; i++)
        {
            word[i] = 0;
        }
        for(i = 0; i < 80; i++)
        {
            char temp = fgetc(input);
            if(!isalnum(temp))
            {
                break;
            }
            word[i] = temp;
        }

        
        if( feof(input) )
        { 
            break;
        }
        total_words += 1;
        int j;
        for(j = 0; j < unique_words; j++)
        {
            
            if( strcmp(array1[j], word) == 0)
            {

                array2[j] += 1;
                unique = 0;
            }
        }
        
        
        if(unique){
            if(!strcmp(word, ""))
            {
                total_words--;
                continue;
            }
            array1[unique_words] = strdup(word);
            array2[unique_words] = 1;
            
            if(unique_words == arrsize - 1)
            {
                int newsize = arrsize * 2;
                array1 = (char**)realloc(array1, newsize * sizeof(char*));
                array2 = (int*)realloc(array2, newsize * sizeof(int));
                arrsize = arrsize * 2;
                for(j = unique_words+1; j < arrsize - 1; j++)
                {
                    array1[j + 1] = NULL;
                    array2[j + 1] = 0;
                }
                printf("Re-allocated parallel arrays to be size %d.\n", arrsize);
            }
            unique_words += 1;
        }
    }
    printf("All done (successfully read %d words; %d unique words).\n", total_words, unique_words);
    
    if(to_show == 0)
    {
        to_show = unique_words;
        printf("All words (and corresponding counts) are:\n");
    }
    else
    {
        printf("First %d words (and corresponding counts) are:\n", to_show);
    }
    int i;
    for(i = 0; i < to_show; i++)
    {
        printf("%s", array1[i]);
        printf(" -- %d\n", array2[i]);
    }
    for(i = 0; i < arrsize; i++)
    {
        free(array1[i]);
    }
    free(array1);
    free(array2);
    free(input);
    return 0;
}