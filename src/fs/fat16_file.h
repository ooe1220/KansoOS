
// ファイル名を8.3形式に変換　例）"test2.bin"→"TEST2   BIN"
void to_83_format(const char* filename, char* result);

// ファイルを検索して情報を取得
int fat16_find_file(const char* filename, uint32_t* start_cluster, uint32_t* file_size);

