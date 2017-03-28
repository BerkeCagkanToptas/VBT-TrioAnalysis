//  CVcfAnalyzer.cpp
//  VCFComparison
//
//  Created by Berke.Toptas on 12/8/16.
//  Copyright © 2016 Seven Bridges Genomics. All rights reserved.
//

#include "CVcfAnalyzer.h"
#include "iostream"
#include <math.h>
#include <algorithm>
#include "CGa4ghOutputProvider.h"
#include <fstream>



void CVcfAnalyzer::Run(int argc, char** argv)
{
    
    std::time_t start;
    double duration;
    
    //Read command line parameters to m_config object
    bool isSuccess = ReadParameters(argc, argv);
    
    if(!isSuccess)
        return;
    
    start = std::time(0);
    
    //Initialize Variant providers which contains VCF and FASTA files
    isSuccess = m_provider.InitializeReaders(m_config);
    
    duration = std::difftime(std::time(0) ,start);
    std::cout << "Vcf and fasta Parser read completed in " << duration << " secs" << std::endl;

    if(!isSuccess)
        return;
    
    std::time_t start1 = std::time(0);
    //Creates the threads according to given memory and process the data
    if(m_config.m_bIsPlatformMode)
        SetThreadsCustom(48 * 1024);
    else
        SetThreadsCustom(8 * 1024);
    duration = std::difftime(std::time(0), start1);
    std::cout << "Processing Chromosomes completed in " << duration << " secs" << std::endl;
    duration = std::difftime(std::time(0), start);
    std::cout << "Total execution time is " << duration << " secs" << std::endl;
}

void CVcfAnalyzer::SetThreadsPlatform()
{
    std::vector<int> chromosomeIds;
 
    std::thread threadPool[CHROMOSOME_COUNT];
    
    m_provider.GetUniqueChromosomeIds(chromosomeIds);

    for(int k = 0; k < chromosomeIds.size(); k++)
    {
        threadPool[k] = std::thread(&CVcfAnalyzer::ThreadFunc, this, chromosomeIds[k]);
    }
    
    for(int k = 0; k < chromosomeIds.size(); k++)
    {
        threadPool[k].join();
    }
}

void CVcfAnalyzer::SetThreadsCustom(int a_nMemoryInMB)
{
    std::vector<int> chromosomeIds;
    m_provider.GetUniqueChromosomeIds(chromosomeIds);
    
    //IF WE HAVE ENOUGH MEMORY RUN ALL THREADS IN PARALLEL
    if(a_nMemoryInMB > 32 * 1024)
    {
        SetThreadsPlatform();
    }

    //IF WE HAVE LESS MEMORY USE SINGLE THREADING
    else if(a_nMemoryInMB < 4096)
    {
        for(int k = 0; k < chromosomeIds.size(); k++)
        {
            ThreadFunc(chromosomeIds[k]);
        }
    }

    //ELSE CREATE 4 GB MEMORY (ON AVG) FOR EACH THREAD
    else
    {
        const int maxThreadCount = std::min(static_cast<int>(chromosomeIds.size()), std::min(CHROMOSOME_COUNT, static_cast<int>(floor(a_nMemoryInMB / 4096))));
        std::vector<int>* chrArrs = new std::vector<int>[maxThreadCount];

        for(int k = 0, p = 0; k < chromosomeIds.size(); k++)
        {
            chrArrs[p].push_back(chromosomeIds[k]);
            p++;
            p = p % maxThreadCount;
        }
    
        std::thread *threadPool = new std::thread[maxThreadCount];
    
        for(int k = 0; k < maxThreadCount; k++)
        {
            threadPool[k] = std::thread(&CVcfAnalyzer::ThreadFunc2, this, chrArrs[k]);
        }
        
        for(int k = 0; k < maxThreadCount; k++)
        {
            threadPool[k].join();
        }
    }
    
    CGa4ghOutputProvider outputprovider;
    outputprovider.SetVcfPath(m_config.m_pOutputDirectory);
    outputprovider.SetVariantProvider(&m_provider);
    outputprovider.SetBestPaths(m_aBestPaths, m_aBestPathsAllele);
    outputprovider.GenerateGa4ghVcf();
    
    m_resultLogger.SetLogPath(m_config.m_pOutputDirectory);
    m_resultLogger.WriteStatistics();
    
}

void CVcfAnalyzer::ThreadFunc(int a_nChromosomeId)
{
    std::vector<const CVariant*> varListBase = m_provider.GetVariantList(eBASE, a_nChromosomeId);
    std::vector<const CVariant*> varListCalled = m_provider.GetVariantList(eCALLED, a_nChromosomeId);
    std::vector<const COrientedVariant*> ovarListBase = m_provider.GetOrientedVariantList(eBASE, a_nChromosomeId, true);
    std::vector<const COrientedVariant*> ovarListCalled = m_provider.GetOrientedVariantList(eCALLED, a_nChromosomeId, true);

    CPathReplay pathReplay(varListBase, varListCalled, ovarListBase, ovarListCalled);
    SContig ctg;
    m_provider.GetContig(a_nChromosomeId, ctg);
    
    //Find Best Path [GENOTYPE MATCH]
    m_aBestPaths[a_nChromosomeId] = pathReplay.FindBestPath(ctg, true);
    
    //Genotype Match variants
    const std::vector<const COrientedVariant*>& includedVarsBase = m_aBestPaths[a_nChromosomeId].m_baseSemiPath.GetIncludedVariants();
    const std::vector<const COrientedVariant*>& includedVarsCall = m_aBestPaths[a_nChromosomeId].m_calledSemiPath.GetIncludedVariants();
    
    //Variants that will be passed for allele match check
    std::vector<const CVariant*> excludedVarsBase = m_provider.GetVariantList(eBASE, a_nChromosomeId, m_aBestPaths[a_nChromosomeId].m_baseSemiPath.GetExcluded());
    std::vector<const CVariant*> excludedVarsCall = m_provider.GetVariantList(eCALLED, a_nChromosomeId,m_aBestPaths[a_nChromosomeId].m_calledSemiPath.GetExcluded());

    //Fill oriented variants for allele match
    m_provider.FillAlleleMatchVariantList(a_nChromosomeId, excludedVarsBase, excludedVarsCall);
    
    //Clear old variant pointers
    ovarListBase.clear();
    ovarListCalled.clear();
    varListBase.clear();
    varListCalled.clear();
    
    //Set new variant pointers for allele match comparison
    varListBase = excludedVarsBase;
    varListCalled = excludedVarsCall;
    ovarListBase = m_provider.GetOrientedVariantList(eBASE, a_nChromosomeId, false);
    ovarListCalled = m_provider.GetOrientedVariantList(eCALLED, a_nChromosomeId, false);
    pathReplay.Clear();
    
    //Find Best Path [ALLELE MATCH]
    m_aBestPathsAllele[a_nChromosomeId] = pathReplay.FindBestPath(ctg, false);
    
    //No Match variants
    std::vector<const CVariant*> excludedVarsBase2 = m_provider.GetVariantList(excludedVarsBase,
                                                                               m_aBestPathsAllele[a_nChromosomeId].m_baseSemiPath.GetExcluded());
    std::vector<const CVariant*> excludedVarsCall2 = m_provider.GetVariantList(excludedVarsCall,
                                                                               m_aBestPathsAllele[a_nChromosomeId].m_calledSemiPath.GetExcluded());
    
    //Allele Match variants
    const std::vector<const COrientedVariant*>& includedVarsBase2 = m_aBestPathsAllele[a_nChromosomeId].m_baseSemiPath.GetIncludedVariants();
    const std::vector<const COrientedVariant*>& includedVarsCall2 = m_aBestPathsAllele[a_nChromosomeId].m_calledSemiPath.GetIncludedVariants();

    
    m_resultLogger.LogStatistic(a_nChromosomeId,
                                static_cast<int>(includedVarsCall.size()),
                                static_cast<int>(includedVarsBase.size()),
                                static_cast<int>(includedVarsCall2.size()),
                                static_cast<int>(includedVarsBase2.size()),
                                static_cast<int>(excludedVarsCall2.size()),
                                static_cast<int>(excludedVarsBase2.size()));
    
    m_provider.SetVariantStatus(excludedVarsBase2, eNO_MATCH);
    m_provider.SetVariantStatus(excludedVarsCall2, eNO_MATCH);
    m_provider.SetVariantStatus(includedVarsCall,  eGENOTYPE_MATCH);
    m_provider.SetVariantStatus(includedVarsBase,  eGENOTYPE_MATCH);
    m_provider.SetVariantStatus(includedVarsCall2, eALLELE_MATCH);
    m_provider.SetVariantStatus(includedVarsBase2, eALLELE_MATCH);
    
    //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("FP_") + std::to_string(a_nChromosomeId +1) + std::string(".txt")  , excludedVarsCall);
    //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("TP_BASE_") + std::to_string(a_nChromosomeId + 1) + std::string(".txt")  , includedVarsBase);
    //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("TP_CALLED_") + std::to_string(a_nChromosomeId + 1) + std::string(".txt") , includedVarsCall);
    //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("FN_") + std::to_string(a_nChromosomeId + 1) + std::string(".txt")  , excludedVarsBase);
    
    return;
    
    
}

void CVcfAnalyzer::ThreadFunc2(std::vector<int> a_nChrArr)
{
    for(int k = 0; k < a_nChrArr.size(); k++)
    {
        std::vector<const CVariant*> varListBase = m_provider.GetVariantList(eBASE, a_nChrArr[k]);
        std::vector<const CVariant*> varListCalled = m_provider.GetVariantList(eCALLED, a_nChrArr[k]);
        std::vector<const COrientedVariant*> ovarListBase = m_provider.GetOrientedVariantList(eBASE, a_nChrArr[k], true);
        std::vector<const COrientedVariant*> ovarListCalled = m_provider.GetOrientedVariantList(eCALLED, a_nChrArr[k], true);
        
        CPathReplay pathReplay(varListBase, varListCalled, ovarListBase, ovarListCalled);
        SContig ctg;
        m_provider.GetContig(a_nChrArr[k], ctg);
        
        //Find Best Path [GENOTYPE MATCH]
        m_aBestPaths[a_nChrArr[k]] = pathReplay.FindBestPath(ctg,true);
        
        //Genotype Match variants
        const std::vector<const COrientedVariant*>& includedVarsBase = m_aBestPaths[a_nChrArr[k]].m_baseSemiPath.GetIncludedVariants();
        const std::vector<const COrientedVariant*>& includedVarsCall = m_aBestPaths[a_nChrArr[k]].m_calledSemiPath.GetIncludedVariants();
        
        //Variants that will be passed for allele match check
        std::vector<const CVariant*> excludedVarsBase = m_provider.GetVariantList(eBASE, a_nChrArr[k], m_aBestPaths[a_nChrArr[k]].m_baseSemiPath.GetExcluded());
        std::vector<const CVariant*> excludedVarsCall = m_provider.GetVariantList(eCALLED, a_nChrArr[k], m_aBestPaths[a_nChrArr[k]].m_calledSemiPath.GetExcluded());
        
        //Fill oriented variants for allele match
        m_provider.FillAlleleMatchVariantList(a_nChrArr[k], excludedVarsBase, excludedVarsCall);
        
        //Clear old variant pointers
        ovarListBase.clear();
        ovarListCalled.clear();
        varListBase.clear();
        varListCalled.clear();
        
        //Set new variant pointers for allele match comparison
        varListBase = excludedVarsBase;
        varListCalled = excludedVarsCall;
        ovarListBase = m_provider.GetOrientedVariantList(eBASE, a_nChrArr[k], false);
        ovarListCalled = m_provider.GetOrientedVariantList(eCALLED, a_nChrArr[k], false);
        pathReplay.Clear();
        
        //Find Best Path [ALLELE MATCH]
        m_aBestPathsAllele[a_nChrArr[k]] = pathReplay.FindBestPath(ctg, false);
        
        //No Match variants
        std::vector<const CVariant*> excludedVarsBase2 = m_provider.GetVariantList(excludedVarsBase,
                                                                                   m_aBestPathsAllele[a_nChrArr[k]].m_baseSemiPath.GetExcluded());
        std::vector<const CVariant*> excludedVarsCall2 = m_provider.GetVariantList(excludedVarsCall,
                                                                                   m_aBestPathsAllele[a_nChrArr[k]].m_calledSemiPath.GetExcluded());
        
        //Allele Match variants
        const std::vector<const COrientedVariant*>& includedVarsBase2 = m_aBestPathsAllele[a_nChrArr[k]].m_baseSemiPath.GetIncludedVariants();
        const std::vector<const COrientedVariant*>& includedVarsCall2 = m_aBestPathsAllele[a_nChrArr[k]].m_calledSemiPath.GetIncludedVariants();
        
        
        m_resultLogger.LogStatistic(a_nChrArr[k],
                                    static_cast<int>(includedVarsCall.size()),
                                    static_cast<int>(includedVarsBase.size()),
                                    static_cast<int>(includedVarsCall2.size()),
                                    static_cast<int>(includedVarsBase2.size()),
                                    static_cast<int>(excludedVarsCall2.size()),
                                    static_cast<int>(excludedVarsBase2.size()));
        
        m_provider.SetVariantStatus(excludedVarsBase2, eNO_MATCH);
        m_provider.SetVariantStatus(excludedVarsCall2, eNO_MATCH);
        m_provider.SetVariantStatus(includedVarsCall,  eGENOTYPE_MATCH);
        m_provider.SetVariantStatus(includedVarsBase,  eGENOTYPE_MATCH);
        m_provider.SetVariantStatus(includedVarsCall2, eALLELE_MATCH);
        m_provider.SetVariantStatus(includedVarsBase2, eALLELE_MATCH);
        
        //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("FP_") + std::to_string(a_nChrArr[k] + 1) + std::string(".txt")  , excludedVarsCall);
        //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("TP_BASE_") + std::to_string(a_nChrArr[k] +1) + std::string(".txt")  , includedVarsBase);
        //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("TP_CALLED_") + std::to_string(a_nChrArr[k] + 1) + std::string(".txt")  , includedVarsCall);
        //PrintVariants(std::string(m_config.m_pOutputDirectory), std::string("FN_") + std::to_string(a_nChrArr[k] + 1) + std::string(".txt")  , excludedVarsBase);
        
        //std::ofstream syncpointFile;
        //std::string fileName = std::string(m_config.m_pOutputDirectory) + std::string("/SyncPoint") + std::to_string(a_nChrArr[k] +1) + std::string(".txt");
        //syncpointFile.open(fileName.c_str());
        
        //for(int x = (int)m_aBestPaths[a_nChrArr[k]].m_aSyncPointList.size()-1; x >=0 ; x--)
        //    syncpointFile << m_aBestPaths[a_nChrArr[k]].m_aSyncPointList[x] << std::endl;
        
    }
}


void CVcfAnalyzer::PrintVariants(std::string a_outputDirectory, std::string a_FileName, const std::vector<const COrientedVariant*>& a_rOvarList) const
{
    std::ofstream outputFile;
    
    std::string fileName = a_outputDirectory + std::string("/") + a_FileName;
    
    outputFile.open(fileName.c_str());
    
    for(int k = (int)a_rOvarList.size()-1; k >= 0; k--)
        outputFile << a_rOvarList[k]->GetVariant().ToString() << std::endl;
    
    outputFile.close();
}

void CVcfAnalyzer::PrintVariants(std::string a_outputDirectory, std::string a_FileName, const std::vector<const CVariant*>& a_rVarList) const
{
    std::ofstream outputFile;
    
    std::string fileName = a_outputDirectory + std::string("/") + a_FileName;
    
    outputFile.open(fileName.c_str());
    
    for(const CVariant* pVar : a_rVarList)
        outputFile << pVar->ToString() << std::endl;

    outputFile.close();
}


bool CVcfAnalyzer::ReadParameters(int argc, char** argv)
{

    const char* PARAM_BASE = "-base";
    const char* PARAM_CALLED = "-called";
    const char* PARAM_FILTER = "-filter";
    const char* PARAM_REFERENCE = "-ref";
    const char* PARAM_HELP = "--help";
    const char* PARAM_SAMPLE_BASE = "-SampleBase";
    const char* PARAM_SAMPLE_CALLED = "-SampleCalled";
    const char* PARAM_SNP_ONLY = "--SNP_ONLY";
    const char* PARAM_INDEL_ONLY = "--INDEL_ONLY";
    const char* PARAM_OUTPUT_DIR = "-outDir";
    const char* PARAM_REF_OVERLAP = "--ref-overlap";
    const char* PARAM_PLATFORM = "--platform-mode";
    
    bool bBaselineSet = false;
    bool bCalledSet = false;
    bool bReferenceSet = false;
    bool bOutputDirSet = false;
    
    //Start from index 2 since first parameter will be variant comparison mode indicator
    int it = 2;
    
    if(argc < 2)
    {
        PrintHelp();
        return false;
    }
    
    while(it < argc)
    {
        if(0 == strcmp(argv[it], PARAM_HELP))
        {
            PrintHelp();
            return false;
        }
        
        else if(0 == strcmp(argv[it], PARAM_BASE))
        {
            m_config.m_pBaseVcfFileName = argv[it+1];
            bBaselineSet = true;
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_CALLED))
        {
            m_config.m_pCalledVcfFileName = argv[it+1];
            bCalledSet = true;
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_REFERENCE))
        {
            m_config.m_pFastaFileName = argv[it+1];
            bReferenceSet = true;
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_OUTPUT_DIR))
        {
            m_config.m_pOutputDirectory = argv[it+1];
            bOutputDirSet = true;
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_REF_OVERLAP))
        {
            m_config.m_bIsRefOverlap = true;
            it++;
        }
        
        else if(0 == strcmp(argv[it], PARAM_FILTER))
        {
            if(0 == strcmp("none", argv[it+1]))
                m_config.m_bIsFilterEnabled = false;
            else
            {
                m_config.m_bIsFilterEnabled = true;
                m_config.m_pFilterName = argv[it+1];
            }
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_SAMPLE_BASE))
        {
            m_config.m_bBaseSampleEnabled = true;
            m_config.m_pBaseSample = argv[it+1];
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_SAMPLE_CALLED))
        {
            m_config.m_bCalledSampleEnabled = true;
            m_config.m_pCalledSample = argv[it+1];
            it += 2;
        }
        
        else if(0 == strcmp(argv[it], PARAM_SNP_ONLY))
        {
            m_config.m_bSNPOnly = true;
            it++;
        }
        
        else if(0 == strcmp(argv[it], PARAM_INDEL_ONLY))
        {
            m_config.m_bINDELOnly = true;
            it++;
        }
        
        else if(0 == strcmp(argv[it], PARAM_PLATFORM))
        {
            m_config.m_bIsPlatformMode = true;
            it++;
        }
        else
            it++;
    }
    
    if(!bBaselineSet)
        std::cout << "Baseline vcf file is not set" << std::endl;
    else if(!bCalledSet)
        std::cout << "Called vcf file is not set" << std::endl;
    else if(!bReferenceSet)
        std::cout << "Reference fasta file is not set" << std::endl;
    else if(!bOutputDirSet)
        std::cout << "Output Directory is not set" << std::endl;
    
    
    return bCalledSet & bBaselineSet & bReferenceSet & bOutputDirSet;
}

void CVcfAnalyzer::PrintHelp() const
{
    std::cout << "==== SBG VCF COMPARISON TOOL VERSION 1.0 (Beta) ==== " << std::endl;
    std::cout << "Based on paper: http://biorxiv.org/content/biorxiv/early/2015/08/02/023754.full.pdf" << std::endl;
    std::cout << "Author: Berke Cagkan Toptas (berke.toptas@sbgenomics.com)" << std::endl;
    std::cout << "Please notify me if program fails or return unexpected results" << std::endl;
    std::cout << "COPYRIGHT (C) 2016 SEVEN BRIDGES GENOMICS." << std::endl;
    std::cout << std::endl;
    std::cout << " --- PARAMETERS --- " << std::endl;
    std::cout << "-base <baseline_vcf_path>    [Required.Add baseline VCF file.]" << std::endl;
    std::cout << "-called <called_vcf_path>    [Required.Add called VCF file.]" << std::endl;
    std::cout << "-ref <reference_fasta_path>  [Required.Add reference FASTA file]" << std::endl;
    std::cout << "-outDir <output_directory>   [Required.Add output directory]" << std::endl;
    std::cout << "-filter <filter_name>        [Optional.Filter variants based on filter column. Default value is PASS. Use 'none' to unfilter]" << std::endl;
    std::cout << "-SNP_ONLY                    [Optional.Filter INDELs out from both base and called VCF file.]" << std::endl;
    std::cout << "-INDEL_ONLY                  [Optional.Filter SNPs out from both base and called VCF file.]" << std::endl;
    std::cout << "-SampleBase <sample_name>    [Optional.Read only the given sample in base VCF. Default is the first sample.]" << std::endl;
    std::cout << "-SampleCalled <sample_name>  [Optional.Read only the given sample in called VCF. Default is the first sample.]" << std::endl;
    std::cout << "-ref-overlap                 [Optional.Allow reference overlapping by trimming nucleotides and ignoring 0 genotype.]" << std::endl;
    std::cout << "-platform-mode               [Optional.Allow to run program with the thread number of different chromosome count.]" << std::endl;
    std::cout << std::endl;
    std::cout << "Example Commands:" << std::endl;
    std::cout << "./sbgVcfComp -called called.vcf -base base.vcf -ref reference.fa -outDir SampleResultDir -filter none" << std::endl;
    std::cout << "./sbgVcfComp -called called2.vcf -base base2.vcf -ref reference.fa -outDir SampleResultDir -filter PASS -SNP_ONLY -SampleBase sample0 -SampleCalled sample01" << std::endl;
}