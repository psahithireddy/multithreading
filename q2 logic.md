All the zones , students and companies are threads.
each struct[i] corresponds to data  by thread[i]. (for all  i in valid range)

INITALISATION:
all the data is filled and threads are created.


COMPANY PRODUCTION:
1)threads corresponding to company run comp_production
LOGIC:
Each company produces certain no,of batches random between 1 and 5.
Then they busy wait for a zone to be free ,ie they check if any zone needs vaccine,untill all batches are  distributed.
if it is free locks it and sends vaccines.(changes appropriate variables)
Once all batches are taken by zones company resumes production by calling the same function again.


VACCINATION ZONE:
2)threads corresponding to zones run zone_distribution
LOGIC:
Each zone intialises itself with it needs a vaccine.(with variable vaccneed).It waits till vaccine is provided. 
As soon as company gives vaccine it make slots.
each struct of zone has array of 8 slots ,if any slot  is -2 employees slot isnt provided ,else -1 student can take slot,after student take slot it value inside slot will be studentthread index.
Then after slots are made ,if waits till all the current slots provided are used up by students.
then for the slots it provided it will vaccinate students. and no.of students vaccianted are incremented(inside a mutex lock)
it runs till vaccines are over.once vaccines are over it justs make vaccneed to 1 ie saying i need vaccines to comapnies :)


STUDENTS:
3)threads corresponding to students execute student_coming
LOGIC:
as thread starts executing iy sleeps for time of arrival seconds ,then mark student arrived and waiting.
then in a loop of 3 , students busywait for any of the slots in zones to be free ,ie slot value to be -1,so that slot value can be student thread index.(note :since corresponding struct has all info.about student)
if they find a slot they decrement slot count,waiting count,and changes their status to waiting for vacciantion inside a mutexlock.
then wait untill till they are vaccianted by zones.
after that ,they take antibody test .
ANTIBODY TEST :
a random number between 0 to 1 is generated 
if that number is grater than probability 
then tested positive ,else negative

if tested positive variable is set and loop is broken,then if no threads are waiting it sleeps for sometime (waits if any comes).else it checked whether all arrived if yes then exits program ,else cancels cureent thread.
if there are students waiting ,then it gets cancelled.

if tested negative the students waiting is incremented and loop continues.

EXIT ELABORATED:
exit condition in students waiting is 0 , any student thread executing this will wait execute this(so as to make sure if any comes late they can come) { Helping friends XD}
then in a while loop of students waiting=0, check if all students have arrived and no students waiting if yes exit else cancel current thread.

(worst case : i did students_waiting -- when u get a slot , so if slot is provided and vaccination isnt done,and at the same time a thread checks arrived and no waiting it would exit without this last person getting vaccianted)
(so at worst case a student is left out but thats rare)