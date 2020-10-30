
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>



#define MAX_FILENAME 150
#define ENDOFDATA  -1
#define MAX_FILES 5
#define MAX_MSG_SIZE 4096



struct Node* listheads[MAX_FILES];
char filenames[MAX_FILES][MAX_FILENAME];
pthread_t lists[MAX_FILES];
char outfilename[MAX_FILENAME];


struct Node
{
        char data[1024];
        int count;
        struct Node *next;
};

void handleParent(int fnumbers);
void *handleChild(void *arg);
void addlist (struct Node** head, char word[]);
void printList(struct Node *head);
void addParentlist (struct Node** head, char word[], int count);
int getSize(struct Node *head);
void removeDuplicateElements(struct Node *temp);


int getSize(struct Node *head)
{
    int size = 0;
    struct Node *temp = head;
    while(temp)
    {
        size++;
    }
    return size;
}
void *handleChild(void *arg)
{
    FILE   *fp;
    int tNum;
    char word[1024];
    
    tNum = (intptr_t) arg;
    
    
    struct Node *head = NULL;
    
    fp = fopen (filenames[tNum], "r");
    if(fp == NULL)
    {
        printf("Cannot open the file\n");
        exit(1);
    }
    
    
    while (fscanf (fp, "%s",  word) == 1)
    {
        addlist(&head, word);
    }
    
    
    listheads[tNum] = head;
    


    fclose (fp);
    pthread_exit (0);

}

void handleParent(int fnumbers)
{
    FILE *fp;
    struct Node head[MAX_FILES];
    int endedQueueCount = 0;
    struct Node *temp;
    struct Node *tmp;
    int file_count;
    


    file_count = fnumbers;
    

    fp = fopen (outfilename, "w");

    if (fp == NULL) 
    {
        perror ("fopen failed\n");
        exit (1);
    }
    endedQueueCount = 0;
    

    for (int i = 0; i < file_count; ++i)
    {
        if (listheads[i] == NULL) 
        {
            head[i].count = ENDOFDATA;
            endedQueueCount++;
        }
        else 
        {
            strcpy(head[i].data,listheads[i]->data);
            head[i].count = listheads[i]->count;
            tmp = listheads[i];
            listheads[i] = listheads[i]->next;
            free (tmp);
            
        }


    }

    
   while(endedQueueCount<file_count)
    {

        int q = -1;
        char min[1024] = "zzz";
        int currCount = 0;
        for(int i = 0; i < file_count; i++)
        {
            
            if(head[i].count != ENDOFDATA)
            {
                
                if( strcmp(head[i].data, min) < 0)
                {
                    strcpy(min,head[i].data);
                    currCount = head[i].count;
                    q = i;
                }
            }
        }
        addParentlist(&temp, min, currCount);
        
        
        if (listheads[q] == NULL) 
        {
            head[q].count = ENDOFDATA;
            endedQueueCount++;
        }
        else
        {
            strcpy(head[q].data,listheads[q]->data);
            head[q].count = listheads[q]->count;
            tmp = listheads[q];
            listheads[q] = listheads[q]->next;
            free (tmp);
        }
        
    
    
    }
    removeDuplicateElements(temp);

    

    
    while(temp)
    {
        fprintf(fp, "%s  %d\n", temp->data, temp->count);
        temp = temp->next;
    }

    
    fclose (fp);
    
    
    

}

void removeDuplicateElements(struct Node *temp)
{
    struct Node *ptr, *ptr2, *duplicate;
    ptr = temp;

    while (ptr != NULL && ptr->next != NULL)
    {
        ptr2 = ptr;

        
        while (ptr2->next != NULL)
        {
            if (ptr->data == ptr2->next->data)
            {
                duplicate = ptr2->next;
                ptr2->next = ptr2->next->next;
                free(duplicate);
            }
            else
                ptr2 = ptr2->next;
        }
        ptr = ptr->next;
    }
}

void printList(struct Node *head)
{
    struct Node *temp = head;
    while(temp)
    {
        printf("%s  freq: %d\n", temp->data, temp->count);
        temp = temp->next;
    }
}

struct Node* contains(struct Node *head, char word[])
{
    struct Node *temp = head;
    while(temp)
    {
        if(strcmp(temp->data, word) == 0)
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}


void addParentlist (struct Node** head, char word[], int count)
{
    if(contains(*head,word) != NULL)
    {
        struct Node *index = contains(*head, word);
        index->count = index->count + count;
    }
    else
    {
        struct Node *newNode;
        struct Node *temp;
        

        newNode = (struct Node *) malloc(sizeof(struct Node));

        if (newNode == NULL) 
        {
            perror ("malloc failed\n");
            exit (1);
        }
        strcpy(newNode->data, word);
        newNode->count = count;
        newNode->next = NULL;
        if ( (*head) == NULL)
        {
            *head = newNode;
            return;
        } 
        temp = *head;

        while(temp != NULL && temp->next != NULL)
            temp = temp->next;

        temp->next = newNode;

        
    }
}

void addlist (struct Node** head, char word[])
{
    if(contains(*head,word) != NULL)
    {
        struct Node *index = contains(*head, word);
        index->count = index->count + 1;
    }
    else
    {
        struct Node *newNode;
        struct Node *curr;
        

        newNode = (struct Node *) malloc(sizeof(struct Node));

        if (newNode == NULL) 
        {
            perror ("malloc failed\n");
            exit (1);
        }
        strcpy(newNode->data, word);
        newNode->count = 1;
        newNode->next = NULL;

        if ( (*head) == NULL)
        {
            *head = newNode;
            return;
        } 
        else if(strcmp(word,(*head)->data)<=0)
        {
            newNode->next = (*head);
            *head = newNode;
            return;
        }
        else
        {
            curr = *head;
            while(curr->next)
            {
                if(strcmp(word,((curr->next)->data))<0)
                {
                    break;
                }
                else
                    curr = curr->next;
            }
            if(curr->next == NULL)
            {
                curr->next = newNode;
                return;
            }
            else
            {
                newNode->next = curr->next;
                curr->next = newNode;
                return;
            }
        }
    }
}

int main(int argc, char const **argv)
{
    
    int i;
    int num_files;
    

    
    
    
    
    

    num_files = atoi(argv[1]);

    for (i = 0; i < num_files; ++i)
    {
        strcpy(filenames[i], argv[2+i]);
    }

    strcpy (outfilename, argv[2+num_files]);

    for (i = 0; i < num_files; ++i)
        listheads[i] = NULL;
    
    
    struct timeval start_time, end_time;
    float elapsed_time;

    gettimeofday(&start_time, NULL);

    for (i = 0; i < num_files; ++i) 
    {
        int ret = pthread_create (&(lists[i]), NULL, handleChild, (void *)(intptr_t) i);
        if (ret != 0) 
        {
            perror ("thread create failed\n");
            exit (1);
        }
    }
    for (int i = 0; i < num_files; ++i)
        pthread_join (lists[i], NULL);


    handleParent(num_files);
    
    
    gettimeofday(&end_time, NULL);
    
    // Calculating elapsed time in microseconds
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000;
    elapsed_time += (end_time.tv_usec - start_time.tv_usec);

    printf("\n\nElapsed time for multi-thread application: %f ms\n", elapsed_time);
                
    pthread_exit(0);
    
    return 0;
}
