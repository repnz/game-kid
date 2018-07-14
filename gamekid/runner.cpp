#include "runner.h"
#include "rom/cartridge.h"
#include <sstream>
#include "utils/convert.h"
#include "utils/str.h"

using namespace gamekid;

runner::runner(rom::cartridge&& cart) : 
_cart(cart), _rom_map(cart.create_rom_map()), _memory_map(*_rom_map, _lcd),
_system(_memory_map), _set(_system.cpu()), _decoder(_set){

    if (!_cart.validate_header_checksum()) {
        throw std::exception("Header checksum error");
    }
}

void runner::add_breakpoint(word address){
    _breakpoints.insert(address);
}

void runner::run_until_break(){
    do {
        next();
    } while (_breakpoints.find(_system.cpu().PC.load()) == _breakpoints.end());
}

void runner::next(){
    const word opcode_word = _system.memory().load_word(_system.cpu().PC.load());
    gamekid::cpu::opcode* opcode = _decoder.decode(opcode_word);

    if (opcode == nullptr) {
        throw std::exception("InvalidOpcode");
    }

    opcode->run();
    _system.cpu().PC.store(_system.cpu().PC.load() + opcode->full_size());
}

void runner::run(){
    while (true){
        next();
    }
}

cpu::cpu& runner::cpu() {
    return _system.cpu();
}

std::vector<byte> runner::dump(word address_to_view, word length_to_view) {
    std::vector<byte> bytes(length_to_view);

    for (int i = 0; i < length_to_view; ++i) {
        bytes[i] = _system.memory().load<byte>(address_to_view+i);
    }

    return bytes;
}

std::vector<std::string> runner::list(int count) {
    std::vector<std::string> opcodes(count);
    
    word pc = _system.cpu().PC.load();

    for (int i = 0; i < count; i++) {
        const word opcode_word = _system.memory().load_word(pc);
        gamekid::cpu::opcode* op = _decoder.decode(opcode_word);

        // Write the address
        opcodes[i] = utils::convert::to_hex(pc) + "  ";

        if (op == nullptr)  {
            const byte current_byte = static_cast<byte>(opcode_word);
            opcodes[i] += utils::convert::to_hex<byte>(current_byte);
            opcodes[i] += "            .byte";
            pc += 1;
        } else {
            const word imm_bytes = _system.memory().load_word(pc + op->size());
            const byte* imm_ptr = (byte*)(&imm_bytes);

            for (byte b : op->full_opcode(imm_ptr)) {
                opcodes[i] += utils::convert::to_hex<byte>(b) + ' ';
            }

            for (size_t j=12; j>=op->full_size()*3+1; --j) {
                opcodes[i] += " ";
            }
            
            opcodes[i] += op->to_str(imm_ptr);
            pc += op->full_size();
        }
    }

    return opcodes;
}
