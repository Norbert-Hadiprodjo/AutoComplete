#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "autocomplete.h"

int len_substring(char* str)
{
    int len = 0;
    while (isalpha(str[len]) != 0)
    {
        len++;
    }
    return len;
}

int sort_lexico(const void* a, const void* b)
{
    return(strcmp(((struct term*)a)->term, ((struct term*)b)->term));
}

int sort_weight(const void* a, const void* b)
{
    double res = (-((struct term*)a)->weight + ((struct term*)b)->weight);
    if (res > 0)
    {
        return 1;
    }
    else if (res < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void read_in_terms(struct term** terms, int* pnterms, char* filename)
{
    char line[200];
    FILE* fp = fopen(filename, "r");
    fgets(line, sizeof(line), fp);
    *pnterms = atoi(line);
    *terms = (struct term*)malloc(sizeof(struct term) * (*pnterms));

    for (int i = 0; i < *pnterms; i++)
    {
        fgets(line, sizeof(line), fp);
        int index = 0;
        strcpy((*terms + i)->term, line);
        while ((isspace((*terms + i)->term[index])) != 0)
        {
            index++;
        }
        int weight_start = index;

        while ((isspace((*terms + i)->term[index])) == 0)
        {
            index++;
        }
        int weight_end = index - 1;
        int term_start = index + 1;
        int term_end = strlen((*terms + i)->term) - 2;

        char weight[200];
        strncpy(weight, (*terms + i)->term + weight_start, weight_end);
        (*terms + i)->weight = (double)atol(weight);

        char term[200];
        strncpy(term, (*terms + i)->term + term_start, term_end);
        strcpy((*terms + i)->term, term);
        int len = strlen((*terms + i)->term);
        if ((*terms + i)->term[len - 1] == '\n')
        {
            (*terms + i)->term[len - 1] = '\0';
        }
    }

    qsort(*terms, *pnterms, sizeof(struct term), sort_lexico);
    fclose(fp);
}

int lowest_match(struct term* terms, int nterms, char* substr)
{
    int lower = 0;
    int upper = nterms;
    int len = len_substring(substr);

    while(lower < upper) 
    {
        int mid = (lower + upper) / 2;
        if (strncmp(substr, (terms + mid)->term, len) != 0 && strcmp((terms + mid)->term, substr) < 0)
        {
            lower = mid + 1;
        } 
        else if (strncmp(substr, (terms + mid)->term, len) != 0 && strcmp((terms + mid)->term, substr) > 0) 
        {
            upper = mid;
        }
        else if(mid > 0 && strncmp(substr, (terms + mid)->term, len) == 0) 
        {
            upper = mid;
        }
        else 
        {
            return mid;
        }
    }

    if (lower == upper)
    {
        return lower;
    }
}

int highest_match(struct term* terms, int nterms, char* substr)
{
    int mid = nterms/2;
    int len = len_substring(substr);

    if (nterms <= 2)
    {   
        if (strncmp((terms + mid)->term, substr, len) == 0)
        {
            return mid;
        }
        else if (strncmp((terms + mid - 1)->term, substr, len) == 0)
        {
            return mid - 1;
        }
    }

    else if (strncmp((terms + mid)->term, substr, len) == 0)
    {
        if (strncmp((terms + mid)->term, substr, len) > 0)
        {
            return (highest_match(terms, mid, substr));
        }
        else
        {
            struct term* newterm = (terms + mid + 1);
            return (mid + 1 + highest_match(newterm, mid, substr));
        }

    }

    else if (strcmp((terms + mid)->term, substr) < 0)
    {   
        struct term* newterm = (terms + mid + 1);
        return (mid + 1 + highest_match(newterm, mid, substr));
    }

    else if (strcmp((terms + mid)->term, substr) > 0)
    {
        return (highest_match(terms, mid, substr));
    }

    return mid;
}

void autocomplete(struct term** answer, int* n_answer, struct term* terms, int nterms, char* substr)
{
    int higher_match = highest_match(terms, nterms, substr);
    int lower_match = lowest_match(terms, nterms, substr);

    *n_answer = (higher_match - lower_match) + 1;
    *answer = (struct term*)malloc(sizeof(struct term) * (*n_answer));

    int counter = 0;

    for (int i = lower_match; i < lower_match + *n_answer; i++)
    {
        strcpy((*answer + counter)->term, (terms + i)->term);
        (*answer + counter)->weight = (terms + i)->weight;
        counter++;
    }

    qsort(*answer, *n_answer, sizeof(struct term), sort_weight);
}