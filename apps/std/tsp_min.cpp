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
using tsp::path::operator<<;
using namespace tsp;

#ifdef __DEBUG__
const char * const BUILDTYPE = "DEBUG";
#else
const char * const BUILDTYPE = "RELEASE";
#endif

void tsp_min(int argc, char* argv[])
{
    stringstream        cmdline, strategy;
    ParameterList       parameters;

    //-----------------------------------------------------------------
    //                      PARAMETERS
    //-----------------------------------------------------------------

    for(int i = 1; i < argc; i++ ) cmdline << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    //loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(cmdline,parameters);

    //-----------------------------------------------------------------
    //                      STRATEGY
    //-----------------------------------------------------------------
    tsp::problem_data_t data;

    Onion< path::Creator >   create( make_shared<path::CreateGreedy >( data ) );
    //Onion< path::Creator > create( make_shared<path::CreateRandom >( data.size() ) );

    Onion< path::Neighbor > neighborhood( make_shared<path::RemoveReinsert >() );
    //Onion< path::Neighbor > neighborhood( make_shared< path::_2opt >() );

    Onion< path::Objective > objective( make_shared< path::tspObjective >( data ) );

    Onion< tsp::Accept > accept( make_shared< tsp::Accept1st >() );
    //Onion< tsp::Accept > accept( make_shared< tsp::AcceptBest >() );

    //-----------------------------------------------------------------
    //                     STRATEGY OUTPUT
    //-----------------------------------------------------------------
    strategy << create.core().getLabel() << "/"
             << neighborhood.core().getLabel() << "/"
             << accept.core().getLabel();

    if ( parameters.contains("strategy") ){
        std::cout<< "[I] - [TSP STRATEGY (" << BUILDTYPE << ")]: " << strategy.str() << endl;
        return;
    }
    //-----------------------------------------------------------------
    //                      DATA FILE
    //-----------------------------------------------------------------
    ifstream file( parameters("file_name") );
    file.exceptions( istream::failbit | istream::badbit  );
    data = cops::tsp::tsplibDataLoader()(file);
    file.close();

    //-----------------------------------------------------------------
    //                      LOOP CONTROLLERS & UPDATES
    //-----------------------------------------------------------------
    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>( parameters("repetitions").as<unsigned>() ) );

    Onion< path::Updater > updateIntensification;
    Onion< path::Updater > updateExploration;
    Onion< path::Updater > updateRun;

    //-----------------------------------------------------------------
    //                      Recorders
    //-----------------------------------------------------------------
    auto update_run_recorder    = updateRun.addLayer< path::UpdateLocalRecorder >();
    auto update_exp_recorder    = updateExploration.addLayer< path::UpdateLocalRecorder >();
    //-----------------------------------------------------------------
    //                      Total Obj. Fcn Calls
    //-----------------------------------------------------------------
    auto objective_calls_total  = objective.addLayer< path::ObjectiveCallsCounter >();
    auto objective_calls_at_exp = objective.addLayer< path::ObjectiveCallsCounter >();
    //-----------------------------------------------------------------
    //                      Total Loop Counters
    //-----------------------------------------------------------------
    auto repLoopCountTotal = repetitions_loop_controller.addLayer< LoopCounter >();
    auto expLoopCountTotal = exploration_loop_controller.addLayer< LoopCounter >();
    auto intLoopCountTotal = intensification_loop_controller.addLayer< LoopCounter >();
    //-----------------------------------------------------------------
    //                      Exploration Time
    //-----------------------------------------------------------------
    auto exploration_timer = exploration_loop_controller.addLayer< LoopTimer >();
    Track<double> track_exp_time("exp. time", *exploration_timer );
    update_exp_recorder->addTrack(track_exp_time);
    //-----------------------------------------------------------------
    //                      Stag. Counter
    //-----------------------------------------------------------------
    auto accept_stag_cnt = accept.addLayer< tsp::AcceptStagCounter >();
    //Track<unsigned> track_stag_exp("stag. count", *accept_stag_cnt );
    //update_exp_recorder->addTrack(track_stag_exp);
    //exploration_loop_controller.core().resetObject( *accept_stag_cnt );

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
    //auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, 1e06);
    //intensification_loop_controller.core().addStopCondition(int_obj_calls_stop, LoopController::ON_STOP::RESET );
    //-----------------------------------------------------------------
    //     Stop intens. at 1 stagnation when _2opt neighborhood is used
    //-----------------------------------------------------------------
    auto stag_stop = StopCondition<>("Stagnations", *accept_stag_cnt, 500);
    intensification_loop_controller.core().addStopCondition(stag_stop,LoopController::ON_STOP::RESET);
    //-----------------------------------------------------------------
    //              Stop EXPS. at 1MM * reps objc. fcn. calls
    //-----------------------------------------------------------------
    auto total_max_obj_fcn_calls = parameters("obj_fcn_calls").as<unsigned>();
    auto obj_calls_stop_at_exp = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, total_max_obj_fcn_calls );
    exploration_loop_controller.core().addStopCondition(obj_calls_stop_at_exp);
    //-----------------------------------------------------------------
    //              Stop INTS. at 1MM * reps objc. fcn. calls
    //-----------------------------------------------------------------
    auto obj_calls_stop_at_int = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, total_max_obj_fcn_calls );
    intensification_loop_controller.core().addStopCondition(obj_calls_stop_at_int);

    update_exp_recorder->start();
    update_run_recorder->start();

    Timer               totalTimer;
    tsp::path_t         best_overall_path = create();
    unsigned            best_overall_cost = objective(best_overall_path);

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
        updateRun(best_overall_path,best_overall_cost,rep_best_path,rep_cost);
    }

    cout<< endl;
    cout<< "File name (runs/exps./intens.)            : " << parameters("file_name").as<string>() << " (" << parameters("repetitions").as<unsigned>() << "/"
                                                          << parameters("explorations").as<unsigned>() << "/" << parameters("intensifications").as<unsigned>() << ")\n";
    cout << strategy.str() << "\n";

    cout<< "Stop Condition (run/exp./inten.)          : " << repetitions_loop_controller.core().getStopCondition() << " / "
                                                          << exploration_loop_controller.core().getStopCondition() << " / "
                                                          << intensification_loop_controller.core().getStopCondition() << endl;
    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << " (s)\n";
    cout<< "Obj. fcn. calls                           : " << objective_calls_total->getValue()  << "\n";
    cout<< "Total Runs/Explorations/Intensifications  : " << repLoopCountTotal->getValue() << " / " << expLoopCountTotal->getValue()<< " / " << intLoopCountTotal->getValue() << endl;

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto repStats   = TrackStats<unsigned>( update_run_recorder.get()->getLocalTrack<unsigned>() );
    //auto stgStats   = TrackStats<unsigned>( track_stag_exp );
    auto impStats   = TrackStats<double>( track_improv );

    cout<< "Run time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << expStats.min() << " / " << expStats.max() << " / " << expStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;
    cout<< "Improvement  (all exps., min/max/avg)     : " << fixed << setprecision(1) << impStats.min() * 100.0 << " / " << impStats.max() * 100.0 << " / " << impStats.average() * 100.0 << " (%)\n";
    //cout<< "Stagnation   (only reps., min/max/avg)    : " << setprecision(0) << stgStats.min() << " / " << stgStats.max() << " / " << stgStats.average() << endl;

    return;
}
