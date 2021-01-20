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

#ifdef __DEBUG__
const char * const BUILDTYPE = "DEBUG";
#else
const char * const BUILDTYPE = "RELEASE";
#endif

void mkp_test(int argc, char* argv[])
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

    ///-----------------------------------------------------------------
    //                      STRATEGY
    //-----------------------------------------------------------------
    mkp::mkp_data_t data;
    //Onion< mkp::Creator > create  ( make_shared<mkp::CreateByItemOrder >( data ) );
    //Onion< mkp::Creator > create  ( make_shared<mkp::CreateByItemProfit>( data ) );
    Onion< mkp::Creator > create  ( make_shared<mkp::CreateRandom>( data ) );
    //Onion< mkp::Creator > create  ( make_shared<mkp::CreateByItemUtilityTW2CR>( data ) );
    //Onion< mkp::Creator > create  ( make_shared<mkp::CreateByItemUtilityWI2CR>( data ) );

    //Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::DEFAULT ) );
    //Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::PROFIT ) );
    //Onion< mkp::Neighbor > neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::RANDOM ) );
    //Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::P2TWR ) );
    Onion< mkp::Neighbor >      neighborhood ( make_shared<mkp::InsertRepair>( data, ProductRankStrategy::P2WIR ) );

    Onion< mkp::Objective >     objective( make_shared< mkp::mkpObjective >( data ) );

    Onion< mkp::Accept >        accept( make_shared< mkp::Accept1st >() );
    //Onion< mkp::Accept >        accept( make_shared< mkp::AcceptBest >() );

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
    data = cops::mkp::mkp_datDataLoader()(file);
    file.close();

    auto fname          = parameters("file_name").as<string>();
    const auto start    = fname.find_last_of("/") + 1;
    const auto end      = fname.find_last_of(".");
    fname               = fname.substr( start,  end-start );
    //-----------------------------------------------------------------
    //                      LOOP CONTROLLERS & UPDATES
    //-----------------------------------------------------------------

    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< mkp::Updater >  updateIntensification;
    Onion< mkp::Updater >  updateExploration;
    //-----------------------------------------------------------------
    //                      Recorders
    //-----------------------------------------------------------------
    auto update_exp_recorder    = updateExploration.addLayer< mkp::UpdateLocalRecorder >();
    //-----------------------------------------------------------------
    //                      Total Obj. Fcn Calls
    //-----------------------------------------------------------------
    auto objective_calls_total  = objective.addLayer< mkp::ObjectiveCallsCounter >();
    auto objective_calls_at_exp = objective.addLayer< mkp::ObjectiveCallsCounter >();
    //-----------------------------------------------------------------
    //                      Total Loop Counters
    //-----------------------------------------------------------------
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

    update_exp_recorder->start();

    Timer   totalTimer;
    totalTimer.start();
    auto best_oa = create();
    auto cost_oa = objective(best_oa);

    while( exploration_loop_controller() )
    {
        onion::reset_random_engine();
        auto exp_s = create();
        auto exp_c = objective( exp_s );

        while( intensification_loop_controller() )
        {
            auto neighbor = neighborhood(exp_s);
            auto newcost = objective( neighbor );
            auto accepted = accept( exp_c, newcost );
            if ( accepted )
                updateIntensification(
                            exp_s, exp_c,
                            neighbor.at(accepted.index()),
                            newcost.at(accepted.index())
                            );
        }
        updateExploration(best_oa,cost_oa,exp_s,exp_c);
    }

    totalTimer.stop();

//    auto fname = parameters("file_name").as<string>();
//    fname = fname.substr( fname.find_last_of("/") + 1 );


    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto impStats   = TrackStats<double>( track_improv );

    cout << fname << "\t";
    cout << strategy.str() << "\t";
    cout << parameters("explorations").as<unsigned>() << "\t";
    cout << objective_calls_total->getValue() << "\t";
    cout << fixed << setprecision(2) << totalTimer.getValue() << "\t";
    cout << fixed << setprecision(4) << timeStats << "\t";
    cout << fixed << setprecision(0) << expStats << "\t";
    cout << fixed << setprecision(4) << impStats << "\t\n";

#ifdef __DEBUG__
    assert( objective( best_oa ) == expStats.max() );
#endif

    return;
}
