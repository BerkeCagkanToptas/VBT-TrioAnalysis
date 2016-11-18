#include "CPath.h"
#include <iostream>

CPath::CPath()
{}

CPath::CPath(const char* a_aRefSequence, int a_nRefSize)
: m_baseSemiPath(a_aRefSequence, a_nRefSize, eBASE),
  m_calledSemiPath(a_aRefSequence, a_nRefSize, eCALLED),
  m_nCSinceSync(0),
  m_nBSinceSync(0)
{
    //m_PathID = "-";
}

CPath::CPath(const CPath& a_rObj) 
: m_baseSemiPath(a_rObj.m_baseSemiPath),
  m_calledSemiPath(a_rObj.m_calledSemiPath)
{
    m_aSyncPointList = std::vector<int>(a_rObj.m_aSyncPointList);
    m_nCSinceSync = a_rObj.m_nCSinceSync;
    m_nBSinceSync = a_rObj.m_nBSinceSync;
    
    //m_PathID = a_rObj.m_PathID;
}

CPath::CPath(const CPath& a_rObj, int  a_nSyncPointToPush)
: m_baseSemiPath(a_rObj.m_baseSemiPath),
  m_calledSemiPath(a_rObj.m_calledSemiPath)
{
    m_aSyncPointList = std::vector<int>(a_rObj.m_aSyncPointList);
    m_aSyncPointList.push_back(a_nSyncPointToPush);
    m_nCSinceSync = a_rObj.m_nCSinceSync;
    m_nBSinceSync = a_rObj.m_nBSinceSync;
    
    //m_PathID = a_rObj.m_PathID;
}

bool CPath::IsEqual(const CPath& a_rObj) const
{
    return (CompareTo(a_rObj) == 0);
}

int CPath::CompareTo(const CPath& a_rObj) const
{
    int res = m_calledSemiPath.CompareTo(a_rObj.m_calledSemiPath);
    
    if(res != 0)
        return res;
    else
        return m_baseSemiPath.CompareTo(a_rObj.m_baseSemiPath);
}

CPath& CPath::Exclude(EVcfName a_nVCF, const CVariant& a_rVariant, int a_nVariantIndex)
{      
    switch(a_nVCF)
    {
        case eBASE:
            m_baseSemiPath.ExcludeVariant(a_rVariant, a_nVariantIndex);
            break;
        case eCALLED:
            m_calledSemiPath.ExcludeVariant(a_rVariant, a_nVariantIndex);
        break;
    }

    return *this;
}

CPath& CPath::Include(EVcfName a_nVCF, const COrientedVariant& a_rVariant, int a_nVariantIndex)
{
    switch(a_nVCF)
    {
        case eBASE:
            m_baseSemiPath.IncludeVariant(a_rVariant, a_nVariantIndex);
            m_nBSinceSync++;
            break;
        case eCALLED:
            m_calledSemiPath.IncludeVariant(a_rVariant, a_nVariantIndex);
            m_nCSinceSync++;
            break;
    }

    return *this;
}

int CPath::AddVariant(CPath* a_pPathList, EVcfName a_nVcfName, const CVariant& a_rVariant, int a_nVariantIndex)
{
    int pathCount = 0;
    
    if (true == InSync()) 
    {
        m_nCSinceSync = 0;
        m_nBSinceSync = 0;

        // Create a path extension that excludes this variant
        a_pPathList[pathCount] = CPath(*this, m_calledSemiPath.GetPosition());
        a_pPathList[pathCount].Exclude(a_nVcfName, a_rVariant, a_nVariantIndex);
        //a_pPathList[pathCount].m_PathID += "1";
        pathCount++;
    
        // Create a path extension that includes this variant in the possible phases
        if (!a_rVariant.IsHeterozygous())
        {
            a_pPathList[pathCount] =  CPath(*this, m_calledSemiPath.GetPosition());
            COrientedVariant Ovar1(a_rVariant, true);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar1, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "2";
            pathCount++;
        }
        else
        {
            //Include with ordered genotype
            a_pPathList[pathCount] =  CPath(*this, m_calledSemiPath.GetPosition());
            COrientedVariant Ovar1(a_rVariant, true);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar1, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "2";
            pathCount++;
        
            //Include with unordered genotype
            a_pPathList[pathCount] =  CPath(*this, m_calledSemiPath.GetPosition());
            COrientedVariant Ovar2(a_rVariant, false);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar2, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "3";
            pathCount++;
        }
        
    }
    
    else
    {
        // Create a path extension that excludes this variant
        a_pPathList[pathCount] = CPath(*this);
        a_pPathList[pathCount].Exclude(a_nVcfName, a_rVariant, a_nVariantIndex);
        //a_pPathList[pathCount].m_PathID += "1";
        pathCount++;
        
        // Create a path extension that includes this variant in the possible phases
        if (!a_rVariant.IsHeterozygous())
        {
            a_pPathList[pathCount] =  CPath(*this);
            COrientedVariant Ovar1(a_rVariant, true);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar1, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "2";
            pathCount++;
        }
        else
        {
            //Include with ordered genotype
            a_pPathList[pathCount] =  CPath(*this);
            COrientedVariant Ovar1(a_rVariant, true);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar1, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "2";
            pathCount++;
            
            //Include with unordered genotype
            a_pPathList[pathCount] =  CPath(*this);
            COrientedVariant Ovar2(a_rVariant, false);
            a_pPathList[pathCount].Include(a_nVcfName, Ovar2, a_nVariantIndex);
            //a_pPathList[pathCount].m_PathID += "3";
            pathCount++;
        }
    }
    
    
    return pathCount;
}

bool CPath::InSync() const
{
    if (m_calledSemiPath.CompareHaplotypePositions() != 0)
      return false;
    else if (m_baseSemiPath.CompareHaplotypePositions() != 0)
      return false;
    else if (m_calledSemiPath.GetPosition() != m_baseSemiPath.GetPosition()) 
      return false;
    else if (m_calledSemiPath.GetPosition() < m_calledSemiPath.GetVariantEndPosition())
      return false;
    else if (m_baseSemiPath.GetPosition() < m_baseSemiPath.GetVariantEndPosition())
      return false;
    else if (!m_calledSemiPath.IsOnTemplate())
      return false;
    else if (!m_baseSemiPath.IsOnTemplate())
      return false;
    else
      return true;
}


bool CPath::HasFinished() const
{
    return m_baseSemiPath.HasFinished() && m_calledSemiPath.HasFinished();
}

bool CPath::HasNoOperation() const
{
    return (m_nBSinceSync == 0 && m_nCSinceSync > 0) || (m_nCSinceSync == 0 && m_nBSinceSync > 0);
}


void CPath::Step()
{
    if(m_calledSemiPath.CompareHaplotypePositions() > 0)
    {
        //make haplotype B catch up haplotype A
        m_calledSemiPath.StepHaplotypeB();
        m_baseSemiPath.StepHaplotypeB();
    }
    else if(m_calledSemiPath.CompareHaplotypePositions() < 0)
    {
        //make haplotype A catch up haplotype B
        m_calledSemiPath.StepHaplotypeA();
        m_baseSemiPath.StepHaplotypeA();
    }
    else
    {
        //Step both
        m_calledSemiPath.StepHaplotypeA();
        m_calledSemiPath.StepHaplotypeB();
        m_baseSemiPath.StepHaplotypeA();
        m_baseSemiPath.StepHaplotypeB();
    }

}

void CPath::MoveForward(int a_nPosition)
{
    m_calledSemiPath.MoveForward(a_nPosition);
    m_baseSemiPath.MoveForward(a_nPosition);
}

bool CPath::Matches()
{
    return m_calledSemiPath.Matches(m_baseSemiPath);
}

void CPath::Print() const
{
    std::cout << "Printing Path:-----------------------------------------------" << std::endl;
    std::cout << "Sync:" << m_nBSinceSync << " " << m_nCSinceSync << " " << std::endl;
    std::cout << "Sync Points: ";
    for (int i : m_aSyncPointList)
        std::cout << i << " ";
    std::cout<< std::endl;
    std::cout << "--Base Semipath--" << std::endl;
    m_baseSemiPath.Print();
    std::cout << "--Called Semipath--" << std::endl;
    m_calledSemiPath.Print();    
}
