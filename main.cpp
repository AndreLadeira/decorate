#include "./apps/std/tsp_min.h"
#include "./apps/std/tsp_full.h"
#include "./apps/std/tsp_test.h"

int main(int argc, char* argv[])
try
{
    tsp_test(argc,argv);

}
catch(runtime_error& e){
    cout<< "Run time error : " << e.what() << endl;
}
