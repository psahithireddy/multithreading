INTIALISATION:
Took stages and coordinators as resources .
musicians as threads.
for musicians who perform on either electric or acoustic , i created a one more thread of same type
if 0 to 100 are original threads 200 to 101 range is duplicates.
copy all data of original to duplicate , and linked these both threads using a variable called 'child' inside struct.
The value of child in original thread is id of duplicate thread and value of child in duplicate is id of original thread.
for musicians playing only one instrument 
the value of child is its id itself 


IMPLEMENTATION:
All the duplicate and original threads run stage execution part.
if stage is 0 its acoustic if 1 electric 
SINGLE STAGE :
    i have initialised semaphore value to a and e for acoustic and electric stages respectively.
    if stages are available the perfoem else they wait .for only a specific time which i implemented by using sem_timeswait.
BOTH STAGES:
    original thread execute one stage (electrical)
    duplicate thread executes the other stage (acoustic)
    if any one is timed out both threads are cancelled ,(since they have lil time diff between arrival,which theoritically shouldnt be there){ASSUMING THIS IS OK}
    else if any stage accept one thread among these two , they check if other one got stage or not.
    if other one got stage it cancels itself ,else it sends musician to perform.
    since both thread are symmetric in execution , the other one cancels itself as itsees this one got the stage.


SINGERS:
     An universal variable of current electrical and acostic is taken ,each threads executes this , semaphore si initialised to current solo performers plus current stages of 1 type
    after semaphore admits them hreads check whether the current stage is empty or musician solo is performing ,if empty decrements currcount else gets name using find function and joins it , sends it a signal so thatits performnace is increased by 2 secs/

TSHIRTS:
    at the end of every performance tshirts function is called which have a semaphore initialised to no.of coordinators.If coordinators are free they admit musicians else musicians wait for them to become free.
