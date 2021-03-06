#ifndef TSP_FULL_H
#define TSP_FULL_H

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
#include "lib/trackutil.h"
#include "cops/tsp.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
using tsp::path::operator<<;
using namespace tsp;

int tsp_full(int argc, char* argv[])
{
    stringstream        ss;
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
    auto data = cops::tsp::tsplibDataLoader()(file);
    file.close();

    // Loop Controller Onions

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>( parameters("repetitions").as<unsigned>() ) );

    // algorithm onions

    Onion< path::Creator >              create          ( make_shared<path::CreateRandom>( data.size() ) );
    //Onion< path::Creator >              create          ( make_shared<path::CreateGreedy>( data ) );

    Onion< path::Neighbor >             neighborhood    ( make_shared<path::RemoveReinsert>() );
    //Onion< path::Neighbor >             neighborhood    ( make_shared< path::_2opt>() );

    Onion< path::Objective >            objective       ( make_shared< path::tspObjective >( data ) );
    Onion< tsp::Accept >                accept( make_shared< tsp::Accept1st >() );;
    Onion< path::Updater >              updateInt;
    Onion< path::Updater >              updateExp;
    Onion< path::Updater >              updateRun;

    // Decorator layers

    auto exploration_timer          = exploration_loop_controller.addLayer< LoopTimer >();
    auto intensification_loop_rec   = intensification_loop_controller.addLayer< LoopRecorder >( parameters("intensifications").as<unsigned>()/100 );
    //intensification_loop_controller.    addLayer< LoopTimer >();

    auto repLoopCountTotal = repetitions_loop_controller.addLayer< LoopCounter >();
    auto expLoopCountTotal = exploration_loop_controller.addLayer< LoopCounter >();
    auto intLoopCountTotal = intensification_loop_controller.addLayer< LoopCounter >();

    create.addLayer< path::CreatorCallsCounter >();

    auto objective_calls_at_exp = objective.addLayer< path::ObjectiveCallsCounter >();
    auto objective_calls_total  = objective.addLayer< path::ObjectiveCallsCounter >();

    auto updateRunRec = updateRun.addLayer< path::UpdateRecorder >();
    auto updateExpRec = updateExp.addLayer< path::UpdateRecorder >();
    auto updateExpStagCnt = updateExp.addLayer< path::UpdateStagnationCounter >();

    // stop conditions
    //auto total_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_total, 2e6);

    //repetitions_loop_controller     .core().addStopCondition( total_obj_calls_stop );
    //exploration_loop_controller     .core().addStopCondition( total_obj_calls_stop );
    //intensification_loop_controller .core().addStopCondition( total_obj_calls_stop );

    //auto exp_time_stop = StopCondition<double>("Exploration time", *exploration_timer, 1.0);
    //exploration_loop_controller     .core().addStopCondition( exp_time_stop );
    //intensification_loop_controller .core().addStopCondition( exp_time_stop );


    //auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, 1e8);
    intensification_loop_controller.core().resetObject( *objective_calls_at_exp );


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

    MultiTrack<unsigned> mtrack_exp_cost        ("exp. cost",        (onion::RefValue<unsigned>(exp_cost) ),    intensification_loop_controller.core() );
    MultiTrack<double>   mtrack_exp_time        ("exp. time",        *exploration_timer,                        intensification_loop_controller.core() );
    MultiTrack<unsigned> mtrack_obj_fcn_calls   ("obj. fcn. calls",  *objective_calls_at_exp,                   intensification_loop_controller.core() );

    intensification_loop_rec->addTrack(mtrack_exp_cost);
    intensification_loop_rec->addTrack(mtrack_exp_time);
    intensification_loop_rec->addTrack(mtrack_obj_fcn_calls);
    intensification_loop_rec->start();

    Track<double> track_exp_time    ("exp. time", *exploration_timer );
    Track<unsigned> track_exp_cost  ("exp. cost", onion::RefValue<unsigned>(exp_cost)    );
    Track<unsigned> track_rep_cost  ("rep. cost", onion::RefValue<unsigned>(rep_cost)    );

    Track<unsigned> track_stag_run("stagnation",*updateExpStagCnt);
    updateRunRec->addTrack(track_stag_run);
    //repetitions_loop_controller.core().resetObject(*updateExpStagCnt);

    updateExpRec->addTrack(track_exp_time);
    updateExpRec->addTrack(track_exp_cost);
    updateRunRec->addTrack(track_rep_cost);

    updateExpRec->start();
    updateRunRec->start();
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

        auto rep_best_path = create();
        rep_cost = objective(rep_best_path);

        while( exploration_loop_controller() )
        {
            auto exp_best_path = create();
            exp_cost = objective( exp_best_path );

            while( intensification_loop_controller() )
            {
                auto neighbor   = neighborhood(exp_best_path);
                auto newcost    = objective( neighbor );
                auto accepted   = accept( exp_cost, newcost );
                if ( accepted )
                    updateInt(
                                exp_best_path, exp_cost,
                                neighbor.at(accepted.index()),
                                newcost.at(accepted.index())
                                );
            }
            updateExp(rep_best_path,rep_cost,exp_best_path,exp_cost);
        }
        updateRun(best_overall_path,best_overall_cost,rep_best_path,rep_cost);
    }

    std::cout << "Load Parameters=" << loadParameters.getLayers() << endl;

    std::cout<<"Runs LC=" << repetitions_loop_controller.getLayers()
             << "("   << repetitions_loop_controller.core().getConditions() << ")\n";
    std::cout<<"Exps LC=" << exploration_loop_controller.getLayers()
             << "("   << exploration_loop_controller.core().getConditions() << ")\n";
    std::cout<<"Ints LC=" << intensification_loop_controller.getLayers()
             << "("   << intensification_loop_controller.core().getConditions() << ")\n";

    std::cout << "Create=" << create.getLayers() << endl;
    std::cout << "Neighborhood=" << neighborhood.getLayers() << endl;
    std::cout << "Objective=" << objective.getLayers() << endl;
    std::cout << "Accept=" << accept.getLayers() << endl;

    std::cout << "Update Runs=" << updateRun.getLayers() << endl;
    std::cout << "Update Exps=" << updateExp.getLayers() << endl;
    std::cout << "Update Ints=" << updateInt.getLayers() << endl;

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto costStats  = TrackStats<unsigned>( track_exp_cost );
    auto repStats   = TrackStats<unsigned>( track_rep_cost );
    auto stagStats  = TrackStats<unsigned>( getStagnationTrack( mtrack_exp_cost ) );
    auto stgStats  = TrackStats<unsigned>( track_stag_run );

    cout<< endl;
    cout<< "File name (runs/exps./intens.)            : " << parameters("file_name").as<string>() << " (" << parameters("repetitions").as<unsigned>() << "/"
                                                          << parameters("explorations").as<unsigned>() << "/" << parameters("intensifications").as<unsigned>() << ")\n";
    cout<< "Stop Condition (run/exp./inten.)          : " << repetitions_loop_controller.core().getStopCondition() << " / "
                                                          << exploration_loop_controller.core().getStopCondition() << " / "
                                                          << intensification_loop_controller.core().getStopCondition() << endl;
    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << " (s)\n";
    cout<< "Total Runs/Explorations/Intensifications  : " << repLoopCountTotal->getValue() << " / " << expLoopCountTotal->getValue()<< " / " << intLoopCountTotal->getValue() << endl;

    cout<< "Obj. fcn. calls                           : "  << objective_calls_total->getValue()  << "\n";
    cout<< "Run time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << costStats.min() << " / " << costStats.max() << " / " << costStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;
    cout<< "Stagnation (only reps., min/max/avg)      : " << setprecision(0) << stgStats.min() << " / " << stgStats.max() << " / " << stgStats.average() << endl;

    unsigned multp = parameters("intensifications").as<unsigned>() / 100;

    cout<< "Stagnation Stats. (min/max/avg)           : " << setprecision(0) << stagStats.min() * multp << "/" << stagStats.max() * multp << "/" << stagStats.average() * multp << endl;

//    cout<< "\nmin/max/average curves\n\n";
//    cout << mtrack_exp_cost;

//    cout<< "\nAverage results x average obj fcn calls\n\n";

//    printAverages<unsigned,unsigned>(mtrack_obj_fcn_calls,mtrack_exp_cost);

    TrackPrinter printer;
    std::vector<unsigned> exps(mtrack_obj_fcn_calls.size());
    std::iota(exps.begin(),exps.end(),0);
    for(auto& v : exps ) v *= multp;

    printer << TrackPrinter::track<unsigned>("exploration",exps) << mtrack_exp_time
            << mtrack_exp_cost << mtrack_obj_fcn_calls;
    printer.print(cout);

    cout<< endl;

    //assert(bestStats.min() == objective(best));
    return 0;
}


#endif
