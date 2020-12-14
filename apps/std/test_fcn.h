#ifndef TEST_FCN_H
#define TEST_FCN_H

#include "../../lib/onionmh.h"
#include <fstream>
#include <iostream>
using namespace std;

template<
        typename problem_data_t,
        typename solution_t,
        typename cost_t,
        typename onion::Compare<cost_t>::compare_fcn_t compare
        >
void test_fcn(
    ostream&                                            _os,
    vector<string>                                      _files,
    onion::DataLoader<problem_data_t>                   _data_loader,
    vector< onion::Creator<solution_t> >                _create,
    vector< onion::Neighbor<solution_t> >               _neighbor,
    vector< onion::Accept<cost_t,compare> >             _accept,
    onion::Objective<solution_t,problem_data_t,cost_t>  _objective,
    onion::LoopController                               _loops[3],
    onion::Updater<solution_t,cost_t,compare>           _updates[3]
    )
{
    ifstream fs;
    fs.exceptions( istream::failbit | istream::badbit  );

    for(const auto& fname : _files){

        _os << fname << "\t";

        fs.open(fname);
        if (!fs.is_open()) { _os<< "\n"; continue;}

        auto data = _data_loader(fs);
        fs.close();

        for(auto& _c : _create)
        for(auto& _n : _neighbor)
        for(auto& _a : _accept){

            solution_t best_overall = _c();
            cost_t best_cost = _objective(best_overall);

            while( _loops[0]() )
            {
                onion::reset_random_engine();

                auto rep_best = _c();
                auto rep_cost = objective(rep_best);

                while( _loops[1]()){

                    auto exp_best = _c();
                    auto exp_cost = objective( exp_best );

                    while( _loops[2]() ){

                        auto neighbor   = _n(exp_best);
                        auto newcost    = _objective( neighbor );
                        auto accepted   = _a( exp_cost, newcost );
                        if ( accepted )
                            _updates[2](exp_best, exp_cost,
                                        neighbor.at(accepted.index()),newcost.at(accepted.index()));
                    }
                    _updates[1](rep_best,rep_cost,exp_best,exp_cost);
                }
                _updates[0](best_overall,best_cost,rep_best,rep_cost);
            }
            // output
        }
    }
}

/*
#include "test_fcn.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip> // setprecision
#include <cassert>
#include <dirent.h>

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

void tsp_test(int argc, char* argv[])
{
    stringstream        ss;
    ParameterList       parameters;

    // parameters

    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

    Onion<ParameterLoader> loadParameters;
    loadParameters.addLayer<mh::RRGAParametersChecker>();
    loadParameters(ss,parameters);

    // get data files

    ifstream fs( parameters("file_name") );
    fs.exceptions( istream::failbit | istream::badbit  );
    vector<string> data_files;

    while(!fs.eof()){
        string str;
        std::getline(fs,str);
        data_files.push_back( str );
    }
    fs.close();

    // onions

    //-----------------------------------------------------------------
    //                      Loop Controllers
    //----------------------------------------------------------------
    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>( parameters("explorations").as<unsigned>() ) );
    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>( parameters("intensifications").as<unsigned>() ) );
    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>( parameters("repetitions").as<unsigned>() ) );
    //-----------------------------------------------------------------
    //                      Algorithm (create, neighbor etc)
    //-----------------------------------------------------------------

    tsp::problem_data_t data;

    Onion< path::Creator >              createGreedy( make_shared<path::CreateGreedy>( data ) );
    Onion< path::Creator >              createReandom( make_shared<path::CreateRandom>( data.size() ) );
    Onion< path::Neighbor >             neighborRR( make_shared<path::RemoveReinsert>(3) );
    Onion< path::Neighbor >             neighbor2opt( make_shared< path::_2opt>() );
    Onion< path::Objective >            objective   ( make_shared< path::tspObjective >( data ) );
    Onion< tsp::Accept >                accept1st   ( make_shared< tsp::Accept1st >() );
    Onion< tsp::Accept >                acceptBst   ( make_shared< tsp::Accept1st >() );

    Onion< path::Updater >              updateIntensification;
    Onion< path::Updater >              updateExploration;
    Onion< path::Updater >              updateRun;

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
    //                      Exp. Improvement Meter
    //-----------------------------------------------------------------
    auto improvement_meter = updateIntensification.addLayer< path::UpdateImprovementMeter >();
    exploration_loop_controller.core().resetObject(*improvement_meter);
    Track<double> track_improv("Improvement", *improvement_meter );
    update_exp_recorder->addTrack(track_improv);
    //-----------------------------------------------------------------
    //          Stop intensifications at 1MM obj. fcn. calls
    //-----------------------------------------------------------------
    auto int_obj_calls_stop = StopCondition<>("Objective fcn calls", *objective_calls_at_exp, 1e06);
    intensification_loop_controller.core().addStopCondition(int_obj_calls_stop, LoopController::ON_STOP::RESET );

    update_exp_recorder->start();
    update_run_recorder->start();

    // get output file name based on the onions components

    // run test





    best_overall_path = create();
    best_overall_cost = objective(best_overall_path);

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

    cout<< "Stop Condition (run/exp./inten.)          : " << repetitions_loop_controller.core().getStopCondition() << " / "
                                                          << exploration_loop_controller.core().getStopCondition() << " / "
                                                          << intensification_loop_controller.core().getStopCondition() << endl;
    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << " (s)\n";
    cout<< "Obj. fcn. calls                           : " << objective_calls_total->getValue()  << "\n";
    //cout<< "Total Runs/Explorations/Intensifications  : " << repLoopCountTotal->getValue() << " / " << expLoopCountTotal->getValue()<< " / " << intLoopCountTotal->getValue() << endl;

    auto timeStats  = TrackStats<double>( track_exp_time );
    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
    auto repStats   = TrackStats<unsigned>( update_run_recorder.get()->getLocalTrack<unsigned>() );
    auto stgStats   = TrackStats<unsigned>( track_stag_exp );
    auto impStats   = TrackStats<double>( track_improv );

    cout<< "Run time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << expStats.min() << " / " << expStats.max() << " / " << expStats.average() << endl;
    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;
    cout<< "Improvement  (all exps., min/max/avg)     : " << fixed << setprecision(1) << impStats.min() * 100.0 << " / " << impStats.max() * 100.0 << " / " << impStats.average() * 100.0 << " (%)\n";
    cout<< "Stagnation   (only reps., min/max/avg)    : " << setprecision(0) << stgStats.min() << " / " << stgStats.max() << " / " << stgStats.average() << endl;

    return;
}
*/

#endif // TEST_FCN_H
