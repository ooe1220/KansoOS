int g = 0x42;        // .data 行き（初期値あり）
int h;               // .bss 行き（初期値0＝無指定）
int arr[10];         // .bss 行き

int main() {
    h = 1;           // .bss を使わせる（最適化で消えないように）
    arr[0] = 2;
    return g;
}