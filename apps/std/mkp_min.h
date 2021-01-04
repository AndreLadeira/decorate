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

    onion::reset_random_engine();


    //-----------------------------------------------------------------
    //                      STRATEGY
    //-----------------------------------------------------------------
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemOrder>( data ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemProfit>( data ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateRandom>( data ) );
    //Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemUtilityTW2CR>( data ) );
    Onion< mkp::Creator >       create  ( make_shared<mkp::CreateByItemUtilityWI2CR>( data ) );

    Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::P2WI ) );

    Onion< mkp::Objective >     objective( make_shared< mkp::mkpObjective >( data ) );
    Onion< mkp::Accept >        accept( make_shared< mkp::Accept1st >() );

    //-----------------------------------------------------------------
    //                      LOOP CONTROLLERS & UPDATES
    //-----------------------------------------------------------------

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>( parameters("repetitions").as<unsigned>() ) );

    Onion< mkp::Updater >   updateIntensification;
    Onion< mkp::Updater >   updateExploration;
    Onion< mkp::Updater >   updateRun;

    //-----------------------------------------------------------------
    //                      Total Obj. Fcn Calls
    //-----------------------------------------------------------------
    auto objective_calls_total  = objective.addLayer< mkp::ObjectiveCallsCounter >();
    auto objective_calls_at_exp = objective.addLayer< mkp::ObjectiveCallsCounter >();
    //-----------------------------------------------------------------
    //                      Recorders
    //-----------------------------------------------------------------
    auto update_run_recorder    = updateRun.addLayer< mkp::UpdateLocalRecorder >();
    auto update_exp_recorder    = updateExploration.addLayer< mkp::UpdateLocalRecorder >();
    auto int_loop_rec           = intensification_loop_controller.addLayer< LoopRecorder >( parameters("intensifications").as<unsigned>()/100 );
    //-----------------------------------------------------------------
    //                      Exploration Time
    //-----------------------------------------------------------------
    auto exploration_timer = exploration_loop_controller.addLayer< LoopTimer >();
    Track<double> track_exp_time("exp. time", *exploration_timer );
    update_exp_recorder->addTrack(track_exp_time);
    //-----------------------------------------------------------------
    //                      Stag. Counter
    //-----------------------------------------------------------------
    auto accept_stag_cnt = accept.addLayer< mkp::AcceptStagCounter >();
    Track<unsigned> track_stag_exp("stag. count", *accept_stag_cnt );
    update_exp_recorder->addTrack(track_stag_exp);
    exploration_loop_controller.core().resetObject( *accept_stag_cnt );
    //-----------------------------------------------------------------
    //                      Exp. Improvement Meter
    //-----------------------------------------------------------------
    auto improvement_meter = updateIntensification.addLayer< mkp::UpdateImprovementMeter >();
    auto resetter = updateExploration.addLayer< mkp::UpdateResetObject >();
    resetter->setObject(improvement_meter);
    Track<double> track_improv("Improvement", *improvement_meter );
    update_exp_recorder->addTrack(track_improv);
    //-----------------------------------------------------------------
    //              Stop intens. at 1MM objc. fcn. calls
    //-----------------------------------------------------------------
    auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, parameters("objfcncalls").as<unsigned>() );
    intensification_loop_controller.core().addStopCondition(int_obj_calls_stop, LoopController::ON_STOP::RESET );
    //-----------------------------------------------------------------
    //              Detailed Execution Tracks
    //-----------------------------------------------------------------
    /*

    MultiTrack<unsigned> mtrack_exp_cost        ("exp. cost",        (onion::RefValue<unsigned>(exp_cost) ),    intensification_loop_controller.core() );
    MultiTrack<double>   mtrack_exp_time        ("exp. time",        *exploration_timer,                        intensification_loop_controller.core() );
    MultiTrack<unsigned> mtrack_obj_fcn_calls   ("obj. fcn. calls",  *objective_calls_at_exp,                   intensification_loop_controller.core() );

    intensification_loop_rec->addTrack(mtrack_exp_cost);
    intensification_loop_rec->addTrack(mtrack_exp_time);
    intensification_loop_rec->addTrack(mtrack_obj_fcn_calls);
    intensification_loop_rec->start();

*/

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
    auto stgStats   = TrackStats<unsigned>( track_stag_exp );
    auto repStats   = TrackStats<unsigned>( update_run_recorder.get()->getLocalTrack<unsigned>() );
    auto impStats   = TrackStats<double>( track_improv );

    cout<< "Exp time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << expStats.min() << " / " << expStats.max() << " / " << expStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;
    cout<< "Improvement  (all exps., min/max/avg)     : " << fixed << setprecision(1) << impStats.min() * 100.0 << " / " << impStats.max() * 100.0 << " / " << impStats.average() * 100.0 << " (%)\n";
    cout<< "Stagnation   (only reps., min/max/avg)    : " << setprecision(0) << stgStats.min() << " / " << stgStats.max() << " / " << stgStats.average() << endl;

#ifdef __DEBUG__

    assert( objective(best_overall_sol) == expStats.max() );

#endif

    return 0;
}

#endif // MKP_MIN_H
