#include "tsp_min.h"

int main(int argc, char* argv[])
try
{
    tsp_min(argc,argv);
}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}
