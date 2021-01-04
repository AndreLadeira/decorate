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
#include "cops/bmkfcns.h"

using namespace std;
using namespace onion;
using namespace onion::cops;
using namespace bmf;

#ifdef __DEBUG__
const char * const BUILDTYPE = "DEBUG";
#else
const char * const BUILDTYPE = "RELEASE";
#endif

void bmf_full(int argc, char* argv[])
{
    stringstream        cmdline, strategy;
    ParameterList       parameters;

    //-----------------------------------------------------------------
    //                      PARAMETERS
    //-----------------------------------------------------------------

    for(int i = 1; i < argc; i++ ) cmdline << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    loadParameters(cmdline,parameters);
    //-----------------------------------------------------------------
    //                      PROBLEM DATA
    //-----------------------------------------------------------------

    bmf::Range range = {-500,500};
    const unsigned S = 30;

    //-----------------------------------------------------------------
    //                      LOOP CONTROLLERS & UPDATES
    //-----------------------------------------------------------------
    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< bmf::Updater<S> >  updateIntensification;
    Onion< bmf::Updater<S> >  updateExploration;
    //-----------------------------------------------------------------
    //                      STRATEGY
    //-----------------------------------------------------------------


    Onion< bmf::Creator<S> > create( make_shared< bmf::CreateRandom<S> >( range ) );

   // Onion< bmf::Neighbor<S> > neighborhood( make_shared< bmf::RandomNeighbor<S> >( range ) );
    Onion< bmf::Neighbor<S> > neighborhood( make_shared< bmf::RandomDecreasingRangeNeighbor<S> >(
                                                range,parameters("intensifications").as<unsigned>() ) ); // linear decay

//    Onion< bmf::Neighbor<S> > neighborhood( make_shared< bmf::RandomDecreasingRangeNeighbor<S> >(
//                                                range,parameters("intensifications").as<unsigned>() , bmf::exp_decay ) );// exp decay

//    Onion< bmf::Neighbor<S> > neighborhood( make_shared< bmf::RandomDecreasingRangeNeighbor<S> >(
//                                                range,parameters("intensifications").as<unsigned>() , bmf::cos_decay ) ); // exp decay

    intensification_loop_controller.core().resetObject( dynamic_cast<onion::AResettable&>( neighborhood.core() ) );


    Onion< bmf::Objective<S> > objective( make_shared< bmf::bmfObjective<S> >( bmf::f8 ) );

    //Onion< bmf::Accept > accept( make_shared< bmf::Accept1st >() );
    Onion< bmf::Accept > accept( make_shared< bmf::AcceptBest >() );

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
    //                      Recorders
    //-----------------------------------------------------------------
    auto update_exp_recorder        = updateExploration.addLayer< bmf::UpdateLocalRecorder<S> >();
    auto regularity                 = parameters("intensifications").as<unsigned>() / 10;
    auto intensification_loop_rec   = intensification_loop_controller.addLayer< LoopRecorder >( regularity );
    //-----------------------------------------------------------------
    //                      Total Obj. Fcn Calls
    //-----------------------------------------------------------------
    auto objective_calls_total  = objective.addLayer< bmf::ObjectiveCallsCounter<S> >();
    auto objective_calls_at_exp = objective.addLayer< bmf::ObjectiveCallsCounter<S> >();
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
    auto accept_stag_cnt = accept.addLayer< bmf::AcceptStagCounter >();
//    Track<unsigned> track_stag_exp("stag. count", *accept_stag_cnt );
//    update_exp_recorder->addTrack(track_stag_exp);
//    exploration_loop_controller.core().resetObject( *accept_stag_cnt );

    //-----------------------------------------------------------------
    //                      Exp. Improvement Meter
    //-----------------------------------------------------------------
    auto improvement_meter = updateIntensification.addLayer< bmf::UpdateImprovementMeter<S> >();
    auto resetter = updateExploration.addLayer< bmf::UpdateResetObject<S> >();
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
    //     2 - By Total number of objective fcn calls
    //
    //-----------------------------------------------------------------
    auto total_max_obj_fcn_calls = parameters("obj_fcn_calls").as<unsigned>();
    auto obj_calls_stop_at_int   = StopCondition<>("Objective fcn calls",
                                                 *objective_calls_at_exp,
                                                 total_max_obj_fcn_calls );
    intensification_loop_controller.core().addStopCondition(obj_calls_stop_at_int);
    //-----------------------------------------------------------------
    //     Stop EXPLORATIONS
    //     1 - By Total number of objective fcn calls
    //-----------------------------------------------------------------
    auto obj_calls_stop_at_exp = StopCondition<>("Objective fcn calls",
                                                 *objective_calls_at_exp,
                                                 total_max_obj_fcn_calls );
    exploration_loop_controller.core().addStopCondition(obj_calls_stop_at_exp);

    //-----------------------------------------------------------------
    //             best solution, best cost
    //-----------------------------------------------------------------
    auto best_s = create();
    auto best_c = objective(best_s);
    decltype (best_s) exp_s;
    decltype (best_c) exp_c;

    //-----------------------------------------------------------------
    //              Multi-Tracks
    //-----------------------------------------------------------------
    MultiTrack<double>      mtrack_exp_cost        ("exp. cost",        (onion::RefValue<double>(exp_c) ),  intensification_loop_controller.core() );
    MultiTrack<double>      mtrack_exp_time        ("exp. time",        *exploration_timer,                 intensification_loop_controller.core() );
    MultiTrack<unsigned>    mtrack_obj_fcn_calls   ("obj. fcn. calls",  *objective_calls_at_exp,            intensification_loop_controller.core() );
    intensification_loop_rec->addTrack(mtrack_exp_cost);
    intensification_loop_rec->addTrack(mtrack_exp_time);
    intensification_loop_rec->addTrack(mtrack_obj_fcn_calls);
    intensification_loop_controller.core().resetObject(*objective_calls_at_exp);
    //-----------------------------------------------------------------
    //              Start timer & recorders
    //-----------------------------------------------------------------
    Timer totalTimer;
    totalTimer.start();
    update_exp_recorder->start();
    intensification_loop_rec->start();

    while( exploration_loop_controller() )
    {
        exp_s = create();
        exp_c = objective(exp_s);

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
        updateExploration(best_s,best_c,exp_s,exp_c);
    }

    totalTimer.stop();

//    auto timeStats  = TrackStats<double>( track_exp_time );
//    auto expStats   = TrackStats<double>( update_exp_recorder.get()->getLocalTrack<double>() );
//    auto impStats   = TrackStats<double>( track_improv );

//    cout << strategy.str() << "\t";
//    cout << expLoopCountTotal->getValue()<< "/" << intLoopCountTotal->getValue() << "\t";
//    cout << objective_calls_total->getValue() << "\t";
//    cout << fixed << setprecision(2) << totalTimer.getValue() << "\t";
//    cout << fixed << setprecision(4) << timeStats << "\t";
//    cout << fixed << setprecision(2) << expStats << "\t";
//    cout << fixed << setprecision(4) << impStats << "\t\n";

    TrackPrinter printer;
    std::vector<unsigned> exps(mtrack_obj_fcn_calls.size());
    std::iota(exps.begin(),exps.end(),0);
    for(auto& v : exps ) v *= regularity;

    printer << TrackPrinter::track<unsigned>("exploration",exps) //<< mtrack_exp_time
            << mtrack_exp_cost ;//<< mtrack_obj_fcn_calls;
    printer.print(cout);

    cout<< endl;

    return;
}
