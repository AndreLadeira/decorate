#include <iostream>
#include "./apps/std/apps.h"

using namespace std;

int main(int argc, char* argv[])
try
{
    bmf_full(argc,argv);
}
catch(runtime_error& e){
    cout<< "Run time error : " << e.what() << endl;
}
