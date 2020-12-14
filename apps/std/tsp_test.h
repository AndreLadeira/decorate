#ifndef TSP_TEST_H
#define TSP_TEST_H
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip> // setprecision
#include <cassert>

#include "lib/onionmh.h"
#include "mh/rrga.h"
#include "lib/stddecorators.h"
#include "lib/trackutil.h"
#include "cops/tsp.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
//using tsp::path::operator<<;
using namespace tsp;

void tsp_test(int argc, char* argv[])
{
    stringstream        ss;
    ParameterList       parameters;

    // parameters

    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    //loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(ss,parameters);

    // data file

    ifstream file( parameters("file_name") );
    file.exceptions( istream::failbit | istream::badbit  );
    auto data = cops::tsp::tsplibDataLoader()(file);
    file.close();

    // Loop Controller Onions

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );

    // algorithm onions
    //onion::reset_random_engine(0);
    Onion< path::Creator >              create      ( make_shared<path::CreateGreedy>( data ) );
    //Onion< path::Creator >              create      ( make_shared<path::CreateRandom>( data.size() ) );

    //Onion< path::Neighbor >             neighborhood( make_shared<path::RemoveReinsert>() );
    Onion< path::Neighbor >             neighborhood( make_shared< path::_2opt>() );

    Onion< path::Objective >            objective   ( make_shared< path::tspObjective >( data ) );

    //Onion< tsp::Accept >                accept      ( make_shared< tsp::Accept1st >() );
    Onion< tsp::Accept >                accept      ( make_shared< tsp::AcceptBest >() );

    Onion< path::Updater >              updateIntensification;
    Onion< path::Updater >              updateExploration;

    //-----------------------------------------------------------------
    //                      Recorders
    //-----------------------------------------------------------------
    auto update_exp_recorder    = updateExploration.addLayer< path::UpdateLocalRecorder >();
    //-----------------------------------------------------------------
    //                      Total Obj. Fcn Calls
    //-----------------------------------------------------------------
    auto objective_calls_total  = objective.addLayer< path::ObjectiveCallsCounter >();
    auto objective_calls_at_exp = objective.addLayer< path::ObjectiveCallsCounter >();
    //-----------------------------------------------------------------
    //                      Total Loop Counters
    //-----------------------------------------------------------------
//    auto repLoopCountTotal = repetitions_loop_controller.addLayer< LoopCounter >();
//    auto expLoopCountTotal = exploration_loop_controller.addLayer< LoopCounter >();
//    auto intLoopCountTotal = intensification_loop_controller.addLayer< LoopCounter >();
    //-----------------------------------------------------------------
    //                      Exploration Time
    //-----------------------------------------------------------------
    auto exploration_timer = exploration_loop_controller.addLayer< LoopTimer >();
    Track<double> track_exp_time("exp. time", *exploration_timer );
    update_exp_recorder->addTrack(track_exp_time);
    //-----------------------------------------------------------------
    //                      Stag. Counter
    //-----------------------------------------------------------------
//    auto accept_stag_cnt = accept.addLayer< tsp::AcceptStagCounter >();
//    Track<unsigned> track_stag_exp("stag. count", *accept_stag_cnt );
//    update_exp_recorder->addTrack(track_stag_exp);
//    exploration_loop_controller.core().resetObject( *accept_stag_cnt );

    //-----------------------------------------------------------------
    //                      Exp. Improvement Meter
    //-----------------------------------------------------------------
    auto improvement_meter = updateIntensification.addLayer< path::UpdateImprovementMeter >();
    auto resetter = updateExploration.addLayer< path::UpdateResetObject >();
    resetter->setObject(improvement_meter);
    Track<double> track_improv("Improvement", *improvement_meter );
    update_exp_recorder->addTrack(track_improv);

    //-----------------------------------------------------------------
    //              Stop intens. at 1MM objc. fcn. calls
    //-----------------------------------------------------------------
    auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, parameters("objfcncalls").as<unsigned>() );
    intensification_loop_controller.core().addStopCondition(int_obj_calls_stop, LoopController::ON_STOP::RESET );

    update_exp_recorder->start();

    Timer   totalTimer;
    totalTimer.start();

    auto rep_best_path = create();
    auto rep_cost = objective(rep_best_path);

    while( exploration_loop_controller() )
    {
        onion::reset_random_engine();
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

    totalTimer.stop();

    auto fname = parameters("file_name").as<string>();
    fname = fname.substr( fname.find_last_of("/") + 1 );

    stringstream st;
    st << create.core().getLabel() << "/" <<
          neighborhood.core().getLabel() << "/" <<
          accept.core().getLabel();

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto impStats   = TrackStats<double>( track_improv );

    cout << fname << "\t";
    cout << st.str() << "\t";
    cout << parameters("explorations").as<unsigned>() << "\t";
    cout << objective_calls_total->getValue() << "\t";
    cout << fixed << setprecision(2) << totalTimer.getValue() << "\t";
    cout << fixed << setprecision(4) << timeStats << "\t";
    cout << fixed << setprecision(0) << expStats << "\t";
    cout << fixed << setprecision(4) << impStats << "\t\n";


    return;
}


#endif // TSP_TEST_H
