#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "midi.h"
#include "mock/tusb.h"

TEST_GROUP(MidiTestGroup)
{
    void teardown()
    {
        mock().clear();
    }
};


TEST(MidiTestGroup, MidiTest)
{
    mock().expectNCalls(2, "uart_write_blocking");
    MIDI_note_on(8, 127);
    MIDI_note_off(8);
    mock().checkExpectations();
}