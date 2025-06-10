#include <stdio.h>
// Basic functions!
int sched_class;
void setschedclass(int choice){
    sched_class = choice;
}
int getschedclass(){
    return sched_class;
}
int get_random_value(){
    return (int)expdev(0.1);
}