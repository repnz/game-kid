#pragma once
#include "opcode.h"
#include <functional>
#include <GameKid/utils/function_tools.h>
#include "GameKid/utils/str.h"
#include "builders/cpu_operation.h"

template <typename... operand_types>
class operands_opcode : public opcode
{
private:
    cpu_operation<operand_types...> _operation;
    operand_tuple<operand_types...> _operands;
public:
    operands_opcode(cpu& cpu, const std::string& name, const std::vector<byte>& value, byte cycles,
                    operand_tuple<operand_types...>& operands,
                    cpu_operation<operand_types...> operation);

    void run() override;

    std::vector<byte> bytes(const std::vector<std::string>& operands) override;

    std::string to_str(byte* next) override;
};

template <typename ... operand_types>
operands_opcode<operand_types...>::operands_opcode(
    cpu& cpu, const std::string& name, const std::vector<byte>& value,
    byte cycles, operand_tuple<operand_types...>& operands,
    cpu_operation<operand_types...> operation) :
    opcode(cpu, name, value, cycles), _operation(operation), _operands(operands)
{
}

template <typename ... operand_types>
void operands_opcode<operand_types...>::run()
{
    
    auto args = std::tuple_cat(std::tuple<cpu&>(_cpu), _operands);
    function_tools::forward_tuple(_operation, args);
}

template <typename ... operand_types>
std::vector<byte> operands_opcode<operand_types...>::bytes(const std::vector<std::string>& operands)
{
    std::vector<byte> v;

    for (byte opcode_byte : value)
    {
        v.push_back(opcode_byte);
    }

    int index = 0;

    function_tools::for_each(_operands, [&](auto& operand)
    {
        for (byte b : operand.bytes(operands[index]))
        {
            v.push_back(b);
            ++index;
        }
    });

    return v;
}

template <typename ... operand_types>
std::string operands_opcode<operand_types...>::to_str(byte* next)
{
    std::vector<std::string> operands_str;
    
    function_tools::for_each(_operands, [&](auto& op)
    {
        operands_str.push_back(op.to_str(next));
    });

    return name + string_tools::join(operands_str);
}