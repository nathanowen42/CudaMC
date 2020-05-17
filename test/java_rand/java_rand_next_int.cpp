#include <iostream>
#include "java_random.h"

int main()
{
    java::random rand{25};
    
    for(int i = 0; i < 1000000; i++)
    {
        std::cout << rand.next_int() << std::endl;
    }
}
