#include "analy_arm.h"
#include "analy_register.h"
#include "armdis.h"

void AnalyArmDisassembler::init(Analyser *A)
{
    disasm = new ArmDisassembler();
    AnalyDisassembler::init(A);
}

ObjectID AnalyArmDisassembler::getObjectID() const
{
    return ATOM_ANALY_ARM;
}

Address *AnalyArmDisassembler::branchAddr(OPCODE *opcode, branch_enum_t branchtype, bool examine)
{
    ArmDisInsn *insn = static_cast<ArmDisInsn *>(opcode);
    if (insn->offset != ~0u)
        return new AddressFlat32(insn->offset);
    return new InvalidAddress();
}

void AnalyArmDisassembler::examineOpcode(OPCODE *opcode)
{
    ArmDisInsn *insn = static_cast<ArmDisInsn *>(opcode);
    if (insn->offset == ~0u)
        return;

    if (insn->opstr[0] == 'b')
        return;

    AddressFlat32 addr(insn->offset);
    if (!analy->validAddress(&addr, scvalid))
        return;

    analy->data->setIntAddressType(&addr, dst_idword, 4);
    analy->addXRef(&addr, analy->addr, xrefread);
}

branch_enum_t AnalyArmDisassembler::isBranch(OPCODE *opcode)
{
    ArmDisInsn *dis_insn = static_cast<ArmDisInsn *>(opcode);
    if (dis_insn->opstr[0] != 'b')
    {
        if (strstr(dis_insn->opstr, "pc, lr") != NULL)
            return dis_insn->iscond ? br_nobranch : br_return;
        if (strstr(dis_insn->opstr, "pc}") != NULL
            && dis_insn->opstr[0] == 'l')
            return dis_insn->iscond ? br_nobranch : br_return;
        if (strstr(dis_insn->opstr, "  pc,") != NULL
            || strstr(dis_insn->opstr, "\tpc,") != NULL)
            return dis_insn->iscond ? br_jXX : br_jump;
        return br_nobranch;
    }
    if (dis_insn->opstr[1] <= ' ')
        return br_jump;
    if (dis_insn->opstr[1] == 'l' && !dis_insn->iscond)
        return br_call;
    if (dis_insn->opstr[1] == 'x' && !dis_insn->iscond)
    {
        if (strstr(dis_insn->opstr + 2, "lr") != NULL)
            return br_return;
        return br_jump;
    }
    return br_jXX;
}
