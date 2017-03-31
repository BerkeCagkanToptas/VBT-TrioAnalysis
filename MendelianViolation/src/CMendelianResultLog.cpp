//
//  CMendelianResultLog.cpp
//  VCFComparison
//
//  Created by Berke.Toptas on 3/28/17.
//  Copyright © 2017 Seven Bridges Genomics. All rights reserved.
//

#include "CMendelianResultLog.h"
#include <fstream>
#include <iomanip>


void CMendelianResultLog::SetLogDirectory(std::string a_rLogDirectory)
{
    m_aLogDirectory = a_rLogDirectory;
}


void CMendelianResultLog::WriteRow(std::ofstream* a_pOutput, const std::string& a_name, int* a_pColumns)
{
    const char separator = ' ';
    
    (*a_pOutput) << "|--------------------+-------------------+----------------+----------------+----------------|" << std::endl;
    (*a_pOutput) << "|" << std::left << std::setw(20) << std::setfill(separator) << a_name;
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(15) << std::setfill(separator) << a_pColumns[0];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << a_pColumns[1];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << a_pColumns[2];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << a_pColumns[3];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::endl;
}


void CMendelianResultLog::WriteRowSum(std::ofstream* a_pOutput, const std::string& a_name)
{
    int compliantTotal[4] = {0, 0 ,0, 0};
    
    for(int k = 0; k < 4 ; k++)
    {
        compliantTotal[k] = m_DetailedLogEntries.m_nSNP[k];
        compliantTotal[k] += m_DetailedLogEntries.m_nInsertSmall[k] + m_DetailedLogEntries.m_nInsertMedium[k] + m_DetailedLogEntries.m_nInsertLarge[k];
        compliantTotal[k] += m_DetailedLogEntries.m_nDeleteSmall[k] + m_DetailedLogEntries.m_nDeleteMedium[k] + m_DetailedLogEntries.m_nDeleteLarge[k];
        compliantTotal[k] += m_DetailedLogEntries.m_nComplexSmall[k] + m_DetailedLogEntries.m_nComplexMedium[k] + m_DetailedLogEntries.m_nComplexLarge[k];
    }
    
    const char separator = ' ';

    (*a_pOutput) << "|--------------------+-------------------+----------------+----------------+----------------|" << std::endl;
    (*a_pOutput) << "|" << std::left << std::setw(20) << std::setfill(separator) << a_name;
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(15) << std::setfill(separator) << compliantTotal[0];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << compliantTotal[1];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << compliantTotal[2];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::left << std::setw(12) << std::setfill(separator) << compliantTotal[3];
    (*a_pOutput) << std::left << std::setw(5)  << std::setfill(separator) << "|";
    (*a_pOutput) << std::endl;
    
}



void CMendelianResultLog::WriteGenotypeRow(std::ofstream* a_pOutput,
                                           const std::string& a_name,
                                           SMendelianDetailedLogGenotypes& a_genotypes,
                                           int a_nVariantCategoryIndex,
                                           bool a_bIsCompliantTable)
{
    const char separator = ' ';
    const char separator2 = '-';
    
    auto genotypeArr = a_bIsCompliantTable ? a_genotypes.genotypesCompliant : a_genotypes.genotypesViolation;
    
    (*a_pOutput) << "|--------------------";
    for(int k = 0; k < 28; k++)
    {
        (*a_pOutput) << std::left << std::setw(7)  << std::setfill(separator2);
        (*a_pOutput) << "+";
    }
    (*a_pOutput) << "|" << std::endl;

    (*a_pOutput) << "|" << std::left << std::setw(20) << std::setfill(separator) << a_name;
    (*a_pOutput) << "|";
    
    for(int k = 0; k < 28; k++)
    {
        (*a_pOutput) << std::left << std::setw(6) << std::setfill(separator) << genotypeArr[a_nVariantCategoryIndex][k];
        (*a_pOutput) << "|";
    }
    (*a_pOutput) << std::endl;

}

void CMendelianResultLog::WriteGenotypeRowSum(std::ofstream* a_pOutput, const std::string& a_name, bool a_bIsCompliantTable)
{
    const char separator = ' ';
    const char separator2 = '-';
    
    auto genotypeArr = a_bIsCompliantTable ? m_DetailedLogGenotypes.genotypesCompliant : m_DetailedLogGenotypes.genotypesViolation;
    
    (*a_pOutput) << "|--------------------";
    for(int k = 0; k < 28; k++)
    {
        (*a_pOutput) << std::left << std::setw(7)  << std::setfill(separator2);
        (*a_pOutput) << "+";
    }
    (*a_pOutput) << "|" << std::endl;
    
    (*a_pOutput) << "|" << std::left << std::setw(20) << std::setfill(separator) << a_name;
    (*a_pOutput) << "|";
    
    int grandTotal = 0;
    
    for(int k = 0; k < 28; k++)
    {
        int columnTotal = 0;
        for(int aa = 0; aa < 10; aa++)
        {
            columnTotal += genotypeArr[aa][k];
        }
        
        (*a_pOutput) << std::left << std::setw(6) << std::setfill(separator) << columnTotal;
        (*a_pOutput) << "|";
        
        grandTotal += columnTotal;
    }
    (*a_pOutput) << std::endl;
    
    (*a_pOutput) << "+--------------------";
    for(int k = 0; k < 28; k++)
    {
        (*a_pOutput) << std::left << std::setw(7)  << std::setfill(separator2);
        (*a_pOutput) << "-";
    }
    (*a_pOutput) << "+" << std::endl;
    (*a_pOutput) << "GRAND TOTAL:" << grandTotal << std::endl;

}




void CMendelianResultLog::LogBestPathStatistic(bool a_bIsFatherChild, int a_nChromosomeId, int a_nTpCalled, int a_nTpBaseline, int a_nFalsePositive, int a_nFalseNegative)
{
    SMendelianBestPathLogEntry* entry;
    
    if(true == a_bIsFatherChild)
        entry = &m_aFatherChildLogEntries[a_nChromosomeId];
    else
        entry = &m_aMotherChildLogEntries[a_nChromosomeId];
    
    entry->m_nTpBase = a_nTpBaseline;
    entry->m_nTpCalled = a_nTpCalled;
    entry->m_nFn = a_nFalseNegative;
    entry->m_nFP = a_nFalsePositive;
    entry->m_bIsNull = false;
}

void CMendelianResultLog::LogDetailedReport(SMendelianDetailedLogEntry& a_rLogEntry)
{
    m_DetailedLogEntries = a_rLogEntry;
}

void CMendelianResultLog::LogGenotypeMatrix(SMendelianDetailedLogGenotypes& a_rLogEntry)
{
    for(int vars = 0; vars < 10; vars++)
        for(int k = 0; k < 28; k++)
        {
            m_DetailedLogGenotypes.genotypesCompliant[vars][k] = a_rLogEntry.genotypesCompliant[vars][k];
            m_DetailedLogGenotypes.genotypesViolation[vars][k] = a_rLogEntry.genotypesViolation[vars][k];
        }
}

void CMendelianResultLog::LogShortReport(int a_nChrId, int a_nSNPcompliant, int a_nSNPviolation, int a_nINDELcompliant, int a_nINDELviolation)
{
    m_aShortLogEntries[a_nChrId].m_nSNPcompliant = a_nSNPcompliant;
    m_aShortLogEntries[a_nChrId].m_nSNPviolation = a_nSNPviolation;
    m_aShortLogEntries[a_nChrId].m_nINDELcompliant = a_nINDELcompliant;
    m_aShortLogEntries[a_nChrId].m_nINDELviolation = a_nINDELviolation;
}

void CMendelianResultLog::WriteBestPathStatistics()
{
    const char separator    = ' ';
    
    std::ofstream outputLog;
    std::string path = m_aLogDirectory + "/BestPathLogs.txt";
    outputLog.open(path.c_str());
    
    outputLog << "====== FATHER CHILD COMPARISON ======" << std::endl;
    outputLog << std::left << std::setw(10) << std::setfill(separator) << "CHR ID";
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "True-Pos-Child";
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "True-Pos-Father";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "False-Pos";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "False-Neg";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "Precision";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "Recall";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "F-measure" << std::endl;
    
    
    for(int k = 0; k < CHROMOSOME_COUNT; k++)
    {
        if(m_aFatherChildLogEntries[k].m_bIsNull)
            continue;
        
        std::string chrId = k < 22 ? "Chr" + std::to_string(k+1) : "ChrX";
        int TPbase = m_aFatherChildLogEntries[k].m_nTpBase;
        int TPcalled = m_aFatherChildLogEntries[k].m_nTpCalled;
        int FP = m_aFatherChildLogEntries[k].m_nFP;
        int FN = m_aFatherChildLogEntries[k].m_nFn;
        
        outputLog << std::left << std::setw(10) << std::setfill(separator) << chrId;
        outputLog << std::left << std::setw(20) << std::setfill(separator) << TPcalled;
        outputLog << std::left << std::setw(20) << std::setfill(separator) << TPbase;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << FP;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << FN;
        
        double Precision = static_cast<double>(TPbase) / static_cast<double>(TPbase + FP);
        double Recall = static_cast<double>(TPbase) / static_cast<double>(TPbase + FN);
        double Fmeasure = Precision + Recall == 0.0 ? 0.0 : (2.0 * Precision * Recall) / (Precision + Recall);
        
        outputLog.precision(4);
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Precision;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Recall;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Fmeasure;
        outputLog << std::endl;
    }
    
    outputLog << std::endl;
    outputLog << "====== MOTHER CHILD COMPARISON ======" << std::endl;
    outputLog << std::left << std::setw(10) << std::setfill(separator) << "CHR ID";
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "True-Pos-Child";
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "True-Pos-Father";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "False-Pos";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "False-Neg";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "Precision";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "Recall";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << "F-measure" << std::endl;
    
    for(int k = 0; k < CHROMOSOME_COUNT; k++)
    {
        if(m_aMotherChildLogEntries[k].m_bIsNull)
            continue;
        
        std::string chrId = k < 22 ? "Chr" + std::to_string(k+1) : "ChrX";
        int TPbase = m_aMotherChildLogEntries[k].m_nTpBase;
        int TPcalled = m_aMotherChildLogEntries[k].m_nTpCalled;
        int FP = m_aMotherChildLogEntries[k].m_nFP;
        int FN = m_aMotherChildLogEntries[k].m_nFn;
        
        outputLog << std::left << std::setw(10) << std::setfill(separator) << chrId;
        outputLog << std::left << std::setw(20) << std::setfill(separator) << TPcalled;
        outputLog << std::left << std::setw(20) << std::setfill(separator) << TPbase;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << FP;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << FN;
        
        double Precision = static_cast<double>(TPbase) / static_cast<double>(TPbase + FP);
        double Recall = static_cast<double>(TPbase) / static_cast<double>(TPbase + FN);
        double Fmeasure = Precision + Recall == 0.0 ? 0.0 : (2.0 * Precision * Recall) / (Precision + Recall);
        
        outputLog.precision(4);
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Precision;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Recall;
        outputLog << std::left << std::setw(15) << std::setfill(separator) << Fmeasure;
        outputLog << std::endl;
    }
    
    
    
    outputLog.close();
}

void CMendelianResultLog::WriteDetailedReportTable()
{
    const char separator    = ' ';
    
    std::ofstream outputLog;
    std::string path = m_aLogDirectory + "/DetailedReportTable.txt";
    outputLog.open(path.c_str());
    
    outputLog << "+" << std::left << std::setw(91) << std::setfill('-') << "-" << "+" << std::endl;
    
    outputLog << "|" << std::left << std::setw(20) << std::setfill(separator) << " VARIANT TYPES" << "|";
    outputLog << std::left << std::setw(19) << std::setfill(separator) << " COMPLIANT NO" << "|" ;
    outputLog << std::left << std::setw(16) << std::setfill(separator) << " VIOLATION NO" << "|";
    outputLog << std::left << std::setw(16) << std::setfill(separator) << " NOCALL PARENT" << "|";
    outputLog << std::left << std::setw(16) << std::setfill(separator) << " NOCALL CHILD" <<  "|";
    outputLog << std::endl;
    
    WriteRow(&outputLog, "SNP", m_DetailedLogEntries.m_nSNP);
    WriteRow(&outputLog, "INSERT(<=5)",   m_DetailedLogEntries.m_nInsertSmall);
    WriteRow(&outputLog, "INSERT(6-15)",  m_DetailedLogEntries.m_nInsertMedium);
    WriteRow(&outputLog, "INSERT(>=15)",  m_DetailedLogEntries.m_nInsertLarge);
    WriteRow(&outputLog, "DELETE(<=5)",   m_DetailedLogEntries.m_nDeleteSmall);
    WriteRow(&outputLog, "DELETE(6-15)",  m_DetailedLogEntries.m_nDeleteMedium);
    WriteRow(&outputLog, "DELETE(>=15)",  m_DetailedLogEntries.m_nDeleteLarge);
    WriteRow(&outputLog, "COMPLEX(<=5)",  m_DetailedLogEntries.m_nComplexSmall);
    WriteRow(&outputLog, "COMPLEX(6-15)", m_DetailedLogEntries.m_nComplexMedium);
    WriteRow(&outputLog, "COMPLEX(>=15)", m_DetailedLogEntries.m_nComplexLarge);
    WriteRowSum(&outputLog, "TOTAL");
    
    outputLog << "+" << std::left << std::setw(91) << std::setfill('-') << "-" << "+" << std::endl;
    
    outputLog << std::endl;
    outputLog << "Description of Columns for tables below:  There are 28 genotype columns for each category which are 3x3x3 genotype possibilitis + 1 for multi-allelelic site." << std::endl;
    outputLog << "Table is oriented as CHILD FATHER MOTHER the column calculation is 9 x sum(mother gt) + 3 x sum(father gt) + sum(child gt). The last column is the multi-allelelic site." << std::endl;
    
    outputLog << std::endl << std::endl << std:: endl;
    
    outputLog << "+--------------------";
    for(int k = 0; k < 28; k++)
        outputLog << std::left << std::setw(7)  << std::setfill('-') << "-";
    outputLog << "+" << std::endl;
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "| VARIANT TYPES";
    outputLog << std::left << std::setw(7*28+1) << "   GENOTYPES <COMPLIANT>" <<  "|" << std::endl;

    WriteGenotypeRow(&outputLog, "SNP",           m_DetailedLogGenotypes, 0, true);
    WriteGenotypeRow(&outputLog, "INSERT(<=5)",   m_DetailedLogGenotypes, 1, true);
    WriteGenotypeRow(&outputLog, "INSERT(6-15)",  m_DetailedLogGenotypes, 2, true);
    WriteGenotypeRow(&outputLog, "INSERT(>=15)",  m_DetailedLogGenotypes, 3, true);
    WriteGenotypeRow(&outputLog, "DELETE(<=5)",   m_DetailedLogGenotypes, 4, true);
    WriteGenotypeRow(&outputLog, "DELETE(6-15)",  m_DetailedLogGenotypes, 5, true);
    WriteGenotypeRow(&outputLog, "DELETE(>=15)",  m_DetailedLogGenotypes, 6, true);
    WriteGenotypeRow(&outputLog, "COMPLEX(<=5)",  m_DetailedLogGenotypes, 7, true);
    WriteGenotypeRow(&outputLog, "COMPLEX(6-15)", m_DetailedLogGenotypes, 8, true);
    WriteGenotypeRow(&outputLog, "COMPLEX(>=15)", m_DetailedLogGenotypes, 9, true);
    WriteGenotypeRowSum(&outputLog, "TOTAL", true);
    
    
    outputLog << std::endl << std::endl << std:: endl;

    outputLog << "+--------------------";
    for(int k = 0; k < 28; k++)
        outputLog << std::left << std::setw(7)  << std::setfill('-') << "-";
    outputLog << "+" << std::endl;
    outputLog << std::left << std::setw(20) << std::setfill(separator) << "| VARIANT TYPES";
    outputLog << std::left << std::setw(7*28 +1) << "   GENOTYPES <VIOLATION>" <<  "|" << std::endl;
    
    
    WriteGenotypeRow(&outputLog, "SNP",           m_DetailedLogGenotypes, 0, false);
    WriteGenotypeRow(&outputLog, "INSERT(<=5)",   m_DetailedLogGenotypes, 1, false);
    WriteGenotypeRow(&outputLog, "INSERT(6-15)",  m_DetailedLogGenotypes, 2, false);
    WriteGenotypeRow(&outputLog, "INSERT(>=15)",  m_DetailedLogGenotypes, 3, false);
    WriteGenotypeRow(&outputLog, "DELETE(<=5)",   m_DetailedLogGenotypes, 4, false);
    WriteGenotypeRow(&outputLog, "DELETE(6-15)",  m_DetailedLogGenotypes, 5, false);
    WriteGenotypeRow(&outputLog, "DELETE(>=15)",  m_DetailedLogGenotypes, 6, false);
    WriteGenotypeRow(&outputLog, "COMPLEX(<=5)",  m_DetailedLogGenotypes, 7, false);
    WriteGenotypeRow(&outputLog, "COMPLEX(6-15)", m_DetailedLogGenotypes, 8, false);
    WriteGenotypeRow(&outputLog, "COMPLEX(>=15)", m_DetailedLogGenotypes, 9, false);
    WriteGenotypeRowSum(&outputLog, "TOTAL", false);

}


void CMendelianResultLog::WriteShortReportTable()
{
    const char separator    = ' ';
    
    std::ofstream outputLog;
    std::string path = m_aLogDirectory + "/ShortReportTable.txt";
    outputLog.open(path.c_str());
    
    outputLog << "+" << std::left << std::setw(57) << std::setfill('-') << "-" << "+" << std::endl;
    
    outputLog << "|" << std::left << std::setw(25) << std::setfill(separator) << "VARIANT TYPES";
    outputLog << std::left << std::setw(18) << std::setfill(separator) << "COMPLIANT NO";
    outputLog << std::left << std::setw(14) << std::setfill(separator) << "VIOLATION NO" << "|";
    outputLog << std::endl;

    int totalSNPcompliant = 0;
    int totalSNPviolation = 0;
    int totalINDELcompliant = 0;
    int totalINDELviolation = 0;

    for(SMendelianShortLogEntry entry : m_aShortLogEntries)
    {
        totalSNPcompliant += entry.m_nSNPcompliant;
        totalSNPviolation += entry.m_nSNPviolation;
        totalINDELcompliant += entry.m_nINDELcompliant;
        totalINDELviolation += entry.m_nINDELviolation;
    }
    
    outputLog << "|" << std::left << std::setw(20) << std::setfill(separator) << "SNP";
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << totalSNPcompliant;
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::left << std::setw(12) << std::setfill(separator) << totalSNPviolation;
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::endl;
    
    outputLog << "|" << std::left << std::setw(20) << std::setfill(separator) << "INDEL";
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::left << std::setw(15) << std::setfill(separator) << totalINDELcompliant;
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::left << std::setw(12) << std::setfill(separator) << totalINDELviolation;
    outputLog << std::left << std::setw(5)  << std::setfill(separator) << "|";
    outputLog << std::endl;
    
    outputLog << "+" << std::left << std::setw(57) << std::setfill('-') << "-" << "+" << std::endl;

}

void SMendelianDetailedLogEntry::clear()
{
    for(int k=0; k < 4; k++)
    {
        m_nSNP[k] = 0;
        m_nInsertSmall[k] = 0;
        m_nInsertMedium[k] = 0;
        m_nInsertLarge[k] = 0;
        m_nDeleteSmall[k] = 0;
        m_nDeleteMedium[k] = 0;
        m_nDeleteLarge[k] = 0;
        m_nComplexSmall[k] = 0;
        m_nComplexMedium[k] = 0;
        m_nComplexLarge[k] = 0;
    }

}

void SMendelianDetailedLogGenotypes::clear()
{
    for(int var = 0; var < 10; var++)
        for(int col = 0; col < 28; col++)
        {
            genotypesCompliant[var][col] = 0;
            genotypesViolation[var][col] = 0;
        }
}






