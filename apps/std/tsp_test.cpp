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

#ifdef __DEBUG__
const char * const BUILDTYPE = "DEBUG";
#else
const char * const BUILDTYPE = "RELEASE";
#endif

//unsigned getStagLimit(size_t data_sz, unsigned obj_fcn_calls_limit){
//      auto n = static_cast<unsigned>(data_sz - 2);
////    auto limit = n*(n-1)/2;

////    // average neighbors per loop (remove reinsert)
////    auto anpl = std::ceil( n /2.0 );

////    // upper limit on the number of loops
////    auto loops = std::floor(obj_fcn_calls_limit / anpl);

////    // limit to at least 15 restarts
////    auto looplimit = static_cast<unsigned> (loops/15.0);

////    if ( limit < looplimit )
////        return limit;
////    else
////        return looplimit;
//    return 1000 * 1000;
//}

void tsp_test(int argc, char* argv[])
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
    //         RANDON NUMBER GEN RESET USING MILLISECONDS SEED
    //-----------------------------------------------------------------
    onion::reset_random_engine();
    //-----------------------------------------------------------------
    //                      STRATEGY
    //-----------------------------------------------------------------
    tsp::problem_data_t data;

    Onion< path::Creator >   create( make_shared<path::CreateGreedy >( data ) );
    //Onion< path::Creator > create( make_shared<path::CreateRandom >( data ) );

    //Onion< path::Neighbor > neighborhood( make_shared<path::RemoveReinsert >() );
    Onion< path::Neighbor > neighborhood( make_shared< path::_2opt >() );

    Onion< path::Objective > objective( make_shared< path::tspObjective >( data ) );

    //Onion< tsp::Accept > accept( make_shared< tsp::Accept1st >() );
    Onion< tsp::Accept > accept( make_shared< tsp::AcceptBest >() );

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

    auto fname          = parameters("file_name").as<string>();
    const auto start    = fname.find_last_of("/") + 1;
    const auto end      = fname.find_last_of(".");
    fname               = fname.substr( start,  end-start );

    if ( data.size() > 1000){
        cout<< fname << "\t" << "skipped (size = " << data.size() << ")\n";
        return;
    }

    //-----------------------------------------------------------------
    //                      LOOP CONTROLLERS & UPDATES
    //-----------------------------------------------------------------
    unsigned exps = parameters("explorations").str() == "auto" ? 0 : parameters("explorations").as<unsigned>();
    unsigned ints = parameters("intensifications").str() == "auto" ? static_cast<unsigned>(data.size() * 100) : parameters("intensifications").as<unsigned>();

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( exps ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( ints ) );
    Onion< path::Updater >  updateIntensification;
    Onion< path::Updater >  updateExploration;

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
    //auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, parameters("objfcncalls").as<unsigned>() );
    //intensification_loop_controller.core().addStopCondition(int_obj_calls_stop, LoopController::ON_STOP::RESET );
    //-----------------------------------------------------------------
    //     Stop INTENSIFICATIONS
    //     1 - By stagnation count. Must be 1 for _2opt
    //     2 - By Total number of objective fcn calls
    //-----------------------------------------------------------------
    auto total_max_obj_fcn_calls = parameters("obj_fcn_calls").as<unsigned>();

    auto stag_limit =   parameters.contains("maxstag_at_int") == false ||  parameters("maxstag_at_int").str() == "auto" ?
                        0 : parameters("maxstag_at_int").as<unsigned>();

    auto obj_calls_stop_at_int   = StopCondition<>("Objective fcn calls",
                                                 *objective_calls_at_exp,
                                                 total_max_obj_fcn_calls );
    intensification_loop_controller.core().addStopCondition(obj_calls_stop_at_int);
    if ( stag_limit ) {
        auto stag_stop              = StopCondition<>("Stagnations", *accept_stag_cnt, stag_limit);
        intensification_loop_controller.core().addStopCondition(stag_stop,LoopController::ON_STOP::RESET);
    }
    //-----------------------------------------------------------------
    //     Stop EXPLORATIONS
    //     1 - By Total number of objective fcn calls
    //-----------------------------------------------------------------
    auto obj_calls_stop_at_exp = StopCondition<>("Objective fcn calls",
                                                 *objective_calls_at_exp,
                                                 total_max_obj_fcn_calls );
    exploration_loop_controller.core().addStopCondition(obj_calls_stop_at_exp);
    //-----------------------------------------------------------------
    //              Start timer & recorders
    //-----------------------------------------------------------------
    Timer totalTimer;
    totalTimer.start();
    update_exp_recorder->start();
    //-----------------------------------------------------------------
    //             best path, best cost
    //-----------------------------------------------------------------
    auto best_path = create();
    auto best_cost = objective(best_path);

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
        updateExploration(best_path,best_cost,exp_best_path,exp_cost);
    }

    totalTimer.stop();

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto impStats   = TrackStats<double>( track_improv );

    cout << fname << "\t";
    cout << data.size() << "\t";
    cout << strategy.str() << "\t";
    cout << expLoopCountTotal->getValue()<< "/" << intLoopCountTotal->getValue() << "\t";
    cout << stag_limit << "\t";
    cout << objective_calls_total->getValue() << "\t";
    cout << fixed << setprecision(2) << totalTimer.getValue() << "\t";
    cout << fixed << setprecision(4) << timeStats << "\t";
    cout << fixed << setprecision(0) << expStats << "\t";
    cout << fixed << setprecision(4) << impStats << "\n";

    return;
}
