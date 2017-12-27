/*
 *
 * Copyright 2017 Seven Bridges Genomics Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  CVcfAnalyzer.h
 *  VariantBenchmarkingTools
 *
 *  Created by Berke Cagkan Toptas on 12/8/16.
 *
 */

#ifndef C_VCF_ANALYZER_H
#define C_VCF_ANALYZER_H

#include <thread>
#include "CPathReplay.h"
#include "SConfig.h"
#include "CVariantProvider.h"
#include "CResultLog.h"
#include <mutex>

namespace duocomparison
{

/**
 * @brief Duo Comparison Tool Capability Class
 *
 * CVcfAnalyzer object is a tool itself that performs variant comparison operation. Its output is very close to vcfeval.
 * The differences:
 * 1. Variants with 0/0 genotype is eliminated from output while vcfeval keeps them
 * 2. Cutoff limits during variant replay is configurable with VBT while it is fixed in vcfeval
 * 3. In -ref-overlap mode, VBT does a clever trimming strategy that can find more TPs in repetitive regions rather than naive approach of vcfeval
 */
class CVcfAnalyzer
{

public:
    
    ///Run the application
    void Run(int argc, char** argv);
    
    
private:

    //Read Parameters from command line. If the mandatory arguments are given, return true.
    bool ReadParameters(int argc, char** argv);
    
    //Prints the help menu at console
    void PrintHelp() const;
    
    //Divide the jobs between different threads homogeneously for given number of thread count. Return the actual thread count
    int AssignJobsToThreads(int a_nThreadCount);
        
    //Function that process chromosome in bulk for SPLIT mode (process either genotype or allele match)
    void ThreadFunctionSPLIT(std::vector<SChrIdTuple> a_aTuples, bool a_bIsGenotypeMatch);

    //Function that process chromosome in bulk for GA4GH mode (process both genotype and allele matches)
    void ThreadFunctionGA4GH(std::vector<SChrIdTuple> a_aTuples);

    //Calculate the syncronization point list for the given tuple
    void CalculateSyncPointList(const SChrIdTuple& a_rTuple, std::vector<core::CSyncPoint>& a_rSyncPointList);
    
    //[TEST PURPOSE] Print given variants to an external file
    void PrintVariants(std::string a_outputDirectory, std::string a_FileName, const std::vector<const core::COrientedVariant*>& a_rOvarList) const;
    void PrintVariants(std::string a_outputDirectory, std::string a_FileName, const std::vector<const CVariant*>& a_rVarList) const;
    
    
    //Configurations for Vcf comparison
    SConfig m_config;
    
    //Object to write variant statistic into a file
    CResultLog m_resultLogger;
    
    //Variant provider instance
    CVariantProvider m_provider;
    
    //Best Paths written by each thread for each unique chromosome exists
    std::vector<core::CPath> m_aBestPaths;
    
    //Best Paths written by each thread to find Allele matches for each unique chromosome exists
    std::vector<core::CPath> m_aBestPathsAllele;
    
    //Thread pool we have for multitasking by per chromosome
    std::thread *m_pThreadPool;
    
    //To prevent data race in multi-thread mode
    std::mutex mtx;

};
    
}

#endif //C_VCF_ANALYZER_H
