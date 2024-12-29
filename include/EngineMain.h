#pragma once
//in any header file, it is good practice to add this
//why? if we include one header file multiple times, then system will give error if this line is not there as symbols will be added repeatedly
//this line tells compiler, include this header file only once even though user have included multiple times in code

#define MYICON 789
//101 is not hardcoded, we can use any number . Just this number must be unique to resource
//rhis line tells resource of name MYICON is given unique number and it is defined
