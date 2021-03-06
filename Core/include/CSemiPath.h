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
 * Originated from: https://github.com/RealTimeGenomics/rtg-tools/blob/master/src/com/rtg/vcf/eval/HalfPath.java
 * Copyright (c) 2014. Real Time Genomics Limited.
 * Licensed under the Simplified BSD License: https://github.com/RealTimeGenomics/rtg-tools/blob/master/LICENSE.txt
 *
 *
 * Ported to C++ by Berke Cagkan Toptas
 *
 */

#ifndef _C_SEMI_PATH_H_
#define _C_SEMI_PATH_H_

#include "CHaplotypeSequence.h"
#include "EVcfName.h"
#include <vector>

namespace core
{

/**
 * @brief A Container that stores the current state of 1 vcf side during variant replay
 *
 * CSemiPath stores the variant replay information of single vcf. Each CSemipath contains two haplotype 
 * (since human is diploid). Variants included/excluded so far is stored at CSemipath level.
 *
 */
class CSemiPath
{
    public:

    CSemiPath();
    CSemiPath(const char* a_aRefSequence, int a_nRefSize, EVcfName a_uVcfName);
    CSemiPath(const CSemiPath& a_rObj);

    ///Include the given variant to this semipath
    void IncludeVariant(const COrientedVariant& a_rVariant, int a_nVariantIndex);
    
    ///Exclude the given variant to this semipath
    void ExcludeVariant(const CVariant& a_rVariant, int a_nVariantIndex);

    ///Compare and returns the template position diffence of hapA and hapB
    int CompareHaplotypePositions() const;

    ///Return vcf name of the semi path
    EVcfName GetVcfName() const;
    
    ///Return the list of excluded variants
    const std::vector<int>& GetExcluded() const;
    
    ///Gets the end position the semipath (max of haplotypeA and haplotypeB)
    int GetPosition() const;
    
    ///Return the end position of last variant added
    int GetVariantEndPosition() const;
    
    ///Return the end position of last included variant
    int GetIncludedVariantEndPosition() const;
    
    ///Return the index of last variant added
    int GetVariantIndex() const;
    
    ///Set the index of last variant added
    void SetVariantIndex(int a_nVariantIndex);
    
    ///Return pointer to included variants
    const std::vector<const COrientedVariant*>& GetIncludedVariants() const;
    
    ///Check whether this half path is fully on the template (i.e. no haplotypes are within a variant)
    bool IsOnTemplate() const;
    
    ///Detects overlapping variants
    bool IsNew(const COrientedVariant& a_rVar) const;
    
    ///Compare this half path with the given half path
    int CompareTo(const CSemiPath& a_rObj) const;
    
    ///Check whether this half path is equal to the given half path
    bool IsEqual(const CSemiPath& a_rObj) const;

    ///Checks if both haplotype A and B is finished
    bool HasFinished() const;

    ///Force semipath to move the given position
    void MoveForward(int a_nPosition);
    
    ///Chech whether this half path matches with the given half path
    bool Matches(const CSemiPath& a_rOther);

    /**
     *Test whether a deficit of variant bases are upstream in the queue in order to perform a step.
     *return false indicates that no variants need to be immediately enqueued
     */
    bool WantsFutureVariantBases() const;
  
    bool FinishedHaplotypeA() const;
    bool FinishedHaplotypeB() const;

    ///Return the next base on the B haplotype
    char NextHaplotypeBBase() const;
    
    ///Return the next base on the A haplotype
    char NextHaplotypeABase() const;

    void StepHaplotypeA();
    void StepHaplotypeB();

    ///Clear the included variants
    void ClearIncludedVariants();
    ///Set the included variants
    void AddIncludedVariants(std::vector<const COrientedVariant*>& a_rIncludedVarList);
    
    ///Clear the included variants
    void ClearExcludedVariants();
    ///Set the included variants
    void AddExcludedVariants(std::vector<int>& a_rExcludedVarList);

    ///Sorts included variants according to variant ids
    void SortIncludedVariants();
    
    ///[TEST Purpose]Print semipath
    void Print() const;
    
    private:

    ///name of the semipath
    EVcfName m_uVcfName;
    ///Index of last variant added
    int m_nVariantIndex;
    ///End of last variant added
    int m_nVariantEndPosition;
    ///Last variant included
    int m_nIncludedVariantEndPosition;

    std::vector<const COrientedVariant*> m_aIncludedVariants;
    std::vector<int> m_aExcludedVariants;

    CHaplotypeSequence m_haplotypeA;
    CHaplotypeSequence m_haplotypeB;

    bool m_bFinishedHapA;
    bool m_bFinishedHapB;

};

}

#endif // _C_SEMI_PATH_H_
