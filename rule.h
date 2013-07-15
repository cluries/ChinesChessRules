//
//  rule.h
//
//  Created by cluries on 11-3-25.
//  Copyright (c) 2011年 cluries. All rights reserved.
//

#ifndef __RULE_H__
#define __RULE_H__

#include "ChessTypes.h"

#define RULE_INLAY_SIZE 19

#if __cplusplus
extern "C" {
#endif
    
 
typedef struct _rule_locations_t
{
    int                 count;           
    piece_location_t    locations[RULE_INLAY_SIZE];  
} rule_locations_t;
 
rule_locations_t* rules(piece_name_t piecename, piece_location_t* current,piece_t* pieces);

 
int rule_can_move_to(piece_name_t piecename, piece_location_t* current,piece_t* pieces,piece_location_t* target);

#if __cplusplus
}
#endif
        

#endif
