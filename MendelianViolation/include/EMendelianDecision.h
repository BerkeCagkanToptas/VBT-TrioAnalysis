//
//  EMendelianDecision.h
//  VariantBenchmarkingTools
//
//  Created by Berke.Toptas on 3/9/17.
//  Copyright © 2016 Seven Bridges Genomics.
//            © 2017 SBGD Inc.
//  All rights reserved.
//

#ifndef _E_MENDELIAN_DECISION_H_
#define _E_MENDELIAN_DECISION_H_

namespace mendelian
{

/**
 * @brief ENUM for merged trio anotations. Indicate the final decision of mendelian tool for variant
 *
 */
enum EMendelianDecision
{
    eUnknown = 0,
    eCompliant = 1,
    eViolation = 2,
    eNoCallParent = 3,
    eNoCallChild = 4,
    eSkipped = 5
};
    
}

#endif // _E_MENDELIAN_DECISION_H_
