//to compile: gcc <filename>.c -lpthread
#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<time.h>          
#include <unistd.h>         
//#include <wait.h>
//#include <sys/shm.h>
#include <errno.h>
#define ANSI_CYAN "\033[1;36m"
#define ANSI_RED "\033[1;31m"
#define ANSI_GREEN "\033[1;32m"
#define ANSI_YELLOW "\033[1;33m"
#define ANSI_MAGENTA "\033[1;35m"
#define ANSI_DEFAULT "\033[0m"

typedef struct company{
    float x ; //probability//
    int r; //no of batches//
    int w; //time taken//
    int p; //no.of vaccines in a batch//
}company;

typedef struct vacc_zones{
    int k ; //no .of vaccines given//
    int curr_slots;//current slots
    float probab;//probability of sucess of vaccine
    int vaccneed; //0 if not needed ,1 if needed
    int availslots[8]; //every zone can have atmax 8 slots at once // -2 are slots which arent meant to be distributes , -1 are slots which are meant to be availed by students//
}vacc_zones;

typedef struct student{
    int turn; //how much time did he/she get vaccinated
    int status; //0 for free ,1 in vaccination
    int time;//arrival time
    int azone; //alotted zone -1 if not alloted
}student;

int n_companies,m_zones,o_students;
pthread_t companies[200];
pthread_t vaccinezones[200];
pthread_t students[200];
company *comp_thread[200];
vacc_zones *zone_thread[200];
student *student_thread[200];
pthread_mutex_t mutex,mutex1,mutex2,mutex3;
int students_waiting=0;
int students_vaccinated=0;
int negstudents=0;
int no_of_students_arrived=0;

int min(int a ,int b ,int c)
{
    for(int i=0;i<=8;i++)
    {
        if(i==a) return a;
        else if(i==b) return b;
        else if(i==c)return c;
    }
}


void *student_coming(void *args){
    int num1=*(int *)args;
    sleep(student_thread[num1]->time);
    students_waiting++;
    no_of_students_arrived++;
    while(student_thread[num1]->turn<3)
    {
        printf(ANSI_MAGENTA"Student %d has arrived for his %d round of Vaccination\n",num1,student_thread[num1]->turn+1);
        while(1)      //coz it should be in busy waiting
        {   
            
            int i;
            for(i=0;i<m_zones;i++)
            {
                for(int j=1;j<=8;j++)
                {   
                    pthread_mutex_lock(&mutex1);
                    if(zone_thread[i]->availslots[j]==-1)
                    {
                        zone_thread[i]->availslots[j]=num1;
                        printf(ANSI_GREEN"Student %d assigned a slot on the Vaccination Zone %d and waiting to be vaccinated\n",num1,i);
                        student_thread[num1]->azone=i;
                        student_thread[num1]->status=1;
                        student_thread[num1]->turn++;
                        zone_thread[i]->curr_slots--;
                        students_waiting--;
                    }
                    pthread_mutex_unlock(&mutex1);
                    if(student_thread[num1]->azone !=-1)
                        break;
                }
                if(student_thread[num1]->azone !=-1)
                    break;
            }
            if(student_thread[num1]->azone !=-1)
                break;           
        }
        //wait till vaccinated
        while(student_thread[num1]->status==1)
        {
            ;
        }
        printf(ANSI_YELLOW"Student %d on Vaccination Zone %d has been vaccinated which has success probability %f\n",num1,student_thread[num1]->azone,zone_thread[student_thread[num1]->azone]->probab);
        //check antibody result
        int a=rand()%1000;
        float b= a/1000.0;
        if(zone_thread[student_thread[num1]->azone]->probab < b)
        {
            pthread_mutex_lock(&mutex3);
            if(student_thread[num1]->turn!=3)
            {
                students_waiting++;
                negstudents++;
            }    
            student_thread[num1]->azone=-1; //looking for zone
            printf(ANSI_RED"Student %d tested negative for antibodies\n",num1);
            pthread_mutex_unlock(&mutex3);   

        }
        else
        {
            printf(ANSI_RED"Student %d tested positive for antibodies\n",num1);
            break;
        }

    }
    if(student_thread[num1]->turn==3 && student_thread[num1]->azone==-1)
    {    
        printf(ANSI_CYAN"Student %d is sent home\n",num1); 
        
    }
    if(students_waiting==0)
    {   
        sleep(15);
    }
    while(students_waiting==0)
    {    
        if(no_of_students_arrived==o_students)
        {    
            pthread_mutex_lock(&mutex);
            printf(ANSI_MAGENTA"no.of students waiting are %d and no.of vaccines used in total are %d\n",students_waiting,students_vaccinated);
            printf(ANSI_GREEN "Simulation over :) \n");
            pthread_mutex_unlock(&mutex);
            exit(0);
        }
    }     
    pthread_cancel(students[num1]); 
    return 0;
}




void *zonal_distribution(void *num){   //slots that can be taken by students are -1 ,slots that are not allocated are -2
    int num1=*(int *)num;
    zone_thread[num1]->vaccneed=1; 
    while(1)
    {
        sleep(5);
        zone_thread[num1]->availslots[8]=-2;
        while(zone_thread[num1]->k==0)
        {
            ; //zone needs vaccines,wait till a company gives u vaccine
        }
        
        zone_thread[num1]->curr_slots=min(8,students_waiting,zone_thread[num1]->k);
        for(int i=1;i<=zone_thread[num1]->curr_slots;i++)
            zone_thread[num1]->availslots[i]=-1;
        if(zone_thread[num1]->curr_slots>0)
            printf(ANSI_CYAN"Vaccination Zone %d is ready to vaccinate with %d slots \n",num1,zone_thread[num1]->curr_slots);    
        while(zone_thread[num1]->curr_slots!=0 && students_waiting!=0)
        {
            ;
        }
        //vaccinate the students
        if(zone_thread[num1]->curr_slots==0){
            if(students_waiting!=0)
             printf(ANSI_GREEN"Vaccination Zone %d entering Vaccination Phase\n",num1);
        for(int i=1;i<=8;i++)
        {
            if(zone_thread[num1]->availslots[i]==-2)
                break;                      //these are slots which arent provided
            else
            {
                int studentindex=zone_thread[num1]->availslots[i];
                zone_thread[num1]->k--;
                student_thread[studentindex]->status=0; //vaccination completed
                pthread_mutex_lock(&mutex2);
                students_vaccinated++;
                pthread_mutex_unlock(&mutex2);
            }   
            
        }}
        if(zone_thread[num1]->k==0)
        {   
            printf(ANSI_CYAN"Vaccination Zone %d has run out of vaccines\n",num1); 
            zone_thread[num1]->vaccneed=1;   
        } 
        for(int i=1;i<=8;i++)
        {
            zone_thread[num1]->availslots[i]=-2;   //again block all the slots and mext iteration it releases only designated slots
        }  
    }
}    



void *comp_production(void *args){
    int inp=*((int *)args);
    comp_thread[inp]->w=rand()%4 + 2;
    comp_thread[inp]->r=rand()%5 +1;
    comp_thread[inp]->p=rand()%10+10;
    printf(ANSI_DEFAULT"pharma company %d is preparing %d no.of batches of vaccines which have success probability %f\n",inp,comp_thread[inp]->r,comp_thread[inp]->x); 
    sleep(comp_thread[inp]->w);  
    printf(ANSI_CYAN"pharma company %d has prepared %d no.of batches of vaccines which have success probability %f ,Waiting for all the vaccines to be used to resume production\n",inp,comp_thread[inp]->r,comp_thread[inp]->x);
    while(1)
    {
        while(comp_thread[inp]->r>0){
            for(int i=0;i<m_zones;i++)
            {
               
                if(zone_thread[i]->vaccneed==1){ 
                    pthread_mutex_lock(&mutex);
                    if(zone_thread[i]->k==0){
                    zone_thread[i]->k=comp_thread[inp]->p;
                    zone_thread[i]->vaccneed=0;
                    printf(ANSI_RED"pharma company %d is delivering vaccine batch to vaccination zone %d which has success probability %f\n",inp,i,comp_thread[inp]->x );
                    zone_thread[i]->probab=comp_thread[inp]->x;
                    comp_thread[inp]->r--;}
                    pthread_mutex_unlock(&mutex);
                    break;
                }    
            }
        }  
        printf(ANSI_GREEN"All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n",inp);
        comp_production(args);
    }   
    return NULL;
}




int main(){
    printf("-------------------------------Welcome to vaccination---------------------------------\n");
    srand(time(0));
    printf("\n enter no of companies : ");
    scanf("%d",&n_companies);
    if(n_companies==0)
    {  printf("Sorry! No companies to produce vaccine :'( \n"); return 0;}

    printf("\n enter no. of vaccination zones : ");
    scanf("%d",&m_zones);
    if(m_zones==0)
    {    printf("Ask IIIT-H to make zones :'( \n"); return 0;}

    printf("\n enter no. of students : ");
    scanf("%d",&o_students);
    if(o_students==0)
    { printf("NO STUDENTS :) \n");return 0;}


    for(int i=0; i<n_companies;i++){
        comp_thread[i]=(struct company*)malloc(sizeof(struct company));
        printf("\n enter probability of success of company %d: ",i+1);
        scanf("%f",&comp_thread[i]->x);
    }

    for(int i=0; i<m_zones;i++){
        zone_thread[i]=(struct vacc_zones*)malloc(sizeof(struct vacc_zones));
        zone_thread[i]->k=0;        //initially
        for(int j=1;j<=8;j++)
            zone_thread[i]->availslots[j]=-2;
    }

    
    for(int i = 0 ; i < o_students;i++){
        student_thread[i]=(struct student*)malloc(sizeof(struct student));
        student_thread[i]->turn=0;
        student_thread[i]->status=0;
        student_thread[i]->azone=-1;

    }
    printf("hai ,creating threads\n");
    for(int i=0; i<n_companies;i++)
    {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        int e=pthread_create(&companies[i],NULL,comp_production,arg);
        sleep(1);
        if(e!=0)
        {
            perror("pthread_create");
        }
    }

    for(int i=0; i<m_zones;i++)
    {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        int e=pthread_create(&vaccinezones[i],NULL,zonal_distribution,arg);
       // printf("thread done\n");
        sleep(1);
        if(e!=0)
        {
            perror("pthread_create");
        }
    }
   for(int i=0; i<o_students;i++){
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        int e=pthread_create(&students[i],NULL,student_coming,arg);
        //printf("thread done\n");
        sleep(1);
        if(e!=0)
        {
            perror("pthread_create");
        }

    }
   
    for(int i=0; i<o_students;i++)
    {
        pthread_join(students[i],NULL);
    }
    
    for(int i=0; i<m_zones;i++)
    {
        pthread_join(vaccinezones[i],NULL);
    } 
   
    sleep(m_zones);
    return 0;
    
}
