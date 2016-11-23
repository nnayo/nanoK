#ifndef __MAJORITY_VOTING_H__
# define __MAJORITY_VOTING_H__

# include "type_def.h"


//----------------------------------------------------------------------------
// public types
//

enum voter_result {
        VOTER_UNKNOWN,          // given voter is unknown
        VOTER_OK,               // operation success

        VOTER_3_ON_3 = 0x33,    // specific for voting
        VOTER_2_ON_3 = 0x23,
        VOTER_1_ON_3 = 0x13,
        VOTER_2_ON_2 = 0x22,
        VOTER_1_ON_2 = 0x12,
        VOTER_1_ON_1 = 0x11,
};

enum voter_origin {
        VOTER_OTHER0,
        VOTER_OTHER1,
};

typedef void (*voter_tx)(const void* const data, const u8 len);
typedef u32 (*voter_time)(void);


//----------------------------------------------------------------------------
// public functions
//

// initialize majority voting component
// sending functions shall be provided for each other component
// timing function shall be provided to date the vote
void mjv_init(const voter_tx other0, const voter_tx other1, const voter_time time);

// retrieve a free voter
// return NULL if none available
void* mjv_voter(void);

// assign a function to be called when a majority is issued
enum voter_result mjv_voter_function_set(const void* const voter_id, const voter_tx maj);

// vote on a data block using the given voter
enum voter_result mjv_vote(const void* const voter_id, void* const data, u8* const data_len);

// callback function to call when a vote is received by com layer
void mjv_callback(enum voter_origin orig, const void* const data, const u8 data_len);

#endif // __MAJORITY_VOTING_H__
