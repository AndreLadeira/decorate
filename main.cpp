#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip> // setprecision
#include <cassert>

#include "lib/onion.h"
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

    Decorator<ParameterLoader> ploader;
    ParameterList plist;

    ploader.decorate_with<mh::RRGAParametersChecker>();
    ploader->load(ss,plist);

    ifstream data_file( plist.getValue("file_name") );
    data_file.exceptions( istream::failbit | istream::badbit  );

    auto data = cops::tsp::tsp_tsplibDataLoader().load(data_file);

    Decorator<LoopController> outerloop;
    outerloop.decorate_with< LoopCallsCounter >();

    Decorator<LoopController> innerloop;
    innerloop.decorate_with< LoopCallsCounter >();

    Decorator< tsp::path::Creator > creator( new tsp::path::CreateRandom(data.size()) );
    creator.decorate_with< tsp::path::CreatorCallsCounter >();

    Decorator< tsp::path::Neighbor > neighborhood( new tsp::path::MaskReinsert() );
    //Decorator< tsp::path::Neighbor > neighborhood( new tsp::path::_2optSingle() );
    //Decorator< tsp::path::Neighbor > neighborhood( new tsp::path::_2optAll() );

    Decorator< tsp::path::ObjectiveBase >
            objective( new tsp::path::Objective(data) );
    objective.decorate_with< tsp::path::ObjectiveCallsCounter >(false);

    Decorator< tsp::AcceptBest > accept;
    Decorator< tsp::path::Updater > updateInner, updateOuter;

    updateOuter.decorate_with< min::UpdateStagnationCounter<tsp::path_t> >();
    Timer timer;
    //outerloop.staticObject().addTrigger( Trigger<double>("Timer",timer,2.0));
//    outerloop().addTrigger( Trigger<>("Exploration loops",
//                                      outerloop.as<Counter>(),
//                                      plist.getValue("outer_loops").as<unsigned>()));
//    outerloop().addTrigger( Trigger<>("Create calls",creator.as<Counter>(),
//                                                     100));

    outerloop.staticObject().addTrigger( Trigger<>("Objective fcn calls",objective.as<Counter>(),1e6));
    //innerloop.staticObject().addTrigger( Trigger<double>("Timer",timer,2.0));
    innerloop.staticObject().addTrigger( Trigger<>("Intensification loops",
                                      innerloop.as<Counter>(),
                                      plist.getValue("inner_loops").as<unsigned>()));
    innerloop.staticObject().addTrigger( Trigger<>("Objective fcn calls",objective.as<Counter>(),1e6));
//    innerloop().addTrigger( Trigger<double>("Inner loop timer",Timer(),0.01));

//    outerloop().addTrigger( Trigger<>("Stagnation Counter",
//                                      updateOuter.as<Counter>(),
//                                      20));
    tsp::path_t best;
    unsigned best_cost = numeric_limits<unsigned>::max();

    outerloop.staticObject().resetTriggers();

    while( outerloop->running() )
    {
        auto current = creator->create();
        auto current_cost = objective->get( current );

        innerloop.staticObject().resetTriggers();

        while( innerloop->running() )
        {
            auto neighbor = neighborhood->get(current);
            auto newcost = objective->get( neighbor );
            auto accepted = accept->get( current_cost, newcost );
            if ( accepted )
                updateInner->update(current, current_cost,
                                    neighbor.at(accepted.get()),
                                    newcost.at(accepted.get()));
        }
        updateOuter->update(best,best_cost,current,current_cost);
    }

    cout<< "Execution stop triggered by         : " << outerloop.staticObject().getTrigger() << endl;
    cout<< "Execution time                      : " << fixed << setprecision(4) << timer.getValue() << "s\n";
    cout<< "Times create called                 : " << creator.as<Counter>().getValue() << endl;
    cout<< "Times objective called              : " << objective.as<Counter>().getValue() << endl;
    cout<< "Outer loops count                   : " << outerloop.as<Counter>().getValue() << endl;
    cout<< "Final result                        : " << best_cost << endl;
    cout<< "Final path                          : " << best;

#ifdef __DEBUG__
    assert(best_cost = objective->get(best));
#endif


}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}
