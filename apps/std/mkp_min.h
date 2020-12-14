#ifndef MKP_MIN_H
#define MKP_MIN_H

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
#include "cops/mkp.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
using namespace mkp;

int mkp_min(int argc, char* argv[])
{
    stringstream        ss;
    Timer               totalTimer;
    ParameterList       parameters;
    unsigned            best_overall_cost;
    mkp::mkp_sol_t      best_overall_sol;

    // parameters

    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(ss,parameters);

    // data file

    ifstream file( parameters("file_name") );
    file.exceptions( istream::failbit | istream::badbit  );
    auto data = cops::mkp::mkp_datDataLoader()(file);
    file.close();


    // Loop Controller Onions

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>("Exploration Loop",       parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>("Intensification Loop",   parameters("intensifications").as<unsigned>() ) );
    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>("Repetitions Loop",       parameters("repetitions").as<unsigned>() ) );

    // algorithm onions

    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemOrder>( data ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemProfit>( data ) );
    Onion< mkp::Creator >       create  ( make_shared<mkp::CreateRandom>( data. ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemUtilityTW2CR>( data ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemUtilityWI2CR>( data ) );


    Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::P2WI ) );
    //auto s = create();
    //auto n = neighborhood( s );
    //Onion< path::Neighbor >             neighborhood    ( make_shared< path::_2optSingle>() );
    //Onion< path::Neighbor >             neighborhood    ( make_shared< path::_2optAll>() );

    Onion< mkp::Objective >     objective( make_shared< mkp::mkpObjective >( data ) );
    Onion< mkp::Accept1st >     accept;

    Onion< mkp::Updater >       updateIntensification;
    Onion< mkp::Updater >       updateExploration;
    Onion< mkp::Updater >       updateRun;

    // layers

    auto objective_calls_total  = objective.addLayer< mkp::ObjectiveCallsCounter >();
    auto update_run_recorder    = updateRun.addLayer< mkp::UpdateLocalRecorder >();
    auto update_exp_recorder    = updateExploration.addLayer< mkp::UpdateLocalRecorder >();
    auto exploration_timer      = exploration_loop_controller.addLayer< LoopTimer >();

    //MultiTrack<unsigned> mtrack_exp_cost("exp. cost",(onion::RefValue<unsigned>(exp_cost) ),intensification_loop_controller.core() );

    Track<double> track_exp_time("exp. time", *exploration_timer );
    update_exp_recorder->addTrack(track_exp_time);

    update_exp_recorder->start();
    update_run_recorder->start();

    best_overall_sol = create();
    best_overall_cost = objective(best_overall_sol);
    totalTimer.start();

    while( repetitions_loop_controller() )
    {
        onion::reset_random_engine();

        auto rep_best_path = create();
        auto rep_cost = objective(rep_best_path);

        while( exploration_loop_controller() )
        {
            auto exp_best_path = create();
            auto exp_cost = objective( exp_best_path );

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
        updateRun(best_overall_sol,best_overall_cost,rep_best_path,rep_cost);
    }

    cout<< endl;
    cout<< "File name (runs/exps./intens.)            : " << parameters("file_name").as<string>() << " (" << parameters("repetitions").as<unsigned>() << "/"
                                                          << parameters("explorations").as<unsigned>() << "/" << parameters("intensifications").as<unsigned>() << ")\n";
    cout<< "Stop Condition (run/exp./inten.)          : " << repetitions_loop_controller.core().getStopCondition() << " / "
                                                          << exploration_loop_controller.core().getStopCondition() << " / "
                                                          << intensification_loop_controller.core().getStopCondition() << endl;
    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << " (s)\n";
    cout<< "Obj. fcn. calls                           : " << objective_calls_total->getValue()  << "\n";

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto repStats   = TrackStats<unsigned>( update_run_recorder.get()->getLocalTrack<unsigned>() );

    cout<< "Exp time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << expStats.min() << " / " << expStats.max() << " / " << expStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;

#ifdef __DEBUG__

    assert( objective(best_overall_sol) == expStats.max() );

#endif

    return 0;
}

#endif // MKP_MIN_H
