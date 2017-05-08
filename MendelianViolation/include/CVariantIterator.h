//
//  CCallIteratorV2.h
//  VCFComparison
//
//  Created by Berke.Toptas on 1/13/17.
//  Copyright © 2016 Seven Bridges Genomics. All rights reserved.
//
//Combines included/excluded calls into one stream

#ifndef _C_CALL_ITERATOR_H_
#define _C_CALL_ITERATOR_H_

#include <vector>
#include "COrientedVariant.h"


class CVariantIterator
{
public:
    
    CVariantIterator(const std::vector<const COrientedVariant*>& includedGT, const std::vector<const COrientedVariant*>& includedAM)
    : m_aIncludedGT(includedGT),
    m_aIncludedAM(includedAM)
    {
        it_IncludedGT = m_aIncludedGT.begin();
        it_IncludedAM = m_aIncludedAM.begin();
    }
    
    bool hasNext()
    {
        bool hasNext = (it_IncludedGT != m_aIncludedGT.end() || it_IncludedAM != m_aIncludedAM.end());
        return hasNext;
    }
    
    const COrientedVariant* Next()
    {
        const COrientedVariant* toRet;
        
        if(it_IncludedAM == m_aIncludedAM.end() || (it_IncludedGT != m_aIncludedGT.end() && (*it_IncludedGT)->GetVariant().GetOriginalPos() < (*it_IncludedAM)->GetVariant().GetOriginalPos()))
        {
            toRet = *it_IncludedGT;
            it_IncludedGT++;
        }
        else
        {
            toRet = *it_IncludedAM;
            it_IncludedAM++;
        }
        
        return toRet;
    }
    
private:
    
    std::vector<const COrientedVariant*>::const_iterator it_IncludedGT;
    std::vector<const COrientedVariant*>::const_iterator it_IncludedAM;
    
    const std::vector<const COrientedVariant*>& m_aIncludedGT;
    const std::vector<const COrientedVariant*>& m_aIncludedAM;
    
};


#endif // _C_CALL_ITERATOR_H_