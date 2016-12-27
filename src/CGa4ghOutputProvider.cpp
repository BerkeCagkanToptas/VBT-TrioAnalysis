//
//  CGa4ghOutputProvider.cpp
//  VCFComparison
//
//  Created by Berke.Toptas on 12/20/16.
//  Copyright © 2016 Seven Bridges Genomics. All rights reserved.
//

#include "CGa4ghOutputProvider.h"
#include <sstream>
#include "CPath.h"
#include "CCallIterator.h"
#include "CVariantProvider.h"
#include <iostream>

void CGa4ghOutputProvider::SetVariantProvider(CVariantProvider* a_pProvider)
{
    m_pVariantProvider = a_pProvider;
}

void CGa4ghOutputProvider::SetBestPaths(CPath* a_pBestPathList)
{
    m_pBestPaths = a_pBestPathList;
}

void CGa4ghOutputProvider::SetVcfPath(const std::string& a_rVcfPath)
{
    m_vcfPath = a_rVcfPath;
}

void CGa4ghOutputProvider::GenerateGa4ghVcf()
{
    m_vcfWriter.CreateVcf(m_vcfPath.c_str());
    FillHeader();
    
    for(int k = 0; k < CHROMOSOME_COUNT; k++)
        AddRecords(m_pBestPaths[k], k);
    
    m_vcfWriter.CloseVcf();
}

void CGa4ghOutputProvider::FillHeader()
{
    //INIT VCF HEADER
    m_vcfWriter.InitHeader();
    m_vcfWriter.AddHeaderLine("##source= SBG Vcf Comparison Tool Ver. 1.0 (Beta), 2016");
    
    //ADD REQUIRED FORMATS BY GA4GH
    m_vcfWriter.AddHeaderLine("##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">");
    m_vcfWriter.AddHeaderLine("##FORMAT=<ID=BD,Number=1,Type=String,Description=\"Decision for call (TP/FP/FN/N)\">");
    m_vcfWriter.AddHeaderLine("##FORMAT=<ID=BK,Number=1,Type=String,Description=\"Sub-type for decision (match/mismatch type)\">");
    
    //ADD CONTIG IDs
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr1,length=249250621>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr2,length=243199373>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr3,length=198022430>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr4,length=191154276>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr5,length=180915260>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr6,length=171115067>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr7,length=159138663>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr8,length=146364022>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr9,length=141213431>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr10,length=135534747>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr11,length=135006516>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr12,length=133851895>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr13,length=115169878>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr14,length=107349540>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr15,length=102531392>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr16,length=90354753>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr17,length=81195210>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr18,length=78077248>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr19,length=59128983>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr20,length=63025520>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr21,length=48129895>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chr22,length=51304566>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chrM,length=16571>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chrX,length=155270560>");
    m_vcfWriter.AddHeaderLine("##contig=<ID=chrY,length=59373566>");

    //ADD REQUIRED SAMPLES
    m_vcfWriter.AddSampleName("TRUTH");
    m_vcfWriter.AddSampleName("QUERY");
    
    //CLOSE HEADER
    m_vcfWriter.WriteHeaderToVcf();
}


void CGa4ghOutputProvider::AddRecords(const CPath& a_rBestPath, int a_nChrId)
{
    std::vector<CVariant*> excludedVarsBase = m_pVariantProvider->GetVariantList(eBASE, a_nChrId, a_rBestPath.m_baseSemiPath.GetExcluded());
    std::vector<CVariant*> excludedVarsCall = m_pVariantProvider->GetVariantList(eCALLED, a_nChrId,a_rBestPath.m_calledSemiPath.GetExcluded());

    std::vector<const COrientedVariant*> includedVarsBase = a_rBestPath.m_baseSemiPath.GetIncludedVariants();
    std::vector<const COrientedVariant*> includedVarsCall = a_rBestPath.m_calledSemiPath.GetIncludedVariants();
    
    CCallIterator baseVariants(includedVarsBase, excludedVarsBase);
    CCallIterator calledVariants(includedVarsCall, excludedVarsCall);

    SVariantSummary varBase;
    SVariantSummary varCalled;
    
    varBase = baseVariants.hasNext() ? baseVariants.next() : SVariantSummary();
    varCalled = calledVariants.hasNext() ? calledVariants.next() : SVariantSummary();
    
    while(!varBase.isNull() && !varCalled.isNull())
    {
        if(CanMerge(varBase.m_pVariant, varCalled.m_pVariant))
        {
            SVcfRecord record;
            std::string decisionBase = varBase.m_bIncluded ? "TP" : "FN";
            std::string decisionCalled = varCalled.m_bIncluded ? "TP" : "FP";
            std::string match = "nm";
            MergeVariants(varBase.m_pVariant, varCalled.m_pVariant, "am", decisionBase, decisionCalled, record);
            m_vcfWriter.AddRecord(record);
            
            //We used both variant. Get the next variant from both list
            varBase = baseVariants.hasNext() ? baseVariants.next() : SVariantSummary();
            varCalled = calledVariants.hasNext() ? calledVariants.next() : SVariantSummary();
            continue;
            
        }
        
        else if(varCalled.isNull() || (!varBase.isNull() && varBase.m_pVariant->GetStart() < varCalled.m_pVariant->GetStart()))
        {
            SVcfRecord record;
            std::string decision = varBase.m_bIncluded ? "TP" : "FN";
            std::string match = "nm";
            VariantToVcfRecord(varBase.m_pVariant, record, true, match, decision);
            m_vcfWriter.AddRecord(record);
            varBase = baseVariants.hasNext() ? baseVariants.next() : SVariantSummary();
            continue;
        }
        
        else
        {
            SVcfRecord record;
            record.m_aSampleData.push_back(SPerSampleData());
            std::string decision = varCalled.m_bIncluded ? "TP" : "FP";
            std::string match = "nm";
            VariantToVcfRecord(varCalled.m_pVariant, record, true, match, decision);
            m_vcfWriter.AddRecord(record);
            varCalled = calledVariants.hasNext() ? calledVariants.next() : SVariantSummary();
            continue;
        }
    }
}


void CGa4ghOutputProvider::VariantToVcfRecord(const CVariant* a_pVariant, SVcfRecord& a_rOutputRec, bool a_bIsBase, const std::string& a_rMatchType, const::std::string& a_rDecision)
{
    //Fill basic variant data
    a_rOutputRec.m_chrName = a_pVariant->m_chrName;
    a_rOutputRec.m_nPosition = a_pVariant->m_nStartPos;
    a_rOutputRec.m_alleles = a_pVariant->m_allelesStr;
    a_rOutputRec.m_filterString = a_pVariant->m_filterString;
    
    //If this is called variant create an empty sample data for TRUTH column
    if(a_bIsBase == false)
        a_rOutputRec.m_aSampleData.push_back(SPerSampleData());
    
    //Fill genotype of sample data
    SPerSampleData data;
    data.m_bIsPhased = a_pVariant->m_bIsPhased;
    data.m_nHaplotypeCount = a_pVariant->m_nZygotCount;
    for(int k = 0; k < data.m_nHaplotypeCount; k++)
        data.m_aGenotype[k] = a_pVariant->m_genotype[k];
    data.m_decisionBD = a_rDecision;
    data.m_matchTypeBK = a_rMatchType;
    a_rOutputRec.m_aSampleData.push_back(data);
}


void CGa4ghOutputProvider::MergeVariants(const CVariant* a_pVariantBase,
                                         const CVariant* a_pVariantCalled,
                                         const std::string& a_rMatchType,
                                         const std::string& a_rDecisionBase,
                                         const std::string& a_rDecisionCalled,
                                         SVcfRecord& a_rOutputRec)
{

    //Fill basic variant data
    a_rOutputRec.m_chrName = a_pVariantBase->m_chrName;
    a_rOutputRec.m_nPosition = a_pVariantBase->m_nStartPos;
    a_rOutputRec.m_alleles = a_pVariantBase->m_allelesStr;
    a_rOutputRec.m_filterString = a_pVariantBase->m_filterString;
    
    //Fill base sample (TRUTH)
    SPerSampleData data;
    data.m_bIsPhased = a_pVariantBase->m_bIsPhased;
    data.m_nHaplotypeCount = a_pVariantBase->m_nZygotCount;
    for(int k = 0; k < data.m_nHaplotypeCount; k++)
        data.m_aGenotype[k] = a_pVariantBase->m_genotype[k];
    data.m_decisionBD = a_rDecisionBase;
    data.m_matchTypeBK = a_rMatchType;
    
    a_rOutputRec.m_aSampleData.push_back(data);
    
    //Fill called sample (QUERY)
    SPerSampleData data2;
    data2.m_bIsPhased = a_pVariantCalled->m_bIsPhased;
    data2.m_nHaplotypeCount = a_pVariantCalled->m_nZygotCount;
    
    std::stringstream ss(a_pVariantBase->m_allelesStr);
    std::vector<std::string> baseVariants;
    std::string substr;
    
    while(getline(ss, substr, ','))
    {
        baseVariants.push_back(substr);
    }
    for(int k=0; k < a_pVariantCalled->m_nZygotCount; k++)
    {
        for(int p = 0; p < baseVariants.size(); p++)
        {
            std::string allele = a_pVariantCalled->m_bIsFirstNucleotideTrimmed ? (a_pVariantCalled->GetRefSeq()[0] + a_pVariantCalled->m_alleles[k].m_sequence) : a_pVariantCalled->m_alleles[k].m_sequence;
            if(0 == baseVariants[p].compare(allele))
            {
                data2.m_aGenotype[k] = p;
                break;
            }
        }
    }

    
    data2.m_decisionBD = a_rDecisionCalled;
    data2.m_matchTypeBK = a_rMatchType;
    a_rOutputRec.m_aSampleData.push_back(data2);
    
}


bool CGa4ghOutputProvider::CanMerge(const CVariant* a_pVariantBase, const CVariant* a_pVariantCalled) const
{
    bool bIsPosEqual = a_pVariantBase->m_nStartPos == a_pVariantCalled->m_nStartPos;
    bool bIsRefEqual = a_pVariantBase->m_refSequence == a_pVariantCalled->m_refSequence;
    
    if(bIsPosEqual && bIsRefEqual)
    {
        
        std::stringstream ss(a_pVariantBase->m_allelesStr);
        std::vector<std::string> baseVariants;
    
        std::string substr;
        while(getline(ss, substr, ','))
        {
            baseVariants.push_back(substr);
        }
    
        bool bIsAlleleExists;
    
        for(int k=0; k < a_pVariantCalled->m_nAlleleCount; k++)
        {
            bIsAlleleExists = false;
            for(int p = 0; p < baseVariants.size(); p++)
            {
                std::string allele = a_pVariantCalled->m_bIsFirstNucleotideTrimmed ? (a_pVariantCalled->GetRefSeq()[0] + a_pVariantCalled->m_alleles[k].m_sequence) : a_pVariantCalled->m_alleles[k].m_sequence;
                if(0 == baseVariants[p].compare(allele))
                {
                    bIsAlleleExists = true;
                    break;
                }
            }
        
            if(false == bIsAlleleExists)
                 return false;
        }
        
        return true;
    }
    else
        return false;
}
































