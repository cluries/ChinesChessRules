//
//  rule.c
//
//  Created by cluries on 11-3-25.
//  Copyright (c) 2011年 cluries. All rights reserved.
//


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "rule.h"

typedef void (*rule_parse_function_t)(piece_name_t,piece_location_t*,piece_t*) ;

static rule_locations_t      rule_location;

static rule_parse_function_t parse_tables[8] = {NULL};


static piece_t* piece_detail(int8_t col , int8_t row ,piece_t* pieces)
{
	if( row > 9 || col > 8) {
        //printf("---%d    %d \n",row,col);
		return NULL;
	}
	
	return pieces + row * kChessColNum + col;
}
 
static void parse_chariot_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
    int i = 0;
    piece_t* p = NULL;
 
    if (current->col) {
        for (i = current->col - 1 ; i >= 0 ; i--) {
            p = pieces + current->row * kChessColNum + i;
            if (!p->name || p->state == kPieceStateDie ) {   
                
                ++rule_location.count;
                
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  i;
            }else if(IS_ALLY(piecename,p->name)) { 
                break;
            }else{ 
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  i;
                break;
            }
        }
    }
   
    for (i = current -> col + 1; i < kChessColNum; i++) {
        p = pieces + current->row * kChessColNum + i;
        if (!p->name || p->state == kPieceStateDie ) {  
            
            ++rule_location.count;
            
            rule_location.locations[rule_location.count - 1].row =  current->row;
            rule_location.locations[rule_location.count - 1].col =  i;
        }else if(IS_ALLY(piecename,p->name)) {  
            break;
        }else{ 
            ++rule_location.count;
            rule_location.locations[rule_location.count - 1].row =  current->row;
            rule_location.locations[rule_location.count - 1].col =  i;
            break;
        }
    }
     
    if(current->row) {
        for (i = current->row - 1 ; i >= 0 ; i--) {
            p = pieces + i  * kChessColNum + current->col;
            if (!p->name || p->state == kPieceStateDie ) {  
                
                ++rule_location.count;
                
                rule_location.locations[rule_location.count - 1].row =  i;
                rule_location.locations[rule_location.count - 1].col =  current->col;
                
            }else if(IS_ALLY(piecename,p->name)) { 
                break;
            }else{  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  i;
                rule_location.locations[rule_location.count - 1].col =  current->col;
                break;
            }
        }
    }
    
 
    for (i = current->row + 1 ; i < kChessRowNum ; i++) {
        p = pieces + i  * kChessColNum + current->col;
        if (!p->name || p->state == kPieceStateDie ) {  
            
            ++rule_location.count;
            
            rule_location.locations[rule_location.count - 1].row =  i;
            rule_location.locations[rule_location.count - 1].col =  current->col;
            
        }else if(IS_ALLY(piecename,p->name)) {  
            break;
        }else{  
            ++rule_location.count;
            rule_location.locations[rule_location.count - 1].row =  i;
            rule_location.locations[rule_location.count - 1].col =  current->col;
            break;
        }
        
    }
}



#define PARSE_HORSE_RULE_NODE(sheltercol,shelterrow,targetcol,targetrow)                                         \
		piece_t* p = piece_detail(current->col + (sheltercol),current->row + (shelterrow),pieces);               \
		if(NULL!=p && (!p->name || p->state == kPieceStateDie)) {                                                \
			p = piece_detail(current->col + (targetcol) , current->row + (targetrow) , pieces);                  \
			if(NULL!=p && (!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename))) {			 \
				++rule_location.count;                                                                           \
            	rule_location.locations[rule_location.count - 1].row =  p->location.row;                         \
           		rule_location.locations[rule_location.count - 1].col =  p->location.col;                         \
			}                                                                                                    \
		}                                                                                                        \
 
 
static void parse_horse_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{	
	if(current->row >= 2 && current->col >= 1) {
		PARSE_HORSE_RULE_NODE(0,-1,-1,-2) ;
    }
	
	if(current->row >= 2 && current->col < 8) {
		PARSE_HORSE_RULE_NODE(0,-1, 1,-2);
    }
	
	if(current->row  > 0 && current->col < 7) {
		PARSE_HORSE_RULE_NODE(1,0,2,-1);
	}
	
	if(current->row >= 1 && current->col < 7) {
		PARSE_HORSE_RULE_NODE(1,0,2,1);
	}
	
	if(current->row < 8 && current->col < 8) {
		PARSE_HORSE_RULE_NODE(0,1,1,2);
	}
	
	if(current->row < 8 && current->col > 0) {
		PARSE_HORSE_RULE_NODE(0,1,-1,2);
	}
	
	if(current->row < 9 && current->col > 1) {
		PARSE_HORSE_RULE_NODE(-1,0,-2,1);
	}
	
	if(current->row >0 && current->col > 1) {
		PARSE_HORSE_RULE_NODE(-1,0,-2,-1);
	}
}





#define PARSE_MINISTERS_RULE_NODE(sheltercol,shelterrow,targetcol,targetrow)                                \
            piece_t* p = piece_detail(current->col + (sheltercol) , current->row + (shelterrow) , pieces);  \
            if(!p->name || p->state == kPieceStateDie) {                                                    \
                p = piece_detail(current->col + (targetcol) , current->row + (targetrow) , pieces);         \
                if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {                 \
                    ++rule_location.count;                                                                  \
                    rule_location.locations[rule_location.count - 1].row =  p->location.row;                \
                    rule_location.locations[rule_location.count - 1].col =  p->location.col;                \
                }\
            }
 
static void parse_ministers_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
    if (piecename > 0) {  
        
        if(current->row  <=7  && current->col <= 6 ) {
            
            PARSE_MINISTERS_RULE_NODE(1, 1, 2, 2);
            
        	/*
            piece_t* p = piece_detail(current->col + 1 , current->row + 1 , pieces);
        	if(!p->name || p->state == kPieceStateDie) {
                p = piece_detail(current->col + 2 , current->row + 2 , pieces);
                if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {
                    ++rule_location.count;
                    rule_location.locations[rule_location.count - 1].row =  p->location.row;
                    rule_location.locations[rule_location.count - 1].col =  p->location.col;
                }
        	}
             */
        }
      
        if (current->row <= 7 && current->col > 1) {
            
            PARSE_MINISTERS_RULE_NODE(-1, 1, -2, 2);
            
            /*
            piece_t* p = piece_detail(current->col - 1 , current->row + 1 , pieces);
        	if(!p->name || p->state == kPieceStateDie) {
                p = piece_detail(current->col - 2 , current->row + 2 , pieces);
                if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {
                    ++rule_location.count;
                    rule_location.locations[rule_location.count - 1].row =  p->location.row;
                    rule_location.locations[rule_location.count - 1].col =  p->location.col;
                }
        	}
            */
        }
        
        if (current->row > 6 && current->col > 1) {
            PARSE_MINISTERS_RULE_NODE(-1, -1, -2, -2);
        }
       
        if (current->row > 6 && current->col < 7) {
            PARSE_MINISTERS_RULE_NODE(1, -1, 2, -2);
        }
    
    }else{   
        if (current->col < 7 && current->row <= 2) {
            PARSE_MINISTERS_RULE_NODE(1, 1, 2, 2);
        }
         
        if (current->col >= 2 && current->row <= 2) {
            PARSE_MINISTERS_RULE_NODE(-1, 1, -2, 2);
        }
         
        if (current->col >= 2 && current->row >= 2) {
            PARSE_MINISTERS_RULE_NODE(-1, -1, -2, -2);
        }
         
        if (current->col < 7 && current->row >= 2 ) {
            PARSE_MINISTERS_RULE_NODE(1, -1, 2, -2);
        }
    }
}


#define PARSE_ADVISOR_RULE_NODE(colo,rowo)                                                  \
        piece_t* p = piece_detail(current->col + (colo), current->row + (rowo) , pieces) ;  \
        if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {         \
            ++rule_location.count;                                                          \
            rule_location.locations[rule_location.count - 1].row =  p->location.row;        \
            rule_location.locations[rule_location.count - 1].col =  p->location.col;        \
        }

 
static void parse_advisor_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
	if (piecename > 0) {  
        if (current->row < 9 && current->col < 5) {
            PARSE_ADVISOR_RULE_NODE(1, 1);
            /*
            piece_t* p = piece_detail(current->col + 1 , current->row + 1 , pieces) ;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  p->location.row;
                rule_location.locations[rule_location.count - 1].col =  p->location.col;
            }
            */
        }
        
        if (current->row< 9 && current->col > 3) {
            PARSE_ADVISOR_RULE_NODE(-1, 1);
        }
         
        if (current->row > 7 && current->col > 3 ) {
            PARSE_ADVISOR_RULE_NODE(-1, -1);
        }
     
        if (current->row > 7 && current->col < 5) {
            PARSE_ADVISOR_RULE_NODE(1, -1);
        }
     }else{   
         if (current->row < 2 && current->col < 5) {
             PARSE_ADVISOR_RULE_NODE(1, 1);
         }
         
         if (current->row<2 && current->col > 3) {
             PARSE_ADVISOR_RULE_NODE(-1, 1);
         }
          
         if (current->row > 0 && current->col > 3) {
             PARSE_ADVISOR_RULE_NODE(-1, -1);
         }
          
         if (current->row > 0 && current->col < 5) {
             PARSE_ADVISOR_RULE_NODE(1, -1);
         }
     }
}



#define PARSE_KING_RULE_NODE(colo,rowo)                                                         \
            piece_t* p = piece_detail(current->col + (colo), current->row + (rowo), pieces);    \
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(p->name,piecename)) {         \
                ++rule_location.count;                                                          \
                rule_location.locations[rule_location.count - 1].row =  p->location.row;        \
                rule_location.locations[rule_location.count - 1].col =  p->location.col;        \
            }
 
static void parse_king_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
   
    if (piecename > 0) { 
        if (current->row > 7) {
            PARSE_KING_RULE_NODE(0, -1);
        }
         
        if (current->row < 9) {
            PARSE_KING_RULE_NODE(0, 1);
        }
       
        char shelter = 0;
        piece_t* p = NULL;
         
        for (int prow = current->row - 1; prow >= 0; --prow) {
            p = pieces + prow * kChessColNum + current->col;
            //printf("try row:%d  = %d \n",prow,current->row);
            if (p->name == kPieiceKing * kPiecesColorBlack && !shelter) {
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row = prow;
                rule_location.locations[rule_location.count - 1].col = current->col;
                break;
            }
             
            if(!(!p->name || p->state == kPieceStateDie)) {
                ++shelter;
                break;
            }
        }
    }else{  
        if (current->row > 0) {
            PARSE_KING_RULE_NODE(0, -1);
        }
         
        if (current->row < 2) {
            PARSE_KING_RULE_NODE(0, 1);
        }
         
        char shelter = 0;
        piece_t* p = NULL; 
        for (int prow = current->row ; prow <= 9 ; ++prow) {
            p = pieces + prow * kChessColNum + current->col;
             
            if (p->name == kPieiceKing  && !shelter) {
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row = prow;
                rule_location.locations[rule_location.count - 1].col = current->col;
                break;
            }
             
            if(!(!p->name || p->state == kPieceStateDie)) {
                ++shelter;
                break;
            }
        }

    } 
    
    if (current->col > 3) {
        PARSE_KING_RULE_NODE(-1, 0);
    }
     
    if (current->col < 5) {
        PARSE_KING_RULE_NODE(1, 0);
    }
}

static void parse_canon_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
    int i = 0;
    piece_t* p = NULL   ;
     
    char shelter = 0    ;
     
    if (current->col){
        for (i = current->col - 1 ; i >= 0 ; i--) {
            p = pieces + current->row * kChessColNum + i;
            if (!shelter) {  
                if( (!p->name || p->state == kPieceStateDie) ) {
                    ++rule_location.count;
                    rule_location.locations[rule_location.count - 1].row =  current->row;
                    rule_location.locations[rule_location.count - 1].col =  i;
                } else {
                    ++shelter;
                }
            }else {
                if(!p->name || p->state == kPieceStateDie) {  
                    continue;
                }else if(IS_ALLY(piecename,p->name)) {  
                    break;
                }else{ 
                    ++rule_location.count;
                    rule_location.locations[rule_location.count - 1].row =  current->row;
                    rule_location.locations[rule_location.count - 1].col =  i;
                    break;
                }
            }
        }
    }
    
    shelter = 0;
     
    for (i = current -> col + 1; i < kChessColNum; i++) {
        p = pieces + current->row * kChessColNum + i;
        if (!shelter) {  
            if( (!p->name || p->state == kPieceStateDie) ) {
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  i;
            } else {
                ++shelter;
            }
        }else {
            if(!p->name || p->state == kPieceStateDie) {  
                continue;
            }else if(IS_ALLY(piecename,p->name)) {  
                break;
            }else{  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  i;
                break;
            }
        }
    }
    
    shelter = 0; 
    if(current->row) {
        for (i = current->row - 1 ; i >= 0 ; i--) {
            p = pieces + i  * kChessColNum + current->col;
            if (!shelter) {
                if (!p->name || p->state == kPieceStateDie ) {
                    
                    ++rule_location.count;
                
                    rule_location.locations[rule_location.count - 1].row =  i;
                    rule_location.locations[rule_location.count - 1].col =  current->col;
                }else {
                    //printf("find shelter at row:%d col:%d\n",i,current->col);
                    ++shelter;
                }
            }else{
                if(!p->name || p->state == kPieceStateDie) {  
                    continue;
                }else if(IS_ALLY(piecename,p->name)) {  
                    break;
                } else{  
                    ++rule_location.count;
                    rule_location.locations[rule_location.count - 1].row =  i;
                    rule_location.locations[rule_location.count - 1].col =  current->col;
                    break;
                }
            }
        }
    }
    
    shelter = 0; 
    for (i = current->row + 1 ; i < kChessRowNum ; i++) {
        p = pieces + i  * kChessColNum + current->col;
        p = pieces + i  * kChessColNum + current->col;
        if (!shelter) {
            if (!p->name || p->state == kPieceStateDie ) {
                
                ++rule_location.count;
                
                rule_location.locations[rule_location.count - 1].row =  i;
                rule_location.locations[rule_location.count - 1].col =  current->col;
            }else {
                ++shelter;
            }
        }else{
            if(!p->name || p->state == kPieceStateDie) {  
                continue;
            }else if(IS_ALLY(piecename,p->name)) {  
                break;
            } else{  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  i;
                rule_location.locations[rule_location.count - 1].col =  current->col;
                break;
            }
        }
    }
}

 
static void parse_pawn_rule(piece_name_t piecename,piece_location_t* current,piece_t* pieces)
{
    piece_t* p = NULL   ;
    if (piecename > 0) {  
        if (current->row >= 1 ) {
            p = pieces + (current->row - 1)* kChessColNum + current->col;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){ 
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row - 1;
                rule_location.locations[rule_location.count - 1].col =  current->col;
            }
        }
     
        if (current->row > 4) {
            return;
        }
 
        if (current->col) {
            p = pieces + (current->row )* kChessColNum + current->col - 1;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  current->col - 1;
            }
        }
         
        if (current -> col < 8   ) {
            p = pieces + (current->row )* kChessColNum + current->col + 1;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  current->col + 1;
            }
        }
    }else{ 
        if (current->row< 9) {
            p = pieces + (current->row + 1)* kChessColNum + current->col;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row + 1;
                rule_location.locations[rule_location.count - 1].col =  current->col;
            }
        }
     
        if (current->row < 5) {
            return;
        }
     
        if (current->col) {
            p = pieces + (current->row )* kChessColNum + current->col - 1;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  current->col - 1;
            }
        }

        if (current -> col < kChessColNum - 2   ) {
            p = pieces + (current->row )* kChessColNum + current->col + 1;
            if(!p->name || p->state == kPieceStateDie || !IS_ALLY(piecename, p->name)){  
                ++rule_location.count;
                rule_location.locations[rule_location.count - 1].row =  current->row;
                rule_location.locations[rule_location.count - 1].col =  current->col + 1;
            }
        }
    }
}

void init_rules()
{
    parse_tables[kPIECEINVALID]     = NULL;
    parse_tables[kPieiceChariot]    = parse_chariot_rule;
    parse_tables[kPieiceHorse]      = parse_horse_rule;
    parse_tables[kPieiceMinisters]  = parse_ministers_rule;
    parse_tables[kPieiceAdvisor]    = parse_advisor_rule;
    parse_tables[kPieiceKing]       = parse_king_rule;
    parse_tables[kPieiceCanon]      = parse_canon_rule;
    parse_tables[kPieicePawn]       = parse_pawn_rule;
}


rule_locations_t* rules(piece_name_t piecename, piece_location_t* current,piece_t* pieces)
{
    if (parse_tables[1] == NULL) {
        init_rules();
    }
    
    bzero(&rule_location, sizeof(rule_locations_t));

    rule_parse_function_t parser = parse_tables[abs(piecename)];

    if (NULL != parser) {
        parser(piecename , current , pieces);
    }
    
    return &rule_location;
}

int rule_can_move_to(piece_name_t piecename, piece_location_t* current,piece_t* pieces,piece_location_t* target)
{
    int ret = 0;
    rule_locations_t* locations = rules(piecename, current, pieces);
    if (locations->count) {
        int i = 0;
        for (i = 0; i < locations->count; i++) {
            if (locations->locations[i].row == target->row && locations->locations[i].col == target->col) {
                ret = 1;
            }
        }
    }
    
    return ret;
}

