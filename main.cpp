#include <stdlib.h>
#include <iostream>

#include "CPathReplay.h"
#include "SConfig.h"
#include <ctime>
#include <fstream>

#include "CVcfAnalyzer.h"
#include "CMendelianAnalyzer.h"
#include <string>

#include "COverlappingVariantEliminator.h"

//void testGa4ghOutput();
//void testRefOverlapOutput(int argc, char** argv);
//void compareTrimmedVariables();
//void testTripletReader2();
//void compare2List(const std::string& s1, const std::string& s2);
//void GenerateTruthSetsMaria(std::string a_rFilename, bool a_bIsFilterOverlap, bool a_bIsFilter00);
//void UnitTestTrioComparison(int a_nChrNumber, bool a_bIsFilterOverlap, bool a_bIsFilter00);

int main (int argc, char** argv)
{
    if(argc == 1)
    {
        std::cerr << "You have not entered an input.Please try either following:" << std::endl;
        std::cerr << "./vbt varcomp [PARAMETERS]" << std::endl;
        std::cerr << "./vbt mendelian [PARAMETERS]" << std::endl;
        std::cerr << "Please type ./vbt <select_feature> --help for short info about the parameter structure." << std::endl;
    }
    
    if(strcmp(argv[1], "mendelian") == 0)
    {
       CMendelianAnalyzer mendelianAnalyzer;
       mendelianAnalyzer.run(argc, argv);
    }
    
    else if(strcmp(argv[1], "varcomp") == 0)
    {
       CVcfAnalyzer analyzer;
       analyzer.Run(argc, argv);
    }
    
    else
    {
        std::cerr << "Invalid feature name.Please try either following:" << std::endl;
        std::cerr << "./vbt varcomp [PARAMETERS]" << std::endl;
        std::cerr << "./vbt mendelian [PARAMETERS]" << std::endl;
        std::cerr << "Please type ./vbt <select_feature> --help for short info about the parameter structure." << std::endl;
    }
}


