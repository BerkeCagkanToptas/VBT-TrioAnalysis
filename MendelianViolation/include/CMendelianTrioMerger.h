//
//  CMendelianTrioMerger.h
//  VCFComparison
//
//  Created by Berke.Toptas on 3/9/17.
//  Copyright © 2017 Seven Bridges Genomics. All rights reserved.
//

#ifndef _C_MENDELIAN_TRIO_MERGER_H_
#define _C_MENDELIAN_TRIO_MERGER_H_

#include "CVcfWriter.h"
#include "CVariant.h"
#include "EMendelianDecision.h"
#include "EMendelianVcfName.h"
#include <vector>
#include "Constants.h"
#include "ENoCallMode.h"
#include "CMendelianResultLog.h"



class CMendelianTrioMerger
{
    
public:
    
    //Generates the trio vcf by merging parent-child variants into single 3-sample vcf file
    void GenerateTrioVcf();
    
    //Set variant references for merge
    void SetVariants(int a_nChromosomeId, EMendelianVcfName a_vcfName, const std::vector<const CVariant*>& a_rVarList);
    
    //Set variant decisions for marking MendelianDecision info columns
    void SetDecisions(int a_nChromosomeId, EMendelianVcfName a_vcfName, const std::vector<EMendelianDecision>& a_rDecisionList);
    
    //Set No Call mode to decide if no calls will be printed as ./. or 0/0s
    void SetNoCallMode(ENoCallMode a_mode);
    
    //Set the full path of output trio vcf
    void SetTrioPath(const std::string& a_nTrioPath);
    
    //Set the access of result log from mendelian vcf analyzer to for detailed logs
    void SetResultLogPointer(CMendelianResultLog* a_pResultLog);
    
private:

    //Merge 3 variant set into one trio.vcf that mendelian decisions are marked
    void AddRecords(int a_nChromosomeId);
    
    //Fill the header part of the trio vcf
    void FillHeader();
    
    //Check if the given two variant share the same position and reference
    bool IsMerge(const CVariant* a_pVar1, const CVariant* a_pVar2);
    
    //Merge 3 variant to single vcf record
    void DoTripleMerge(int a_nChromosomeId, int& a_nChildItr, int& a_nFatherItr, int& a_nMotherItr, EMendelianDecision a_decision);

    //Merge 2 variant to single vcf record
    void DoDoubleMerge(int a_nChromosomeId, int& a_nItr1, int& a_nItr2, EMendelianVcfName a_name1, EMendelianVcfName a_name2, EMendelianDecision a_decision);
 
    //Write single variant to vcf record
    void DoSingleVar(int a_nChromosomeId, int& a_nItr, EMendelianVcfName a_name, EMendelianDecision a_decision);
    
    //Decide the mendelian type of the given three variant in a row
    EMendelianDecision GetMendelianDecision(const CVariant* a_pVarMother, const CVariant* a_pVarFather, const CVariant* a_pVarChild, EMendelianDecision a_initDecision);
    
    //Register a line of merged vcf to the detailed report table [updates m_logEntry]
    EVariantCategory RegisterMergedLine(const CVariant* a_pVariant, EMendelianDecision a_decision);
    
    //Register the genotype of merged vcf to genotype table [updates m_logGenotypes]
    void RegisterGenotype(const CVariant* a_pMother,
                          const CVariant* a_pFather,
                          const CVariant* a_pChild,
                          EMendelianDecision a_initDecision,
                          EVariantCategory a_category);
    
    //Vcf writer instance
    CVcfWriter m_vcfWriter;
    
    std::vector<EMendelianDecision> m_aChildDecisions[CHROMOSOME_COUNT];
    std::vector<EMendelianDecision> m_aFatherDecisions[CHROMOSOME_COUNT];
    std::vector<EMendelianDecision> m_aMotherDecisions[CHROMOSOME_COUNT];

    std::vector<const CVariant*> m_aChildVariants[CHROMOSOME_COUNT];
    std::vector<const CVariant*> m_aFatherVariants[CHROMOSOME_COUNT];
    std::vector<const CVariant*> m_aMotherVariants[CHROMOSOME_COUNT];
    
    ENoCallMode m_noCallMode;
    
    //Output trio full path
    std::string m_trioPath;
    
    //Objects to pass the result Log
    SMendelianDetailedLogEntry m_logEntry;
    SMendelianDetailedLogGenotypes m_logGenotypes;
    
    CMendelianResultLog* m_pResultLog;
};




#endif // _C_MENDELIAN_TRIO_MERGER_H_
