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
 *  CVcfReader.h
 *  VariantBenchmarkingTools
 *
 *  Created by Berke Toptas
 *
 */

#ifndef VCF_READER_H_
#define VCF_READER_H_
#include <string>
#include <vector>
#include "htslib/vcf.h"
#include "CVariant.h"
#include "SConfig.h"
#include <map>

/**
 * @brief Stores name and length of each contig (read from vcf file header)
 *
 */
struct SVcfContig
{
    std::string name;
    int length;
};

/**
 * @brief VCF parser that is based on htslib
 *
 * CVcfReader accept .VCF or .VCF.GZ formats as inputs and contains all functions to read variant records and header info using htslib library
 */
class CVcfReader
{

public:
    CVcfReader();
    CVcfReader(const char * a_pFilename);
    ~CVcfReader();
    
    ///Open a VCF file given file path
    bool Open(const char * a_pFilename);
    
    ///Close a VCF file
    bool Close();
    
    ///Get next record in the file. a_nId sets the id of variant (no need to be set)
    bool GetNextRecord(CVariant* a_pVariant, int a_nId, const SConfig& a_rConfig);
        
    ///Selects the sample name from multi sample VCF file and ignore other samples
    bool SelectSample(std::string a_sampleName);
    
    ///Fills the a_PatientList with sample names
    void GetSampleNames(std::vector<std::string>& a_PatientList);
    
    ///Read the info names from the vcf header
    void GetInfoNames();
    void GetInfoNames(const std::string& a_rInfoColumns);
    
    ///Get the filename
    std::string GetFilename() const {return m_filename;};
    
    ///Get the number of samples in vcf
    int GetNumberOfSamples() const;
    
    ///Get contig size
    inline int GetContigSize() const {return (int)(m_contigs.size());};
    
    ///Return the list of chromosome that vcf file contains
    const std::vector<SVcfContig>& GetContigs() const;
    
    ///Get a contig name
    inline std::string GetContigName(const unsigned int& id) const {if (id < m_contigs.size()) return m_contigs[id].name; else return "";};
    
    ///Get a contig length
    inline int GetContigLength(const unsigned int& id) const {if (id < m_contigs.size()) return m_contigs[id].length; else return -1;};
    
    ///Get contig id by name. -1 means cannot find the contig
    int GetContigId(std::string a_name) const;
    
    ///Prints the vcf file to the screen (Test Purpose)
    void PrintVCF(const SConfig& a_rConfig);

    ///Set an ID to the VCF file
    void setID(int a_nVcfId) {m_nVcfId = a_nVcfId;};

    ///Returns the Filtering string for the given filter key
    const char* getFilterString(int a_nFilterKey);
    
    ///Returns the key id of given Filter string
    int getFilterKey(const char* a_pFilterValue);
    
    //Returns the filter names and descriptions in the vcf file
    void GetFilterInfo(std::vector<std::string>& a_rFilterNames, std::vector<std::string>& a_rFilterDescriptions);
    
    ///Returns Header Pointer of htslib
    bcf_hdr_t* GetHeaderPointer();
    
    ///Returns Record Pointer of htslib
    bcf1_t* GetRecordPointer();
    
    ///Store chromosome name indexes
    std::map<std::string, int> m_chrIndexMap;
    
private:

    /// Trimms the alt string that contains ref allele
    void TrimAllele(SAllele& a_rAllele);
        
    /// Return whether given allele can be trimmed more than one way
    bool HasMultipleTrimming(SAllele& a_rAllele);
    
    ///Check if the first nucleotide for alleles are redundant (for indels)
    bool HasRedundantFirstNucleotide() const;
    
    ///Return the chromosome number [0 to 24]
    int GetChromosomeNumber(const std::string& a_chrName) const;
    
    
    std::string m_filename;
    bool m_bIsOpen;
    htsFile *   m_pHtsFile;
    bcf_hdr_t * m_pHeader;
    bcf1_t *    m_pRecord;  

    std::vector<SVcfContig> m_contigs;
    int m_nVcfId;
  
    
    std::vector<std::string> m_infoNames;
};

#endif //VCF_READER_H_
