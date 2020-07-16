#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip> // setprecision
#include <cassert>

#include "lib/onionmh.h"
#include "mh/rrga.h"
#include "lib/stddecorators.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
using tsp::path::operator<<;

int main(int argc, char* argv[])
try
{

    stringstream ss;
    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    ParameterList parameters;

    loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(ss,parameters);

    ifstream data_file( parameters.getValue("file_name") );
    data_file.exceptions( istream::failbit | istream::badbit  );

    auto data = cops::tsp::tsp_tsplibDataLoader()(data_file);

    Onion<LoopController> outerloop;
    outerloop.addLayer< LoopCallsCounter >();

    Onion<LoopController> innerloop;
    innerloop.addLayer< LoopCallsCounter >();

    Onion<LoopController> repetitionsloop;
    repetitionsloop.addLayer< LoopCallsCounter >();


    Onion< tsp::path::Creator > create( make_shared<tsp::path::CreateRandom>( data.size() ) );
    create.addLayer< tsp::path::CreatorCallsCounter >();


    Onion< tsp::path::Neighbor > neighborhood( make_shared<tsp::path::MaskReinsert>() );
    //Onion< tsp::path::Neighbor > neighborhood( make_shared<tsp::path::_2optSingle>() );
    //Onion< tsp::path::Neighbor > neighborhood( make_shared<tsp::path::_2optAll>() );

    Onion< tsp::path::AbstractObjective > objective( make_shared<tsp::path::Objective >(data) );
    objective.addLayer< tsp::path::ObjectiveCallsCounter >(false);

    Onion< tsp::AcceptBest > accept;
    Onion< tsp::path::Updater > updateInner, updateOuter;

    updateOuter.addLayer< min::UpdateStagnationCounter<tsp::path_t> >();
    //outerloop.core().addTrigger( Trigger<double>("Timer",Timer(),1.0));
    outerloop.core().addTrigger( Trigger<>("Exploration loops",
                                      outerloop.as<LoopCallsCounter>(),
                                      parameters.getValue("outer_loops").as<unsigned>()));
//    outerloop.core().addTrigger( Trigger<>("Create calls",create.as<Counter>(),100));

    outerloop.core().addTrigger( Trigger<>("Objective fcn calls",objective.as<Counter>(),1e6));
    innerloop.core().addTrigger( Trigger<>("Intensification loops",
                                      innerloop.as<Counter>(),
                                      parameters.getValue("inner_loops").as<unsigned>()));
    innerloop.core().addTrigger( Trigger<>("Objective fcn calls",objective.as<Counter>(),1e6));
    //innerloop.core().addTrigger( Trigger<double>("Inner loop timer",Timer(),1.0));

//    outerloop.core().addTrigger( Trigger<>("Stagnation Counter",
//                                      updateOuter.as<Counter>(),
//                                      20));
    repetitionsloop.core().addTrigger( Trigger<>("Repetitions",
                                                 repetitionsloop.as<Counter>(),
                                                 /*parameters.getValue("repetitions").as<unsigned>()*/ 2));

    while( repetitionsloop() )
    {
        //onion::reset_random_engine();

        tsp::path_t best;
        unsigned best_cost = numeric_limits<unsigned>::max();

        outerloop.core().hardReset();
        innerloop.core().hardReset();
        create.as<Counter>().hardReset();

        Timer timer(true);

        while( outerloop() )
        {
            auto current = create();
            auto current_cost = objective( current );

            while( innerloop() )
            {
                auto neighbor = neighborhood(current);
                auto newcost = objective( neighbor );
                auto accepted = accept( current_cost, newcost );
                if ( accepted )
                    updateInner(current, current_cost,
                                neighbor.at(accepted.index()),
                                newcost.at(accepted.index()));
            }
            updateOuter(best,best_cost,current,current_cost);
            innerloop.core().reset();
        }

        cout<< "\nExecution stop triggered by         : " << outerloop.core().getTrigger() << endl;
        cout<< "Execution time                      : " << fixed << setprecision(4) << timer.getValue() << "s\n";
        cout<< "Times create called                 : " << create.as<Counter>().getValue() << endl;
        cout<< "Times objective called              : " << objective.as<Counter>().getValue() << endl;
        cout<< "Outer loops count                   : " << outerloop.as<Counter>().getValue() << endl;
        cout<< "Final result                        : " << best_cost << endl;
        cout<< "Final path                          : " << best;
#ifdef __DEBUG__
        assert(best_cost = objective(best));
#endif
    }

}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}
