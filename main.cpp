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
using tsp::operator<<;

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

    Decorator< Creator<tsp::path_t> > creator( new tsp::path::CreateRandom(data.size()) );
    creator.decorate_with< CreatorCallsCounter<tsp::path_t> >();

    Decorator< Neighbor<tsp::path_t> > neighborhood( new tsp::_2optSingle() );

    Decorator< Objective<tsp::path_t, tsp::tsp_problem_data_t> >
            objective( new tsp::path::Objective(data) );

    Decorator< tsp::Accept1st > accept;
    Decorator< tsp::path::Updater > updateInner, updateOuter;

    updateOuter.decorate_with< UpdateDownStagCounter<tsp::path_t> >();
    Timer timer;
    outerloop().addTrigger( Trigger<double>("Timer",timer,0.5));
    outerloop().addTrigger( Trigger<>("Exploration loops",
                                      outerloop.as<Counter>(),
                                      plist.getValue("outer_loops").as<unsigned>()));
//    outerloop().addTrigger( Trigger<>("Create calls",creator.as<Counter>(),
//                                                     100));
    innerloop().addTrigger( Trigger<>("Intensification loops",
                                      innerloop.as<Counter>(),
                                      plist.getValue("inner_loops").as<unsigned>()));
    innerloop().addTrigger( Trigger<double>("Inner loop timer",Timer(),0.01));

    outerloop().addTrigger( Trigger<>("Stagnation Counter",
                                      updateOuter.as<Counter>(),
                                      10));
    tsp::path_t best;
    unsigned best_cost = numeric_limits<unsigned>::max();

    outerloop().resetTriggers();

    while( outerloop->running() )
    {
        auto current = creator->create();
        auto current_cost = objective->get( current );

        innerloop().resetTriggers();

        while( innerloop->running() )
        {
            auto neighbor = neighborhood->get(current);
            auto newcost = objective->get( neighbor[0] );
            auto accepted = accept->get( current_cost, std::vector<decltype(best_cost)>(1,newcost) );
            if ( accepted )
                updateInner->update(current, current_cost, neighbor.at(accepted.get()), newcost);
        }
        updateOuter->update(best,best_cost,current,current_cost);
    }

    cout<< "Execution stop triggered by         : " << outerloop.staticObject().getTrigger() << endl;
    cout<< "Execution time                      : " << fixed << setprecision(4) << timer.getValue() << "s\n";
    cout<< "Times create called                 : " << fixed << creator.as<Counter>().getValue() << endl;
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
