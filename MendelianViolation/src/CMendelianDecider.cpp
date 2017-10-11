//
//  CMendelianDecider.cpp
//  VariantBenchmarkingTools
//
//  Created by Berke.Toptas on 8/24/17.
//  Copyright © 2017 Seven Bridges Genomics. All rights reserved.
//

#include "CMendelianDecider.h"
#include "CMendelianVariantProvider.h"
#include "SChrIdTriplet.h"
#include "CVariantIterator.h"
#include "ENoCallMode.h"
#include <iostream>
#include "CMendelianResultLog.h"
#include <algorithm>

using namespace mendelian;

CMendelianDecider::CMendelianDecider(std::vector<core::CPath>& a_aBestPathsFatherChildGT,
                                    std::vector<core::CPath>& a_aBestPathsFatherChildAM,
                                     std::vector<core::CPath>& a_aBestPathsMotherChildGT,
                                     std::vector<core::CPath>& a_aBestPathsMotherChildAM,
                                     CMendelianVariantProvider& a_rProvider,
                                     CMendelianResultLog& a_rResultLog) :
m_aBestPathsFatherChildGT(a_aBestPathsFatherChildGT),
m_aBestPathsFatherChildAM(a_aBestPathsFatherChildAM),
m_aBestPathsMotherChildGT(a_aBestPathsMotherChildGT),
m_aBestPathsMotherChildAM(a_aBestPathsMotherChildAM),
m_provider(a_rProvider),
m_resultLog(a_rResultLog)
{
}

void CMendelianDecider::SetNocallMode(ENoCallMode a_nMode)
{
    m_nocallMode = a_nMode;
}


//Checks if the given two range is overlapping
bool isOverlap(int left1, int right1, int left2, int right2)
{
    //If the interval length is 0 (eg. 974791-974791) we need to check if the boundaries matches
    if(right1-left1 == 0 || right2-left2 == 0)
        return left1==left2 || right1 == right2;
    else
        return std::min(right1, right2) - std::max(left1, left2) > 0;
}

//Compare variants according to id for sort operation
bool variantCompare(const CVariant* v1, const CVariant* v2)
{
    return v1->m_nId < v2->m_nId;
}

void CMendelianDecider::CheckUniqueVars(EMendelianVcfName a_checkSide, SChrIdTriplet& a_rTriplet,
                                        const std::vector<const CVariant*>& a_rVariantList,
                                        std::vector<bool>& a_rSideDecisions,
                                        const std::vector<EMendelianDecision>& a_rParentSelfDecisions,
                                        const std::vector<EMendelianDecision>& a_rParentOtherDecisions,
                                        const std::vector<EMendelianDecision>& a_rChildDecisions)
{
    std::vector<const CVariant*> varListToCheckChild = m_provider.GetVariantList(eCHILD, a_rTriplet.m_nCid);
    std::vector<const CVariant*> varListToCheckParent = a_checkSide == eMOTHER ? m_provider.GetVariantList(eFATHER, a_rTriplet.m_nFid) : m_provider.GetVariantList(eMOTHER, a_rTriplet.m_nMid);
    std::vector<const CVariant*> varListToCheckSelf = a_checkSide == eFATHER ? m_provider.GetVariantList(eFATHER, a_rTriplet.m_nFid) : m_provider.GetVariantList(eMOTHER, a_rTriplet.m_nMid);
    
    
    int varItrChild = 0;
    int varItrParent = 0;
    int varItrSelf = 0;
    
    for(int k = 0; k < (int)a_rVariantList.size(); k++)
    {
        //Check if the variant is already marked
        if(a_rParentSelfDecisions[a_rVariantList[k]->m_nId] != eUnknown)
        {
            a_rSideDecisions[k] = a_rParentSelfDecisions[a_rVariantList[k]->m_nId] == eCompliant ? true : false;
            continue;
        }
        
        //If variant does not have 0 allele
        else if(a_rVariantList[k]->m_genotype[0] != 0 && a_rVariantList[k]->m_genotype[1] != 0)
        {
            a_rSideDecisions[k] = false;
            continue;
        }
        
        //Skip child variants until end position of child variant is more than our current Parent variant start position
        while(varItrChild <  (int)varListToCheckChild.size() && varListToCheckChild[varItrChild]->m_nEndPos < a_rVariantList[k]->m_nStartPos)
            varItrChild++;
        
        //Skip parent variants until end position of child variant is more than our current Parent variant start position
        while(varItrParent <  (int)varListToCheckParent.size() && varListToCheckParent[varItrParent]->m_nEndPos < a_rVariantList[k]->m_nStartPos)
            varItrParent++;
        
        
        //Skip parent variants until end position of child variant is more than our current Parent variant start position
        while(varItrSelf <  (int)varListToCheckSelf.size() && varListToCheckSelf[varItrSelf]->m_nEndPos < a_rVariantList[k]->m_nStartPos)
            varItrSelf++;
        
        
        int counterChild = 0;
        int counterParent = 0;
        int counterSelf = 0;
        
        std::vector<const CVariant*> childSideMatches;
        std::vector<const CVariant*> parentSideMatches;
        std::vector<const CVariant*> selfSideMatches;
        
        //Get All overlapping variants in child side
        while(varItrChild + counterChild < (int)varListToCheckChild.size() && varListToCheckChild[varItrChild+counterChild]->m_nStartPos < a_rVariantList[k]->m_nEndPos)
        {
            if(isOverlap(a_rVariantList[k]->m_nStartPos, a_rVariantList[k]->m_nEndPos, varListToCheckChild[varItrChild+counterChild]->m_nStartPos, varListToCheckChild[varItrChild+counterChild]->m_nEndPos))
                childSideMatches.push_back(varListToCheckChild[varItrChild+counterChild]);
            counterChild++;
        }
        
        //Get All overlapping variants in other parent side
        while(varItrParent + counterParent < (int)varListToCheckParent.size() && varListToCheckParent[varItrParent+counterParent]->m_nStartPos < a_rVariantList[k]->m_nEndPos)
        {
            if(isOverlap(a_rVariantList[k]->m_nStartPos, a_rVariantList[k]->m_nEndPos, varListToCheckParent[varItrParent+counterParent]->m_nStartPos, varListToCheckParent[varItrParent+counterParent]->m_nEndPos))
                parentSideMatches.push_back(varListToCheckParent[varItrParent+counterParent]);
            counterParent++;
        }
        
        //Get All overlapping variants in self side
        while(varItrSelf + counterSelf < (int)varListToCheckSelf.size() && varListToCheckSelf[varItrSelf+counterSelf]->m_nStartPos < a_rVariantList[k]->m_nEndPos)
        {
            if(isOverlap(a_rVariantList[k]->m_nStartPos, a_rVariantList[k]->m_nEndPos, varListToCheckSelf[varItrSelf+counterSelf]->m_nStartPos, varListToCheckSelf[varItrSelf+counterSelf]->m_nEndPos))
                selfSideMatches.push_back(varListToCheckSelf[varItrSelf+counterSelf]);
            counterSelf++;
        }
        
        bool selfCheck = true;
        bool parentCheck = true;
        bool childCheck = true;
        
        //=====STEP 1: Check self side=====
        if(selfSideMatches.size() > 2)
            selfCheck = false;
        //check if all variants is 0/x form
        for(unsigned int m=0; m < selfSideMatches.size(); m++)
        {
            if(selfSideMatches[m]->m_genotype[0] != 0 && selfSideMatches[m]->m_genotype[1] != 0)
            {
                selfCheck = false;
                break;
            }
        }
        
        //===== STEP 2: Check the other Parent Side =======
        for(unsigned int m=0; m < parentSideMatches.size(); m++)
        {
            if(a_rParentOtherDecisions[varItrParent+m] == eViolation)
            {
                parentCheck = false;
                break;
            }
            
            else if(parentSideMatches[m]->m_nOriginalPos == a_rVariantList[k]->m_nOriginalPos && parentSideMatches[m]->m_genotype[0] != 0 && parentSideMatches[m]->m_genotype[1] != 0)
            {
                parentCheck = false;
                break;
            }
        }
        
        //===== STEP 3: Check child Side =========
        for(unsigned int m=0; m < childSideMatches.size(); m++)
        {
            if(a_rChildDecisions[varListToCheckChild[varItrChild+m]->m_nId] == eCompliant)
            {
                childCheck = true;
                continue;
            }
            
            else if(a_rChildDecisions[varListToCheckChild[varItrChild+m]->m_nId] == eViolation)
            {
                childCheck = false;
                break;
            }
            
            else if(childSideMatches[m]->m_genotype[0] != 0 && childSideMatches[m]->m_genotype[1] != 0)
            {
                childCheck = false;
                break;
            }
        }
        
        a_rSideDecisions[k] = parentCheck && selfCheck && childCheck;
        
    }
    
}

void CMendelianDecider::GetSyncPointList(SChrIdTriplet& a_rTriplet, bool a_bIsFatherChild, std::vector<core::CSyncPoint>& a_rSyncPointList, bool a_bIsGT)
{
    std::vector<const core::COrientedVariant*> pBaseIncluded;
    std::vector<const core::COrientedVariant*> pCalledIncluded;
    
    std::vector<const CVariant*> pBaseExcluded;
    std::vector<const CVariant*> pCalledExcluded;
    
    core::CPath *pPath;
    core::CPath *pPathSync;
    
    if(a_bIsGT == false)
    {
        pPath = a_bIsFatherChild ? &m_aBestPathsFatherChildAM[a_rTriplet.m_nTripleIndex] : &m_aBestPathsMotherChildAM[a_rTriplet.m_nTripleIndex];
        core::CPath *pPathGT = a_bIsFatherChild ? &m_aBestPathsFatherChildGT[a_rTriplet.m_nTripleIndex] : &m_aBestPathsMotherChildGT[a_rTriplet.m_nTripleIndex];
        pPathSync = pPathGT;
        
        std::vector<const CVariant*> excludedVarsBase = m_provider.GetVariantList(a_bIsFatherChild ? eFATHER : eMOTHER, a_bIsFatherChild ? a_rTriplet.m_nFid : a_rTriplet.m_nMid, pPathGT->m_baseSemiPath.GetExcluded());
        std::vector<const CVariant*> excludedVarsCalled = m_provider.GetVariantList(eCHILD, a_rTriplet.m_nCid, pPathGT->m_calledSemiPath.GetExcluded());
        
        pBaseIncluded = pPath->m_baseSemiPath.GetIncludedVariants();
        pCalledIncluded = pPath->m_calledSemiPath.GetIncludedVariants();
        
        pBaseExcluded = m_provider.GetVariantList(excludedVarsBase, pPath->m_baseSemiPath.GetExcluded());
        pCalledExcluded = m_provider.GetVariantList(excludedVarsCalled, pPath->m_calledSemiPath.GetExcluded());
    }
    else
    {
        pPath = a_bIsFatherChild ? &m_aBestPathsFatherChildGT[a_rTriplet.m_nTripleIndex] : &m_aBestPathsMotherChildGT[a_rTriplet.m_nTripleIndex];
        pPathSync = pPath;
        
        pBaseIncluded = pPath->m_baseSemiPath.GetIncludedVariants();
        pCalledIncluded = pPath->m_calledSemiPath.GetIncludedVariants();
        
        pBaseExcluded = m_provider.GetVariantList(a_bIsFatherChild ? eFATHER : eMOTHER, a_bIsFatherChild ? a_rTriplet.m_nFid : a_rTriplet.m_nMid, pPath->m_baseSemiPath.GetExcluded());
        pCalledExcluded = m_provider.GetVariantList(eCHILD, a_rTriplet.m_nCid, pPath->m_calledSemiPath.GetExcluded());
    }
    
    
    int baseIncludedItr = 0;
    int baseExcludedItr = 0;
    int calledIncludedItr = 0;
    int calledExcludedItr = 0;
    
    for(int k = 0; k < (int)pPathSync->m_aSyncPointList.size(); k++)
    {
        core::CSyncPoint ssPoint;
        ssPoint.m_nStartPosition = k > 0 ? pPathSync->m_aSyncPointList[k-1] : 0;
        ssPoint.m_nEndPosition = pPathSync->m_aSyncPointList[k];
        ssPoint.m_nIndex = k;
        int bound = ssPoint.m_nStartPosition == ssPoint.m_nEndPosition ? 1 : 0;
        
        while(baseIncludedItr < (int)pBaseIncluded.size() && pBaseIncluded[baseIncludedItr]->GetStartPos() < (pPathSync->m_aSyncPointList[k] + bound))
        {
            const core::COrientedVariant* pOvar = pBaseIncluded[baseIncludedItr];
            ssPoint.m_baseVariantsIncluded.push_back(pOvar);
            baseIncludedItr++;
        }
        
        while(calledIncludedItr < (int)pCalledIncluded.size() && pCalledIncluded[calledIncludedItr]->GetStartPos() < (pPathSync->m_aSyncPointList[k] + bound))
        {
            const core::COrientedVariant* pOvar = pCalledIncluded[calledIncludedItr];
            ssPoint.m_calledVariantsIncluded.push_back(pOvar);
            calledIncludedItr++;
        }
        
        while(baseExcludedItr < (int)pBaseExcluded.size() && pBaseExcluded[baseExcludedItr]->m_nStartPos < (pPathSync->m_aSyncPointList[k] + bound))
        {
            ssPoint.m_baseVariantsExcluded.push_back(pBaseExcluded[baseExcludedItr]);
            baseExcludedItr++;
        }
        
        while(calledExcludedItr < (int)pCalledExcluded.size() && pCalledExcluded[calledExcludedItr]->m_nStartPos < (pPathSync->m_aSyncPointList[k] + bound))
        {
            ssPoint.m_calledVariantsExcluded.push_back(pCalledExcluded[calledExcludedItr]);
            calledExcludedItr++;
        }
        
        a_rSyncPointList.push_back(ssPoint);
    }
    
    //Add Remaining variants to the last syncPoint
    core::CSyncPoint sPoint;
    sPoint.m_nStartPosition = pPathSync->m_aSyncPointList[pPathSync->m_aSyncPointList.size()-1];
    sPoint.m_nEndPosition = INT_MAX;
    sPoint.m_nIndex = static_cast<int>(pPathSync->m_aSyncPointList.size()-1);
    
    while(baseIncludedItr < (int)pBaseIncluded.size() && pBaseIncluded[baseIncludedItr]->GetStartPos() <= sPoint.m_nEndPosition)
    {
        sPoint.m_baseVariantsIncluded.push_back(pBaseIncluded[baseIncludedItr]);
        baseIncludedItr++;
    }
    while(calledIncludedItr < (int)pCalledIncluded.size() && pCalledIncluded[calledIncludedItr]->GetStartPos() <= sPoint.m_nEndPosition)
    {
        sPoint.m_calledVariantsIncluded.push_back(pCalledIncluded[calledIncludedItr]);
        calledIncludedItr++;
    }
    
    while(baseExcludedItr < (int)pBaseExcluded.size() && pBaseExcluded[baseExcludedItr]->m_nStartPos <= sPoint.m_nEndPosition)
    {
        sPoint.m_baseVariantsExcluded.push_back(pBaseExcluded[baseExcludedItr]);
        baseExcludedItr++;
    }
    
    while(calledExcludedItr < (int)pCalledExcluded.size() && pCalledExcluded[calledExcludedItr]->m_nStartPos <= sPoint.m_nEndPosition)
    {
        sPoint.m_calledVariantsExcluded.push_back(pCalledExcluded[calledExcludedItr]);
        calledExcludedItr++;
    }
    a_rSyncPointList.push_back(sPoint);
    
}

void CMendelianDecider::CheckFor0PathFor00(SChrIdTriplet& a_rTriplet,
                                            bool a_bIsFatherChild,
                                            std::vector<const CVariant*>& a_rVarList,
                                            std::vector<const CVariant*>& a_rViolationList,
                                            std::vector<const CVariant*>& a_rCompliantList)
{
    //Get sync point list
    std::vector<core::CSyncPoint> a_rSyncPointList;
    GetSyncPointList(a_rTriplet, a_bIsFatherChild, a_rSyncPointList, true);
    
    int varlistItr = 0;
    for(int k = 0; k < (int)a_rSyncPointList.size() && varlistItr < (int)a_rVarList.size(); k++)
    {
        //If we check that syncpoint
        bool bDoCheck = false;
        std::vector<const CVariant*> tmpVarList;
        
        //Exclude variant if we somehow skip the syncpoint intervals
        while(varlistItr < (int)a_rVarList.size() -1 && a_rSyncPointList[k].m_nStartPosition > a_rVarList[varlistItr]->m_nStartPos)
        {
            a_rViolationList.push_back(a_rVarList[varlistItr]);
            varlistItr++;
        }
        
        //Check if the sync interval contains 0/x child variants
        for(int m = 0; m < (int)a_rSyncPointList[k].m_calledVariantsIncluded.size() && varlistItr != (int)a_rVarList.size(); m++)
        {
            if(a_rSyncPointList[k].m_calledVariantsIncluded[m]->GetVariant().m_nId == a_rVarList[varlistItr]->m_nId)
            {
                tmpVarList.push_back(a_rVarList[varlistItr]);
                varlistItr++;
                bDoCheck = true;
            }
        }
        
        if(true == bDoCheck)
        {
            bool bIsCompliant = true;
            
            for(int m = 0; m < (int)a_rSyncPointList[k].m_baseVariantsExcluded.size(); m++)
            {
                const CVariant* pVar = a_rSyncPointList[k].m_baseVariantsExcluded[m];
                
                if(isOverlap(pVar->GetStart(), pVar->GetEnd(), tmpVarList[0]->GetStart(), tmpVarList[0]->GetEnd()))
                {
                    if(pVar->m_genotype[0] != 0 && pVar->m_genotype[1] != 0)
                    {
                        bIsCompliant = false;
                        break;
                    }
                }
            }
            
            if(bIsCompliant == true)
            {
                for(int aa = 0; aa < (int)tmpVarList.size(); aa++)
                    a_rCompliantList.push_back(tmpVarList[aa]);
            }
            
            else
            {
                for(int aa = 0; aa < (int)tmpVarList.size(); aa++)
                    a_rViolationList.push_back(tmpVarList[aa]);
            }
        }
    }
    
}


void CMendelianDecider::CheckFor0Path(SChrIdTriplet& a_rTriplet,
                                       bool a_bIsFatherChild,
                                       std::vector<const CVariant *> &a_pVarList,
                                       std::vector<const CVariant *> &a_pViolantList,
                                       std::vector<const CVariant *> &a_pCompliantList,
                                       std::vector<EMendelianDecision>& a_rParentDecisions,
                                       bool a_bIsUpdateDecisionList)
{
    
    //Get sync point list
    std::vector<core::CSyncPoint> a_rSyncPointList;
    GetSyncPointList(a_rTriplet, a_bIsFatherChild, a_rSyncPointList);
    
    
    int varlistItr = 0;
    for(int k = 0; k < (int)a_rSyncPointList.size() && varlistItr < (int)a_pVarList.size(); k++)
    {
        //If we check that syncpoint
        bool bDoCheck = false;
        std::vector<const CVariant*> tmpVarList;
        
        //Exclude variant if we somehow skip the syncpoint intervals
        while(varlistItr < (int)a_pVarList.size() -1 && a_rSyncPointList[k].m_nStartPosition > a_pVarList[varlistItr]->m_nStartPos)
        {
            a_pViolantList.push_back(a_pVarList[varlistItr]);
            varlistItr++;
        }
        
        //Check if the sync interval contains 0/x child variants
        for(int m = 0; m < (int)a_rSyncPointList[k].m_calledVariantsExcluded.size() && varlistItr != (int)a_pVarList.size(); m++)
        {
            if(a_rSyncPointList[k].m_calledVariantsExcluded[m]->m_nId == a_pVarList[varlistItr]->m_nId)
            {
                tmpVarList.push_back(a_pVarList[varlistItr]);
                varlistItr++;
                bDoCheck = true;
            }
        }
        
        if(true == bDoCheck)
        {
            bool bIsCompliant = true;
            
            for(int m = 0; m < (int)a_rSyncPointList[k].m_baseVariantsExcluded.size(); m++)
            {
                const CVariant* pVar = a_rSyncPointList[k].m_baseVariantsExcluded[m];
                
                if(isOverlap(pVar->GetStart(), pVar->GetEnd(), tmpVarList[0]->GetStart(), tmpVarList[0]->GetEnd()))
                {
                    
                    if(pVar->m_genotype[0] != 0 && pVar->m_genotype[1] != 0)
                    {
                        if(a_bIsUpdateDecisionList)
                        {
                            //We are marking decision of mother/father variant as violation
                            if(true == a_bIsFatherChild)
                                a_rParentDecisions[pVar->m_nId] = eViolation;
                        }
                        
                        bIsCompliant = false;
                        break;
                    }
                    
                    else
                    {
                        if(a_bIsUpdateDecisionList)
                        {
                            //We are marking decision of mother/father variant as compliant
                            if(true == a_bIsFatherChild)
                                a_rParentDecisions[pVar->m_nId] = eCompliant;
                        }
                    }
                }
            }
            
            if(bIsCompliant == true)
            {
                for(int aa = 0; aa < (int)tmpVarList.size(); aa++)
                    a_pCompliantList.push_back(tmpVarList[aa]);
            }
            
            else
            {
                for(int aa = 0; aa < (int)tmpVarList.size(); aa++)
                    a_pViolantList.push_back(tmpVarList[aa]);
            }
        }
    }
    
}

void CMendelianDecider::CheckFor00Child(SChrIdTriplet& a_rTriplet,
                                         std::vector<const CVariant*>& a_rOvarList,
                                         std::vector<const CVariant*>& a_rViolationList,
                                         std::vector<const CVariant*>& a_rCompliantList,
                                         bool a_bIsGTMatch,
                                        std::vector<EMendelianDecision>& a_rMotherDecisions,
                                        std::vector<EMendelianDecision>& a_rFatherDecisions)
{
    std::vector<const CVariant*> fatherCompliants;
    std::vector<const CVariant*> motherCompliants;
    std::vector<const CVariant*> fatherViolants;
    std::vector<const CVariant*> motherViolants;
    
    std::sort(a_rOvarList.begin(), a_rOvarList.end(), variantCompare);
    
    if(false == a_bIsGTMatch)
    {
        CheckFor0Path(a_rTriplet, true,  a_rOvarList, fatherViolants, fatherCompliants, a_rFatherDecisions, false);
        CheckFor0Path(a_rTriplet, false, a_rOvarList, motherViolants, motherCompliants, a_rMotherDecisions, false);
    }
    else
    {
        CheckFor0PathFor00(a_rTriplet, true,  a_rOvarList, fatherViolants, fatherCompliants);
        CheckFor0PathFor00(a_rTriplet, false, a_rOvarList, motherViolants, motherCompliants);
    }
    
    //Intersect father and mother compliant variants and write to a_rCompliantList
    for (std::vector<const CVariant*>::iterator i = fatherCompliants.begin(); i != fatherCompliants.end(); ++i)
    {
        if (std::find(motherCompliants.begin(), motherCompliants.end(), *i) != motherCompliants.end())
        {
            a_rCompliantList.push_back(*i);
        }
    }
    
    //Write all other variants to a_rViolationList
    for (std::vector<const CVariant*>::iterator i = a_rOvarList.begin(); i != a_rOvarList.end(); ++i)
    {
        if (std::find(a_rCompliantList.begin(), a_rCompliantList.end(), *i) == a_rCompliantList.end())
        {
            a_rViolationList.push_back(*i);
        }
    }
    
}

void CMendelianDecider::AssignDecisionToParentVars(EMendelianVcfName a_checkSide, SChrIdTriplet& a_rTriplet, std::vector<EMendelianDecision>& a_rParentDecisions, std::vector<EMendelianDecision>& a_rChildDecisions)
{
    std::vector<const CVariant*> varListToCheckParent = a_checkSide == eMOTHER ? m_provider.GetVariantList(eMOTHER, a_rTriplet.m_nMid) : m_provider.GetVariantList(eFATHER, a_rTriplet.m_nFid);
    
    assert(varListToCheckParent.size() == a_rParentDecisions.size());
    
    //Get sync point list
    std::vector<core::CSyncPoint> aSyncPointList;
    bool bIsFatherChild = a_checkSide == eFATHER ? true : false;
    GetSyncPointList(a_rTriplet, bIsFatherChild, aSyncPointList);
    
    //Iterator to synchronization point list
    unsigned int itrSyncPList = 0;
    
    for(unsigned int k = 0; k < varListToCheckParent.size(); k++)
    {
        //Skipped assigned variants
        if(a_rParentDecisions[varListToCheckParent[k]->m_nId] != eUnknown)
            continue;
        
        //Skip irrelevant sync points
        while(itrSyncPList < aSyncPointList.size() && aSyncPointList[itrSyncPList].m_nEndPosition <= varListToCheckParent[k]->m_nStartPos)
            itrSyncPList++;
        
        //Terminate if the sync point list is ended
        if(itrSyncPList == aSyncPointList.size())
            break;
        
        bool bIsViolationFound = false;
        
        if(aSyncPointList[itrSyncPList].m_calledVariantsIncluded.size() == 0 && aSyncPointList[itrSyncPList].m_calledVariantsExcluded.size() == 0)
        {
            if(varListToCheckParent[k]->m_variantStatus == eNO_MATCH)
            {
                bIsViolationFound = true;
                a_rParentDecisions[varListToCheckParent[k]->m_nId] = eViolation;
            }
        }
        
        for(unsigned int m = 0; m < aSyncPointList[itrSyncPList].m_calledVariantsIncluded.size(); m++)
        {
            if(a_rChildDecisions[aSyncPointList[itrSyncPList].m_calledVariantsIncluded[m]->GetVariant().m_nId] == eViolation)
            {
                a_rParentDecisions[varListToCheckParent[k]->m_nId] = eViolation;
                bIsViolationFound = true;
                break;
            }
        }
        
        for(unsigned int m = 0; m < aSyncPointList[itrSyncPList].m_calledVariantsExcluded.size(); m++)
        {
            if(a_rChildDecisions[aSyncPointList[itrSyncPList].m_calledVariantsExcluded[m]->m_nId] == eViolation)
            {
                a_rParentDecisions[varListToCheckParent[k]->m_nId] = eViolation;
                bIsViolationFound = true;
                break;
            }
        }
        
        if(!bIsViolationFound)
            a_rParentDecisions[varListToCheckParent[k]->m_nId] = eCompliant;
    }
}

void CMendelianDecider::ReportChildChromosomeData(SChrIdTriplet& a_rTriplet, std::vector<const CVariant*>& a_rCompliants, std::vector<const CVariant*>& a_rViolations)
{
    int compliantSNPcount = 0;
    int compliantINDELcount = 0;
    int violationSNPcount = 0;
    int violationINDELcount = 0;
    
    for(const CVariant* pVar : a_rCompliants)
    {
        if(pVar->GetVariantType() == eSNP)
            compliantSNPcount++;
        else
            compliantINDELcount++;
    }
    
    for(const CVariant* pVar : a_rViolations)
    {
        if(pVar->GetVariantType() == eSNP)
            violationSNPcount++;
        else
            violationINDELcount++;
    }
    
    m_resultLog.LogShortReport(a_rTriplet.m_chrName, compliantSNPcount, violationSNPcount, compliantINDELcount, violationINDELcount);
}

void CMendelianDecider::MergeFunc(SChrIdTriplet& a_triplet,
                                  std::vector<EMendelianDecision>& a_rMotherDecisions,
                                  std::vector<EMendelianDecision>& a_rFatherDecisions,
                                  std::vector<EMendelianDecision>& a_rChildDecisions)
{
    std::vector<const CVariant*> compliants;
    std::vector<const CVariant*> violations;
    
    std::vector<const CVariant*> SameAlleleMatchViolationVars;
    std::vector<const CVariant*> MendelianCompliantVars;
    
    std::vector<const CVariant*> motherChildOnly;
    std::vector<const CVariant*> fatherChildOnly;
    
    std::vector<const CVariant*> check0atMotherSide;
    std::vector<const CVariant*> check0atFatherSide;
    std::vector<const CVariant*> check00Child;
    std::vector<const CVariant*> childUniqueList;
    std::vector<const CVariant*> check00ChildGTMatched;
    
    //Sort variants according to variant ids
    m_aBestPathsFatherChildGT[a_triplet.m_nTripleIndex].SortIncludedVariants();
    m_aBestPathsFatherChildAM[a_triplet.m_nTripleIndex].SortIncludedVariants();
    m_aBestPathsMotherChildGT[a_triplet.m_nTripleIndex].SortIncludedVariants();
    m_aBestPathsMotherChildAM[a_triplet.m_nTripleIndex].SortIncludedVariants();
    
    //Included lists of child
    CVariantIterator FatherChildVariants(m_aBestPathsFatherChildGT[a_triplet.m_nTripleIndex].m_calledSemiPath.GetIncludedVariants(),
                                         m_aBestPathsFatherChildAM[a_triplet.m_nTripleIndex].m_calledSemiPath.GetIncludedVariants());
    
    CVariantIterator MotherChildVariants(m_aBestPathsMotherChildGT[a_triplet.m_nTripleIndex].m_calledSemiPath.GetIncludedVariants(),
                                         m_aBestPathsMotherChildAM[a_triplet.m_nTripleIndex].m_calledSemiPath.GetIncludedVariants());
    
    //Check if the two list have common variants
    if(FatherChildVariants.hasNext() == false && MotherChildVariants.hasNext() == false)
        return;
    
    const core::COrientedVariant* varMC = (MotherChildVariants.hasNext() ? MotherChildVariants.Next() : NULL);
    const core::COrientedVariant* varFC = (FatherChildVariants.hasNext() ? FatherChildVariants.Next() : NULL);
    
    while(varMC != NULL && varFC != NULL)
    {
        //If Mother variant and Father variant is Common check for same allele match condition
        if(varMC->GetVariant().m_nId == varFC->GetVariant().m_nId)
        {
            if(varMC->GetVariant().m_genotype[0] == 0 && varMC->GetVariant().m_genotype[1] == 0)
                check00ChildGTMatched.push_back(&varMC->GetVariant());
            
            else if(varMC->GetVariant().m_bIsHeterozygous)
            {
                //ELIMINATE SAME ALLELE MATCHING(EXCEPTION 1)
                if(varMC->GetAlleleIndex() == varFC->GetAlleleIndex())
                {
                    if(varMC->GetVariant().m_variantStatus == eGENOTYPE_MATCH
                       ||
                       varFC->GetVariant().m_variantStatus == eGENOTYPE_MATCH)
                        MendelianCompliantVars.push_back(&varMC->GetVariant());
                    else
                        SameAlleleMatchViolationVars.push_back(&varMC->GetVariant());
                }
                else
                    MendelianCompliantVars.push_back(&varMC->GetVariant());
            }
            else
                MendelianCompliantVars.push_back(&varMC->GetVariant());
            
            if(MotherChildVariants.hasNext() && FatherChildVariants.hasNext())
            {
                varMC = MotherChildVariants.Next();
                varFC = FatherChildVariants.Next();
            }
            
            else
            {
                varMC = NULL;
                varFC = NULL;
                break;
            }
        }
        
        //If we have variant match with Father side only, we filter 0/x variants and rest of them are marked as violation
        else if(varMC->GetVariant().m_nId > varFC->GetVariant().m_nId)
        {
            if(varFC->GetVariant().m_bIsHeterozygous)
            {
                if(varFC->GetVariant().m_genotype[0] == 0 || varFC->GetVariant().m_genotype[1] == 0)
                    check0atMotherSide.push_back(&varFC->GetVariant());
                else
                    fatherChildOnly.push_back(&varFC->GetVariant());
            }
            else
            {
                if(varFC->GetVariant().m_genotype[0] == 0)
                    check0atMotherSide.push_back(&varFC->GetVariant());
                else
                    fatherChildOnly.push_back(&varFC->GetVariant());
            }
            
            if(FatherChildVariants.hasNext())
                varFC = FatherChildVariants.Next();
            else
            {
                varFC = NULL;
                break;
            }
        }
        
        //If we have variant match with Mother side only, we filter 0/x variants and rest of them are marked as violation
        else
        {
            if(varMC->GetVariant().m_bIsHeterozygous)
            {
                if(varMC->GetVariant().m_genotype[0] == 0 || varMC->GetVariant().m_genotype[1] == 0)
                    check0atFatherSide.push_back(&varMC->GetVariant());
                else
                    motherChildOnly.push_back(&varMC->GetVariant());
            }
            else
            {
                if(varMC->GetVariant().m_genotype[0] == 0)
                    check0atFatherSide.push_back(&varMC->GetVariant());
                else
                    motherChildOnly.push_back(&varMC->GetVariant());
            }
            
            if(MotherChildVariants.hasNext())
                varMC = MotherChildVariants.Next();
            else
            {
                varMC = NULL;
                break;
            }
        }
        
    }
    
    
    //Process remaining vars in FatherChild explained as above
    while(true)
    {
        if(varFC == NULL)
        {
            if(FatherChildVariants.hasNext())
                varFC = FatherChildVariants.Next();
            else
                break;
        }
        
        if(varFC->GetVariant().m_bIsHeterozygous)
        {
            if(varFC->GetVariant().m_genotype[0] == 0 || varFC->GetVariant().m_genotype[1] == 0)
                check0atMotherSide.push_back(&varFC->GetVariant());
            else
                fatherChildOnly.push_back(&varFC->GetVariant());
        }
        else
        {
            if(varFC->GetVariant().m_genotype[0] == 0)
                check0atMotherSide.push_back(&varFC->GetVariant());
            else
                fatherChildOnly.push_back(&varFC->GetVariant());
        }
        
        if(!FatherChildVariants.hasNext())
            break;
        else
            varFC = FatherChildVariants.Next();
    }
    
    //Process remaining vars in MotherChild explined as above
    while(true)
    {
        if(varMC == NULL)
        {
            if(MotherChildVariants.hasNext())
                varMC = MotherChildVariants.Next();
            else
                break;
        }
        
        if(varMC->GetVariant().m_bIsHeterozygous)
        {
            if(varMC->GetVariant().m_genotype[0] == 0 || varMC->GetVariant().m_genotype[1] == 0)
                check0atFatherSide.push_back(&varMC->GetVariant());
            else
                motherChildOnly.push_back(&varMC->GetVariant());
        }
        else
        {
            if(varMC->GetVariant().m_genotype[0] == 0)
                check0atFatherSide.push_back(&varMC->GetVariant());
            else
                motherChildOnly.push_back(&varMC->GetVariant());
        }
        
        
        if(!MotherChildVariants.hasNext())
            break;
        else
            varMC = MotherChildVariants.Next();
    }
    
    std::vector<const CVariant*> compliantVarsFrom0CheckMother;
    std::vector<const CVariant*> compliantVarsFrom0CheckFather;
    std::vector<const CVariant*> violationVarsFrom0CheckMother;
    std::vector<const CVariant*> violationVarsFrom0CheckFather;
    
    //Check for 0/x child variant set at father side
    CheckFor0Path(a_triplet, true, check0atFatherSide, violationVarsFrom0CheckFather, compliantVarsFrom0CheckFather, a_rFatherDecisions);
    //Check for 0/x child variant set at the mother side
    CheckFor0Path(a_triplet, false, check0atMotherSide, violationVarsFrom0CheckMother, compliantVarsFrom0CheckMother, a_rMotherDecisions);
    
    
    std::vector<const CVariant*> compliantVarsFrom00CheckGT;
    std::vector<const CVariant*> violationVarsFrom00CheckGT;
    
    //Check for 0/0 child variant set for both parent
    //CheckFor00Child(a_triplet, check00ChildGTMatched, violationVarsFrom00CheckGT, compliantVarsFrom00CheckGT, true);
    
    //Gather all compliant variants of child we found so far
    compliants.insert(std::end(compliants), std::begin(MendelianCompliantVars), std::end(MendelianCompliantVars));
    compliants.insert(std::end(compliants), std::begin(compliantVarsFrom0CheckFather), std::end(compliantVarsFrom0CheckFather));
    compliants.insert(std::end(compliants), std::begin(compliantVarsFrom0CheckMother), std::end(compliantVarsFrom0CheckMother));
    compliants.insert(std::end(compliants), std::begin(compliantVarsFrom00CheckGT), std::end(compliantVarsFrom00CheckGT));
    std::sort(compliants.begin(), compliants.end(), variantCompare);
    
    //Gather all violation variants of child we found so far
    violations.insert(std::end(violations), std::begin(SameAlleleMatchViolationVars), std::end(SameAlleleMatchViolationVars));
    violations.insert(std::end(violations), std::begin(violationVarsFrom0CheckFather), std::end(violationVarsFrom0CheckFather));
    violations.insert(std::end(violations), std::begin(violationVarsFrom0CheckMother), std::end(violationVarsFrom0CheckMother));
    violations.insert(std::end(violations), std::begin(fatherChildOnly), std::end(fatherChildOnly));
    violations.insert(std::end(violations), std::begin(motherChildOnly), std::end(motherChildOnly));
    violations.insert(std::end(violations), std::begin(violationVarsFrom00CheckGT), std::end(violationVarsFrom00CheckGT));
    std::sort(violations.begin(), violations.end(), variantCompare);
    
    //Find Child Unique variants
    std::vector<const CVariant*> childVariants = m_provider.GetSortedVariantList(eCHILD, a_triplet.m_nCid);
    std::vector<int>childProcessedArray(childVariants.size());
    for(int k = 0; k <  (int)childProcessedArray.size(); k++)
        childProcessedArray[k] = 0;
    
    //Mark mendelian compliant vars as processed
    for(int k = 0; k < (int)compliants.size(); k++)
    {
        childProcessedArray[compliants[k]->m_nId]++;
    }
    //Mark mendelian violation vars as processed
    for(int k = 0; k < (int)violations.size(); k++)
    {
        childProcessedArray[violations[k]->m_nId]++;
    }
    
    for(int childItr = 0; childItr < (int)childProcessedArray.size(); childItr++)
    {
        if(childProcessedArray[childItr] == 0)
        {
            if(childVariants[childItr]->m_genotype[0] == 0 && childVariants[childItr]->m_genotype[1] == 0)
            {
                check00Child.push_back(childVariants[childItr]);
            }
            else
            {
                const CVariant* pVar = childVariants[childItr];
                childUniqueList.push_back(pVar);
            }
        }
    }
    
    std::vector<const CVariant*> compliantVarsFrom00Check;
    std::vector<const CVariant*> violationVarsFrom00Check;
    
    //Check for 0/0 unique child variants for both parent
    CheckFor00Child(a_triplet, check00Child, violationVarsFrom00Check, compliantVarsFrom00Check, false, a_rMotherDecisions, a_rFatherDecisions);
    
    //Add the new compliants we found to compliants list
    compliants.insert(std::end(compliants), std::begin(compliantVarsFrom00Check), std::end(compliantVarsFrom00Check));
    std::sort(compliants.begin(), compliants.end(), variantCompare);
    
    //Add the new violations we found to violation list
    violations.insert(std::end(violations), std::begin(violationVarsFrom00Check), std::end(violationVarsFrom00Check));
    violations.insert(std::end(violations), std::begin(childUniqueList), std::end(childUniqueList));
    std::sort(violations.begin(), violations.end(), variantCompare);
    
    //We looked up all child variants. Now, we will look at parent variants where there is no corresponding child variant exist in the child.vcf (check for hidden 0/0 child variants)
    
    //Fill the child decision array
    std::vector<const CVariant*>::iterator compliantsIterator = compliants.begin();
    std::vector<const CVariant*>::iterator violationsIterator = violations.begin();
    
    int counterCompliant = 0;
    int counterViolation = 0;
    
    for(int k = 0; k < (int)childVariants.size(); k++)
    {
        if(compliantsIterator != compliants.end() && childVariants[k]->m_nId == (*compliantsIterator)->m_nId)
        {
            a_rChildDecisions[childVariants[k]->m_nId] = EMendelianDecision::eCompliant;
            compliantsIterator++;
            counterCompliant++;
        }
        else if(violationsIterator != violations.end() && childVariants[k]->m_nId == (*violationsIterator)->m_nId)
        {
            a_rChildDecisions[childVariants[k]->m_nId] = EMendelianDecision::eViolation;
            violationsIterator++;
            counterViolation++;
        }
        else
            a_rChildDecisions[childVariants[k]->m_nId] = EMendelianDecision::eUnknown;
    }
    
    //Excluded Mother variant Check - If we can find 0/0 hidden child site correspond to mother variant
    std::vector<const CVariant*> uniqueMotherVars = m_provider.GetVariantList(m_provider.GetVariantList(eMOTHER, a_triplet.m_nMid,  m_aBestPathsMotherChildGT[a_triplet.m_nTripleIndex].m_baseSemiPath.GetExcluded()),
                                                                              m_aBestPathsMotherChildAM[a_triplet.m_nTripleIndex].m_baseSemiPath.GetExcluded());
    std::vector<bool> motherDecisions(uniqueMotherVars.size());
    CheckUniqueVars(eMOTHER, a_triplet, uniqueMotherVars, motherDecisions, a_rMotherDecisions, a_rFatherDecisions, a_rChildDecisions);
    
    //Excluded Father variant Check - If we can find 0/0 hidden child site correspond to father variant
    std::vector<const CVariant*> uniqueFatherVars = m_provider.GetVariantList(m_provider.GetVariantList(eFATHER, a_triplet.m_nFid,  m_aBestPathsFatherChildGT[a_triplet.m_nTripleIndex].m_baseSemiPath.GetExcluded()),
                                                                              m_aBestPathsFatherChildAM[a_triplet.m_nTripleIndex].m_baseSemiPath.GetExcluded());
    std::vector<bool> fatherDecisions(uniqueFatherVars.size());
    CheckUniqueVars(eFATHER, a_triplet, uniqueFatherVars, fatherDecisions, a_rFatherDecisions, a_rMotherDecisions, a_rChildDecisions);
    
    //Fill the mother decision array
    for(int k = 0; k < (int)motherDecisions.size(); k++)
        a_rMotherDecisions[uniqueMotherVars[k]->m_nId] = (motherDecisions[k] ? eCompliant : eViolation);
    
    //Fill the father decision array
    for(int k = 0; k < (int)fatherDecisions.size(); k++)
        a_rFatherDecisions[uniqueFatherVars[k]->m_nId] = (fatherDecisions[k] ? eCompliant : eViolation);
    
    //If NoCall Mode Is not enabled, mark all decisions of nocall childs as NoCallChild and all nocall parents as NoCallParent
    if(m_nocallMode != eNone)
    {
        std::vector<const CVariant*> motherVariants = m_provider.GetVariantList(eMOTHER, a_triplet.m_nMid);
        std::vector<const CVariant*> fatherVariants = m_provider.GetVariantList(eFATHER, a_triplet.m_nFid);
        
        for(int k = 0; k < (int)motherVariants.size(); k ++)
        {
            if(motherVariants[k]->m_bIsNoCall)
                a_rMotherDecisions[motherVariants[k]->m_nId] = eNoCallParent;
        }
        
        for(int k = 0; k < (int)fatherVariants.size(); k ++)
        {
            if(fatherVariants[k]->m_bIsNoCall)
                a_rFatherDecisions[fatherVariants[k]->m_nId] = eNoCallParent;
        }
        
        for(int k = 0; k < (int)childVariants.size(); k ++)
        {
            if(childVariants[k]->m_bIsNoCall)
                a_rChildDecisions[childVariants[k]->m_nId] = eNoCallChild;
        }
    }
    
    //Assign the remaining unassigned parent variants
    AssignDecisionToParentVars(eMOTHER, a_triplet, a_rMotherDecisions, a_rChildDecisions);
    AssignDecisionToParentVars(eFATHER, a_triplet, a_rFatherDecisions, a_rChildDecisions);
    

    ReportChildChromosomeData(a_triplet, compliants, violations);
    
    std::cout << "===================== STATISTICS " << a_triplet.m_chrName << " ===================" << std::endl;
    std::cout << "Total Compliants:" << compliants.size() << std::endl;
    std::cout << "Total Violations:" << violations.size() << std::endl;
    std::cout << "Child Var Size:" << childVariants.size()<< std::endl;
    std::cout << "=====================================================" << std::endl << std::endl;
    
}
