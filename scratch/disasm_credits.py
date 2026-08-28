import capstone

with open(r'E:\Juegos\CnCReloaded - 2.7 - development\gamemd.exe', 'rb') as f:
    exe_data = f.read()

# PE header parsing to find RVA mapping
pe_offset = int.from_bytes(exe_data[0x3C:0x40], 'little')
num_sections = int.from_bytes(exe_data[pe_offset+6:pe_offset+8], 'little')
opt_header_size = int.from_bytes(exe_data[pe_offset+20:pe_offset+22], 'little')
sec_table_offset = pe_offset + 24 + opt_header_size

sections = []
for i in range(num_sections):
    sec = exe_data[sec_table_offset + i*40 : sec_table_offset + (i+1)*40]
    name = sec[:8].strip(b'\x00').decode('latin1')
    vsize = int.from_bytes(sec[8:12], 'little')
    vaddr = int.from_bytes(sec[12:16], 'little')
    rsize = int.from_bytes(sec[16:20], 'little')
    rptr = int.from_bytes(sec[20:24], 'little')
    sections.append((vaddr, vsize, rptr, rsize))

def va_to_offset(va):
    rva = va - 0x400000
    for vaddr, vsize, rptr, rsize in sections:
        if vaddr <= rva < vaddr + vsize:
            return rptr + (rva - vaddr)
    return None

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
for va in [0x4A24D0, 0x4A25D0]:
    start_off = va_to_offset(va)
    code = exe_data[start_off:start_off+25]
    for insn in md.disasm(code, va):
        print(f"0x{insn.address:X}: {insn.mnemonic} {insn.op_str}")
    print()
