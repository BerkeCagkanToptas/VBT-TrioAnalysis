//
//  CVariantProvider.cpp
//  VCFComparison
//
//  Created by Berke.Toptas
//  Copyright © 2016 Seven Bridges Genomics. All rights reserved.
//

#include "CVariantProvider.h"
#include "COrientedVariant.h"
#include "CSimpleBEDParser.h"
#include "Utils/CUtils.h"
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace duocomparison;

CVariantProvider::CVariantProvider()
{
    m_bIsHomozygousOvarListInitialized = false;
}

CVariantProvider::~CVariantProvider()
{
    for(unsigned int k= 0; k < m_aContigList.size(); k++)
    {
        if(m_aContigList[k].m_nRefLength > 0 && m_aContigList[k].m_pRefSeq != 0)
            delete[] m_aContigList[k].m_pRefSeq;
    }
}

void CVariantProvider::SetChromosomeIdTuples()
{
    int tupleIndex = 0;
    
    for(auto baseItr = m_baseVCF.m_chrIndexMap.begin(); baseItr != m_baseVCF.m_chrIndexMap.end(); baseItr++)
    {
        for(auto calledItr = m_calledVCF.m_chrIndexMap.begin(); calledItr != m_calledVCF.m_chrIndexMap.end(); calledItr++)
        {
            if(baseItr->first == calledItr->first)
            {
                if(m_aBaseVariantList[baseItr->second].size() > LEAST_VARIANT_THRESHOLD
                   &&
                   m_aCalledVariantList[calledItr->second].size() > LEAST_VARIANT_THRESHOLD)
                {
                    m_aCommonChrTupleList.push_back(SChrIdTuple(baseItr->second, calledItr->second, baseItr->first, tupleIndex++));
                }
            }
        }
    }
    
    std::sort(m_aCommonChrTupleList.begin(), m_aCommonChrTupleList.end(), [](const SChrIdTuple& t1, const SChrIdTuple& t2){return t1.m_nBaseId < t2.m_nBaseId;});
}

bool CVariantProvider::InitializeReaders(const SConfig& a_rConfig)
{
    bool bIsSuccess;

    m_config = a_rConfig;
    
    //OPEN VCF FILES
    bIsSuccess = m_baseVCF.Open(a_rConfig.m_pBaseVcfFileName);
    if(!bIsSuccess)
    {
        std::cerr << "Baseline VCF file is unable to open!: " << a_rConfig.m_pBaseVcfFileName << std::endl;
        return false;
    }
    m_baseVCF.setID(0);

    bIsSuccess = m_calledVCF.Open(a_rConfig.m_pCalledVcfFileName);
    if(!bIsSuccess)
    {
        std::cerr << "Called VCF file is unable to open!: " << a_rConfig.m_pCalledVcfFileName << std::endl;
        return false;
    }
    m_calledVCF.setID(1);
    
    //SET SAMPLE NAME TO READ ONLY ONE SAMPLE FROM THE VCF
    if (true == m_config.m_bBaseSampleEnabled)
       bIsSuccess = m_baseVCF.SelectSample(m_config.m_pBaseSample);
    else
    {
        std::vector<std::string> sampleNames;
        m_baseVCF.GetSampleNames(sampleNames);
       bIsSuccess = m_baseVCF.SelectSample(sampleNames[0]);
    }
    
    if(!bIsSuccess)
    {
        std::cerr << "Baseline Sample name is incorrect!" << std::endl;
        return false;
    }
    
    if (true == m_config.m_bCalledSampleEnabled)
        bIsSuccess = m_calledVCF.SelectSample(m_config.m_pCalledSample);
    else
    {
        std::vector<std::string> sampleNames;
        m_calledVCF.GetSampleNames(sampleNames);
        bIsSuccess = m_calledVCF.SelectSample(sampleNames[0]);
    }
    
    if(!bIsSuccess)
    {
        std::cerr << "Called Sample name is incorrect!" << std::endl;
        return false;
    }

    
    // OPEN FASTA FILE
    bIsSuccess = m_referenceFasta.OpenFastaFile(a_rConfig.m_pFastaFileName);
    if(!bIsSuccess)
    {
        std::cerr << "FASTA file is unable to open!: " << a_rConfig.m_pFastaFileName << std::endl;
        return false;
    }
    
    if(bIsSuccess)
    {
        //Fill variant lists from VCF files
        FillVariantLists();
        
        //Generate OrientedVariant list using the variant list
        FillOrientedVariantLists();
        
        //Set the common chromosome list for processing
        SetChromosomeIdTuples();
    }
    
    return bIsSuccess;
}

void CVariantProvider::FillVariantForSample(int a_nSampleId, SConfig& a_rConfig)
{
    CSimpleBEDParser bedParser;
    if(true == a_rConfig.m_bInitializeFromBed)
        bedParser.InitBEDFile(a_rConfig.m_pBedFileName);
    unsigned int remainingBedContigCount = bedParser.m_nTotalContigCount;

    EVcfName sampleName = static_cast<EVcfName>(a_nSampleId);
    std::string sampleNameStr = sampleName == eBASE ? "base" : "called";
    
    CVcfReader* pReader = a_nSampleId == 0 ? &m_baseVCF : &m_calledVCF;
    std::vector<std::vector<CVariant>>* pNonAssessedVariants = a_nSampleId == 0 ? &m_aBaseNotAssessedVariantList : &m_aCalledNotAssessedVariantList;
    std::vector<std::vector<CVariant>>* pVariants = a_nSampleId == 0 ? &m_aBaseVariantList : &m_aCalledVariantList;
    
    
    CVariant variant;
    int id = 0;
    std::string preChrId = "";
    unsigned int regionIterator = 0;
    
    std::vector<CVariant> multiTrimmableVarList;
    
    while(pReader->GetNextRecord(&variant, id++, a_rConfig))
    {
        if(preChrId != variant.m_chrName)
        {
            //We update the remaining contig count in BED file
            if(bedParser.m_regionMap[preChrId].size() > 0)
                remainingBedContigCount--;
            
            regionIterator = 0;
            preChrId = variant.m_chrName;
            std::cout << "Processing chromosome " << preChrId << " of " << sampleNameStr  << " vcf" << std::endl;
        }
        
        if(a_rConfig.m_bInitializeFromBed)
        {
            //All BED regions are finished
            if(remainingBedContigCount == 0)
                break;

            //No Region exist for this chromosome
            if(regionIterator == bedParser.m_regionMap[variant.m_chrName].size())
                continue;
            
            //Skip to next region
            while(regionIterator < bedParser.m_regionMap[variant.m_chrName].size()
                  &&
                  variant.m_nOriginalPos >= bedParser.m_regionMap[variant.m_chrName][regionIterator].m_nEndPos)
            {
                regionIterator++;
            }
            
            //No remaining Region exist for this chromosome
            if(regionIterator == bedParser.m_regionMap[variant.m_chrName].size())
                continue;

            //Variant Could not pass from BED region
            if(bedParser.m_regionMap[variant.m_chrName][regionIterator].m_nStartPos >= (variant.m_nOriginalPos + (int)variant.m_refSequence.length()))
                continue;
        }
        
        if(!variant.m_bIsNoCall && CUtils::IsHomRef(variant))
            continue;
        
        if(a_rConfig.m_bIsFilterEnabled && variant.m_bIsFilterPASS == false)
            (*pNonAssessedVariants)[variant.m_nChrId].push_back(variant);
        
        else if(a_rConfig.m_bSNPOnly && variant.GetVariantType() != eSNP)
            (*pNonAssessedVariants)[variant.m_nChrId].push_back(variant);
        
        else if(a_rConfig.m_bINDELOnly && variant.GetVariantType() != eINDEL)
            (*pNonAssessedVariants)[variant.m_nChrId].push_back(variant);
        
        else if(CUtils::IsStructuralVariant(variant, a_rConfig.m_nMaxVariantSize))
            (*pNonAssessedVariants)[variant.m_nChrId].push_back(variant);
        
        else if(true == variant.m_bHaveMultipleTrimOption)
            multiTrimmableVarList.push_back(variant);
        
        else
            (*pVariants)[variant.m_nChrId].push_back(variant);
    }
    
    FindOptimalTrimmings(multiTrimmableVarList, sampleName);
    AppendTrimmedVariants(multiTrimmableVarList, sampleName);
    
    for(unsigned int k = 0; k < pReader->GetContigs().size(); k++)
    {
        std::sort((*pNonAssessedVariants)[k].begin(), (*pNonAssessedVariants)[k].end(), CUtils::CompareVariants);
        std::sort((*pVariants)[k].begin(), (*pVariants)[k].end(), CUtils::CompareVariants);
    }
}


void CVariantProvider::FillVariantLists()
{
    //Initialize variantLists
    m_aBaseVariantList = std::vector<std::vector<CVariant>>(m_baseVCF.GetContigs().size());
    m_aCalledVariantList = std::vector<std::vector<CVariant>>(m_calledVCF.GetContigs().size());
    m_aBaseNotAssessedVariantList = std::vector<std::vector<CVariant>>(m_baseVCF.GetContigs().size());
    m_aCalledNotAssessedVariantList = std::vector<std::vector<CVariant>>(m_calledVCF.GetContigs().size());
    
    FillVariantForSample(eBASE, m_config);
    FillVariantForSample(eCALLED, m_config);
}

void CVariantProvider::FillOrientedVariantLists()
{
    //Initialize OrientedVariantLists
    m_aBaseOrientedVariantList = std::vector<std::vector<core::COrientedVariant>>(m_baseVCF.GetContigs().size());
    m_aCalledOrientedVariantList = std::vector<std::vector<core::COrientedVariant>>(m_calledVCF.GetContigs().size());
    
    
    for(unsigned int i=0; i < m_aBaseOrientedVariantList.size(); i++)
    {
        for(unsigned int j=0; j < m_aBaseVariantList[i].size(); j++)
        {
            m_aBaseOrientedVariantList[i].push_back(core::COrientedVariant(m_aBaseVariantList[i][j], true));
            m_aBaseOrientedVariantList[i].push_back(core::COrientedVariant(m_aBaseVariantList[i][j], false));
        }
    }
    
    for(unsigned int i=0; i < m_aCalledOrientedVariantList.size(); i++)
    {
        for(unsigned int j=0; j < m_aCalledVariantList[i].size(); j++)
        {
            m_aCalledOrientedVariantList[i].push_back(core::COrientedVariant(m_aCalledVariantList[i][j], true));
            m_aCalledOrientedVariantList[i].push_back(core::COrientedVariant(m_aCalledVariantList[i][j], false));
        }
    }
}

void CVariantProvider::FillAlleleMatchVariantList(SChrIdTuple& a_tuple, std::vector<const CVariant*>& a_rBaseVariants, std::vector<const CVariant*>& a_rCalledVariants)
{
    //Initialize HomozygousOrientedVariantLists
    if(false == m_bIsHomozygousOvarListInitialized)
    {
        m_aBaseHomozygousOrientedVariantList = std::vector<std::vector<core::COrientedVariant>>(m_baseVCF.GetContigs().size());
        m_aCalledHomozygousOrientedVariantList = std::vector<std::vector<core::COrientedVariant>>(m_calledVCF.GetContigs().size());
        m_bIsHomozygousOvarListInitialized = true;
    }
    
    for(unsigned int j=0; j < a_rBaseVariants.size(); j++)
    {
        m_aBaseHomozygousOrientedVariantList[a_tuple.m_nBaseId].push_back(core::COrientedVariant(*a_rBaseVariants[j], 0));
        m_aBaseHomozygousOrientedVariantList[a_tuple.m_nBaseId].push_back(core::COrientedVariant(*a_rBaseVariants[j], 1));
    }
        
    for(unsigned int j=0; j < a_rCalledVariants.size(); j++)
    {
        m_aCalledHomozygousOrientedVariantList[a_tuple.m_nCalledId].push_back(core::COrientedVariant(*a_rCalledVariants[j], 0));
        m_aCalledHomozygousOrientedVariantList[a_tuple.m_nCalledId].push_back(core::COrientedVariant(*a_rCalledVariants[j], 1));
    }
}

std::vector<const CVariant*> CVariantProvider::GetVariantList(EVcfName a_uFrom, int a_nChrNo, const std::vector<int>& a_VariantIndexes)
{
    std::vector<const CVariant*> result;

    switch (a_uFrom)
    {
        case eBASE:
            for(unsigned int k=0; k < a_VariantIndexes.size(); k++)
                result.push_back(&(m_aBaseVariantList[a_nChrNo][a_VariantIndexes[k]]));
            break;
            
        case eCALLED:
            for(unsigned int k=0; k < a_VariantIndexes.size(); k++)
                result.push_back(&(m_aCalledVariantList[a_nChrNo][a_VariantIndexes[k]]));
            
        default:
            break;
    }

    return result;
}

std::vector<const CVariant*> CVariantProvider::GetVariantList(EVcfName a_uFrom, int a_nChrNo)
{
    unsigned long size = a_uFrom == eBASE ? m_aBaseVariantList[a_nChrNo].size() : m_aCalledVariantList[a_nChrNo].size();
    std::vector<const CVariant*> result(size);
    
    switch (a_uFrom)
    {
        case eBASE:
            for(int k=0; k < (int)size; k++)
                result[k] = &(m_aBaseVariantList[a_nChrNo][k]);
            break;
            
        case eCALLED:
            for(int k=0; k< (int)size; k++)
                result[k] = &(m_aCalledVariantList[a_nChrNo][k]);
            
        default:
            break;
    }
    
    return result;
}


std::vector<const CVariant*> CVariantProvider::GetVariantList(std::vector<const CVariant*>& a_varList, const std::vector<int>& a_VariantIndexes)
{
    std::vector<const CVariant*> result(a_VariantIndexes.size());
    
    for(unsigned int k = 0; k < a_VariantIndexes.size(); k++)
    {
        result[k] = a_varList[a_VariantIndexes[k]];
    }
    return result;
}


std::vector<const core::COrientedVariant*> CVariantProvider::GetOrientedVariantList(EVcfName a_uFrom, int a_nChrNo, bool a_bIsGenotypeMatch)
{
    if(a_bIsGenotypeMatch)
    {
        unsigned long size = a_uFrom == eBASE ? m_aBaseOrientedVariantList[a_nChrNo].size() : m_aCalledOrientedVariantList[a_nChrNo].size();
        std::vector<const core::COrientedVariant*> result(size);
        
        switch (a_uFrom)
        {
            case eBASE:
                for(unsigned int k=0; k < size; k++)
                    result[k] = &(m_aBaseOrientedVariantList[a_nChrNo][k]);
                break;
                
            case eCALLED:
                for(unsigned int k=0; k < size; k++)
                    result[k] = &(m_aCalledOrientedVariantList[a_nChrNo][k]);
                
            default:
                break;
        }

        return result;
    }
    
    else
    {
        unsigned long size = a_uFrom == eBASE ? m_aBaseHomozygousOrientedVariantList[a_nChrNo].size() : m_aCalledHomozygousOrientedVariantList[a_nChrNo].size();
        std::vector<const core::COrientedVariant*> result(size);
        
        switch (a_uFrom)
        {
            case eBASE:
                for(unsigned int k=0; k < size; k++)
                    result[k] = &(m_aBaseHomozygousOrientedVariantList[a_nChrNo][k]);
                break;
                
            case eCALLED:
                for(unsigned int k=0; k < size; k++)
                    result[k] = &(m_aCalledHomozygousOrientedVariantList[a_nChrNo][k]);
                
            default:
                break;
        }

        return result;
    }
    
}

std::vector<SChrIdTuple>& CVariantProvider::GetChromosomeIdTuples()
{
    return m_aCommonChrTupleList;
}

void CVariantProvider::GetFilterInfo(EVcfName a_vcfType, std::vector<std::string>& a_rFilterNames, std::vector<std::string>& a_rFilterDescriptions)
{
    switch (a_vcfType)
    {
        case eBASE:
            m_baseVCF.GetFilterInfo(a_rFilterNames, a_rFilterDescriptions);
            break;
        case eCALLED:
            m_calledVCF.GetFilterInfo(a_rFilterNames, a_rFilterDescriptions);
            break;
        default:
            break;
    }
}

std::vector<const CVariant*> CVariantProvider::GetNotAssessedVariantList(EVcfName a_uFrom, int a_nChrNo)
{
    std::vector<const CVariant*> result;
    
    
    if(eBASE == a_uFrom)
    {
        for(int k = 0; k < (int)m_aBaseNotAssessedVariantList[a_nChrNo].size(); k++)
            result.push_back(&m_aBaseNotAssessedVariantList[a_nChrNo][k]);
    }
    else
    {
        for(int k = 0; k < (int)m_aCalledNotAssessedVariantList[a_nChrNo].size(); k++)
            result.push_back(&m_aCalledNotAssessedVariantList[a_nChrNo][k]);
    }
    
    return result;
}

std::vector<const CVariant*> CVariantProvider::GetSkippedComplexVariantList(EVcfName a_uFrom, int a_nChrNo)
{
    std::vector<const CVariant*> result;
    
    if(a_uFrom == eBASE)
    {
        for(CVariant var : m_aBaseVariantList[a_nChrNo])
        {
            if(var.m_variantStatus == eCOMPLEX_SKIPPED)
            {
                var.m_variantStatus = eNOT_ASSESSED;
                result.push_back(&var);
            }
        }
    }

    return result;
}


const std::vector<SVcfContig>& CVariantProvider::GetContigs() const
{
    return m_calledVCF.GetContigs();
}

void CVariantProvider::FindOptimalTrimmings(std::vector<CVariant>& a_rVariantList, EVcfName a_uFrom)
{
    if(a_rVariantList.size() == 0)
        return;
    
    std::vector<std::vector<CVariant>>* allVarList = a_uFrom == eBASE ? &m_aBaseVariantList : &m_aCalledVariantList;
    
    unsigned int varItr[2];
    varItr[0] = 0;
    varItr[1] = 0;
    
    int currentChrId = a_rVariantList[0].m_nChrId;
    
    for(unsigned int k = 0; k < a_rVariantList.size(); k++)
    {
        for(int i = 0; i < 2; i++)
        {
            if(a_rVariantList[k].m_alleles[i].m_bIsIgnored || a_rVariantList[k].m_alleles[i].m_bIsTrimmed)
                continue;
            
            //The maximum possible trimming nucleotide count from start and end of each allele
            unsigned int canTrimStart, canTrimEnd;
            a_rVariantList[k].GetMaxTrimStartEnd(i, canTrimStart, canTrimEnd);
            
            std::vector<CVariant> tmpoverlapVariants;
            
            if(currentChrId != a_rVariantList[k].m_nChrId)
            {
                varItr[0] = 0;
                varItr[1] = 0;
                currentChrId = a_rVariantList[k].m_nChrId;
            }
            
            while(varItr[i] < (*allVarList)[currentChrId].size() && a_rVariantList[k].m_alleles[i].m_nStartPos > (*allVarList)[currentChrId][varItr[i]].m_nEndPos)
                varItr[i]++;
            
            if(varItr[i] == (*allVarList)[currentChrId].size())
                continue;
            
            unsigned int secondItr = varItr[i];

            while(secondItr < (*allVarList)[currentChrId].size() && a_rVariantList[k].m_alleles[i].m_nEndPos >= (*allVarList)[currentChrId][secondItr].m_nStartPos)
            {
                if(CUtils::IsOverlap(a_rVariantList[k].m_alleles[i].m_nStartPos,
                             a_rVariantList[k].m_alleles[i].m_nEndPos,
                             (*allVarList)[currentChrId][secondItr].m_nStartPos,
                             (*allVarList)[currentChrId][secondItr].m_nEndPos))
                    tmpoverlapVariants.push_back((*allVarList)[currentChrId][secondItr]);
                secondItr++;
            }
            
            //Trim variants as standard if there is no overlap
            if(tmpoverlapVariants.size() == 0)
                a_rVariantList[k].TrimVariant(i, m_config.m_bTrimBeginningFirst);
            
            else
            {
                for(unsigned int ovarItr = 0; ovarItr < tmpoverlapVariants.size(); ovarItr++)
                {
                    //Check each allele of overlapping variant
                    for(int tmpItr = 0; tmpItr < 2; tmpItr++)
                    {
                        //If the allele does not overlap, continue
                        if(!CUtils::IsOverlap(a_rVariantList[k].m_alleles[i].m_nStartPos,
                                     a_rVariantList[k].m_alleles[i].m_nEndPos,
                                     tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nStartPos,
                                     tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nEndPos))
                            continue;
                        
                        unsigned int overlapStart = std::max(a_rVariantList[k].m_alleles[i].m_nStartPos, tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nStartPos);
                        unsigned int overlapEnd = std::min(a_rVariantList[k].m_alleles[i].m_nEndPos, tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nEndPos);
                        
                        //Trim from beginning
                        if(a_rVariantList[k].m_alleles[i].m_nStartPos == (int)overlapStart)
                        {
                            if(a_rVariantList[k].m_alleles[i].m_nStartPos + (int)canTrimStart >= tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nEndPos)
                            {
                                int toClip = overlapEnd - a_rVariantList[k].m_alleles[i].m_nStartPos;
                                if(toClip > 0 && toClip <= (int)canTrimStart)
                                {
                                    a_rVariantList[k].TrimVariant(i, toClip, 0);
                                    canTrimStart -= toClip;
                                    continue;
                                }
                            }
                        }
                        
                        //Trim from beginning or end --
                        else if(a_rVariantList[k].m_alleles[i].m_nEndPos > (int) overlapEnd)
                        {
                            //Try to trim from end
                            if(a_rVariantList[k].m_alleles[i].m_nEndPos - (int)canTrimEnd <= tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nStartPos)
                            {
                                int toClip = a_rVariantList[k].m_alleles[i].m_nEndPos - overlapStart;
                                if(toClip > 0 && toClip <= (int)canTrimEnd)
                                {
                                    a_rVariantList[k].TrimVariant(i, 0, toClip);
                                    canTrimEnd -= toClip;
                                    continue;
                                }
                            }
                            
                            //Try to trim from front
                            if(a_rVariantList[k].m_alleles[i].m_nStartPos + (int)canTrimStart >= tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nEndPos)
                            {
                                int toClip = overlapEnd - a_rVariantList[k].m_alleles[i].m_nStartPos;
                                if(toClip > 0 && toClip <= (int)canTrimStart)
                                {
                                    a_rVariantList[k].TrimVariant(i, toClip, 0);
                                    canTrimStart -= toClip;
                                    continue;
                                }
                            }

                        }
                        
                        //Trim from end
                        else
                        {
                            if(a_rVariantList[k].m_alleles[i].m_nEndPos - (int)canTrimEnd < tmpoverlapVariants[ovarItr].m_alleles[tmpItr].m_nStartPos)
                            {
                                int toClip = a_rVariantList[k].m_alleles[i].m_nEndPos - overlapStart;
                                if(toClip > 0 && toClip <= (int)canTrimEnd)
                                {
                                    a_rVariantList[k].TrimVariant(i, 0, toClip);
                                    canTrimEnd -= toClip;
                                    continue;
                                }
                            }
                        }
                    }
                }
                
                if(!a_rVariantList[k].m_alleles[i].m_bIsTrimmed)
                    a_rVariantList[k].TrimVariant(i, m_config.m_bTrimBeginningFirst);
                else
                    a_rVariantList[k].TrimVariant(i, canTrimStart, canTrimEnd);
            }
        }
        
        
        //Update Variant Range
        int minStart = INT_MAX;
        int maxEnd = -1;
        
        for(int i =0; i < a_rVariantList[k].m_nAlleleCount; i++)
        {
            if(!a_rVariantList[k].m_alleles[i].m_bIsIgnored)
            {
                maxEnd = std::max(maxEnd, static_cast<int>(a_rVariantList[k].m_alleles[i].m_nEndPos));
                minStart = std::min(minStart, static_cast<int>(a_rVariantList[k].m_alleles[i].m_nStartPos));
            }
        }
        
        a_rVariantList[k].m_nEndPos = maxEnd == -1 ? a_rVariantList[k].m_nOriginalPos : maxEnd;
        a_rVariantList[k].m_nStartPos = minStart == INT_MAX ? a_rVariantList[k].m_nOriginalPos : minStart;
        
    }
    
}

void CVariantProvider::AppendTrimmedVariants(std::vector<CVariant>& a_rVariantList, EVcfName a_uFrom)
{
    std::vector<std::vector<CVariant>>& variantList = (a_uFrom == eCALLED) ? m_aCalledVariantList : m_aBaseVariantList;
    
    for(unsigned int k = 0; k < a_rVariantList.size(); k++)
        variantList[a_rVariantList[k].m_nChrId].push_back(a_rVariantList[k]);
}
