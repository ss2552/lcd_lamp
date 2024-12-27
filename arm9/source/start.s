.section.text.start, "ax", %progbits
.align 4
.global _start
_start:
    @ cpsr_cxsfをスーパーユーザーにする。
    msr cpsr_cxsf, #0xD3
    
    @ 引数の削除
    mov r0, #0
    mov r1, #0
    mov r2, #0
    
    mov sp, #0x08100000

    @ cachesとMPUを無効にする
    mrc p15, 0, r4, c1, c0, 0
    bic r4, #(1<<16)
    bic r4, #(1<<12)
    bic r4, #(1<<2)
    bic r4, #(1<<0)
    mcr p15, 0, r4, c1, c0, 0
    
    @キャッシュを0埋めする
    mov r4, #0
    mcr p15, 0, r4, c7, c5, 0
    mcr p15, 0, r4, c7, c6, 0
    mcr p15, 0, r4, c7, c10, 4

    ldr r0, =0x33333333
    mcr p15, 0, r0, c5, c0, 2
    mcr p15, 0, r0, c5, c0, 3

    ldr r0, =0xFFFF001D
    ldr r1, =0xFFF0001B
    ldr r2, =0x01FF801D
    ldr r3, =0x08000027
    ldr r4, =0x10000029
    ldr r5, =0x20000035
    ldr r6, =0x1FF00027
    ldr r7, =0x1800002D
    mov r8, #0x29
    mcr p15, 0, r0, c6, c0, 0
    mcr p15, 0, r1, c6, c1, 0
    mcr p15, 0, r2, c6, c2, 0
    mcr p15, 0, r3, c6, c3, 0
    mcr p15, 0, r4, c6, c4, 0
    mcr p15, 0, r5, c6, c5, 0
    mcr p15, 0, r6, c6, c6, 0
    mcr p15, 0, r7, c6, c7, 0
    mcr p15, 0, r8, c3, c0, 0
    mcr p15, 0, r8, c2, c0, 0
    mcr p15, 0, r8, c2, c0, 1
    
    ldr r1, =0xFFF0000A
    mcr p15, 0, r1, c9, c1, 0 
    
    @ 設定 MCU cache ITCMを有効にする。
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #(1<<18)
    orr r0, r0, #(1<<16)
    orr r0, r0, #(1<<13)
    orr r0, r0, #(1<<12)
    orr r0, r0, #(1<<2) 
    orr r0, r0, #(1<<0) 
    mcr p15, 0, r0, c1, c0, 0
    
    @ 宣言の初期化
    ldr r0, =__bss_start__
    mov r1, #0
    ldr r2, =__bss_end__
    sub r2, r0
    bl memset

    @ memcpy(__itcm_start__, __itcm_lma__, __itcm_bss_start__ - __itcm_start__);
    ldr r0, =__itcm_start__
    ldr r1, =__itcm_lma__
    ldr r2, =__itcm_bss_start__
    sub r2, r0
    bl memcpy
    
    @ memset(__itcm_bss_start__, 0, __itcm_end__ - __itcm_bss_start__);
    ldr r0, =__itcm_bss_start__
    mov r1, #0
    ldr r2, =__itcm_end__
    sub r2, r0
    bl memset

    @ 定数の初期化
    bl __libc_init_array
    
    @ メイン関数の実行
    b main
.pool