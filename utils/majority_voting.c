#include "majority_voting.h"

#include <string.h> // memcpy(), memcmp()


//----------------------------------------------------------------------------
// private defines
//

#define VOTER_NB 5
#define VOTER_DATA_SIZE 20

#define VOTER_OPEN_WINDOW_DELAY 7   // [ms] delay before self timestamp
#define VOTER_CLOSE_WINDOW_DELAY 3  // [ms] delay after self timestamp


//----------------------------------------------------------------------------
// private types
//

struct voter_block {
        u32 timestamp;
        u8 id;
        u8 data_len;
        u8 data[VOTER_DATA_SIZE];
};

struct voter {
        struct voter_block self;
        struct voter_block other0;
        struct voter_block other1;
        voter_tx maj;
};


//----------------------------------------------------------------------------
// private variables
//

static struct {
        struct voter voter[VOTER_NB];

        u8 nb;                  // number of voters currently in use

        voter_tx tx0;           // functions to send a packet to other components
        voter_tx tx1;
        voter_time time;        // function to date the packet to send
} voters;


//----------------------------------------------------------------------------
// private functions
//

static enum voter_result mjv_vote_check_2(const struct voter_block* const self, const struct voter_block* const other)
{
        // check lengths
        if (self->data_len != other->data_len)
                return VOTER_1_ON_2;

        // check data
        if (0 != memcmp(self->data, other->data, self->data_len))
                return VOTER_1_ON_2;

        return VOTER_2_ON_2;
}

static enum voter_result mjv_vote_check_3(struct voter* const voter)
{
        struct voter_block* const self = &voter->self;
        const struct voter_block* const other0 = &voter->other0;
        const struct voter_block* const other1 = &voter->other1;

        enum voter_result s_o0 = mjv_vote_check_2(self, other0);
        enum voter_result s_o1 = mjv_vote_check_2(self, other1);
        enum voter_result o0_o1 = mjv_vote_check_2(other0, other1);

        // even if local packet is correct, overwrite it
        // to have a constant time on every target
        if (o0_o1 == VOTER_2_ON_2) {
                memcpy(self->data, other0->data, other0->data_len);
                self->data_len = other0->data_len;
        }

        // every packet is the same
        if (s_o0 == VOTER_2_ON_2
                        && s_o1 == VOTER_2_ON_2
                        && o0_o1 == VOTER_2_ON_2)
                return VOTER_3_ON_3;

        // all packets are different
        if (s_o0 != VOTER_2_ON_2
                        && s_o1 != VOTER_2_ON_2
                        && o0_o1 != VOTER_2_ON_2)
                return VOTER_1_ON_3;

        // only remains the case in-between
        return VOTER_2_ON_3;
}

static enum voter_result mjv_vote_check(struct voter* const voter)
{
        const struct voter_block* self = &voter->self;
        const struct voter_block* volatile other0 = &voter->other0;
        const struct voter_block* volatile other1 = &voter->other1;

        u32 time = voters.time();
        u32 open_time = self->timestamp - VOTER_OPEN_WINDOW_DELAY;
        u32 close_time = self->timestamp + VOTER_CLOSE_WINDOW_DELAY;

        // wait till the time-out elapses
        u8 other0_cond = 0;
        u8 other1_cond = 0;
        while (close_time < time) {
                other0_cond = (other0->timestamp > open_time)
                                && (other0->timestamp < close_time);
                other1_cond = (other1->timestamp > open_time)
                                && (other1->timestamp < close_time);

                // if other votes arrived before end of window
                // quit immediatly
                if (other0_cond && other1_cond)
                        break;

                // update time
                time = voters.time();
        }

        // check the conditions
        //
        // no packet received before timeout
        if (close_time < time)
                return VOTER_1_ON_1;

        // only 1 packet received (either 0 or 1)
        if (!other0_cond)
                return mjv_vote_check_2(self, other1);

        if (!other1_cond)
                return mjv_vote_check_2(self, other0);

        // both packets received
        return mjv_vote_check_3(voter);
}


//----------------------------------------------------------------------------
// public functions
//

// initialize majority voting component
void mjv_init(const voter_tx tx0, const voter_tx tx1, const voter_time time)
{
        voters.nb = 0;
        for (int i = 0; i < VOTER_NB; ++i)
                voters.voter[i].self.id = -1;

        voters.tx0 = tx0;
        voters.tx1 = tx1;
        voters.time = time;
}

// retrieve a free voter
// return NULL if none available
void* mjv_voter(void)
{
        // too many voters in use
        if (voters.nb >= VOTER_NB)
                return NULL;

        struct voter* voter = &voters.voter[voters.nb];
        voter->self.id = voters.nb;
        voters.nb++;

        return (void*)(int)voters.nb - 1;
}

// assign a function to be called when a majority is issued
enum voter_result mjv_voter_function_set(const void* const voter_id, const voter_tx maj)
{
        // retrieve the voter, if any
        u8 voter_idx = (u8)(int)voter_id;
        if (voter_idx >= voters.nb)
                return VOTER_UNKNOWN;

        struct voter* voter = &voters.voter[voter_idx];

        // update voter fields
        voter->maj = maj;

        return VOTER_OK;
}

// vote on a data block using the given voter
enum voter_result mjv_vote(const void* const voter_id, void* const data, u8* const data_len)
{
        // retrieve the voter, if any
        u8 voter_idx = (u8)(int)voter_id;
        if (voter_idx >= voters.nb)
                return VOTER_UNKNOWN;

        struct voter* voter = &voters.voter[voter_idx];

        // update voter fields
        voter->self.timestamp = voters.time();
        memcpy(voter->self.data, data, *data_len);

        // send vote to each other component
        voters.tx0(&voter->self, sizeof(voter->self));
        voters.tx1(&voter->self, sizeof(voter->self));

        // check if the votes from others are received
        // or time-out has triggered
        enum voter_result res = mjv_vote_check(voter);

        // if the vote has a majority, update the data block and length
        switch (res) {
        case VOTER_3_ON_3:
        case VOTER_2_ON_2:
                // nothing to do
                break;

        case VOTER_2_ON_3:
        case VOTER_1_ON_3:
                // update the data block and length
                *data_len = voter->self.data_len;
                memcpy(data, voter->self.data, *data_len);
                break;

        case VOTER_1_ON_2:
        case VOTER_1_ON_1:
                // correct data can't be retrieved
                break;

        default:
                // shall never happened
                break;
        }

        return res;
}

// callback function to call when a vote is received by com layer
void mjv_callback(enum voter_origin orig, const void* const data, const u8 data_len)
{
        // ignore too big packets as it is not possible to handle them
        if (data_len > sizeof(struct voter_block))
                return;

        // map the packet on the voter block structure to easily retrieve the fields
        const struct voter_block* vb = data;

        // if the vote rid in unknown, ignore the packet
        if (vb->id >= voters.nb)
                return;

        // copy data to voter
        struct voter_block* other = NULL;
        switch (orig) {
        case VOTER_OTHER0:
                other = &voters.voter[vb->id].other0;
                break;
        case VOTER_OTHER1:
                other = &voters.voter[vb->id].other1;
                break;
        }

        memcpy(other, data, data_len);
}
