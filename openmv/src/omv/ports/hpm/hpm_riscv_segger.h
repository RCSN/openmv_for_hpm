#ifndef  __HPM_RISCV_SEGGER_H__
#define  __HPM_RISCV_SEGGER_H__

static inline uint32_t __UXTB_RORn(uint32_t op1, uint32_t rotate)
{
  uint32_t result = 0;
  //__ASM volatile ("uxtb %0, %1, ROR %2" : "=r" (result) : "r" (op1), "i" (rotate) );
  return result;
}

static inline int __ssub16(uint32_t op1, uint32_t op2)
{
    return 0;
}

#endif