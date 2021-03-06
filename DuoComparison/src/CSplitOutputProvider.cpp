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
 *  CSplitOutputProvider.cpp
 *  VariantBenchmarkingTools
 *
 *  Created by Berke Cagkan Toptas on 4/7/17.
 *
 */

#include "CSplitOutputProvider.h"
#include "CPath.h"
#include "CVariantProvider.h"
#include "Constants.h"
#include <algorithm>


using namespace duocomparison;

CSplitOutputProvider::CSplitOutputProvider()
{

}

void CSplitOutputProvider::SetVariantProvider(CVariantProvider* a_pProvider)
{
    m_pProvider = a_pProvider;
}

void CSplitOutputProvider::SetBestPaths(std::vector<core::CPath>& a_rBestPathList)
{
    m_aBestPaths = a_rBestPathList;
}

void CSplitOutputProvider::SetVcfPath(const std::string& a_rVcfPath)
{
    m_vcfsFolder = a_rVcfPath;
}

void CSplitOutputProvider::SetContigList(const std::vector<SVcfContig>& a_rContigs)
{
    m_contigs = a_rContigs;
}

void CSplitOutputProvider::GenerateSplitVcfs(const std::vector<SChrIdTuple>& a_rCommonChromosomes)
{
    GenerateTpCalledVcf(a_rCommonChromosomes);
    GenerateTpBaseVcf(a_rCommonChromosomes);
    GenerateFnVcf(a_rCommonChromosomes);
    GenerateFpVcf(a_rCommonChromosomes);
}

void CSplitOutputProvider::GenerateTpBaseVcf(const std::vector<SChrIdTuple>& a_rCommonChromosomes)
{
    std::vector<SChrIdTuple> commonChromosomesOrdered(a_rCommonChromosomes);
    std::sort(commonChromosomesOrdered.begin(), commonChromosomesOrdered.end(), [](const SChrIdTuple& t1, const SChrIdTuple& t2){ return t1.m_nBaseId < t2.m_nBaseId; });
    
    std::string filePath = m_vcfsFolder + "/TPBase.vcf";
    m_TPBaseWriter.CreateVcf(filePath.c_str());
    
    //Fill the header section
    FillHeader(&m_TPBaseWriter, true);
    
    //Process each chromosome
    for(SChrIdTuple tuple : commonChromosomesOrdered)
    {
        const std::vector<const core::COrientedVariant*> ovarList = m_aBestPaths[tuple.m_nTupleIndex].m_baseSemiPath.GetIncludedVariants();
        std::vector<const core::COrientedVariant*> sortedOvarList(ovarList);
        std::sort(sortedOvarList.begin(), sortedOvarList.end(), [](const core::COrientedVariant* ovar1, const core::COrientedVariant* ovar2){return ovar1->GetVariant().m_nId < ovar2->GetVariant().m_nId;});
        AddRecords(&m_TPBaseWriter, sortedOvarList);
    }
    
    m_TPBaseWriter.CloseVcf();
}

void CSplitOutputProvider::GenerateTpCalledVcf(const std::vector<SChrIdTuple>& a_rCommonChromosomes)
{
    std::vector<SChrIdTuple> commonChromosomesOrdered(a_rCommonChromosomes);
    std::sort(commonChromosomesOrdered.begin(), commonChromosomesOrdered.end(), [](const SChrIdTuple& t1, const SChrIdTuple& t2){ return t1.m_nBaseId < t2.m_nBaseId; });
    
    std::string filePath = m_vcfsFolder + "/TPCalled.vcf";
    m_TPCalledWriter.CreateVcf(filePath.c_str());
    
    //Fill the header section
    FillHeader(&m_TPCalledWriter, false);
    
    //Process each chromosome
    for(SChrIdTuple tuple : commonChromosomesOrdered)
    {
        const std::vector<const core::COrientedVariant*> ovarList = m_aBestPaths[tuple.m_nTupleIndex].m_calledSemiPath.GetIncludedVariants();
        std::vector<const core::COrientedVariant*> sortedOvarList(ovarList);
        std::sort(sortedOvarList.begin(), sortedOvarList.end(), [](const core::COrientedVariant* ovar1, const core::COrientedVariant* ovar2){return ovar1->GetVariant().m_nId < ovar2->GetVariant().m_nId;});
        AddRecords(&m_TPCalledWriter, sortedOvarList);
    }
    
    m_TPCalledWriter.CloseVcf();
}

void CSplitOutputProvider::GenerateFnVcf(const std::vector<SChrIdTuple>& a_rCommonChromosomes)
{
    std::vector<SChrIdTuple> commonChromosomesOrdered(a_rCommonChromosomes);
    std::sort(commonChromosomesOrdered.begin(), commonChromosomesOrdered.end(), [](const SChrIdTuple& t1, const SChrIdTuple& t2){ return t1.m_nBaseId < t2.m_nBaseId; });

    std::string filePath = m_vcfsFolder + "/FN.vcf";
    m_FNWriter.CreateVcf(filePath.c_str());
    
    //Fill the header section
    FillHeader(&m_FNWriter, true);
    
    //Process each chromosome
    for(SChrIdTuple tuple : commonChromosomesOrdered)
    {
        const std::vector<const CVariant*> varList = m_pProvider->GetVariantList(eBASE, tuple.m_nBaseId, m_aBestPaths[tuple.m_nTupleIndex].m_baseSemiPath.GetExcluded());
        std::vector<const CVariant*> sortedVarList(varList);
        std::sort(sortedVarList.begin(), sortedVarList.end(), [](const CVariant* pVar1, const CVariant* pVar2){return pVar1->m_nId < pVar2->m_nId;});
        AddRecords(&m_FNWriter, sortedVarList);
    }
    
    m_FNWriter.CloseVcf();
}

void CSplitOutputProvider::GenerateFpVcf(const std::vector<SChrIdTuple>& a_rCommonChromosomes)
{
    std::vector<SChrIdTuple> commonChromosomesOrdered(a_rCommonChromosomes);
    std::sort(commonChromosomesOrdered.begin(), commonChromosomesOrdered.end(), [](const SChrIdTuple& t1, const SChrIdTuple& t2){ return t1.m_nBaseId < t2.m_nBaseId; });

    std::string filePath = m_vcfsFolder + "/FP.vcf";
    m_FPWriter.CreateVcf(filePath.c_str());
    
    //Fill the header section
    FillHeader(&m_FPWriter, false);
    
    //Process each chromosome
    for(SChrIdTuple tuple : commonChromosomesOrdered)
    {
        const std::vector<const CVariant*> varList = m_pProvider->GetVariantList(eCALLED, tuple.m_nCalledId, m_aBestPaths[tuple.m_nTupleIndex].m_calledSemiPath.GetExcluded());
        std::vector<const CVariant*> sortedVarList(varList);
        std::sort(sortedVarList.begin(), sortedVarList.end(), [](const CVariant* pVar1, const CVariant* pVar2){return pVar1->m_nId < pVar2->m_nId;});
        AddRecords(&m_FPWriter, sortedVarList);
    }
    
    m_FPWriter.CloseVcf();
}

void CSplitOutputProvider::VariantToVcfRecord(const CVariant* a_pVariant, SVcfRecord& a_rOutputRec)
{
    //Fill basic variant data
    a_rOutputRec.m_chrName = a_pVariant->m_chrName;
    a_rOutputRec.m_nPosition = a_pVariant->m_nOriginalPos;
    a_rOutputRec.m_alleles = a_pVariant->m_allelesStr;
    a_rOutputRec.m_aFilterString = a_pVariant->m_filterString;
    
    //Fill genotype of sample data
    SPerSampleData data;
    data.m_bIsPhased = a_pVariant->m_bIsPhased;
    data.m_nHaplotypeCount = a_pVariant->m_nZygotCount;
    data.m_bIsNoCallVariant = a_pVariant->m_bIsNoCall;
    for(int k = 0; k < data.m_nHaplotypeCount; k++)
        data.m_aGenotype[k] = a_pVariant->m_genotype[k];
    
    a_rOutputRec.m_aSampleData.push_back(data);
}

void CSplitOutputProvider::AddRecords(CVcfWriter* a_pWriter, const std::vector<const core::COrientedVariant*>& a_pOvarList)
{
    for(const core::COrientedVariant* pOvar : a_pOvarList)
    {
        SVcfRecord record;
        VariantToVcfRecord(&pOvar->GetVariant(), record);
        a_pWriter->AddRecord(record);
    }
}

void CSplitOutputProvider::AddRecords(CVcfWriter* a_pWriter, const std::vector<const CVariant*>& a_pVarList)
{
    for(const CVariant* pVar : a_pVarList)
    {
        SVcfRecord record;
        VariantToVcfRecord(pVar, record);
        a_pWriter->AddRecord(record);
    }
}

void CSplitOutputProvider::FillHeader(CVcfWriter *a_pWriter, bool a_bIsBaseSide)
{
    //INIT VCF HEADER
    a_pWriter->InitHeader();
    a_pWriter->AddHeaderLine("##source= VBT Variant Comparison Tool " + VBT_VERSION);
    
    //ADD REQUIRED FORMATS BY GA4GH
    a_pWriter->AddHeaderLine("##FORMAT=<ID=GT,Number=1,Type=String,Description=\"Genotype\">");
    
    //ADD FILTER COLUMNS FROM CALL FILE
    std::vector<std::string> filterNames;
    std::vector<std::string> filterDescriptions;
    m_pProvider->GetFilterInfo((a_bIsBaseSide ? eBASE : eCALLED), filterNames, filterDescriptions);
    for(unsigned int k = 1; k < filterNames.size(); k++)
        a_pWriter->AddHeaderLine("##FILTER=<ID=" + filterNames[k] + ",Description=" + filterDescriptions[k] + ">");
    
    //ADD CONTIG IDs
    for(unsigned int k = 0; k < m_contigs.size(); k++)
        a_pWriter->AddHeaderLine("##contig=<ID=" + m_contigs[k].name + ",length=" + std::to_string(m_contigs[k].length) + ">");

    //ADD REQUIRED SAMPLES
    a_pWriter->AddSampleName("S");
    
    //CLOSE HEADER
    a_pWriter->WriteHeaderToVcf();

}
