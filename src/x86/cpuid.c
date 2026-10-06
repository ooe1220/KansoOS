/*
 * CPU名を取得する関数
 * char* buf(32ビット) は取得したCPU名を格納するバッファの先頭アドレス
 * CPUが返す文字列をedi経由でバッファに格納する
 * 呼出側でchar xxxxx[48];のバッファを用意する必要がある
*/
void get_cpu_name(char* buf)
{
    register char* _buf asm("edi") = buf;

    asm volatile (
        "movl $0x80000002, %%eax\n\t"
        "cpuid\n\t"
        "movl %%eax,  0(%%edi)\n\t"
        "movl %%ebx,  4(%%edi)\n\t"
        "movl %%ecx,  8(%%edi)\n\t"
        "movl %%edx, 12(%%edi)\n\t"

        "movl $0x80000003, %%eax\n\t"
        "cpuid\n\t"
        "movl %%eax, 16(%%edi)\n\t"
        "movl %%ebx, 20(%%edi)\n\t"
        "movl %%ecx, 24(%%edi)\n\t"
        "movl %%edx, 28(%%edi)\n\t"

        "movl $0x80000004, %%eax\n\t"
        "cpuid\n\t"
        "movl %%eax, 32(%%edi)\n\t"
        "movl %%ebx, 36(%%edi)\n\t"
        "movl %%ecx, 40(%%edi)\n\t"
        "movl %%edx, 44(%%edi)\n\t"
        :
        :
        : "%eax", "%ebx", "%ecx", "%edx", "memory"
    );

    buf[47] = '\0';
}
