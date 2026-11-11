#include "x86/console.h"
#include "drivers/keyboard.h"
#include "command.h"
#include "user_exec.h"

void shell_run(void) {

    char line[128]; // コマンド入力バッファ
    int len = 0; // 現在の入力位置（文字数）

    while (keyboard_available())keyboard_getchar(); // 実機起動時ゴミが表示される為、完全に捨てる

    kputs("\n>");
    while(1){
         asm volatile("cli"); //
        // キーボード入力を待つ (内部的にはhlt→IRQ1割り込み) (drivers/keyboard.h)
        char c = keyboard_getchar(); 
        
        if (c == '\n') { // ENTER : 命令実行及び改行
            line[len] = 0; // バッファを空に(実際には消さず末尾文字を先頭に付加)
            
            int buildin_res = run_builtin_command(line); // 内部コマンド
            int runfile_res = 0;
            if(buildin_res!=0){
                runfile_res = run_file(line); // 実行ファイル実行
            }
            asm volatile("cli");
            if(buildin_res==-1 && runfile_res==-1){ // 内部コマンドでも実行ファイルでもない
                kputs("Unknown command or file not found: ");
                kputs(line);
            }
            
            len = 0; // 次の命令入力の為に入力バッファを空にする
            kputs("\n>");
        } else if (c == '\b') { // BACKSPACE : 一文字削除
            if (len > 0) {
                // BACKSPACE処理
                // 1. バッファから1文字削除 2. カーソルを左へ 3. その位置を空白で上書き（文字を消す） 4. 空白表示時にカーソルが右へ動く為カーソルを再び左へ
                len--;
                kputc('\b'); kputc(' '); kputc('\b');
            }
        } else { // 入力された文字のASCIIコードをバッファに入れ、画面上に表示
            if (len < sizeof(line)-1) {
                line[len++] = c;
                kputc(c);
            }
        }
    }

}