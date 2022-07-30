#include "CppUTest/TestHarness.h"
#include "CppUTest/UtestMacros.h"
#include "CppUTestExt/MockSupport.h"
#include "sequence.h"
#include <algorithm>
#include <array>

TEST_GROUP(SeqTestGroup)
{
    void teardown()
    {
        mock().clear();
    }
};

TEST(SeqTestGroup, Ser_Deser_Eq)
{
    Sequence seq;
    std::array<uint8_t, 512> buffer;

    std::fill(buffer.begin(), buffer.end(), 0);
    buffer[0] = 2;
    buffer[10] = 10;
    buffer[60] = 3;
    buffer[128] = 9;
    buffer[255] = 4;
    buffer[511] = 128;

    seq.deserialize(buffer);

    std::array<uint8_t, 512> buffer2;

    seq.serialize(buffer2);

    for(int i=0; i < 512; i++){
        CHECK(buffer[i] == buffer2[i]);
    }
}

TEST(SeqTestGroup, Ser_Mapping){
    Sequence seq;

    std::array<uint8_t, 512> buffer;
    std::fill(buffer.begin(), buffer.end(), 0);

    //1 part, 2 step, 2 note
    buffer[9] = 2;
    //4 part 16 step 8 note
    buffer[511] = 7;

    seq.deserialize(buffer);

    uint8_t note_1 = seq.parts[0][1][1];
    uint8_t note_2 = seq.parts[3][15][7];
    uint8_t random = seq.parts[1][1][1];

    CHECK_EQUAL(note_1, 2);
    CHECK_EQUAL(note_2, 7);
    CHECK_EQUAL(random, 0);

}