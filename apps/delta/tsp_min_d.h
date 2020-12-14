//#ifndef TSP_MIN_D_H
//#define TSP_MIN_D_H

//#include <iostream>
//#include <fstream>
//#include <thread>
//#include <chrono>
//#include <iomanip> // setprecision
//#include <cassert>
//#include <clocale>

//#include "lib/onionmh.h"
//#include "mh/rrga.h"
//#include "lib/stddecorators.h"
//#include "lib/trackutil.h"
//#include "cops/tsp.h"

//using namespace std;
//using namespace onion;
//using namespace onion::cops;
//using tsp::path::operator<<;
//using namespace tsp;

//int tsp_min_d(int argc, char* argv[])
//{
//    stringstream        ss;
//    Timer               totalTimer;
//    ParameterList       parameters;
//    unsigned            best_overall_cost;
//    tsp::path_t         best_overall_path;

//    // parameters

//    for(int i = 1; i < argc; i++ ) ss << argv[i] << " ";

//    Onion<ParameterLoader> loadParameters;
//    loadParameters.addLayer<mh::RRGAParametersChecker>();
//    loadParameters(ss,parameters);

//    // data file

//    ifstream file( parameters("file_name") );
//    file.exceptions( istream::failbit | istream::badbit  );
//    auto data = cops::tsp::tsp_tsplibDataLoader()(file);
//    file.close();

//    // Loop Controller Onions

//    Onion< LoopController > exploration_loop_controller     ( make_shared<LoopController>("Exploration Loop",       parameters("explorations").as<unsigned>() ) );
//    Onion< LoopController > intensification_loop_controller ( make_shared<LoopController>("Intensification Loop",   parameters("intensifications").as<unsigned>() ) );
//    Onion< LoopController > repetitions_loop_controller     ( make_shared<LoopController>("Repetitions Loop",       parameters("repetitions").as<unsigned>() ) );

//    // algorithm onions

//    Onion< path::Creator >                create( make_shared<path::CreateRandom>( data.size() ) );

//    Onion< path::delta::Neighbor >        neighborhood    ( make_shared<path::delta::MaskReinsert>() );
//    //Onion< path::Neighbor >             neighborhood    ( make_shared< path::_2opt>() );

//    //auto s = create();
//    //auto n = neighborhood(s);

//    Onion< path::delta::Objective >     objective( make_shared< path::delta::tspObjective >( data ) );
//    Onion< tsp::delta::AcceptBest >     accept;
//    Onion< path::delta::Updater >       updateIntensification;
//    Onion< path::delta::Updater >       updateExploration;
//    Onion< path::delta::Updater >       updateRun;

//    // layers

////    auto objective_calls_total  = objective.addLayer< path::ObjectiveCallsCounter >();
////    auto update_run_recorder    = updateRun.addLayer< path::UpdateLocalRecorder >();
////    auto update_exp_recorder    = updateExploration.addLayer< path::UpdateLocalRecorder >();
////    auto exploration_timer      = exploration_loop_controller.addLayer< LoopTimer >();

////    Track<double> track_exp_time("exp. time", *exploration_timer );
////    update_exp_recorder->addTrack(track_exp_time);

////    update_exp_recorder->start();
////    update_run_recorder->start();

//    best_overall_path = create();
//    //best_overall_cost = objective(best_overall_path);
//    totalTimer.start();

//    while( repetitions_loop_controller() )
//    {
//        onion::reset_random_engine();

//        auto rep_best_path = create();
//        //auto rep_cost = objective(rep_best_path);

//        while( exploration_loop_controller() )
//        {
//            auto exp_best_path = create();
//            //auto exp_cost = objective( exp_best_path );

//            while( intensification_loop_controller() )
//            {
//                auto transfs    = neighborhood(exp_best_path);
//                auto deltas     = objective(exp_best_path,transfs);
//                auto accepted   = accept(deltas);

//                if ( accepted ){

//                   //transform(exp_best_path,transfs.at( accepted.index() ) );


//                }
//            }
//            //updateExploration(rep_best_path,rep_cost,exp_best_path,exp_cost);
//        }
//        //updateRun(best_overall_path,best_overall_cost,rep_best_path,rep_cost);
//    }

//    cout<< endl;
//    cout<< "File name (runs/exps./intens.)            : " << parameters("file_name").as<string>() << " (" << parameters("repetitions").as<unsigned>() << "/"
//                                                          << parameters("explorations").as<unsigned>() << "/" << parameters("intensifications").as<unsigned>() << ")\n";
//    cout<< "Stop Condition (run/exp./inten.)          : " << repetitions_loop_controller.core().getStopCondition() << " / "
//                                                          << exploration_loop_controller.core().getStopCondition() << " / "
//                                                          << intensification_loop_controller.core().getStopCondition() << endl;
//    cout<< "Total time                                : " << fixed << setprecision(2) << totalTimer.getValue() << " (s)\n";
//    cout<< "Avg. time per run                         : " << fixed << setprecision(2) << totalTimer.getValue() / parameters("repetitions").as<unsigned>() << " (s)\n";
//    cout<< "Obj. fcn. calls                           : " << objective_calls_total->getValue()  << "\n";

//    auto timeStats  = TrackStats<double>( track_exp_time );
//    auto expStats   = TrackStats<unsigned>( update_exp_recorder.get()->getLocalTrack<unsigned>() );
//    auto repStats   = TrackStats<unsigned>( update_run_recorder.get()->getLocalTrack<unsigned>() );

//    cout<< "Run time (min/max/avg)                    : " << fixed << setprecision(4) << timeStats.min() << " / " << timeStats.max() << " / " << timeStats.average() << " (s)\n";
//    cout<< "Final result (all exps., min/max/avg)     : " << setprecision(0) << expStats.min() << " / " << expStats.max() << " / " << expStats.average() << endl;
//    cout<< "Final result (only reps., min/max/avg)    : " << setprecision(0) << repStats.min() << " / " << repStats.max() << " / " << repStats.average() << endl;

//    return 0;
//}


//#endif // TSP_MIN_D_H
