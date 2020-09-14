#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip> // setprecision
#include <cassert>
#include <clocale>

#include "lib/onionmh.h"
#include "mh/rrga.h"
#include "lib/stddecorators.h"
#include "lib/trackstats.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
using tsp::path::operator<<;
using namespace tsp;

int main(int argc, char* argv[])
try
{

    stringstream        ss;
    ResettableTimer     runtimer;
    ResettableTimer     exptimer;
    Timer               totalTimer;
    ParameterList       parameters;
    unsigned            best_overall_cost;
    tsp::path_t         best_overall_path;

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
    Onion< path::Updater >              updateIntensification;
    Onion< path::Updater >              updateExploration;
    Onion< path::Updater >              updateRun;

    // layers

    exploration_loop_controller.        addLayer< LoopCallsCounter >();
    //exploration_loop_controller.          addLayer< LoopRecorder >();

    intensification_loop_controller.    addLayer< LoopCallsCounter >();
    intensification_loop_controller.    addLayer< LoopRecorder >( parameters("intensifications").as<unsigned>()/100 );

    repetitions_loop_controller.        addLayer< LoopCallsCounter >(true);
    //repetitions_loop_controller.           addLayer< LoopRecorder >();

    create.             addLayer< path::CreatorCallsCounter >();

    objective.          addLayer< path::ObjectiveCallsCounter >();

    updateRun.          addLayer< path::UpdateRecorder >();

    updateExploration.  addLayer< path::UpdateRecorder >();

    // terminantion triggers

    exploration_loop_controller.          core().addTrigger( Trigger<>("Exploration loops",       exploration_loop_controller.as<ResettableCounter>(),
                                                                       parameters("explorations").as<unsigned>()));
    exploration_loop_controller.          core().addTrigger( Trigger<>("Objective fcn calls",     objective.as<ResettableCounter>(),  1e6));
    intensification_loop_controller.      core().addTrigger( Trigger<>("Intensification loops",   intensification_loop_controller.as<ResettableCounter>(),
                                                                       parameters("intensifications").as<unsigned>()));
    intensification_loop_controller.      core().addTrigger( Trigger<>("Objective fcn calls",     objective.as<ResettableCounter>(),  1e6));
    repetitions_loop_controller.          core().addTrigger( Trigger<>("Repetitions",             repetitions_loop_controller.as<ResettableCounter>(),
                                                                       parameters("repetitions").as<unsigned>()));

    // Tracks
    // Track<>                 bestXexplLoops      (   exploration_loop_controller.as<Counter>(),  BoundValue<unsigned>(run_cost)        );
    // Track<>                 currentXintLoops    (   intensification_loop_controller.as<Counter>(),  BoundValue<unsigned>(current_cost)    );
//    Track<>                 bestXrun            (   repetitions_loop_controller.as<Counter>(),   BoundValue<unsigned>(run_cost)        );
//    Track<unsigned,double>  timeXrun            (   repetitions_loop_controller.as<Counter>(),   runtimer                              );
//    Track<>                 objCallsXrun        (   repetitions_loop_controller.as<Counter>(),   objective.as<Counter>()               );

//    Track<int> trval(val);
//    Track<double> trtime(timer);
//    Track<int> trref(refv);

//    Subject s;
//    MultiTrack<int> mtrVal(val,s);

    // values and tracks
    unsigned exp_cost;
    unsigned rep_cost;

    MultiTrack<unsigned> mtrack_exp_cost( (onion::RefValue<unsigned>(exp_cost) ), intensification_loop_controller.core() );
    MultiTrack<double>   mtrack_exp_time( exptimer, intensification_loop_controller.core() );

    intensification_loop_controller.as<Recorder>().addTrack(mtrack_exp_cost);
    intensification_loop_controller.as<Recorder>().addTrack(mtrack_exp_time);
    intensification_loop_controller.as<Recorder>().start();

    Track<double> track_exp_time(exptimer);
    Track<unsigned> track_exp_cost( (onion::RefValue<unsigned>(exp_cost)) );
    Track<unsigned> track_rep_cost( (onion::RefValue<unsigned>(rep_cost)) );

    updateExploration.as<Recorder>()    .addTrack(track_exp_time);
    updateExploration.as<Recorder>()    .addTrack(track_exp_cost);
    updateRun.as<Recorder>()            .addTrack(track_rep_cost);

    updateExploration.as<Recorder>()    .start();
    updateRun.as<Recorder>()            .start();
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
        rep_cost = objective(rep_best_path);

        while( exploration_loop_controller() )
        {
            auto exp_best_path = create();
            exp_cost = objective( exp_best_path );
            exptimer.reset();

            while( intensification_loop_controller() )
            {
                auto neighbor = neighborhood(exp_best_path);
                auto newcost = objective( neighbor );
                auto accepted = accept( exp_cost, newcost );
                if ( accepted )
                    updateIntensification(
                                exp_best_path, exp_cost,
                                neighbor.at(accepted.index()),
                                newcost.at(accepted.index())
                                );
            }
            updateExploration(rep_best_path,rep_cost,exp_best_path,exp_cost);
        }
        updateRun(best_overall_path,best_overall_cost,rep_best_path,rep_cost);
    }

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto costStats  = TrackStats<unsigned>( track_exp_cost );
    auto repStats   = TrackStats<unsigned>( track_rep_cost );
    auto stagStats  = TrackStats<unsigned>( getStagnationTrack( mtrack_exp_cost ) );

    cout<< endl;
    cout<< "File name (runs/exps./intens.)            : " << parameters("file_name").as<string>() << " (" << parameters("repetitions").as<unsigned>() << "/"
                                                    << parameters("explorations").as<unsigned>() << "/" << parameters("intensifications").as<unsigned>() << ")\n";
    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << "(s)\n";
    cout<< "Total Runs/Explorations/Intensifications  : " << repetitions_loop_controller.as<ResettableCounter>().getValue() << "/"
                                                    << exploration_loop_controller.as<ResettableCounter>().getAccumulated()<< "/"
                                                    << intensification_loop_controller.as<ResettableCounter>().getAccumulated() <<endl;

    cout<< "Obj. fcn. calls                           : "  << objective.as<ResettableCounter>().getAccumulated()  << "\n";
    cout<< "Run time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << "/" << timeStats.max() << "/" << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << costStats.min() << "/" << costStats.max() << "/" << costStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << "/" << repStats.max() << "/" << repStats.average() << endl;
    cout<< "Stagnation Stats. (min/max/avg)           : " << setprecision(0) << stagStats.min() * 100 << "/" << stagStats.max() * 100 << "/" << stagStats.average() * 100 << endl;

    cout<< "\nAll Curves + average curve\n\n";
    cout << mtrack_exp_cost;

    cout<< endl;

    //assert(bestStats.min() == objective(best));

}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}
