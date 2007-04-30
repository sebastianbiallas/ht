#include <string.h>

#include "armdis.h"
#include "snprintf.h"

#include "dis-asm.h"

class ArmDisassembler::BFDInterface
{
    disassemble_info di;
    char *pos;
    char buf[4];
    ArmDisInsn *insn;

    static int read_mem(bfd_vma, bfd_byte *myaddr, unsigned int length,
                        disassemble_info *info)
    {
        BFDInterface *thiz = static_cast<BFDInterface *>(info->application_data);
        memcpy(myaddr, thiz->buf, length);
        return 0;
    }

    static void mem_err(int, bfd_vma, disassemble_info *)
    {}

    static void paf(bfd_vma addr, disassemble_info *info)
    {
        BFDInterface *thiz = static_cast<BFDInterface *>(info->application_data);
        thiz->insn->offset = addr;
        bfdif_printf(info->stream, "@");
    }

    static int bfdif_printf(void *s, const char *fmt, ...)
    {
        BFDInterface *thiz = static_cast<BFDInterface *>(s);

        va_list ap;
        unsigned siz = sizeof(thiz->insn->opstr) - (thiz->pos - thiz->insn->opstr);
        va_start(ap, fmt);
        int n = ht_vsnprintf(thiz->pos, siz, fmt, ap);
        thiz->pos += n;
        va_end(ap);
        return n;
    }

public:
    BFDInterface()
    {
        memset(&di, 0, sizeof(di));
        di.fprintf_func = bfdif_printf;
        di.stream = this;
        di.endian = BFD_ENDIAN_LITTLE;
        di.read_memory_func = read_mem;
        di.print_address_func = paf;
        di.application_data = this;
    }

    void Decode(byte *code, int maxlen, unsigned addr, ArmDisInsn *insn_)
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, code, maxlen > 4 ? 4 : maxlen);
        insn = insn_;
        pos = insn->opstr;
        insn->offset = ~0U;
        // little endian specific
        insn->iscond = (unsigned) buf[3] < 0xe0;
        print_insn_little_arm(addr, &di);
//	FILE *f=fopen("/tmp/htlog","a");fprintf(f,"%s\n",insn->opstr);fclose(f);
    }
};

ArmDisassembler::ArmDisassembler()
{
	bfdif = new BFDInterface;	
}

void ArmDisassembler::load(ObjectStream &f)
{
	bfdif = new BFDInterface;
	Disassembler::load(f);
}

dis_insn *ArmDisassembler::duplicateInsn(dis_insn *disasm_insn)
{
	ArmDisInsn *insn = (ArmDisInsn *) malloc(sizeof(ArmDisInsn));
	*insn = *(ArmDisInsn *)disasm_insn;
	return insn;
}

void ArmDisassembler::getOpcodeMetrics(int &min_length, int &max_length, int &min_look_ahead, int &avg_look_ahead, int &addr_align)
{
	min_length = 4;
	max_length = 4;
	min_look_ahead = 4;
	avg_look_ahead = 4;
	addr_align = 4;
}

byte ArmDisassembler::getSize(dis_insn *disasm_insn)
{
	return 4;//((ArmDisInsn*)disasm_insn)->size;
}

const char *ArmDisassembler::getName()
{
	return "Arm/Disassembler";
}

ObjectID ArmDisassembler::getObjectID() const
{
	return ATOM_DISASM_ARM;
}

bool ArmDisassembler::validInsn(dis_insn *disasm_insn)
{
	return true;
	ArmDisInsn *adi = static_cast<ArmDisInsn *>(disasm_insn);
	return memcmp(adi->opstr, "undefined", 9) != 0;
}

dis_insn *ArmDisassembler::decode(byte *code, int maxlen, CPU_ADDR addr)
{
	bfdif->Decode(code, maxlen, addr.addr32.offset, &insn);
#if 0
	char *pc;
	if (insn.offset == ~0u && (pc = strstr(insn.opstr, ", pc, #")) != 0)
		insn.offset = atol(pc + 7);
#endif
	return &insn;
}

const char *ArmDisassembler::strf(dis_insn *disasm_insn, int style, const char *)
{
	ArmDisInsn *adi = static_cast<ArmDisInsn *>(disasm_insn);

	if (style & DIS_STYLE_HIGHLIGHT) enable_highlighting();

	//const char *cs_default = get_cs(e_cs_default);
	const char *cs_number = get_cs(e_cs_number);
	//const char *cs_symbol = get_cs(e_cs_symbol);

	static char buf[512];
	char *out = buf;
	const char *in = adi->opstr;

	while (in[0] && in[0] != '\t') *out++ = *in++;
	if (!in[0]) goto strfend;

	in++; // skip tab
	while (out < buf + 12) *out++ = ' ';

	while (*in && *in != ';') {
    		if (*in != '@') {
        		*out++ = *in++;
		} else {
        		in++;
	        	CPU_ADDR caddr;
	        	caddr.addr32.offset = adi->offset;
	        	int slen;
	        	char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : 0;
	    		if (s) {
	            		if (out + slen > buf + sizeof(buf) - 1)
	                		slen = buf + sizeof(buf) - 1 - out;
	            		memcpy(out, s, slen);
	            		out[slen] = 0;
	            		out += slen;
        		} else {
            			out += sprintf(out, "%s0x%x", cs_number, adi->offset);
			}
		}
        }

	if (*in == ';' && out[-1] == '\t') out--;

	{
	char *p = strstr(buf, "[pc, #");
	if (p && adi->offset != ~0u) {
		CPU_ADDR caddr;
        	caddr.addr32.offset = adi->offset;
        	int slen;
        	char *s = (addr_sym_func) ? addr_sym_func(caddr, &slen, addr_sym_func_context) : 0;
        	if (s) {
        		if (out + slen > buf + sizeof(buf) - 1)
                		slen = buf + sizeof(buf) - 1 - out;
            		memcpy(out = p, s, slen);
            		out += slen;
        	}
        }
	}
	
strfend:
	*out = 0;
	disable_highlighting();
	return buf;
}
