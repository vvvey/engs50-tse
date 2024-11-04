#pragma once
/* 
 * indexio.h --- 
 * 
 * Author: Vuthy Vey
 * Created: 11-03-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <hash.h>
#include <queue.h>

/*
 * indexio -- save the indexer 
 *
 * returns: 0 for success; nonzero otherwise
 *
 * The suggested format for the file is:
 *   <word> <docID1> <count1> <docID2> <count2> ….<docIDN> <countN>
 *   <word2> <docID1> <count1> <docID2> <count2> ….<docIDN> <countN>
 *  each entry is seperated by a space
 */
int32_t indexsave(hashtable_t *index_p, char *indexnm);

/* 
 * indexload -- loads the indexer from indexnm
 * into a new hashtable (indexer)
 *
 * returns: non-NULL for success; NULL otherwise
 */
hashtable_t *indexload(char *indexnm);