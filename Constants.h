//
//  Constants.h
//  VCFComparison
//
//  Created by Berke.Toptas on 2/1/17.
//  Copyright © 2017 Seven Bridges Genomics. All rights reserved.
//

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <string>

//MAX UNIQUE CHROMOSOME COUNT IN GENOME SEQUENCE (CHR1:CHR22, CHRX, CHRY and CHRMT)
//const int CHROMOSOME_COUNT = 25;

//MAX NUMBER OF THREAD COUNT
const int MAX_THREAD_COUNT = 25;

//LEAST NUMBER OF VARIANT REQUIRED TO PROCESS THE CHROMOSOME
const int LEAST_VARIANT_THRESHOLD = 2;

//THREAD COUNT BY DEFAULT IN CASE PROGRAM WONT WORK IN PLATFORM MODE
const int DEFAULT_THREAD_COUNT = 2;

//DEFAULT BASE PAIR LENGTH OF A VARIANT THAT COMPARISON ENGINE PROCESSS
const int DEFAULT_MAX_BP_LENGTH = 1000;

//DEFAULT SIZE OF PATH SET (For Dynamic Programming result saving. This variable should be increased carefully since it is easy to exceed available memory)
const int DEFAULT_MAX_PATH_SIZE = 150000;

//DEFAULT SIZE OF ITERATION COUNT (For Dynamic Programming result saving. This variable should be increased carefully since it is easy to exceed available memory)
const int DEFAULT_MAX_ITERATION_SIZE = 10000000;

//DEFAULT SIZE OF SMALL VARIANTS FOR MENDELIAN VIOLATION DETECTION
const int SMALL_VARIANT_SIZE = 5;

//DEFAULT SIZE OF MEDIUM VARIANTS FOR MENDELIAN VIOLATION DETECTION
const int MEDIUM_VARIANT_SIZE = 15;

//MAXIMUM ITERATION NUMBER FOR UNIQUE EDGE DETECTION IN GRAPH COMPARISON
const int GRAPH_REPLAY_MAX_ITERATION = 100;

//VBT VERSION AND YEAR TO BE COPIED OUTPUT VCFS
const std::string VBT_VERSION = "v0.7.1 (2017)";

#endif //_CONSTANTS_H_
