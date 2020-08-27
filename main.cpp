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
using namespace tsp;

int main(int argc, char* argv[])
try
{

    stringstream ss;
    ResettableTimer runtimer(false);
    Timer totalTimer;
    ParameterList parameters;
    unsigned best_overall_cost;
    tsp::path_t best_overall_path;

    // parameters

    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(ss,parameters);

    // data file

    ifstream file( parameters("file_name") );
    file.exceptions( istream::failbit | istream::badbit  );
    auto data = cops::tsp::tsp_tsplibDataLoader()(file);
    file.close();

    // onions

    Onion<LoopController>               exploration_loop_controller;
    Onion<LoopController>               intensification_loop_controller;
    Onion<LoopController>               repetitions_loop_controller;
    Onion< path::Creator >              create          ( make_shared<path::CreateRandom>( data.size() ) );

    Onion< path::Neighbor >             neighborhood    ( make_shared<path::MaskReinsert>() );

    //Onion< path::Neighbor >           neighborhood    ( make_shared< path::_2optSingle>() );
    //Onion< path::Neighbor >           neighborhood    ( make_shared< path::_2optAll>() );

    Onion< path::AbstractObjective >    objective       ( make_shared< path::Objective >( data ) );
    Onion< tsp::AcceptBest >            accept;
    Onion< path::Updater >              updateInner;
    Onion< path::Updater >              updateOuter;
    Onion< path::Updater >              updateRun;

    // layers

    exploration_loop_controller.          addLayer< LoopCallsCounter >();
    //exploration_loop_controller.          addLayer< LoopRecorder >();

    intensification_loop_controller.          addLayer< LoopCallsCounter >();
    //intensification_loop_controller.          addLayer< LoopRecorder >( parameters("inner_loops").as<unsigned>()/100 );

    repetitions_loop_controller.           addLayer< LoopCallsCounter >();
    //repetitions_loop_controller.           addLayer< LoopRecorder >();

    create.             addLayer< path::CreatorCallsCounter >();

    objective.          addLayer< path::ObjectiveCallsCounter >();

    updateRun.          addLayer< path::UpdateRecorder >();

    // terminantion triggers

    exploration_loop_controller.          core().addTrigger( Trigger<>("Exploration loops",       exploration_loop_controller.as<ResettableCounter>(), 2 ));// parameters("outer_loops").as<unsigned>()));
    exploration_loop_controller.          core().addTrigger( Trigger<>("Objective fcn calls",     objective.as<ResettableCounter>(),  1e6));
    intensification_loop_controller.      core().addTrigger( Trigger<>("Intensification loops",   intensification_loop_controller.as<ResettableCounter>(),  parameters("inner_loops").as<unsigned>()));
    intensification_loop_controller.      core().addTrigger( Trigger<>("Objective fcn calls",     objective.as<ResettableCounter>(),  1e6));
    repetitions_loop_controller.          core().addTrigger( Trigger<>("Repetitions",             repetitions_loop_controller.as<ResettableCounter>(),   parameters("repetitions").as<unsigned>()));

    // Tracks
    // Track<>                 bestXexplLoops      (   exploration_loop_controller.as<Counter>(),  BoundValue<unsigned>(run_cost)        );
    // Track<>                 currentXintLoops    (   intensification_loop_controller.as<Counter>(),  BoundValue<unsigned>(current_cost)    );
//    Track<>                 bestXrun            (   repetitions_loop_controller.as<Counter>(),   BoundValue<unsigned>(run_cost)        );
//    Track<unsigned,double>  timeXrun            (   repetitions_loop_controller.as<Counter>(),   runtimer                              );
//    Track<>                 objCallsXrun        (   repetitions_loop_controller.as<Counter>(),   objective.as<Counter>()               );

    // recorders

    //exploration_loop_controller.as<Recorder>()    .addTrack(  bestXexplLoops  );
    //intensification_loop_controller.as<Recorder>()    .addTrack(  currentXintLoops );
//    updateRun.as<Recorder>()    .addTrack(  bestXrun );
//    updateRun.as<Recorder>()    .addTrack(  timeXrun );
//    updateRun.as<Recorder>()    .addTrack(  objCallsXrun );


    best_overall_path = create();
    best_overall_cost = objective(best_overall_path);
    totalTimer.start();

    while( repetitions_loop_controller() )
    {
        onion::reset_random_engine();
        runtimer.reset();

        auto rep_best_path = create();
        auto rep_cost = objective(rep_best_path);

        while( exploration_loop_controller() )
        {
            auto exp_best_path = create();
            auto exp_cost = objective( exp_best_path );

            intensification_loop_controller.as<LoopController>().reset();

            while( intensification_loop_controller() )
            {
                auto neighbor = neighborhood(exp_best_path);
                auto newcost = objective( neighbor );
                auto accepted = accept( exp_cost, newcost );
                if ( accepted )
                    updateInner(exp_best_path, exp_cost,
                                neighbor.at(accepted.index()),
                                newcost.at(accepted.index()));
            }
            updateOuter(rep_best_path,rep_cost,exp_best_path,exp_cost);
        }
        updateRun(best_overall_path,best_overall_cost,rep_best_path,rep_cost);
    }

//    auto timeStats  = Stats<unsigned,double>( timeXrun );
//    auto bestStats  = Stats<>( bestXrun );
//    auto objFStats  = Stats<>( objCallsXrun );

    cout<< endl;
    cout<< "Total time                          : " << fixed << setprecision(2) << totalTimer.getValue() << "(s)\n";
    cout<< "Runs/Explorations/Intensifications  : " << repetitions_loop_controller.as<Counter>().getValue() << "/"
                                                    << repetitions_loop_controller.as<Counter>().getValue() << "/"
                                                    << exploration_loop_controller.as<Counter>().getValue() <<endl;
    //cout<< "Obj. fcn. calls (min/max/avg)       : " << fixed << setprecision(0) << objFStats.min() << "/" << objFStats.max() << "/" << objFStats.average() << "\n";
    //cout<< "Run time (min/max/avg)              : " << fixed << setprecision(2) << timeStats.min() << "/" << timeStats.max() << "/" << timeStats.average() << "(s)\n";
    //cout<< "Final result                         : " << setprecision(0) << bestStats.min() << "/" << bestStats.max() << "/" << bestStats.average() << endl;
    cout<< "Final result                         : " << setprecision(0) << best_overall_cost << endl;
    cout<< endl;

    //assert(bestStats.min() == objective(best));

}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}
