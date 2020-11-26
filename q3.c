//Piano p Acoustic, Electric
//Guitar g Acoustic, Electric
//Violin v Acoustic
//Bass b Electric                                                        \
//Singer* (special case) s Acoustic, Electric
#include <stdio.h>                                
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#define ANSI_CYAN "\033[1;36m"
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_DEFAULT "\033[0m"
//stages and coordinators are resources , performer is threads

typedef struct performer{
    char name[10];
    char instru_char;//character of instrument
    int time;       //arrival time
    int ptime; // time he performed
    int status; //0 for NOT pERFORMED ,1 for PERFORMED (performing), 2 performance completed
    int sem_no;  //semaphore number of current musician (for singer to join) //not yet used
    int stage;    // 0 for acoustic , 1 for electrical
    int child;    //for musicians with two stages   ,childs will have parent id , parents will have childid, if there is no child it will have its own id
    int singer; // 0 no singer joined , 1 singer joined ,2 this itself is singer
    int singerno;
    int signal;
}performer;

performer *musician_thread[200]; //range 200 to 100 are childs , 0 to 100 are parents
pthread_t musicians[100];
pthread_mutex_t mutex; 
sem_t acc;
sem_t elec;
sem_t coordinator_sem;
int cursem; 
int cursem1;
int k,a,e,c,t1,t2,t;
sem_t sacc;
sem_t selec;

int current_accoustic_stages,current_electric_stages;
int current_asolo_performers=0,current_esolo_performers=0; //which are empty//


int  find(int stageno, int singernum) //for acoustic stages
{
    int i=0;
    while(i<=k)
    {
        if(musician_thread[i]->singer==0 && musician_thread[i]->status==1 && stageno==musician_thread[i]->sem_no && musician_thread[i]->stage==0)      //shouldnt be singer , a  musician should be performing at this stage //performs solo
            return i;
        else if(musician_thread[200-i]->singer==0 && musician_thread[200-i]->status==1 && stageno==musician_thread[i]->sem_no)
            return 200-i;    
        i++ ;   
    }
    return -1;
}
int  finde(int stageno, int singernum) // for electric stages
{
    int i=0;
    while(i<=k)
    {
        if(musician_thread[i]->singer==0 && musician_thread[i]->status==1 && stageno==musician_thread[i]->sem_no && musician_thread[i]->stage==1)      //shouldnt be singer , a  musician should be performing at this stage //performs solo
            return i;
        else if(musician_thread[200-i]->singer==0 && musician_thread[200-i]->status==1 && stageno==musician_thread[i]->sem_no)
            return 200-i;    
        i++ ;   
    }
    return -1;
}

void tshirts(int num){
    printf(ANSI_MAGENTA"%s waiting to collect tshirt\n",musician_thread[num]->name);
    sem_wait(&coordinator_sem);
    sleep(1);
    printf(ANSI_YELLOW"%s is collecting tshirt\n",musician_thread[num]->name);
    sem_post(&coordinator_sem);
    printf(ANSI_RED"%s exited\n",musician_thread[num]->name);
}

void *singer_execution(void * args){
    printf("entered\n");
    int num=*(int *)args;
    sleep(musician_thread[num]->time);
    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)       
        printf(ANSI_CYAN"%s who plays %c arrived at srujana , thread %d \n",musician_thread[num]->name,musician_thread[num]->instru_char,num);
    if(musician_thread[num]->stage==0 && musician_thread[num]->singer==2)     //accoustic stage and singer
    {
        //initialise semaphore value to solo musicians plus empty stages (only accoustic)
        sem_getvalue(&acc,&current_accoustic_stages);
        sem_init(&sacc,0,current_accoustic_stages+current_asolo_performers);
        struct timespec ts;
	    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	    {
    	    return NULL;
	    }
	        int s;
	        ts.tv_sec += t;
            while ((s = sem_timedwait(&sacc,&ts)) == -1 && errno == EINTR)
            continue;  
		    if (s == -1)
		    {
    	        if (errno == ETIMEDOUT)
        	    {
                    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                        printf(ANSI_RED"%s left without performing due to impatience\n",musician_thread[num]->name);
                    int num1;
                    num1=musician_thread[num]->child; 
                    pthread_cancel(musicians[num]);
                    pthread_cancel(musicians[num1]); 
                }
    	        else
        	        perror("sem_timedwait");
		    }
            else
            { 
                if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                {
                       
                    sem_getvalue(&sacc,&musician_thread[num]->sem_no);
                    int eg=find(musician_thread[num]->sem_no,num);
                    if(eg==-1) //empty stage
                    {
                        current_accoustic_stages--;
                        musician_thread[num]->status=1;
                        sem_wait(&acc);
                        int time1=rand()%(t2 -t1) +t1;
                         //performing here
                        printf(ANSI_YELLOW"%s is performing %c at acoustic stage no. %d for %d sec\n",musician_thread[num]->name,musician_thread[num]->instru_char,musician_thread[num]->sem_no,time1);
                        sleep(time1);
                        printf(ANSI_GREEN"%s performance at accoustic stage no.%d ended\n",musician_thread[num]->name,musician_thread[num]->sem_no);
                        musician_thread[num]->status=2;
                        sem_post(&acc);
                        sem_post(&sacc);
                        current_accoustic_stages++;
                    }
                    else
                    {   
                         musician_thread[num]->status=1;
                        if(musician_thread[eg]->status!=1)
                        {    printf(ANSI_CYAN"OOps error (due time gap)  :( \n");exit(0);}
                        current_asolo_performers--;
                        musician_thread[eg]->singerno=num;
                        musician_thread[eg]->signal=1;
                        printf(ANSI_YELLOW"%s joined %s performance on accoustic stage %d ,and time is increased by 2 seconds\n",musician_thread[num]->name,musician_thread[eg]->name,musician_thread[num]->sem_no);
                        //sem post will be called by musician
                   
                    }    
                    
                }    
                else{
                    //since its parent/child ran this destroy this
                    sem_post(&sacc);
                    pthread_cancel(musicians[num]);
                    }
            }  
          //  tshirts(num); 

    }
      //singers and electric stage part
     else if(musician_thread[num]->stage==1 && musician_thread[num]->singer==2)     //electric stage and singer
    {
        //initialise semaphore value to solo musicians plus empty stages (only accoustic)
        sem_getvalue(&elec,&current_electric_stages);
        sem_init(&selec,0,current_electric_stages+current_esolo_performers);
        struct timespec ts;
	    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	    {
    	    return NULL;
	    }
	        int s;
	        ts.tv_sec += t;
            while ((s = sem_timedwait(&sacc,&ts)) == -1 && errno == EINTR)
            continue;  
		    if (s == -1)
		    {
    	        if (errno == ETIMEDOUT)
        	    {
                    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                        printf(ANSI_RED"%s left without performing due to impatience\n",musician_thread[num]->name);
                    int num1;
                    num1=musician_thread[num]->child; 
                    pthread_cancel(musicians[num]);
                    pthread_cancel(musicians[num1]); 
                }
    	        else
        	        perror("sem_timedwait");
		    }
            else
            { 
                if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                {
                    sem_getvalue(&selec,&musician_thread[num]->sem_no);
                    int eg=finde(musician_thread[num]->sem_no,num);
                    if(eg==-1) //empty stage
                    {
                        
                        musician_thread[num]->status=1;
                        current_electric_stages--; 
                        sem_wait(&elec);
                        int time1=rand()%(t2 -t1) +t1;
                           //performing here
                        printf(ANSI_YELLOW"%s is performing %c at electric stage no. %d for %d sec\n",musician_thread[num]->name,musician_thread[num]->instru_char,musician_thread[num]->sem_no,time1);
                        sleep(time1);
                        printf(ANSI_GREEN"%s performance at electric stage no.%d ended\n",musician_thread[num]->name,musician_thread[num]->sem_no);
                        musician_thread[num]->status=2;
                        sem_post(&elec);
                        sem_post(&selec);
                        current_electric_stages++;
                    }
                    else
                    {   
                        musician_thread[num]->status=1;
                        if(musician_thread[eg]->status!=1)
                        {    printf(ANSI_CYAN"OOps error (due time gap)  :( \n");exit(0);}
                        current_esolo_performers--;
                        musician_thread[eg]->singerno=num;
                        musician_thread[eg]->signal=1;
                        printf(ANSI_YELLOW"%s joined %s performance on electric stage %d ,and time is increased by 2 seconds\n",musician_thread[num]->name,musician_thread[eg]->name,musician_thread[num]->sem_no);
                        //sem post will be called by musician
                   
                    }    
                   
                }    
                else{
                    //since its parent/child ran this destroy this
                    sem_post(&selec);
                    pthread_cancel(musicians[num]);
                    }
            }
            //tshirts(num);    

    }
}
    

void *stage_execution(void * args){
    int num=*(int *)args;
    sleep(musician_thread[num]->time);
    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)       
        printf(ANSI_CYAN"%s who plays %c arrived at srujana , thread %d \n",musician_thread[num]->name,musician_thread[num]->instru_char,num);
    
    if(musician_thread[num]->stage==0)       //violin acoustic
    {
        if(a==0)
        {
            printf(ANSI_GREEN"performance of %s is cancelled , due to stage unavailability\n",musician_thread[num]->name);
            pthread_cancel(musicians[num]);
        }
        else
        {   
            struct timespec ts;
	        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	        {
    	        return NULL;
	        }
	        int s;
	        ts.tv_sec += t;
            while ((s = sem_timedwait(&acc,&ts)) == -1 && errno == EINTR)
            continue;  
		    if (s == -1)
		    {
    	        if (errno == ETIMEDOUT)
        	    {
                    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                        printf(ANSI_RED"%s left without performing due to impatience\n",musician_thread[num]->name);
                    int num1;
                    num1=musician_thread[num]->child; 
                    pthread_cancel(musicians[num]);
                    pthread_cancel(musicians[num1]); 
                }
    	        else
        	        perror("sem_timedwait");
		    }
            else
            { 
                if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                {
                    current_asolo_performers++;
                    int time1=rand()%(t2 -t1) +t1;
                    musician_thread[num]->status=1;     //performing here
                    musician_thread[num]->singer=0;
                    musician_thread[num]->ptime=time1;
                    sem_getvalue(&acc,&musician_thread[num]->sem_no);
                    printf(ANSI_YELLOW"%s is performing %c at acoustic stage no. %d for %d sec\n",musician_thread[num]->name,musician_thread[num]->instru_char,musician_thread[num]->sem_no,time1);
                    sleep(time1);
                    if(musician_thread[num]->signal==1 && musician_thread[musician_thread[num]->singerno]->status==0)
                    {
                       musician_thread[num]->singer=1;
                       sleep(2);  
                    }
                    if( musician_thread[num]->singer=1)
                    {   
                        printf(ANSI_GREEN"%s performance along with singer %s at accoustic stage no %d ended\n",musician_thread[num]->name,musician_thread[musician_thread[num]->singerno]->name,musician_thread[num]->sem_no);
                        musician_thread[musician_thread[num]->singerno]->status=2; //make singer status performed
                        sem_post(&sacc); //release lock of singer
                    }
                    else    
                        printf(ANSI_GREEN"%s performance at accoustic stage no.%d ended\n",musician_thread[num]->name,musician_thread[num]->sem_no);
                    musician_thread[num]->status=2;
                    current_asolo_performers--;
                    sem_post(&acc);
                    tshirts(num);
                }    
                else{
                    //since its parent/child ran this destroy this
                    sem_post(&acc);
                    pthread_cancel(musicians[num]);
                    }
            }
        }

    }
    else if(musician_thread[num]->stage==1)          //guitar electric
    {
        if(e==0)
        {
            printf(ANSI_GREEN"performance of %s is cancelled , due to stage unavailability\n",musician_thread[num]->name);
            pthread_cancel(musicians[num]);
        }
        else{
            struct timespec ts;
	        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	        {
            	return NULL;
	        }
	        int s;
	        ts.tv_sec += t;
            while ((s = sem_timedwait(&elec,&ts)) == -1 && errno == EINTR)
            continue;  
		    if (s == -1)
		    {
    	        if (errno == ETIMEDOUT)  //if one is performing and other is timeout we shouldnt print this
                {
        		    if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                        printf(ANSI_GREEN" %s left without performing due to impatience\n",musician_thread[num]->name);
                    
                    int num1;
                    num1=musician_thread[num]->child; 
                    pthread_cancel(musicians[num]);
                    pthread_cancel(musicians[num1]);   
                }    
    	        else
        	        perror("sem_timedwait");
		    }
            else
            {
                printf(ANSI_DEFAULT"entered\n");
                //sem_wait(&elec);
                if(musician_thread[musician_thread[num]->child]->status==0 && musician_thread[num]->status==0)
                {
                    musician_thread[num]->status=1;
                    musician_thread[num]->singer=0;
                    int time1=rand()%(t2 -t1) +t1;
                    current_esolo_performers++;
                    musician_thread[num]->ptime=time1;
                    sem_getvalue(&elec,&musician_thread[num]->sem_no);
                    printf(ANSI_YELLOW"%s is performing %c at electric stage no.%d for %d sec\n",musician_thread[num]->name,musician_thread[num]->instru_char,musician_thread[num]->sem_no,time1);
                    sleep(time1);
                    if(musician_thread[num]->signal==1 && musician_thread[musician_thread[num]->singerno]->status==0 )
                    {
                       musician_thread[num]->singer=1;
                       sleep(2);  
                    }
                    if( musician_thread[num]->singer=1 && musician_thread[musician_thread[num]->singerno]->status==0)
                    {   
                        printf(ANSI_GREEN"%s performance along with singer %s at accoustic stage no %d ended\n",musician_thread[num]->name,musician_thread[musician_thread[num]->singerno]->name,musician_thread[num]->sem_no);
                        musician_thread[musician_thread[num]->singerno]->status=2; //make singer status performed
                        sem_post(&selec); //release lock of singer
                    }
                    else 
                        printf(ANSI_GREEN"%s performance at electric stage no.%d ended\n",musician_thread[num]->name,musician_thread[num]->sem_no);
                    musician_thread[num]->status=2;
                    sem_post(&elec);
                    tshirts(num);
                }
                else{
                    //since its parent/child ran this destroy this
                    sem_post(&elec);
                    pthread_cancel(musicians[num]);

                }
            }    
        }
    }
}




int main(){
    printf("-----------------------------------------------WELCOME TO Musical Mayhem-----------------------------------------------------------------\n");
    printf("enter no.of musicians : ");
    scanf("%d",&k);
    printf("\nenter no.of accoustic stages : ");
    scanf("%d",&a);
    printf("\nenter no.of electric stages : ");
    scanf("%d",&e);
    printf("\n enter no.of coordinators : ");
    scanf("%d",&c);
    printf("\nenter t1 and t2: ");
    scanf("%d %d",&t1,&t2);
    printf("\nenter t (max_waiting_time): ");
    scanf("%d",&t);
    printf("\nenter details of musicians : \n");
    for(int i=0;i<=200;i++)
        musician_thread[i]=(struct performer*)malloc(sizeof(struct performer));
    for(int i=0 ;i<k;i++)
    {
        
        scanf("%s",musician_thread[i]->name);
        scanf(" %c",&musician_thread[i]->instru_char);
        scanf("%d",&musician_thread[i]->time);
        musician_thread[i]->status=0;
        musician_thread[i]->child=i;
        musician_thread[i]->singer=0;
        if(musician_thread[i]->instru_char=='s')
            musician_thread[i]->singer=2;
    }
    current_accoustic_stages=a;
    current_electric_stages=e;
    for(int i=0 ;i<k;i++)
    printf("%s %c %d\n",musician_thread[i]->name,musician_thread[i]->instru_char,musician_thread[i]->time);
    sem_init(&acc,0,a); //initially all stages are empty
    sem_init(&elec,0,e);
    sem_init(&coordinator_sem,0,c);

    int j=200;

    for(int i=0;i<k;i++)
    { 
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        int e1;
        if(musician_thread[i]->instru_char=='v')
        {
            musician_thread[i]->stage=0;
            e1=pthread_create(&musicians[i],NULL,stage_execution,arg);
        }
        else if(musician_thread[i]->instru_char=='b')
        {
            musician_thread[i]->stage=1;
            e1=pthread_create(&musicians[i],NULL,stage_execution,arg);    
        }
       else if(musician_thread[i]->instru_char=='p' || musician_thread[i]->instru_char=='g' || musician_thread[i]->instru_char=='s')
        {  //copy details of musician of i to j
            musician_thread[i]->stage=1;
            musician_thread[i]->child=j;
            strcpy(musician_thread[j]->name,musician_thread[i]->name);
            musician_thread[j]->instru_char=musician_thread[i]->instru_char;
            musician_thread[j]->time=musician_thread[i]->time;
            musician_thread[j]->status=0;
            musician_thread[j]->child=i;
            musician_thread[j]->stage=0;
            musician_thread[j]->singer=0;
            if(musician_thread[i]->instru_char=='s')
            { printf("i am singer\n");    musician_thread[j]->singer=2;}
            
            int *arg1 = malloc(sizeof(*arg));
            *arg1 = j;
          //  printf(ANSI_DEFAULT"creating child thread %d for parent %d\n", musician_thread[i]->child,i);
            if(musician_thread[i]->instru_char=='s')
            {
                pthread_create(&musicians[j],NULL,singer_execution,arg1);
                e1=pthread_create(&musicians[i],NULL,singer_execution,arg);
            }
           
            else
            {
                pthread_create(&musicians[j],NULL,stage_execution,arg1);
                e1=pthread_create(&musicians[i],NULL,stage_execution,arg);
            }
            j--;
        }
    
       // printf("thread done\n");
        sleep(1);
        if(e1!=0)
        {
            perror("pthread_create");
        }
    }


    for(int i=0;i<k;i++)
    {   
        pthread_join(musicians[i],NULL);                //wait for original
        pthread_join(musicians[musician_thread[i]->child],NULL); //wait for duplicate
    }    
    printf(ANSI_MAGENTA"FINISHED :)\n");
    sleep(5);
 return 0;
}