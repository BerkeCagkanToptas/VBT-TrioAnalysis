//
//  CVcfWriter.h
//  VCFComparison
//
//  Created by Berke.Toptas on 12/16/16.
//  Copyright © 2016 Seven Bridges Genomics. All rights reserved.
//

#ifndef _C_VCF_WRITER_H_
#define _C_VCF_WRITER_H_

#include "htslib/vcf.h"
#include "CVariant.h"
#include <string>

struct SPerSampleData
{
    //FORMAT :  Decison of variant (TP/FP/FN/N)
    std::string m_decisionBD = "";
    //FORMAT : Match type of variant (gt/allele match)
    std::string m_matchTypeBK = "";
    //FORMAT : Weight of the variant
    float m_fWeight;
    //Haplotype count of the variant
    int m_nHaplotypeCount;
    //Genotype of the variant
    int m_aGenotype[2];
    //Is the genotype is phased
    bool m_bIsPhased = false;
};


struct SVcfRecord
{
    //Position of the variant (0 based)
    int m_nPosition;
    //Quality of the variant
    int m_nQuality;
    //Filter string of the variant (eg. "PASS")
    std::string m_filterString;
    //Alleles string separated by comma of the variant (eg. m_alleles = "AT,G")
    std::string m_alleles;
    //Chromosome name of the variant
    std::string m_chrName;
    //Sample Data (Data to store for each sample)
    std::vector<SPerSampleData> m_aSampleData;
};


class CVcfWriter
{

public:
    
    CVcfWriter();
    
    //Creates VCF file at the given path
    void CreateVcf(const char* a_pFileName);
    
    //Creates the vcf header according to ga4gh and writes it to the opened vcf file
    void FillHeader();
    
    //Append the given variant to the opened vcf file
    void AddRecord(const SVcfRecord& a_rVcfRecord);
    
    //Close the VCF file
    void CloseVcf();
  
    //Add line to the header of vcf file
    void AddHeaderLine(const std::string& a_rLine);
    
    //Add new sample to the header
    void AddSampleName(const std::string& a_rSampleName);
    
    //Open header for writing
    void InitHeader();
    
    //Write the header to the vcf - No change should be made to the m_pHeader after this function is called.
    void WriteHeaderToVcf();
    
private:
    
    //Return the current time in YYYYMMDD format
    std::string GetTime();
    
    htsFile *   m_pHtsFile;
    bcf_hdr_t * m_pHeader;
    bcf1_t *    m_pRecord;
    
    //-1 : Close Header / 1: Inside Header / 0: Header Not Opened
    int m_HEADER_GUARD;
};




#endif //_C_VCF_WRITER_H_
