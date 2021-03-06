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
 *  CVcfReader.cpp
 *  VariantBenchmarkingTools
 *
 *  Created by Berke Cagkan Toptas
 *
 */

#include <stdio.h>
#include "CVcfReader.h"
#include <iostream>
#include <sstream>

CVcfReader::CVcfReader()
{
    m_bIsOpen = false;
}

CVcfReader::CVcfReader(const char * a_pFilename)
{
    m_bIsOpen = false;
    Open(a_pFilename);
}

CVcfReader::~CVcfReader()
{
    Close();
}

bool CVcfReader::Open(const char * a_pFilename)
{
    if (true == m_bIsOpen)
    {
        fprintf(stderr, "ERROR: The vcf has been opened.\n");
        return false;
    }
    
    assert(a_pFilename);
    m_pHtsFile = bcf_open(a_pFilename, "r");
    
    if (m_pHtsFile == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open the vcf file: %s.\n", a_pFilename);
        return false;
    }
    
    // Read header
    m_pHeader = bcf_hdr_read(m_pHtsFile);
    if (m_pHeader == NULL)
    {
        fprintf(stderr, "ERROR: Cannot load the header of vcf.\n");
        return false;
    }
    else
    {
        for (int i = 0; i < m_pHeader->n[BCF_DT_CTG]; ++i)
        {
            SVcfContig contig;
            contig.name   = m_pHeader->id[BCF_DT_CTG][i].key;
            contig.length = m_pHeader->id[BCF_DT_CTG][i].val->info[0];
            m_contigs.push_back(contig);
            m_chrIndexMap[contig.name] = i;
        }
    }
        
    m_pRecord  = bcf_init();
    assert(m_pRecord);
    
    m_bIsOpen = true;
    return true;
}

bool CVcfReader::Close()
{
    if (m_bIsOpen)
    {
        bcf_hdr_destroy(m_pHeader);
        bcf_destroy(m_pRecord);
        bcf_close(m_pHtsFile);
        m_bIsOpen = false;
    }
    m_pHeader = NULL;
    m_pRecord = NULL;
    m_pHtsFile = NULL;
    

    return true;
}

bool CVcfReader::GetNextRecord(CVariant * a_pVariant, int a_nId, const SConfig& a_rConfig)
{
    a_pVariant->Clear();
    a_pVariant->m_nVcfId = m_nVcfId;
    int* gt_arr = NULL;
    int ngt_arr = 0;

    int samplenumber = GetNumberOfSamples();
    int zygotCount = 0;
    
    bcf_clear(m_pRecord);
    m_pRecord->d.m_allele = 0;
    const int ok = bcf_read(m_pHtsFile, m_pHeader, m_pRecord);
    bcf_unpack(m_pRecord, BCF_UN_ALL);
    
    if (ok == 0)
    {
        a_pVariant->m_nId = a_nId;
        a_pVariant->m_chrName = m_pHeader->id[BCF_DT_CTG][m_pRecord->rid].key;
        a_pVariant->m_nChrId = m_chrIndexMap[m_pHeader->id[BCF_DT_CTG][m_pRecord->rid].key];
        
        //READ FILTER DATA
        bool isPassed = false;
        
        if(m_pRecord->d.n_flt == 0)
            isPassed = true;
        
        for(int k=0; k< m_pRecord->d.n_flt; k++)
        {
            a_pVariant->m_filterString.push_back(std::string(m_pHeader->id[BCF_DT_ID][m_pRecord->d.flt[k]].key));
                
            if(0 == strcmp(m_pHeader->id[BCF_DT_ID][m_pRecord->d.flt[k]].key, a_rConfig.m_pFilterName))
                isPassed = true;
        }
        a_pVariant->m_bIsFilterPASS = isPassed;
        
        //READ VARIANT ID
        a_pVariant->m_variantIDfromVcf = std::string(m_pRecord->d.id);
        
        //READ QUALITY DATA
        a_pVariant->m_fQuality = m_pRecord->qual;
        
        //READ INFO DATA
        if(true == a_rConfig.m_bIsReadINFO)
        {
            for(int k = 0; k < (int)m_infoNames.size(); k++)
            {
                bcf_info_t* pInfo = bcf_get_info(m_pHeader, m_pRecord, m_infoNames[k].c_str());
                SInfoEntry infoEntry;
            
                if(pInfo == NULL)
                    continue;
                
                infoEntry.n = 0;
                infoEntry.key = m_infoNames[k];
                if(pInfo->type == BCF_BT_INT8 || pInfo->type == BCF_BT_INT16 || pInfo->type == BCF_BT_INT32)
                    infoEntry.type = BCF_HT_INT;
                if(pInfo->type == BCF_BT_FLOAT)
                    infoEntry.type = BCF_HT_REAL;
                if(pInfo->type == BCF_BT_CHAR)
                    infoEntry.type = BCF_HT_STR;
                if(pInfo->type == BCF_BT_NULL)
                    infoEntry.type = BCF_HT_FLAG;
                
                infoEntry.values = malloc(pInfo->vptr_len);
                int isSuccessToStructure = bcf_get_info_values(m_pHeader, m_pRecord, m_infoNames[k].c_str(),
                                                           (void**)(&infoEntry.values), &infoEntry.n, infoEntry.type);
                
                if(isSuccessToStructure <= 0)
                    std::cerr << "BCF info failed while reading tag:" << m_infoNames[k] << std::endl;
                else
                    a_pVariant->m_info.m_infoArray.push_back(infoEntry);
            }
        }
        
        //READ GENOTYPE DATA
        if(samplenumber != 0)
        {
            bcf_get_genotypes(m_pHeader, m_pRecord, &gt_arr, &ngt_arr);
            zygotCount = ngt_arr / samplenumber;
            a_pVariant->m_nAlleleCount = zygotCount;
            if(zygotCount == 2)
                a_pVariant->m_bIsPhased = bcf_gt_is_phased(gt_arr[0]) || bcf_gt_is_phased(gt_arr[1]);
            else if(zygotCount == 1)
                a_pVariant->m_bIsPhased = bcf_gt_is_phased(gt_arr[0]);
        }
        
        //READ SEQUENCE DATA AND FILL ALLELES
        a_pVariant->m_refSequence = std::string(m_pRecord->d.allele[0]);
        
        for (int i = 0; i < zygotCount; ++i)
        {
            int index = bcf_gt_allele(gt_arr[i]) == -1 ? 0 : bcf_gt_allele(gt_arr[i]);
            a_pVariant->m_alleles[i].m_sequence = m_pRecord->d.allele[index];
            a_pVariant->m_alleles[i].m_nStartPos = m_pRecord->pos;
            a_pVariant->m_alleles[i].m_nEndPos = static_cast<int>(m_pRecord->pos + a_pVariant->m_refSequence.length());
        }
        
        //SET ZYGOSITY OF THE VARIANT (HOMOZYGOUS or HETEROZYGOUS)
        if(zygotCount == 2)
        {
            if(a_pVariant->m_alleles[0].m_sequence == a_pVariant->m_alleles[1].m_sequence)
            {
                a_pVariant->m_nAlleleCount = 1;
                a_pVariant->m_bIsHeterozygous = false;
            }
            else
            {
                a_pVariant->m_nAlleleCount = 2;
                a_pVariant->m_bIsHeterozygous = true;
            }
        }
        else if(zygotCount == 1)
        {
            a_pVariant->m_nAlleleCount = 1;
            a_pVariant->m_bIsHeterozygous = false;
        }
        
        //TRIM ALL REDUNDANT NUCLEOTIDES FROM BEGINNING AND END TO ALLOW MORE REFERENCE OVERLAPING
        if(a_rConfig.m_bIsRefOverlap)
        {
            for (int i = 0; i < zygotCount; ++i)
            {
                int curGT = bcf_gt_allele(gt_arr[i]);
                
                if(curGT == 0 || curGT == -1)
                    a_pVariant->m_alleles[i].m_bIsIgnored = true;
                else if(HasMultipleTrimming(a_pVariant->m_alleles[i]))
                    a_pVariant->m_bHaveMultipleTrimOption = true;
                else
                    a_pVariant->TrimVariant(i, a_rConfig.m_bTrimBeginningFirst);
            }
        }
        
        //TRIM FIRST NUCLEOTIDES IF THEY EXIST IN BOTH ALLELE AND IN REFERENCE
        if(!a_rConfig.m_bIsRefOverlap && HasRedundantFirstNucleotide())
        {
            a_pVariant->m_bIsFirstNucleotideTrimmed = true;
            for (int i = 0; i < zygotCount; ++i)
            {
                TrimAllele(a_pVariant->m_alleles[i]);
            }
        }
        
        //SET START AND END POSITION OF VARIANT
        if(samplenumber != 0)
        {
            int maxEnd = -1;
            int minStart = INT_MAX;
            for(int k=0; k < a_pVariant->m_nAlleleCount; k++)
            {
                if(!a_pVariant->m_alleles[k].m_bIsIgnored)
                {
                    maxEnd = std::max(maxEnd, static_cast<int>(a_pVariant->m_alleles[k].m_nEndPos));
                    minStart = std::min(minStart, static_cast<int>(a_pVariant->m_alleles[k].m_nStartPos));
                }
            }
            a_pVariant->m_nEndPos = maxEnd == -1 ? m_pRecord->pos : maxEnd;
            a_pVariant->m_nStartPos = minStart == INT_MAX ? m_pRecord->pos : minStart;
        }
        else
        {
            a_pVariant->m_nStartPos = m_pRecord->pos;
            a_pVariant->m_nEndPos = m_pRecord->pos + static_cast<int>(a_pVariant->m_refSequence.length());
        }
        
        //FILL ORIGINAL ALLELE STR AND GENOTYPE FOR LATER ACCESS
        a_pVariant->m_nZygotCount = zygotCount;
        for(int k = 0; k < m_pRecord->n_allele; k++)
        {
            if(k != 0)
                a_pVariant->m_allelesStr += ",";
            a_pVariant->m_allelesStr += std::string(m_pRecord->d.allele[k]);
        }
        
        //Set original genotypes
        a_pVariant->m_bIsNoCall = true;
        for(int k = 0; k < zygotCount; k++)
        {
            if(bcf_gt_allele(gt_arr[k]) != -1)
                a_pVariant->m_bIsNoCall = false;
            
            a_pVariant->m_genotype[k] = bcf_gt_allele(gt_arr[k]) == -1 ? 0 : bcf_gt_allele(gt_arr[k]);
        }
        
        //Set original position
        a_pVariant->m_nOriginalPos = m_pRecord->pos;
        
        
        //FREE BUFFERS
        free(gt_arr);
        return true;
    }
    else 
    {
        a_pVariant = 0;
        return false;
    }
}

bool CVcfReader::SelectSample(std::string a_sampleName)
{
    int res = bcf_hdr_set_samples(m_pHeader, a_sampleName.c_str(), 0);
    
    if(res == 0)
        return true;
    else
        return false;
    
}

int CVcfReader::GetNumberOfSamples() const
{

    if (!m_bIsOpen)
    {
        fprintf(stderr, "ERROR: The vcf has not been opened.\n");
        return -1;
    }
    
    return bcf_hdr_nsamples(m_pHeader);
}

void CVcfReader::GetSampleNames(std::vector<std::string>& a_pSampleNameList)
{
    int samplecount = bcf_hdr_nsamples(m_pHeader);
    
    for(int k =0; k < samplecount; k++)
        a_pSampleNameList.push_back(m_pHeader->id[BCF_DT_SAMPLE][k].key);
}

int CVcfReader::GetContigId(std::string a_name) const
{
    for (unsigned int i = 0; i < m_contigs.size(); ++i)
    {
        if (m_contigs[i].name.compare(a_name) == 0)
            return i;
    }
    return -1;
}

const char* CVcfReader::getFilterString(int a_nFilterKey)
{
    return m_pHeader->id[BCF_DT_ID][a_nFilterKey].key;
}

int CVcfReader::getFilterKey(const char* a_pFilterValue)
{
    for(int k = 0; k < m_pHeader->n[BCF_DT_ID]; k++)
    {
        if(0 == strcmp(m_pHeader->id[BCF_DT_ID][k].key, a_pFilterValue))
            return k;
    }

    return -1;
}

void CVcfReader::TrimAllele(SAllele& a_rAllele)
{
    a_rAllele.m_sequence = a_rAllele.m_sequence.substr(1, a_rAllele.m_sequence.length() - 1);
    a_rAllele.m_nStartPos += 1;
}

bool CVcfReader::HasRedundantFirstNucleotide() const
{
    for(int k= 0; k < m_pRecord->n_allele; k++)
    {
        if(m_pRecord->d.allele[0][0] != m_pRecord->d.allele[k][0])
            return false;
    }
    return true;
}

const std::vector<SVcfContig>&  CVcfReader::GetContigs() const
{
    return m_contigs;
}

void CVcfReader::GetFilterInfo(std::vector<std::string> &a_rFilterNames, std::vector<std::string> &a_rFilterDescriptions)
{
    
    for(int k = 0; k < m_pHeader->nhrec; k++)
    {
        if(0 == strcmp(m_pHeader->hrec[k]->key, "FILTER"))
        {
            for(int m = 0; m < m_pHeader->hrec[k]->nkeys; m++)
            {
                if(0 == strcmp(m_pHeader->hrec[k]->keys[m], "ID"))
                    a_rFilterNames.push_back(m_pHeader->hrec[k]->vals[m]);
                if(0 == strcmp(m_pHeader->hrec[k]->keys[m], "Description"))
                    a_rFilterDescriptions.push_back(m_pHeader->hrec[k]->vals[m]);
            }
        }
    }
}

bool CVcfReader::HasMultipleTrimming(SAllele& a_rAllele)
{
    if(a_rAllele.m_sequence == "*" || a_rAllele.m_sequence.length() < 2)
        return false;
    
    //Ref string
    std::string refString = m_pRecord->d.allele[0];
    
    //We do not need to check insertion
    if(refString.length() < a_rAllele.m_sequence.length() || refString.length() < 2)
        return false;
    
    unsigned int trimLengthFromBeginning = 0;
    unsigned int trimLengthFromEnd = 0;
    
    //Trim from the beginning
    unsigned int compSize = static_cast<unsigned int>(std::min(refString.size(), a_rAllele.m_sequence.size()));
    for(unsigned int k = 0; k < compSize; k++)
    {
        if(a_rAllele.m_sequence[k] != refString[k])
            break;
        trimLengthFromBeginning++;
    }
    
    //Trim from the end
    for(int k = static_cast<int>(refString.size() - 1), p = static_cast<int>(a_rAllele.m_sequence.size() - 1); k >= 0 && p >= 0;  k--, p--)
    {
        if(a_rAllele.m_sequence[p] != refString[k])
            break;
        trimLengthFromEnd++;
    }
    
    //Our condition to decide if there are multiple option of trimming
    if(trimLengthFromBeginning + trimLengthFromEnd > compSize)
        return true;
    else
        return false;
}

int CVcfReader::GetChromosomeNumber(const std::string& a_chrName) const
{
    if(a_chrName == "x" || a_chrName == "X" || a_chrName == "chrX" || a_chrName == "chrx")
        return 23;
    else if(a_chrName == "y" || a_chrName == "Y" || a_chrName == "chrY" || a_chrName == "chry")
        return 24;
    else if(a_chrName == "MT" || a_chrName == "mt" || a_chrName == "chrMT" || a_chrName == "chrmt")
        return 25;
    else if(a_chrName.length() == 5)
        return atoi(a_chrName.substr(3,2).c_str());
    else if(a_chrName.length() == 4)
        return atoi(a_chrName.substr(3,1).c_str());
    else if(a_chrName.length() <= 2)
        return atoi(a_chrName.c_str());
    else
        return -1;
    
}


void CVcfReader::PrintVCF(const SConfig& a_rConfig)
{
    CVariant variant;

    // READ SAMPLE VCF
    //fprintf(stderr,"#CHR\tPOS\tREF\tALTs\n");
    int k=0;
    while(GetNextRecord(&variant,0, a_rConfig))
    {
        std::cout << k++ << ": " <<  variant.ToString() << std::endl;
    }
}

//Returns Header and Record Pointer of htslib
bcf_hdr_t* CVcfReader::GetHeaderPointer()
{
    return m_pHeader;
}


bcf1_t* CVcfReader::GetRecordPointer()
{
    return m_pRecord;
}


void CVcfReader::GetInfoNames(const std::string& a_rInfoColumns)
{
    std::stringstream ss(a_rInfoColumns);

    while(ss.good())
    {
        std::string substr;
        getline( ss, substr, ',');
        m_infoNames.push_back(substr);
    }
}

void CVcfReader::GetInfoNames()
{
    for(int k = 0; k < m_pHeader->nhrec; k++)
    {
        if(m_pHeader->hrec[k]->type == BCF_HL_INFO)
        {
            for(int m = 0; m < m_pHeader->hrec[k]->nkeys; m++)
            {
                if(strcmp(m_pHeader->hrec[k]->keys[m], "ID"))
                {
                    m_infoNames.push_back(std::string(m_pHeader->hrec[k]->vals[m-1]));
                    break;
                }
            }
        }
    }
}













