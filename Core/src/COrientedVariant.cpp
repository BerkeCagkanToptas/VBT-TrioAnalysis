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
 * Originated from: https://github.com/RealTimeGenomics/rtg-tools/blob/master/src/com/rtg/vcf/eval/OrientedVariant.java
 * Copyright (c) 2014. Real Time Genomics Limited.
 * Licensed under the Simplified BSD License: https://github.com/RealTimeGenomics/rtg-tools/blob/master/LICENSE.txt
 *
 *
 * Ported to C++ by Berke Cagkan Toptas
 *
 */

#include "COrientedVariant.h"
#include <iostream>

using namespace core;


COrientedVariant::COrientedVariant()
{
    m_nAlleleIndex = -1;
    m_nOtherAlleleIndex = -1;
    m_bIsNull = true;
}

COrientedVariant::COrientedVariant(const CVariant& a_rObj, bool a_bIsOrder)
{
    m_variant = &a_rObj;
    m_bIsNull = a_rObj.IsNull();
    
    if(a_rObj.IsHeterozygous())
    {
        if(a_bIsOrder == true)
        {
            m_nAlleleIndex = 0;
            m_nOtherAlleleIndex = 1;
        }
    
        else
        {
            m_nAlleleIndex = 1;
            m_nOtherAlleleIndex = 0;
        }
    }
    else
    {
        m_nAlleleIndex = 0;
        m_nOtherAlleleIndex = 0;
    }

    m_bIsOrderOfGenotype = a_bIsOrder;
}

COrientedVariant::COrientedVariant(const CVariant& a_rObj, int a_nAlleleIndex)
{
    m_variant = &a_rObj;
    m_bIsNull = a_rObj.IsNull();
    m_bIsOrderOfGenotype = true;
    m_nAlleleIndex = a_nAlleleIndex;
    m_nOtherAlleleIndex = a_nAlleleIndex;
}


COrientedVariant::COrientedVariant(const COrientedVariant& a_rObj)
{
    m_variant = a_rObj.m_variant;
    m_nAlleleIndex = a_rObj.m_nAlleleIndex;
    m_nOtherAlleleIndex = a_rObj.m_nOtherAlleleIndex;
    m_bIsOrderOfGenotype = a_rObj.m_bIsOrderOfGenotype;
    m_bIsNull = a_rObj.m_bIsNull;
}


int COrientedVariant::CompareTo(const COrientedVariant& a_rObj) const
{
    //decide by variant id
    int id = (m_variant->GetId() < a_rObj.m_variant->GetId()) ? -1 : ((m_variant->GetId() == a_rObj.m_variant->GetId()) ? 0 : 1);
    if(id != 0)
        return id;
    
    //decide by genotype order
    int genotype = (m_bIsOrderOfGenotype == a_rObj.m_bIsOrderOfGenotype) ? 0 : (m_bIsOrderOfGenotype ? 1 : -1);
    if(genotype != 0)
        return genotype;
    
    // Decide by allele index
    int alleleId = (m_nAlleleIndex < a_rObj.m_nAlleleIndex) ? -1 : ((m_nAlleleIndex == a_rObj.m_nAlleleIndex) ? 0 : 1);
    if (alleleId != 0)
        return alleleId;
    
    //Decide by other allele index
    return (m_nOtherAlleleIndex < a_rObj.m_nOtherAlleleIndex) ? -1 : ((m_nOtherAlleleIndex == a_rObj.m_nOtherAlleleIndex) ? 0 : 1);
}

const SAllele& COrientedVariant::GetAllele() const
{
    return m_variant->m_alleles[m_nAlleleIndex];
}

int COrientedVariant::GetStartPos() const
{
    return m_variant->GetStart();
}

int COrientedVariant::GetEndPos() const
{
    return m_variant->GetEnd();
}

int COrientedVariant::GetAlleleIndex() const
{
    return m_nAlleleIndex;
}

COrientedVariant COrientedVariant::Other() const
{
    COrientedVariant oVar2;
    
    oVar2.m_bIsOrderOfGenotype = !m_bIsOrderOfGenotype;
    oVar2.m_nAlleleIndex = m_nOtherAlleleIndex;
    oVar2.m_nOtherAlleleIndex = m_nAlleleIndex;
    oVar2.m_variant = m_variant;
    oVar2.m_bIsNull = m_bIsNull;
    return oVar2;
}

bool COrientedVariant::IsNull() const
{
    return m_bIsNull;
}

bool COrientedVariant::IsOrderOfGenotype() const
{
    return m_bIsOrderOfGenotype;
}

void COrientedVariant::SetToNull()
{
    m_bIsNull = true;
}

const CVariant& COrientedVariant::GetVariant() const
{
    return *m_variant;
}


void COrientedVariant::Print() const
{
    std::cout << m_nAlleleIndex << ":" << m_nOtherAlleleIndex << " " << (m_bIsOrderOfGenotype ? "true" : "false");
    std::cout << "    " << m_variant->ToString() << "  GT:" << m_variant->m_genotype[0] << "/" << m_variant->m_genotype[1]  <<std::endl;
}














