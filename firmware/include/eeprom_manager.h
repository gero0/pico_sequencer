#ifndef __EEPROM_MNGR__
#define __EEPROM_MNGR__

#include "eeprom_24C256.h"
#include "player.h"
#include "sequence.h"

class EEPROMManager {
public:
    EEPROMManager(EEPROM_24C256* eeprom)
        : eeprom(eeprom)
    {
    }

    void prepare_write(int selected_sequence)
    {
        address_to_write = selected_sequence * 512;
        auto& sequence = Player::Instance()->get_sequence();
        sequence.serialize(buffer_to_write);
        write_scheduled = true;
    }
    void prepare_read(int selected_sequence)
    {
        address_to_read = selected_sequence * 512;
        read_scheduled = true;
        //TODO
    }

    void execute_ops()
    {
        if (write_scheduled) {
            eeprom->write_array(address_to_write, buffer_to_write.data(), buffer_to_write.size());
            auto byte = eeprom->read_byte(address_to_write+1);
            write_scheduled = false;
        }

        if (read_scheduled) {
            for(int i=0; i<512;i++){
                buffer_to_read[i] = eeprom->read_byte(address_to_read + i);
            }
            auto& sequence = Player::Instance()->get_sequence();
            sequence.deserialize(buffer_to_read);
            read_scheduled = false;
        }
    }

private:
    EEPROM_24C256* eeprom;
    bool write_scheduled = false;
    bool read_scheduled = false;
    uint32_t address_to_read = 0;
    uint32_t address_to_write = 0;
    SequenceBuffer buffer_to_write;
    SequenceBuffer buffer_to_read;
};

#endif