
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_FILENAME 100
#define ENDOFDATA  -1
#define MAX_FILES 5
#define MQNAME "/nameis"
#define MAX_MSG_SIZE 4096


struct Node
{
        char data[1024];
        int count;
        struct Node *next;
};
struct message 
{
    char data[1024];
    int count;
};
void handleParent(char *outfile, int file_count, mqd_t mqarray[]);
void handleChild(char infile[], mqd_t mq);
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
void handleChild(char infile[], mqd_t mq)
{
    FILE   *fp;
    struct message msg;
    char word[1024];
    int n;
    
	
    struct Node *head = NULL;
    fp = fopen (infile, "r");
    if(fp == NULL)
    {
        printf("Cannot open the file\n");
        exit(1);
    }
	
    
    while (fscanf (fp, "%s",  word) == 1)
    {
        addlist(&head, word);
    }
    
    
    while(head)
    {
        strcpy(msg.data, head->data);
        msg.count = head->count;
        n = mq_send(mq, (const char *) &msg, sizeof(struct message), 0);
        
        if(n == -1)
        {
            perror("send failed\n");
            exit(1);
        }
        
        head = head->next;
    }
    msg.count = ENDOFDATA;
    
    n = mq_send(mq, (char *) &msg, sizeof(struct message), 0);
    if(n == -1)
    {
        perror("send failed\n");
        exit(1);
    }


    fclose (fp);
    

}

void handleParent(char *outfile, int file_count, mqd_t mqarray[])
{
    FILE *fp;
    struct mq_attr mq_attr;
    struct message *msgptr;
    char *bufptr;
    int buflen;
    struct message head[MAX_FILES];
    int n;
    int endedQueueCount = 0;
    struct Node *tmp = NULL;
    

    fp = fopen (outfile, "w");

    mq_getattr(mqarray[0], &mq_attr);

    buflen = mq_attr.mq_msgsize;
    bufptr = (char *) malloc(buflen);

    for (int i = 0; i < file_count; ++i)
    {
        n = mq_receive(mqarray[i], (char *) bufptr, buflen, NULL);
	    sleep(1);
        msgptr = (struct message *) bufptr;
        if (n == -1)
        {
            perror("receive failed\n");
            exit(1);
        }
        if(msgptr->count == ENDOFDATA)
        {
            endedQueueCount++;
            head[i].count = ENDOFDATA; 
        }else
        {
            strcpy(head[i].data, msgptr->data);
            head[i].count = msgptr->count;
        }

	
       
       
        
      /* while(msgptr->count != ENDOFDATA)
       {
            addParentlist (&tmp, msgptr->data, msgptr->count);
            n = mq_receive(mqarray[i], (char *) bufptr, buflen, NULL);
            msgptr = (struct message *) bufptr;
       }*/


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
        addParentlist(&tmp, min, currCount);
        
        //fprintf(fp, "%s\n", min);
        //fprintf(fp, "%s  %d\n", tmp->data, tmp->count);

        n = mq_receive(mqarray[q], (char *) bufptr, buflen, NULL);
        sleep(1);
        msgptr = (struct message *) bufptr;
        if (n == -1)
        {
            perror("receive failed\n");
            exit(1);
        }
        if(msgptr->count == ENDOFDATA)
        {
            endedQueueCount++;
            head[q].count = ENDOFDATA; 
        }else
        {
            strcpy(head[q].data, msgptr->data);
            head[q].count = msgptr->count;
        }
        
    
    
    }
    removeDuplicateElements(tmp);

    printList(tmp);

    
    while(tmp)
    {
        fprintf(fp, "%s  %d\n", tmp->data, tmp->count);
        tmp = tmp->next;
    }

    free (bufptr);
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
	mqd_t mq;
    pid_t pid;
    int i;
    int num_files;
    

    char  filenames[MAX_FILES][MAX_FILENAME];
    char  outfilename[MAX_FILENAME];
    char  mq_names[MAX_FILES][MAX_FILENAME];
    mqd_t mq_ids[MAX_FILES];
    
    
    
    
    

    pid_t childPid[MAX_FILES];

    num_files = atoi(argv[1]);

    for (i = 0; i < num_files; ++i)
    {
        strcpy(filenames[i], argv[2+i]);
    }

    strcpy (outfilename, argv[2+num_files]);
    struct timeval start_time, end_time;
    float elapsed_time;

    gettimeofday(&start_time, NULL);

   for (i = 0; i < num_files; ++i) 
    {
        
        sprintf (mq_names[i], "%s%d", MQNAME, i);
	
        mq = mq_open(mq_names[i], O_RDWR | O_CREAT, 0666, NULL);
	   
        if (mq == -1) 
        {
            perror("can not create message queue\n");
            exit(1);
        }
        mq_ids[i] = mq;

    }



    for (i = 0; i < num_files; ++i) 
    {
        pid = fork ();
        if (pid == 0) {
                handleChild(filenames[i], mq_ids[i]);
                exit (0);
        }
        childPid[i] = pid;
    }

    for (i = 0; i < num_files; ++i)
    {
        waitpid (childPid[i], NULL, 0);
    }

    handleParent(outfilename, num_files, mq_ids);
    for (i = 0; i < num_files; ++i)
    {
        mq_close (mq_ids[i]);
        mq_unlink (mq_names[i]);
    }
    gettimeofday(&end_time, NULL);
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000;
    elapsed_time += (end_time.tv_usec - start_time.tv_usec);

    printf("\n\nElapsed time for multi-process application: %f ms\n", elapsed_time);
                

    
	return 0;
}
