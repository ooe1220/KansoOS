#include "lib/mystdio.h"

int main(int argc, char **argv){   

    printf_d("test3 argc = %d\n", argc);

    return 0;
}

/*
00000000 <.data>:
   0:	e9 e1 01 00 00       	jmp    0x1e6


 1e6:	55                   	push   %ebp
 1e7:	89 e5                	mov    %esp,%ebp
 1e9:	ff 75 08             	push   0x8(%ebp)
 1ec:	68 00 02 01 00       	push   $0x10200
 1f1:	e8 f8 fe ff ff       	call   0xee
 1f6:	83 c4 08             	add    $0x8,%esp
 1f9:	b8 00 00 00 00       	mov    $0x0,%eax
 1fe:	c9                   	leave  
 1ff:	c3                   	ret      

*/
